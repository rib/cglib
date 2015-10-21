/*
 * CGlib
 *
 * A Low-Level GPU Graphics and Utilities API
 *
 * Copyright (C) 2007,2008,2009,2010 Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *
 *
 * Authors:
 *  Matthew Allum  <mallum@openedhand.com>
 *  Neil Roberts   <neil@linux.intel.com>
 *  Robert Bragg   <robert@linux.intel.com>
 */

#include <cglib-config.h>

#include "cg-debug.h"
#include "cg-private.h"
#include "cg-util.h"
#include "cg-bitmap.h"
#include "cg-bitmap-private.h"
#include "cg-texture-private.h"
#include "cg-texture-2d-gl.h"
#include "cg-texture-2d-private.h"
#include "cg-texture-2d-sliced-private.h"
#include "cg-texture-gl-private.h"
#include "cg-texture-driver.h"
#include "cg-device-private.h"
#include "cg-object-private.h"
#include "cg-spans.h"
#include "cg-pipeline-opengl-private.h"
#include "cg-primitive-texture.h"
#include "cg-error-private.h"
#include "cg-texture-gl-private.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>

static void _cg_texture_2d_sliced_free(cg_texture_2d_sliced_t *tex_2ds);

CG_TEXTURE_DEFINE(Texture2DSliced, texture_2d_sliced);

static const cg_texture_vtable_t cg_texture_2d_sliced_vtable;

typedef struct _foreach_data_t {
    cg_meta_texture_callback_t callback;
    void *user_data;
    float x_normalize_factor;
    float y_normalize_factor;
} foreach_data_t;

static void
re_normalize_sub_texture_coords_cb(cg_texture_t *sub_texture,
                                   const float *sub_texture_coords,
                                   const float *meta_coords,
                                   void *user_data)
{
    foreach_data_t *data = user_data;
    /* The coordinates passed to the span iterating code were
     * un-normalized so we need to renormalize them before passing them
     * on */
    float re_normalized_coords[4] = {
        meta_coords[0] * data->x_normalize_factor,
        meta_coords[1] * data->y_normalize_factor,
        meta_coords[2] * data->x_normalize_factor,
        meta_coords[3] * data->y_normalize_factor
    };

    data->callback(
        sub_texture, sub_texture_coords, re_normalized_coords, data->user_data);
}

static void
_cg_texture_2d_sliced_foreach_sub_texture_in_region(
    cg_texture_t *tex,
    float virtual_tx_1,
    float virtual_ty_1,
    float virtual_tx_2,
    float virtual_ty_2,
    cg_meta_texture_callback_t callback,
    void *user_data)
{
    cg_texture_2d_sliced_t *tex_2ds = CG_TEXTURE_2D_SLICED(tex);
    cg_span_t *x_spans = (cg_span_t *)tex_2ds->slice_x_spans->data;
    cg_span_t *y_spans = (cg_span_t *)tex_2ds->slice_y_spans->data;
    cg_texture_t **textures = (cg_texture_t **)tex_2ds->slice_textures->data;
    float un_normalized_coords[4];
    foreach_data_t data;

    /* NB: its convenient for us to store non-normalized coordinates in
     * our cg_span_ts but that means we need to un-normalize the incoming
     * virtual coordinates and make sure we re-normalize the coordinates
     * before calling the given callback.
     */

    data.callback = callback;
    data.user_data = user_data;
    data.x_normalize_factor = 1.0f / tex->width;
    data.y_normalize_factor = 1.0f / tex->height;

    un_normalized_coords[0] = virtual_tx_1 * tex->width;
    un_normalized_coords[1] = virtual_ty_1 * tex->height;
    un_normalized_coords[2] = virtual_tx_2 * tex->width;
    un_normalized_coords[3] = virtual_ty_2 * tex->height;

    /* Note that the normalize factors passed here are the reciprocal of
     * the factors calculated above because the span iterating code
     * normalizes by dividing by the factor instead of multiplying */
    _cg_texture_spans_foreach_in_region(x_spans,
                                        tex_2ds->slice_x_spans->len,
                                        y_spans,
                                        tex_2ds->slice_y_spans->len,
                                        textures,
                                        un_normalized_coords,
                                        tex->width,
                                        tex->height,
                                        CG_PIPELINE_WRAP_MODE_REPEAT,
                                        CG_PIPELINE_WRAP_MODE_REPEAT,
                                        re_normalize_sub_texture_coords_cb,
                                        &data);
}

static uint8_t *
_cg_texture_2d_sliced_allocate_waste_buffer(cg_texture_2d_sliced_t *tex_2ds,
                                            cg_pixel_format_t format)
{
    cg_span_t *last_x_span;
    cg_span_t *last_y_span;
    uint8_t *waste_buf = NULL;

    /* If the texture has any waste then allocate a buffer big enough to
       fill the gaps */
    last_x_span = &c_array_index(tex_2ds->slice_x_spans,
                                 cg_span_t,
                                 tex_2ds->slice_x_spans->len - 1);
    last_y_span = &c_array_index(tex_2ds->slice_y_spans,
                                 cg_span_t,
                                 tex_2ds->slice_y_spans->len - 1);
    if (last_x_span->waste > 0 || last_y_span->waste > 0) {
        int bpp = _cg_pixel_format_get_bytes_per_pixel(format);
        cg_span_t *first_x_span =
            &c_array_index(tex_2ds->slice_x_spans, cg_span_t, 0);
        cg_span_t *first_y_span =
            &c_array_index(tex_2ds->slice_y_spans, cg_span_t, 0);
        unsigned int right_size = first_y_span->size * last_x_span->waste;
        unsigned int bottom_size = first_x_span->size * last_y_span->waste;

        waste_buf = c_malloc(MAX(right_size, bottom_size) * bpp);
    }

    return waste_buf;
}

static bool
_cg_texture_2d_sliced_set_waste(cg_texture_2d_sliced_t *tex_2ds,
                                cg_bitmap_t *source_bmp,
                                cg_texture_2d_t *slice_tex,
                                uint8_t *waste_buf,
                                cg_span_t *x_span,
                                cg_span_t *y_span,
                                cg_span_iter_t *x_iter,
                                cg_span_iter_t *y_iter,
                                int src_x,
                                int src_y,
                                int dst_x,
                                int dst_y,
                                cg_error_t **error)
{
    bool need_x, need_y;
    cg_device_t *dev = CG_TEXTURE(tex_2ds)->dev;

    /* If the x_span is sliced and the upload touches the
       rightmost pixels then fill the waste with copies of the
       pixels */
    need_x = x_span->waste > 0 && x_iter->intersect_end - x_iter->pos >=
             x_span->size - x_span->waste;

    /* same for the bottom-most pixels */
    need_y = y_span->waste > 0 && y_iter->intersect_end - y_iter->pos >=
             y_span->size - y_span->waste;

    if (need_x || need_y) {
        int bmp_rowstride = cg_bitmap_get_rowstride(source_bmp);
        cg_pixel_format_t source_format = cg_bitmap_get_format(source_bmp);
        int bpp = _cg_pixel_format_get_bytes_per_pixel(source_format);
        uint8_t *bmp_data;
        const uint8_t *src;
        uint8_t *dst;
        unsigned int wy, wx;
        cg_bitmap_t *waste_bmp;

        bmp_data = _cg_bitmap_map(source_bmp, CG_BUFFER_ACCESS_READ, 0, error);
        if (bmp_data == NULL)
            return false;

        if (need_x) {
            src = (bmp_data + ((src_y + (int)y_iter->intersect_start - dst_y) *
                               bmp_rowstride) +
                   (src_x + (int)x_span->start + (int)x_span->size -
                    (int)x_span->waste - dst_x - 1) *
                   bpp);

            dst = waste_buf;

            for (wy = 0; wy < y_iter->intersect_end - y_iter->intersect_start;
                 wy++) {
                for (wx = 0; wx < x_span->waste; wx++) {
                    memcpy(dst, src, bpp);
                    dst += bpp;
                }
                src += bmp_rowstride;
            }

            waste_bmp = cg_bitmap_new_for_data(dev,
                                               x_span->waste,
                                               y_iter->intersect_end -
                                               y_iter->intersect_start,
                                               source_format,
                                               x_span->waste * bpp,
                                               waste_buf);

            if (!cg_texture_set_region_from_bitmap(
                    CG_TEXTURE(slice_tex),
                    0,  /* src_x */
                    0,  /* src_y */
                    x_span->waste,  /* width */
                    /* height */
                    y_iter->intersect_end - y_iter->intersect_start,
                    waste_bmp,
                    /* dst_x */
                    x_span->size - x_span->waste,
                    y_iter->intersect_start - y_span->start,  /* dst_y */
                    0,  /* level */
                    error)) {
                cg_object_unref(waste_bmp);
                _cg_bitmap_unmap(source_bmp);
                return false;
            }

            cg_object_unref(waste_bmp);
        }

        if (need_y) {
            unsigned int copy_width, intersect_width;

            src = (bmp_data +
                   ((src_x + (int)x_iter->intersect_start - dst_x) * bpp) +
                   (src_y + (int)y_span->start + (int)y_span->size -
                    (int)y_span->waste - dst_y - 1) *
                   bmp_rowstride);

            dst = waste_buf;

            if (x_iter->intersect_end - x_iter->pos >=
                x_span->size - x_span->waste)
                copy_width =
                    x_span->size + x_iter->pos - x_iter->intersect_start;
            else
                copy_width = x_iter->intersect_end - x_iter->intersect_start;

            intersect_width = x_iter->intersect_end - x_iter->intersect_start;

            for (wy = 0; wy < y_span->waste; wy++) {
                memcpy(dst, src, intersect_width * bpp);
                dst += intersect_width * bpp;

                for (wx = intersect_width; wx < copy_width; wx++) {
                    memcpy(dst, dst - bpp, bpp);
                    dst += bpp;
                }
            }

            waste_bmp = cg_bitmap_new_for_data(dev,
                                               copy_width,
                                               y_span->waste,
                                               source_format,
                                               copy_width * bpp,
                                               waste_buf);

            if (!cg_texture_set_region_from_bitmap(CG_TEXTURE(slice_tex),
                                                   0, /* src_x */
                                                   0, /* src_y */
                                                   copy_width, /* width */
                                                   y_span->waste, /* height */
                                                   waste_bmp,
                                                   /* dst_x */
                                                   x_iter->intersect_start -
                                                   x_iter->pos,
                                                   /* dst_y */
                                                   y_span->size - y_span->waste,
                                                   0, /* level */
                                                   error)) {
                cg_object_unref(waste_bmp);
                _cg_bitmap_unmap(source_bmp);
                return false;
            }

            cg_object_unref(waste_bmp);
        }

        _cg_bitmap_unmap(source_bmp);
    }

    return true;
}

static bool
_cg_texture_2d_sliced_upload_bitmap(cg_texture_2d_sliced_t *tex_2ds,
                                    cg_bitmap_t *bmp,
                                    cg_error_t **error)
{
    cg_span_t *x_span;
    cg_span_t *y_span;
    cg_texture_2d_t *slice_tex;
    int x, y;
    uint8_t *waste_buf;
    cg_pixel_format_t bmp_format;

    bmp_format = cg_bitmap_get_format(bmp);

    waste_buf =
        _cg_texture_2d_sliced_allocate_waste_buffer(tex_2ds, bmp_format);

    /* Iterate vertical slices */
    for (y = 0; y < tex_2ds->slice_y_spans->len; ++y) {
        y_span = &c_array_index(tex_2ds->slice_y_spans, cg_span_t, y);

        /* Iterate horizontal slices */
        for (x = 0; x < tex_2ds->slice_x_spans->len; ++x) {
            int slice_num = y * tex_2ds->slice_x_spans->len + x;
            cg_span_iter_t x_iter, y_iter;

            x_span = &c_array_index(tex_2ds->slice_x_spans, cg_span_t, x);

            /* Pick the gl texture object handle */
            slice_tex = c_array_index(
                tex_2ds->slice_textures, cg_texture_2d_t *, slice_num);

            if (!cg_texture_set_region_from_bitmap(
                    CG_TEXTURE(slice_tex),
                    x_span->start,  /* src x */
                    y_span->start,  /* src y */
                    x_span->size - x_span->waste,  /* width */
                    y_span->size - y_span->waste,  /* height */
                    bmp,
                    0,  /* dst x */
                    0,  /* dst y */
                    0,  /* level */
                    error)) {
                if (waste_buf)
                    c_free(waste_buf);
                return false;
            }

            /* Set up a fake iterator that covers the whole slice */
            x_iter.intersect_start = x_span->start;
            x_iter.intersect_end =
                (x_span->start + x_span->size - x_span->waste);
            x_iter.pos = x_span->start;

            y_iter.intersect_start = y_span->start;
            y_iter.intersect_end =
                (y_span->start + y_span->size - y_span->waste);
            y_iter.pos = y_span->start;

            if (!_cg_texture_2d_sliced_set_waste(tex_2ds,
                                                 bmp,
                                                 slice_tex,
                                                 waste_buf,
                                                 x_span,
                                                 y_span,
                                                 &x_iter,
                                                 &y_iter,
                                                 0, /* src_x */
                                                 0, /* src_y */
                                                 0, /* dst_x */
                                                 0,
                                                 error)) /* dst_y */
            {
                if (waste_buf)
                    c_free(waste_buf);
                return false;
            }
        }
    }

    if (waste_buf)
        c_free(waste_buf);

    return true;
}

static bool
_cg_texture_2d_sliced_upload_subregion(cg_texture_2d_sliced_t *tex_2ds,
                                       int src_x,
                                       int src_y,
                                       int dst_x,
                                       int dst_y,
                                       int width,
                                       int height,
                                       cg_bitmap_t *source_bmp,
                                       cg_error_t **error)
{
    cg_texture_t *tex = CG_TEXTURE(tex_2ds);
    cg_span_t *x_span;
    cg_span_t *y_span;
    cg_span_iter_t x_iter;
    cg_span_iter_t y_iter;
    cg_texture_2d_t *slice_tex;
    int source_x = 0, source_y = 0;
    int inter_w = 0, inter_h = 0;
    int local_x = 0, local_y = 0;
    uint8_t *waste_buf;
    cg_pixel_format_t source_format;

    source_format = cg_bitmap_get_format(source_bmp);

    waste_buf =
        _cg_texture_2d_sliced_allocate_waste_buffer(tex_2ds, source_format);

    /* Iterate vertical spans */
    for (source_y = src_y,
         _cg_span_iter_begin(&y_iter,
                             (cg_span_t *)tex_2ds->slice_y_spans->data,
                             tex_2ds->slice_y_spans->len,
                             tex->height,
                             dst_y,
                             dst_y + height,
                             CG_PIPELINE_WRAP_MODE_REPEAT);
         !_cg_span_iter_end(&y_iter);
         _cg_span_iter_next(&y_iter), source_y += inter_h) {
        y_span =
            &c_array_index(tex_2ds->slice_y_spans, cg_span_t, y_iter.index);

        /* Iterate horizontal spans */
        for (source_x = src_x,
             _cg_span_iter_begin(&x_iter,
                                 (cg_span_t *)tex_2ds->slice_x_spans->data,
                                 tex_2ds->slice_x_spans->len,
                                 tex->width,
                                 dst_x,
                                 dst_x + width,
                                 CG_PIPELINE_WRAP_MODE_REPEAT);
             !_cg_span_iter_end(&x_iter);
             _cg_span_iter_next(&x_iter), source_x += inter_w) {
            int slice_num;

            x_span =
                &c_array_index(tex_2ds->slice_x_spans, cg_span_t, x_iter.index);

            /* Pick intersection width and height */
            inter_w = (x_iter.intersect_end - x_iter.intersect_start);
            inter_h = (y_iter.intersect_end - y_iter.intersect_start);

            /* Localize intersection top-left corner to slice*/
            local_x = (x_iter.intersect_start - x_iter.pos);
            local_y = (y_iter.intersect_start - y_iter.pos);

            slice_num =
                y_iter.index * tex_2ds->slice_x_spans->len + x_iter.index;

            /* Pick slice texture */
            slice_tex = c_array_index(
                tex_2ds->slice_textures, cg_texture_2d_t *, slice_num);

            if (!cg_texture_set_region_from_bitmap(CG_TEXTURE(slice_tex),
                                                   source_x,
                                                   source_y,
                                                   inter_w, /* width */
                                                   inter_h, /* height */
                                                   source_bmp,
                                                   local_x, /* dst x */
                                                   local_y, /* dst y */
                                                   0, /* level */
                                                   error)) {
                if (waste_buf)
                    c_free(waste_buf);
                return false;
            }

            if (!_cg_texture_2d_sliced_set_waste(tex_2ds,
                                                 source_bmp,
                                                 slice_tex,
                                                 waste_buf,
                                                 x_span,
                                                 y_span,
                                                 &x_iter,
                                                 &y_iter,
                                                 src_x,
                                                 src_y,
                                                 dst_x,
                                                 dst_y,
                                                 error)) {
                if (waste_buf)
                    c_free(waste_buf);
                return false;
            }
        }
    }

    if (waste_buf)
        c_free(waste_buf);

    return true;
}

static int
_cg_rect_slices_for_size(int size_to_fill,
                         int max_span_size,
                         int max_waste,
                         c_array_t *out_spans)
{
    int n_spans = 0;
    cg_span_t span;

    /* Init first slice span */
    span.start = 0;
    span.size = max_span_size;
    span.waste = 0;

    /* Repeat until whole area covered */
    while (size_to_fill >= span.size) {
        /* Add another slice span of same size */
        if (out_spans)
            c_array_append_val(out_spans, span);
        span.start += span.size;
        size_to_fill -= span.size;
        n_spans++;
    }

    /* Add one last smaller slice span */
    if (size_to_fill > 0) {
        span.size = size_to_fill;
        if (out_spans)
            c_array_append_val(out_spans, span);
        n_spans++;
    }

    return n_spans;
}

static int
_cg_pot_slices_for_size(int size_to_fill,
                        int max_span_size,
                        int max_waste,
                        c_array_t *out_spans)
{
    int n_spans = 0;
    cg_span_t span;

    /* Init first slice span */
    span.start = 0;
    span.size = max_span_size;
    span.waste = 0;

    /* Fix invalid max_waste */
    if (max_waste < 0)
        max_waste = 0;

    while (true) {
        /* Is the whole area covered? */
        if (size_to_fill > span.size) {
            /* Not yet - add a span of this size */
            if (out_spans)
                c_array_append_val(out_spans, span);

            span.start += span.size;
            size_to_fill -= span.size;
            n_spans++;
        } else if (span.size - size_to_fill <= max_waste) {
            /* Yes and waste is small enough */
            /* Pick the next power of two up from size_to_fill. This can
               sometimes be less than the span.size that would be chosen
               otherwise */
            span.size = _cg_util_next_p2(size_to_fill);
            span.waste = span.size - size_to_fill;
            if (out_spans)
                c_array_append_val(out_spans, span);

            return ++n_spans;
        } else {
            /* Yes but waste is too large */
            while (span.size - size_to_fill > max_waste) {
                span.size /= 2;
                c_assert(span.size > 0);
            }
        }
    }

    /* Can't get here */
    return 0;
}

static void
_cg_texture_2d_sliced_gl_flush_legacy_texobj_wrap_modes(cg_texture_t *tex,
                                                        GLenum wrap_mode_s,
                                                        GLenum wrap_mode_t,
                                                        GLenum wrap_mode_p)
{
    cg_texture_2d_sliced_t *tex_2ds = CG_TEXTURE_2D_SLICED(tex);
    int i;

    /* Pass the set wrap mode on to all of the child textures */
    for (i = 0; i < tex_2ds->slice_textures->len; i++) {
        cg_texture_2d_t *slice_tex =
            c_array_index(tex_2ds->slice_textures, cg_texture_2d_t *, i);

        _cg_texture_gl_flush_legacy_texobj_wrap_modes(
            CG_TEXTURE(slice_tex), wrap_mode_s, wrap_mode_t, wrap_mode_p);
    }
}

static void
free_spans(cg_texture_2d_sliced_t *tex_2ds)
{
    if (tex_2ds->slice_x_spans != NULL) {
        c_array_free(tex_2ds->slice_x_spans, true);
        tex_2ds->slice_x_spans = NULL;
    }

    if (tex_2ds->slice_y_spans != NULL) {
        c_array_free(tex_2ds->slice_y_spans, true);
        tex_2ds->slice_y_spans = NULL;
    }
}

static bool
setup_spans(cg_device_t *dev,
            cg_texture_2d_sliced_t *tex_2ds,
            int width,
            int height,
            int max_waste,
            cg_pixel_format_t internal_format,
            cg_error_t **error)
{
    int max_width;
    int max_height;
    int n_x_slices;
    int n_y_slices;

    int (*slices_for_size)(int, int, int, c_array_t *);

    /* Initialize size of largest slice according to supported features */
    if (cg_has_feature(dev, CG_FEATURE_ID_TEXTURE_NPOT)) {
        max_width = width;
        max_height = height;
        slices_for_size = _cg_rect_slices_for_size;
    } else {
        max_width = _cg_util_next_p2(width);
        max_height = _cg_util_next_p2(height);
        slices_for_size = _cg_pot_slices_for_size;
    }

    /* Negative number means no slicing forced by the user */
    if (max_waste <= -1) {
        cg_span_t span;

        /* Check if size supported else bail out */
        if (!dev->driver_vtable->texture_2d_can_create(dev, max_width, max_height, internal_format)) {
            _cg_set_error(error,
                          CG_TEXTURE_ERROR,
                          CG_TEXTURE_ERROR_SIZE,
                          "Sliced texture size of %d x %d not possible "
                          "with max waste set to -1",
                          width,
                          height);
            return false;
        }

        n_x_slices = 1;
        n_y_slices = 1;

        /* Init span arrays */
        tex_2ds->slice_x_spans =
            c_array_sized_new(false, false, sizeof(cg_span_t), 1);

        tex_2ds->slice_y_spans =
            c_array_sized_new(false, false, sizeof(cg_span_t), 1);

        /* Add a single span for width and height */
        span.start = 0;
        span.size = max_width;
        span.waste = max_width - width;
        c_array_append_val(tex_2ds->slice_x_spans, span);

        span.size = max_height;
        span.waste = max_height - height;
        c_array_append_val(tex_2ds->slice_y_spans, span);
    } else {
        /* Decrease the size of largest slice until supported by GL */
        while (!dev->driver_vtable->texture_2d_can_create(dev, max_width, max_height, internal_format)) {
            /* Alternate between width and height */
            if (max_width > max_height)
                max_width /= 2;
            else
                max_height /= 2;

            if (max_width == 0 || max_height == 0) {
                /* Maybe it would be ok to just c_warn_if_reached() for this
                 * codepath */
                _cg_set_error(error,
                              CG_TEXTURE_ERROR,
                              CG_TEXTURE_ERROR_SIZE,
                              "No suitable slice geometry found");
                free_spans(tex_2ds);
                return false;
            }
        }

        /* Determine the slices required to cover the bitmap area */
        n_x_slices = slices_for_size(width, max_width, max_waste, NULL);

        n_y_slices = slices_for_size(height, max_height, max_waste, NULL);

        /* Init span arrays with reserved size */
        tex_2ds->slice_x_spans =
            c_array_sized_new(false, false, sizeof(cg_span_t), n_x_slices);

        tex_2ds->slice_y_spans =
            c_array_sized_new(false, false, sizeof(cg_span_t), n_y_slices);

        /* Fill span arrays with info */
        slices_for_size(width, max_width, max_waste, tex_2ds->slice_x_spans);

        slices_for_size(height, max_height, max_waste, tex_2ds->slice_y_spans);
    }

    return true;
}

static void
free_slices(cg_texture_2d_sliced_t *tex_2ds)
{
    if (tex_2ds->slice_textures != NULL) {
        int i;

        for (i = 0; i < tex_2ds->slice_textures->len; i++) {
            cg_texture_2d_t *slice_tex =
                c_array_index(tex_2ds->slice_textures, cg_texture_2d_t *, i);
            cg_object_unref(slice_tex);
        }

        c_array_free(tex_2ds->slice_textures, true);
    }

    free_spans(tex_2ds);
}

static bool
allocate_slices(cg_texture_2d_sliced_t *tex_2ds,
                int width,
                int height,
                int max_waste,
                cg_pixel_format_t internal_format,
                cg_error_t **error)
{
    cg_texture_t *tex = CG_TEXTURE(tex_2ds);
    cg_device_t *dev = tex->dev;
    int n_x_slices;
    int n_y_slices;
    int n_slices;
    int x, y;
    cg_span_t *x_span;
    cg_span_t *y_span;

    tex_2ds->internal_format = internal_format;

    if (!setup_spans(dev, tex_2ds, width, height, max_waste, internal_format, error)) {
        return false;
    }

    n_x_slices = tex_2ds->slice_x_spans->len;
    n_y_slices = tex_2ds->slice_y_spans->len;
    n_slices = n_x_slices * n_y_slices;

    tex_2ds->slice_textures =
        c_array_sized_new(false, false, sizeof(cg_texture_2d_t *), n_slices);

    /* Allocate each slice */
    for (y = 0; y < n_y_slices; ++y) {
        y_span = &c_array_index(tex_2ds->slice_y_spans, cg_span_t, y);

        for (x = 0; x < n_x_slices; ++x) {
            cg_texture_t *slice;

            x_span = &c_array_index(tex_2ds->slice_x_spans, cg_span_t, x);

            CG_NOTE(SLICING,
                    "CREATE SLICE (%d,%d)\tsize (%d,%d)",
                    x,
                    y,
                    (int)(x_span->size - x_span->waste),
                    (int)(y_span->size - y_span->waste));

            slice = CG_TEXTURE(
                cg_texture_2d_new_with_size(dev, x_span->size, y_span->size));

            _cg_texture_copy_internal_format(tex, slice);

            c_array_append_val(tex_2ds->slice_textures, slice);
            if (!cg_texture_allocate(slice, error)) {
                free_slices(tex_2ds);
                return false;
            }
        }
    }

    return true;
}

static void
_cg_texture_2d_sliced_free(cg_texture_2d_sliced_t *tex_2ds)
{
    free_slices(tex_2ds);

    /* Chain up */
    _cg_texture_free(CG_TEXTURE(tex_2ds));
}

static cg_texture_2d_sliced_t *
_cg_texture_2d_sliced_create_base(cg_device_t *dev,
                                  int width,
                                  int height,
                                  int max_waste,
                                  cg_pixel_format_t internal_format,
                                  cg_texture_loader_t *loader)
{
    cg_texture_2d_sliced_t *tex_2ds = c_new0(cg_texture_2d_sliced_t, 1);

    _cg_texture_init(CG_TEXTURE(tex_2ds),
                     dev,
                     width,
                     height,
                     internal_format,
                     loader,
                     &cg_texture_2d_sliced_vtable);

    tex_2ds->max_waste = max_waste;

    return _cg_texture_2d_sliced_object_new(tex_2ds);
}

cg_texture_2d_sliced_t *
cg_texture_2d_sliced_new_with_size(cg_device_t *dev,
                                   int width,
                                   int height,
                                   int max_waste)
{
    cg_texture_loader_t *loader = _cg_texture_create_loader(dev);
    loader->src_type = CG_TEXTURE_SOURCE_TYPE_SIZED;
    loader->src.sized.width = width;
    loader->src.sized.height = height;

    return _cg_texture_2d_sliced_create_base(dev, width, height, max_waste,
                                             CG_PIXEL_FORMAT_RGBA_8888_PRE,
                                             loader);
}

static cg_texture_2d_sliced_t *
_cg_texture_2d_sliced_new_from_bitmap(
    cg_bitmap_t *bmp, int max_waste, bool can_convert_in_place)
{
    cg_texture_loader_t *loader;

    c_return_val_if_fail(cg_is_bitmap(bmp), NULL);

    loader = _cg_texture_create_loader(bmp->dev);
    loader->src_type = CG_TEXTURE_SOURCE_TYPE_BITMAP;
    loader->src.bitmap.bitmap = cg_object_ref(bmp);
    loader->src.bitmap.can_convert_in_place = can_convert_in_place;

    return _cg_texture_2d_sliced_create_base(_cg_bitmap_get_context(bmp),
                                             cg_bitmap_get_width(bmp),
                                             cg_bitmap_get_height(bmp),
                                             max_waste,
                                             cg_bitmap_get_format(bmp),
                                             loader);
}

cg_texture_2d_sliced_t *
cg_texture_2d_sliced_new_from_bitmap(cg_bitmap_t *bmp,
                                     int max_waste)
{
    return _cg_texture_2d_sliced_new_from_bitmap(bmp, max_waste, false);
}

cg_texture_2d_sliced_t *
cg_texture_2d_sliced_new_from_data(cg_device_t *dev,
                                   int width,
                                   int height,
                                   int max_waste,
                                   cg_pixel_format_t format,
                                   int rowstride,
                                   const uint8_t *data,
                                   cg_error_t **error)
{
    cg_bitmap_t *bmp;
    cg_texture_2d_sliced_t *tex_2ds;

    c_return_val_if_fail(format != CG_PIXEL_FORMAT_ANY, NULL);
    c_return_val_if_fail(data != NULL, NULL);

    /* Rowstride from width if not given */
    if (rowstride == 0)
        rowstride = width * _cg_pixel_format_get_bytes_per_pixel(format);

    /* Wrap the data into a bitmap */
    bmp = cg_bitmap_new_for_data(dev, width, height, format, rowstride,
                                 (uint8_t *)data);

    tex_2ds = cg_texture_2d_sliced_new_from_bitmap(bmp, max_waste);

    cg_object_unref(bmp);

    if (tex_2ds && !cg_texture_allocate(CG_TEXTURE(tex_2ds), error)) {
        cg_object_unref(tex_2ds);
        return NULL;
    }

    return tex_2ds;
}

cg_texture_2d_sliced_t *
cg_texture_2d_sliced_new_from_file(cg_device_t *dev,
                                   const char *filename,
                                   int max_waste,
                                   cg_error_t **error)
{
    cg_bitmap_t *bmp;
    cg_texture_2d_sliced_t *tex_2ds = NULL;

    c_return_val_if_fail(error == NULL || *error == NULL, NULL);

    bmp = cg_bitmap_new_from_file(dev, filename, error);
    if (bmp == NULL)
        return NULL;

    tex_2ds = _cg_texture_2d_sliced_new_from_bitmap(
        bmp, max_waste, true); /* can convert in-place */

    cg_object_unref(bmp);

    return tex_2ds;
}

static bool
allocate_with_size(cg_texture_2d_sliced_t *tex_2ds,
                   cg_texture_loader_t *loader,
                   cg_error_t **error)
{
    cg_texture_t *tex = CG_TEXTURE(tex_2ds);
    cg_pixel_format_t internal_format =
        _cg_texture_determine_internal_format(tex, CG_PIXEL_FORMAT_ANY);

    if (allocate_slices(tex_2ds,
                        loader->src.sized.width,
                        loader->src.sized.height,
                        tex_2ds->max_waste,
                        internal_format,
                        error)) {
        _cg_texture_set_allocated(CG_TEXTURE(tex_2ds),
                                  internal_format,
                                  loader->src.sized.width,
                                  loader->src.sized.height);
        return true;
    } else
        return false;
}

static bool
allocate_from_bitmap(cg_texture_2d_sliced_t *tex_2ds,
                     cg_texture_loader_t *loader,
                     cg_error_t **error)
{
    cg_texture_t *tex = CG_TEXTURE(tex_2ds);
    cg_bitmap_t *bmp = loader->src.bitmap.bitmap;
    int width = cg_bitmap_get_width(bmp);
    int height = cg_bitmap_get_height(bmp);
    bool can_convert_in_place = loader->src.bitmap.can_convert_in_place;
    cg_pixel_format_t internal_format;
    cg_bitmap_t *upload_bmp;

    c_return_val_if_fail(tex_2ds->slice_textures == NULL, false);

    internal_format =
        _cg_texture_determine_internal_format(tex, cg_bitmap_get_format(bmp));

    upload_bmp = _cg_bitmap_convert_for_upload(
        bmp, internal_format, can_convert_in_place, error);
    if (upload_bmp == NULL)
        return false;

    if (!allocate_slices(tex_2ds,
                         width,
                         height,
                         tex_2ds->max_waste,
                         internal_format,
                         error)) {
        cg_object_unref(upload_bmp);
        return false;
    }

    if (!_cg_texture_2d_sliced_upload_bitmap(tex_2ds, upload_bmp, error)) {
        free_slices(tex_2ds);
        cg_object_unref(upload_bmp);
        return false;
    }

    cg_object_unref(upload_bmp);

    _cg_texture_set_allocated(tex, internal_format, width, height);

    return true;
}

static bool
_cg_texture_2d_sliced_allocate(cg_texture_t *tex,
                               cg_error_t **error)
{
    cg_texture_2d_sliced_t *tex_2ds = CG_TEXTURE_2D_SLICED(tex);
    cg_texture_loader_t *loader = tex->loader;

    c_return_val_if_fail(loader, false);

    switch (loader->src_type) {
    case CG_TEXTURE_SOURCE_TYPE_SIZED:
        return allocate_with_size(tex_2ds, loader, error);
    case CG_TEXTURE_SOURCE_TYPE_BITMAP:
        return allocate_from_bitmap(tex_2ds, loader, error);
    default:
        break;
    }

    c_return_val_if_reached(false);
}

static bool
_cg_texture_2d_sliced_is_foreign(cg_texture_t *tex)
{
    cg_texture_2d_sliced_t *tex_2ds = CG_TEXTURE_2D_SLICED(tex);
    cg_texture_2d_t *slice_tex;

    /* Make sure slices were created */
    if (tex_2ds->slice_textures == NULL)
        return false;

    /* Pass the call on to the first slice */
    slice_tex = c_array_index(tex_2ds->slice_textures, cg_texture_2d_t *, 0);
    return _cg_texture_is_foreign(CG_TEXTURE(slice_tex));
}

static bool
_cg_texture_2d_sliced_is_sliced(cg_texture_t *tex)
{
    cg_texture_2d_sliced_t *tex_2ds = CG_TEXTURE_2D_SLICED(tex);

    /* It's only after allocating a sliced texture that we will know
     * whether it really needed to be sliced... */
    if (!tex->allocated)
        cg_texture_allocate(tex, NULL);

    if (tex_2ds->slice_x_spans->len != 1 || tex_2ds->slice_y_spans->len != 1)
        return true;
    else
        return false;
}

static bool
_cg_texture_2d_sliced_can_hardware_repeat(cg_texture_t *tex)
{
    cg_texture_2d_sliced_t *tex_2ds = CG_TEXTURE_2D_SLICED(tex);
    cg_texture_2d_t *slice_tex;
    cg_span_t *x_span;
    cg_span_t *y_span;

    /* If there's more than one texture then we can't hardware repeat */
    if (tex_2ds->slice_textures->len != 1)
        return false;

    /* If there's any waste then we can't hardware repeat */
    x_span = &c_array_index(tex_2ds->slice_x_spans, cg_span_t, 0);
    y_span = &c_array_index(tex_2ds->slice_y_spans, cg_span_t, 0);
    if (x_span->waste > 0 || y_span->waste > 0)
        return false;

    /* Otherwise pass the query on to the single slice texture */
    slice_tex = c_array_index(tex_2ds->slice_textures, cg_texture_2d_t *, 0);
    return _cg_texture_can_hardware_repeat(CG_TEXTURE(slice_tex));
}

static bool
_cg_texture_2d_sliced_get_gl_texture(cg_texture_t *tex,
                                     GLuint *out_gl_handle,
                                     GLenum *out_gl_target)
{
    cg_texture_2d_sliced_t *tex_2ds = CG_TEXTURE_2D_SLICED(tex);
    cg_texture_2d_t *slice_tex;

    if (tex_2ds->slice_textures == NULL)
        return false;

    if (tex_2ds->slice_textures->len < 1)
        return false;

    slice_tex = c_array_index(tex_2ds->slice_textures, cg_texture_2d_t *, 0);

    return cg_texture_get_gl_texture(
        CG_TEXTURE(slice_tex), out_gl_handle, out_gl_target);
}

static void
_cg_texture_2d_sliced_gl_flush_legacy_texobj_filters(
    cg_texture_t *tex, GLenum min_filter, GLenum mag_filter)
{
    cg_texture_2d_sliced_t *tex_2ds = CG_TEXTURE_2D_SLICED(tex);
    cg_texture_2d_t *slice_tex;
    int i;

    c_return_if_fail(tex_2ds->slice_textures != NULL);

    /* Apply new filters to every slice. The slice texture itself should
       cache the value and avoid resubmitting the same filter value to
       GL */
    for (i = 0; i < tex_2ds->slice_textures->len; i++) {
        slice_tex =
            c_array_index(tex_2ds->slice_textures, cg_texture_2d_t *, i);
        _cg_texture_gl_flush_legacy_texobj_filters(
            CG_TEXTURE(slice_tex), min_filter, mag_filter);
    }
}

static void
_cg_texture_2d_sliced_pre_paint(cg_texture_t *tex,
                                cg_texture_pre_paint_flags_t flags)
{
    cg_texture_2d_sliced_t *tex_2ds = CG_TEXTURE_2D_SLICED(tex);
    int i;

    c_return_if_fail(tex_2ds->slice_textures != NULL);

    /* Pass the pre-paint on to every slice */
    for (i = 0; i < tex_2ds->slice_textures->len; i++) {
        cg_texture_2d_t *slice_tex =
            c_array_index(tex_2ds->slice_textures, cg_texture_2d_t *, i);
        _cg_texture_pre_paint(CG_TEXTURE(slice_tex), flags);
    }
}

static bool
_cg_texture_2d_sliced_set_region(cg_texture_t *tex,
                                 int src_x,
                                 int src_y,
                                 int dst_x,
                                 int dst_y,
                                 int dst_width,
                                 int dst_height,
                                 int level,
                                 cg_bitmap_t *bmp,
                                 cg_error_t **error)
{
    cg_texture_2d_sliced_t *tex_2ds = CG_TEXTURE_2D_SLICED(tex);
    cg_bitmap_t *upload_bmp;
    bool status;

    upload_bmp = _cg_bitmap_convert_for_upload(bmp,
                                               _cg_texture_get_format(tex),
                                               false, /* can't convert in
                                                         place */
                                               error);
    if (!upload_bmp)
        return false;

    status = _cg_texture_2d_sliced_upload_subregion(tex_2ds,
                                                    src_x,
                                                    src_y,
                                                    dst_x,
                                                    dst_y,
                                                    dst_width,
                                                    dst_height,
                                                    upload_bmp,
                                                    error);
    cg_object_unref(upload_bmp);

    return status;
}

static cg_pixel_format_t
_cg_texture_2d_sliced_get_format(cg_texture_t *tex)
{
    cg_texture_2d_sliced_t *tex_2ds = CG_TEXTURE_2D_SLICED(tex);

    return tex_2ds->internal_format;
}

static GLenum
_cg_texture_2d_sliced_get_gl_format(cg_texture_t *tex)
{
    cg_texture_2d_sliced_t *tex_2ds = CG_TEXTURE_2D_SLICED(tex);
    cg_texture_2d_t *slice_tex;

    /* Assert that we've allocated our slices at this point */
    cg_texture_allocate(tex, NULL); /* (abort on error) */

    /* Pass the call on to the first slice */
    slice_tex = c_array_index(tex_2ds->slice_textures, cg_texture_2d_t *, 0);
    return _cg_texture_gl_get_format(CG_TEXTURE(slice_tex));
}

static cg_texture_type_t
_cg_texture_2d_sliced_get_type(cg_texture_t *tex)
{
    return CG_TEXTURE_TYPE_2D;
}

static const cg_texture_vtable_t cg_texture_2d_sliced_vtable = {
    false, /* not primitive */
    _cg_texture_2d_sliced_allocate,
    _cg_texture_2d_sliced_set_region,
    NULL, /* get_data */
    _cg_texture_2d_sliced_foreach_sub_texture_in_region,
    _cg_texture_2d_sliced_is_sliced,
    _cg_texture_2d_sliced_can_hardware_repeat,
    _cg_texture_2d_sliced_get_gl_texture,
    _cg_texture_2d_sliced_gl_flush_legacy_texobj_filters,
    _cg_texture_2d_sliced_pre_paint,
    _cg_texture_2d_sliced_gl_flush_legacy_texobj_wrap_modes,
    _cg_texture_2d_sliced_get_format,
    _cg_texture_2d_sliced_get_gl_format,
    _cg_texture_2d_sliced_get_type,
    _cg_texture_2d_sliced_is_foreign,
    NULL /* set_auto_mipmap */
};
