/*
 * CGlib
 *
 * A Low-Level GPU Graphics and Utilities API
 *
 * Copyright (C) 2014 Intel Corporation.
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
 */

#include <cglib-config.h>

#include "cg-pixel-format-private.h"

cg_texture_components_t
_cg_pixel_format_get_components(cg_pixel_format_t format)
{
    switch (format) {
    case CG_PIXEL_FORMAT_A_8:
    case CG_PIXEL_FORMAT_A_8SN:
    case CG_PIXEL_FORMAT_A_16U:
    case CG_PIXEL_FORMAT_A_16F:
    case CG_PIXEL_FORMAT_A_32U:
    case CG_PIXEL_FORMAT_A_32F:
        return CG_TEXTURE_COMPONENTS_A;

    case CG_PIXEL_FORMAT_RG_88:
    case CG_PIXEL_FORMAT_RG_88SN:
    case CG_PIXEL_FORMAT_RG_1616U:
    case CG_PIXEL_FORMAT_RG_1616F:
    case CG_PIXEL_FORMAT_RG_3232U:
    case CG_PIXEL_FORMAT_RG_3232F:
        return CG_TEXTURE_COMPONENTS_RG;

    case CG_PIXEL_FORMAT_RGB_565:
    case CG_PIXEL_FORMAT_RGB_888:
    case CG_PIXEL_FORMAT_BGR_888:
    case CG_PIXEL_FORMAT_RGB_888SN:
    case CG_PIXEL_FORMAT_BGR_888SN:
    case CG_PIXEL_FORMAT_RGB_161616U:
    case CG_PIXEL_FORMAT_BGR_161616U:
    case CG_PIXEL_FORMAT_RGB_161616F:
    case CG_PIXEL_FORMAT_BGR_161616F:
    case CG_PIXEL_FORMAT_RGB_323232U:
    case CG_PIXEL_FORMAT_BGR_323232U:
    case CG_PIXEL_FORMAT_RGB_323232F:
    case CG_PIXEL_FORMAT_BGR_323232F:
        return CG_TEXTURE_COMPONENTS_RGB;

    case CG_PIXEL_FORMAT_RGBA_4444:
    case CG_PIXEL_FORMAT_RGBA_4444_PRE:
    case CG_PIXEL_FORMAT_RGBA_5551:
    case CG_PIXEL_FORMAT_RGBA_5551_PRE:
    case CG_PIXEL_FORMAT_RGBA_8888:
    case CG_PIXEL_FORMAT_BGRA_8888:
    case CG_PIXEL_FORMAT_ARGB_8888:
    case CG_PIXEL_FORMAT_ABGR_8888:
    case CG_PIXEL_FORMAT_RGBA_8888_PRE:
    case CG_PIXEL_FORMAT_BGRA_8888_PRE:
    case CG_PIXEL_FORMAT_ARGB_8888_PRE:
    case CG_PIXEL_FORMAT_ABGR_8888_PRE:
    case CG_PIXEL_FORMAT_RGBA_8888SN:
    case CG_PIXEL_FORMAT_BGRA_8888SN:
    case CG_PIXEL_FORMAT_RGBA_1010102:
    case CG_PIXEL_FORMAT_BGRA_1010102:
    case CG_PIXEL_FORMAT_ARGB_2101010:
    case CG_PIXEL_FORMAT_ABGR_2101010:
    case CG_PIXEL_FORMAT_RGBA_1010102_PRE:
    case CG_PIXEL_FORMAT_BGRA_1010102_PRE:
    case CG_PIXEL_FORMAT_ARGB_2101010_PRE:
    case CG_PIXEL_FORMAT_ABGR_2101010_PRE:
    case CG_PIXEL_FORMAT_RGBA_16161616U:
    case CG_PIXEL_FORMAT_BGRA_16161616U:
    case CG_PIXEL_FORMAT_RGBA_32323232U:
    case CG_PIXEL_FORMAT_BGRA_32323232U:
    case CG_PIXEL_FORMAT_RGBA_16161616F:
    case CG_PIXEL_FORMAT_BGRA_16161616F:
    case CG_PIXEL_FORMAT_RGBA_16161616F_PRE:
    case CG_PIXEL_FORMAT_BGRA_16161616F_PRE:
    case CG_PIXEL_FORMAT_RGBA_32323232F:
    case CG_PIXEL_FORMAT_BGRA_32323232F:
    case CG_PIXEL_FORMAT_RGBA_32323232F_PRE:
    case CG_PIXEL_FORMAT_BGRA_32323232F_PRE:
        return CG_TEXTURE_COMPONENTS_RGBA;

    case CG_PIXEL_FORMAT_DEPTH_16:
    case CG_PIXEL_FORMAT_DEPTH_32:
        return CG_TEXTURE_COMPONENTS_DEPTH;
    case CG_PIXEL_FORMAT_DEPTH_24_STENCIL_8:
        return CG_TEXTURE_COMPONENTS_DEPTH_STENCIL;

    case CG_PIXEL_FORMAT_ANY:
        c_assert_not_reached();
    }

    return CG_TEXTURE_COMPONENTS_RGBA;
}

int
_cg_pixel_format_get_bytes_per_pixel(cg_pixel_format_t format)
{
    switch (format) {
    case CG_PIXEL_FORMAT_A_8:
    case CG_PIXEL_FORMAT_A_8SN:
        return 1;

    case CG_PIXEL_FORMAT_A_16U:
    case CG_PIXEL_FORMAT_A_16F:
    case CG_PIXEL_FORMAT_RG_88:
    case CG_PIXEL_FORMAT_RG_88SN:
    case CG_PIXEL_FORMAT_RGB_565:
    case CG_PIXEL_FORMAT_RGBA_4444:
    case CG_PIXEL_FORMAT_RGBA_4444_PRE:
    case CG_PIXEL_FORMAT_RGBA_5551:
    case CG_PIXEL_FORMAT_RGBA_5551_PRE:
    case CG_PIXEL_FORMAT_DEPTH_16:
        return 2;

    case CG_PIXEL_FORMAT_RGB_888:
    case CG_PIXEL_FORMAT_BGR_888:
    case CG_PIXEL_FORMAT_RGB_888SN:
    case CG_PIXEL_FORMAT_BGR_888SN:
        return 3;

    case CG_PIXEL_FORMAT_A_32U:
    case CG_PIXEL_FORMAT_A_32F:
    case CG_PIXEL_FORMAT_RG_1616U:
    case CG_PIXEL_FORMAT_RG_1616F:
    case CG_PIXEL_FORMAT_RGBA_8888:
    case CG_PIXEL_FORMAT_BGRA_8888:
    case CG_PIXEL_FORMAT_ARGB_8888:
    case CG_PIXEL_FORMAT_ABGR_8888:
    case CG_PIXEL_FORMAT_RGBA_8888_PRE:
    case CG_PIXEL_FORMAT_BGRA_8888_PRE:
    case CG_PIXEL_FORMAT_ARGB_8888_PRE:
    case CG_PIXEL_FORMAT_ABGR_8888_PRE:
    case CG_PIXEL_FORMAT_RGBA_8888SN:
    case CG_PIXEL_FORMAT_BGRA_8888SN:
    case CG_PIXEL_FORMAT_RGBA_1010102:
    case CG_PIXEL_FORMAT_BGRA_1010102:
    case CG_PIXEL_FORMAT_ARGB_2101010:
    case CG_PIXEL_FORMAT_ABGR_2101010:
    case CG_PIXEL_FORMAT_RGBA_1010102_PRE:
    case CG_PIXEL_FORMAT_BGRA_1010102_PRE:
    case CG_PIXEL_FORMAT_ARGB_2101010_PRE:
    case CG_PIXEL_FORMAT_ABGR_2101010_PRE:

    case CG_PIXEL_FORMAT_DEPTH_32:
    case CG_PIXEL_FORMAT_DEPTH_24_STENCIL_8:
        return 4;

    case CG_PIXEL_FORMAT_RGB_161616U:
    case CG_PIXEL_FORMAT_BGR_161616U:
    case CG_PIXEL_FORMAT_RGB_161616F:
    case CG_PIXEL_FORMAT_BGR_161616F:
        return 6;

    case CG_PIXEL_FORMAT_RG_3232U:
    case CG_PIXEL_FORMAT_RG_3232F:
    case CG_PIXEL_FORMAT_RGBA_16161616U:
    case CG_PIXEL_FORMAT_BGRA_16161616U:
    case CG_PIXEL_FORMAT_RGBA_16161616F:
    case CG_PIXEL_FORMAT_BGRA_16161616F:
    case CG_PIXEL_FORMAT_RGBA_16161616F_PRE:
    case CG_PIXEL_FORMAT_BGRA_16161616F_PRE:
        return 8;

    case CG_PIXEL_FORMAT_RGB_323232U:
    case CG_PIXEL_FORMAT_BGR_323232U:
    case CG_PIXEL_FORMAT_RGB_323232F:
    case CG_PIXEL_FORMAT_BGR_323232F:
        return 12;

    case CG_PIXEL_FORMAT_RGBA_32323232U:
    case CG_PIXEL_FORMAT_BGRA_32323232U:
    case CG_PIXEL_FORMAT_RGBA_32323232F:
    case CG_PIXEL_FORMAT_BGRA_32323232F:
    case CG_PIXEL_FORMAT_RGBA_32323232F_PRE:
    case CG_PIXEL_FORMAT_BGRA_32323232F_PRE:
        return 16;

    case CG_PIXEL_FORMAT_ANY:
        c_assert_not_reached();
        return 0;
    }

    return 0;
}

bool
_cg_pixel_format_is_endian_dependant(cg_pixel_format_t format)
{
    switch (format) {
    case CG_PIXEL_FORMAT_RGB_565:
    case CG_PIXEL_FORMAT_RGBA_4444:
    case CG_PIXEL_FORMAT_RGBA_4444_PRE:
    case CG_PIXEL_FORMAT_RGBA_5551:
    case CG_PIXEL_FORMAT_RGBA_5551_PRE:
    case CG_PIXEL_FORMAT_RGBA_1010102:
    case CG_PIXEL_FORMAT_BGRA_1010102:
    case CG_PIXEL_FORMAT_ARGB_2101010:
    case CG_PIXEL_FORMAT_ABGR_2101010:
    case CG_PIXEL_FORMAT_RGBA_1010102_PRE:
    case CG_PIXEL_FORMAT_BGRA_1010102_PRE:
    case CG_PIXEL_FORMAT_ARGB_2101010_PRE:
    case CG_PIXEL_FORMAT_ABGR_2101010_PRE:

    case CG_PIXEL_FORMAT_DEPTH_24_STENCIL_8:
        return true;
    default:
        return false;
    }
}

bool
_cg_pixel_format_is_premultiplied(cg_pixel_format_t format)
{
    switch (format) {
    case CG_PIXEL_FORMAT_RGBA_4444_PRE:
    case CG_PIXEL_FORMAT_RGBA_5551_PRE:
    case CG_PIXEL_FORMAT_RGBA_8888_PRE:
    case CG_PIXEL_FORMAT_BGRA_8888_PRE:
    case CG_PIXEL_FORMAT_ARGB_8888_PRE:
    case CG_PIXEL_FORMAT_ABGR_8888_PRE:
    case CG_PIXEL_FORMAT_RGBA_1010102_PRE:
    case CG_PIXEL_FORMAT_BGRA_1010102_PRE:
    case CG_PIXEL_FORMAT_ARGB_2101010_PRE:
    case CG_PIXEL_FORMAT_ABGR_2101010_PRE:
    case CG_PIXEL_FORMAT_RGBA_16161616F_PRE:
    case CG_PIXEL_FORMAT_BGRA_16161616F_PRE:
    case CG_PIXEL_FORMAT_RGBA_32323232F_PRE:
    case CG_PIXEL_FORMAT_BGRA_32323232F_PRE:
        return true;
    default:
        return false;
    }
}

bool
_cg_pixel_format_has_alpha(cg_pixel_format_t format)
{
    switch (format) {
    case CG_PIXEL_FORMAT_A_8:
    case CG_PIXEL_FORMAT_A_8SN:
    case CG_PIXEL_FORMAT_A_16U:
    case CG_PIXEL_FORMAT_A_16F:
    case CG_PIXEL_FORMAT_A_32U:
    case CG_PIXEL_FORMAT_A_32F:
    case CG_PIXEL_FORMAT_RGBA_4444:
    case CG_PIXEL_FORMAT_RGBA_4444_PRE:
    case CG_PIXEL_FORMAT_RGBA_5551:
    case CG_PIXEL_FORMAT_RGBA_5551_PRE:
    case CG_PIXEL_FORMAT_RGBA_8888:
    case CG_PIXEL_FORMAT_BGRA_8888:
    case CG_PIXEL_FORMAT_ARGB_8888:
    case CG_PIXEL_FORMAT_ABGR_8888:
    case CG_PIXEL_FORMAT_RGBA_8888_PRE:
    case CG_PIXEL_FORMAT_BGRA_8888_PRE:
    case CG_PIXEL_FORMAT_ARGB_8888_PRE:
    case CG_PIXEL_FORMAT_ABGR_8888_PRE:
    case CG_PIXEL_FORMAT_RGBA_8888SN:
    case CG_PIXEL_FORMAT_BGRA_8888SN:
    case CG_PIXEL_FORMAT_RGBA_1010102:
    case CG_PIXEL_FORMAT_BGRA_1010102:
    case CG_PIXEL_FORMAT_ARGB_2101010:
    case CG_PIXEL_FORMAT_ABGR_2101010:
    case CG_PIXEL_FORMAT_RGBA_1010102_PRE:
    case CG_PIXEL_FORMAT_BGRA_1010102_PRE:
    case CG_PIXEL_FORMAT_ARGB_2101010_PRE:
    case CG_PIXEL_FORMAT_ABGR_2101010_PRE:
    case CG_PIXEL_FORMAT_RGBA_16161616U:
    case CG_PIXEL_FORMAT_BGRA_16161616U:
    case CG_PIXEL_FORMAT_RGBA_16161616F:
    case CG_PIXEL_FORMAT_BGRA_16161616F:
    case CG_PIXEL_FORMAT_RGBA_16161616F_PRE:
    case CG_PIXEL_FORMAT_BGRA_16161616F_PRE:
    case CG_PIXEL_FORMAT_RGBA_32323232U:
    case CG_PIXEL_FORMAT_BGRA_32323232U:
    case CG_PIXEL_FORMAT_RGBA_32323232F:
    case CG_PIXEL_FORMAT_BGRA_32323232F:
    case CG_PIXEL_FORMAT_RGBA_32323232F_PRE:
    case CG_PIXEL_FORMAT_BGRA_32323232F_PRE:
        return true;

    case CG_PIXEL_FORMAT_RG_88:
    case CG_PIXEL_FORMAT_RG_88SN:
    case CG_PIXEL_FORMAT_RG_1616U:
    case CG_PIXEL_FORMAT_RG_1616F:
    case CG_PIXEL_FORMAT_RG_3232U:
    case CG_PIXEL_FORMAT_RG_3232F:
    case CG_PIXEL_FORMAT_RGB_565:
    case CG_PIXEL_FORMAT_RGB_888:
    case CG_PIXEL_FORMAT_BGR_888:
    case CG_PIXEL_FORMAT_RGB_888SN:
    case CG_PIXEL_FORMAT_BGR_888SN:
    case CG_PIXEL_FORMAT_RGB_161616U:
    case CG_PIXEL_FORMAT_BGR_161616U:
    case CG_PIXEL_FORMAT_RGB_161616F:
    case CG_PIXEL_FORMAT_BGR_161616F:
    case CG_PIXEL_FORMAT_RGB_323232U:
    case CG_PIXEL_FORMAT_BGR_323232U:
    case CG_PIXEL_FORMAT_RGB_323232F:
    case CG_PIXEL_FORMAT_BGR_323232F:
    case CG_PIXEL_FORMAT_DEPTH_16:
    case CG_PIXEL_FORMAT_DEPTH_32:
    case CG_PIXEL_FORMAT_DEPTH_24_STENCIL_8:
        return false;

    case CG_PIXEL_FORMAT_ANY:
        c_assert_not_reached();
        return false;
    }

    c_assert_not_reached();
    return false;
}

bool
_cg_pixel_format_has_depth(cg_pixel_format_t format)
{
    cg_texture_components_t components = _cg_pixel_format_get_components(format);

    if (components == CG_TEXTURE_COMPONENTS_DEPTH ||
        components == CG_TEXTURE_COMPONENTS_DEPTH_STENCIL)
        return true;
    else
        return false;
}

bool
_cg_pixel_format_can_be_premultiplied(cg_pixel_format_t format)
{
    return (format != CG_PIXEL_FORMAT_A_8 && _cg_pixel_format_has_alpha(format));
}

cg_pixel_format_t
_cg_pixel_format_premult_stem(cg_pixel_format_t format)
{

    switch (format) {
    case CG_PIXEL_FORMAT_RGBA_4444_PRE:
        return CG_PIXEL_FORMAT_RGBA_4444;
    case CG_PIXEL_FORMAT_RGBA_5551_PRE:
        return CG_PIXEL_FORMAT_RGBA_5551;
    case CG_PIXEL_FORMAT_RGBA_8888_PRE:
        return CG_PIXEL_FORMAT_RGBA_8888;
    case CG_PIXEL_FORMAT_BGRA_8888_PRE:
        return CG_PIXEL_FORMAT_BGRA_8888;
    case CG_PIXEL_FORMAT_ARGB_8888_PRE:
        return CG_PIXEL_FORMAT_ARGB_8888;
    case CG_PIXEL_FORMAT_ABGR_8888_PRE:
        return CG_PIXEL_FORMAT_ABGR_8888;
    case CG_PIXEL_FORMAT_RGBA_1010102_PRE:
        return CG_PIXEL_FORMAT_RGBA_1010102;
    case CG_PIXEL_FORMAT_BGRA_1010102_PRE:
        return CG_PIXEL_FORMAT_BGRA_1010102;
    case CG_PIXEL_FORMAT_ARGB_2101010_PRE:
        return CG_PIXEL_FORMAT_ARGB_2101010;
    case CG_PIXEL_FORMAT_ABGR_2101010_PRE:
        return CG_PIXEL_FORMAT_ABGR_2101010;
    case CG_PIXEL_FORMAT_RGBA_16161616F_PRE:
        return CG_PIXEL_FORMAT_RGBA_16161616F;
    case CG_PIXEL_FORMAT_BGRA_16161616F_PRE:
        return CG_PIXEL_FORMAT_BGRA_16161616F;
    case CG_PIXEL_FORMAT_RGBA_32323232F_PRE:
        return CG_PIXEL_FORMAT_RGBA_32323232F;
    case CG_PIXEL_FORMAT_BGRA_32323232F_PRE:
        return CG_PIXEL_FORMAT_BGRA_32323232F;
    default:
        return format;
    }
}

cg_pixel_format_t
_cg_pixel_format_premultiply(cg_pixel_format_t format)
{
    switch (format) {
    case CG_PIXEL_FORMAT_RGBA_4444:
    case CG_PIXEL_FORMAT_RGBA_4444_PRE:
        return CG_PIXEL_FORMAT_RGBA_4444_PRE;
    case CG_PIXEL_FORMAT_RGBA_5551:
    case CG_PIXEL_FORMAT_RGBA_5551_PRE:
        return CG_PIXEL_FORMAT_RGBA_5551_PRE;
    case CG_PIXEL_FORMAT_RGBA_8888:
    case CG_PIXEL_FORMAT_RGBA_8888_PRE:
        return CG_PIXEL_FORMAT_RGBA_8888_PRE;
    case CG_PIXEL_FORMAT_BGRA_8888:
    case CG_PIXEL_FORMAT_BGRA_8888_PRE:
        return CG_PIXEL_FORMAT_BGRA_8888_PRE;
    case CG_PIXEL_FORMAT_ARGB_8888:
    case CG_PIXEL_FORMAT_ARGB_8888_PRE:
        return CG_PIXEL_FORMAT_ARGB_8888_PRE;
    case CG_PIXEL_FORMAT_ABGR_8888:
    case CG_PIXEL_FORMAT_ABGR_8888_PRE:
        return CG_PIXEL_FORMAT_ABGR_8888_PRE;
    case CG_PIXEL_FORMAT_RGBA_1010102:
    case CG_PIXEL_FORMAT_RGBA_1010102_PRE:
        return CG_PIXEL_FORMAT_RGBA_1010102_PRE;
    case CG_PIXEL_FORMAT_BGRA_1010102:
    case CG_PIXEL_FORMAT_BGRA_1010102_PRE:
        return CG_PIXEL_FORMAT_BGRA_1010102_PRE;
    case CG_PIXEL_FORMAT_ARGB_2101010:
    case CG_PIXEL_FORMAT_ARGB_2101010_PRE:
        return CG_PIXEL_FORMAT_ARGB_2101010_PRE;
    case CG_PIXEL_FORMAT_ABGR_2101010:
    case CG_PIXEL_FORMAT_ABGR_2101010_PRE:
        return CG_PIXEL_FORMAT_ABGR_2101010_PRE;
    case CG_PIXEL_FORMAT_RGBA_16161616F:
    case CG_PIXEL_FORMAT_RGBA_16161616F_PRE:
        return CG_PIXEL_FORMAT_RGBA_16161616F_PRE;
    case CG_PIXEL_FORMAT_BGRA_16161616F:
    case CG_PIXEL_FORMAT_BGRA_16161616F_PRE:
        return CG_PIXEL_FORMAT_BGRA_16161616F_PRE;
    case CG_PIXEL_FORMAT_RGBA_32323232F:
    case CG_PIXEL_FORMAT_RGBA_32323232F_PRE:
        return CG_PIXEL_FORMAT_RGBA_32323232F_PRE;
    case CG_PIXEL_FORMAT_BGRA_32323232F:
    case CG_PIXEL_FORMAT_BGRA_32323232F_PRE:
        return CG_PIXEL_FORMAT_BGRA_32323232F_PRE;
    default:
        c_assert_not_reached();
        return CG_PIXEL_FORMAT_RGBA_8888_PRE;
    }
}

cg_pixel_format_t
_cg_pixel_format_toggle_premult_status(cg_pixel_format_t format)
{
    if (_cg_pixel_format_is_premultiplied(format))
        return _cg_pixel_format_premult_stem(format);
    else
        return _cg_pixel_format_premultiply(format);
}

cg_pixel_format_t
_cg_pixel_format_flip_rgb_order(cg_pixel_format_t format)
{
    switch (format) {
    case CG_PIXEL_FORMAT_ANY:
    case CG_PIXEL_FORMAT_A_8:
    case CG_PIXEL_FORMAT_A_8SN:
    case CG_PIXEL_FORMAT_A_16U:
    case CG_PIXEL_FORMAT_A_16F:
    case CG_PIXEL_FORMAT_A_32U:
    case CG_PIXEL_FORMAT_A_32F:
    case CG_PIXEL_FORMAT_RG_88:
    case CG_PIXEL_FORMAT_RG_88SN:
    case CG_PIXEL_FORMAT_RG_1616U:
    case CG_PIXEL_FORMAT_RG_1616F:
    case CG_PIXEL_FORMAT_RG_3232U:
    case CG_PIXEL_FORMAT_RG_3232F:
    case CG_PIXEL_FORMAT_RGB_565:
    case CG_PIXEL_FORMAT_RGBA_4444:
    case CG_PIXEL_FORMAT_RGBA_4444_PRE:
    case CG_PIXEL_FORMAT_RGBA_5551:
    case CG_PIXEL_FORMAT_RGBA_5551_PRE:
    case CG_PIXEL_FORMAT_DEPTH_16:
    case CG_PIXEL_FORMAT_DEPTH_32:
    case CG_PIXEL_FORMAT_DEPTH_24_STENCIL_8:
        c_assert_not_reached();
        return format;

    case CG_PIXEL_FORMAT_RGB_888:
        return CG_PIXEL_FORMAT_BGR_888;
    case CG_PIXEL_FORMAT_BGR_888:
        return CG_PIXEL_FORMAT_RGB_888;
    case CG_PIXEL_FORMAT_RGB_888SN:
        return CG_PIXEL_FORMAT_BGR_888SN;
    case CG_PIXEL_FORMAT_BGR_888SN:
        return CG_PIXEL_FORMAT_RGB_888SN;
    case CG_PIXEL_FORMAT_RGB_161616U:
        return CG_PIXEL_FORMAT_BGR_161616U;
    case CG_PIXEL_FORMAT_BGR_161616U:
        return CG_PIXEL_FORMAT_RGB_161616U;
    case CG_PIXEL_FORMAT_RGB_161616F:
        return CG_PIXEL_FORMAT_BGR_161616F;
    case CG_PIXEL_FORMAT_BGR_161616F:
        return CG_PIXEL_FORMAT_RGB_161616F;
    case CG_PIXEL_FORMAT_RGB_323232U:
        return CG_PIXEL_FORMAT_BGR_323232U;
    case CG_PIXEL_FORMAT_BGR_323232U:
        return CG_PIXEL_FORMAT_RGB_323232U;
    case CG_PIXEL_FORMAT_RGB_323232F:
        return CG_PIXEL_FORMAT_BGR_323232F;
    case CG_PIXEL_FORMAT_BGR_323232F:
        return CG_PIXEL_FORMAT_RGB_323232F;
    case CG_PIXEL_FORMAT_RGBA_8888:
        return CG_PIXEL_FORMAT_BGRA_8888;
    case CG_PIXEL_FORMAT_BGRA_8888:
        return CG_PIXEL_FORMAT_RGBA_8888;
    case CG_PIXEL_FORMAT_ARGB_8888:
        return CG_PIXEL_FORMAT_ABGR_8888;
    case CG_PIXEL_FORMAT_ABGR_8888:
        return CG_PIXEL_FORMAT_ARGB_8888;
    case CG_PIXEL_FORMAT_RGBA_8888_PRE:
        return CG_PIXEL_FORMAT_BGRA_8888_PRE;
    case CG_PIXEL_FORMAT_BGRA_8888_PRE:
        return CG_PIXEL_FORMAT_RGBA_8888_PRE;
    case CG_PIXEL_FORMAT_ARGB_8888_PRE:
        return CG_PIXEL_FORMAT_ABGR_8888_PRE;
    case CG_PIXEL_FORMAT_ABGR_8888_PRE:
        return CG_PIXEL_FORMAT_ARGB_8888_PRE;
    case CG_PIXEL_FORMAT_RGBA_8888SN:
        return CG_PIXEL_FORMAT_BGRA_8888SN;
    case CG_PIXEL_FORMAT_BGRA_8888SN:
        return CG_PIXEL_FORMAT_RGBA_8888SN;
    case CG_PIXEL_FORMAT_RGBA_1010102:
        return CG_PIXEL_FORMAT_BGRA_1010102;
    case CG_PIXEL_FORMAT_BGRA_1010102:
        return CG_PIXEL_FORMAT_RGBA_1010102;
    case CG_PIXEL_FORMAT_ARGB_2101010:
        return CG_PIXEL_FORMAT_ABGR_2101010;
    case CG_PIXEL_FORMAT_ABGR_2101010:
        return CG_PIXEL_FORMAT_ARGB_2101010;
    case CG_PIXEL_FORMAT_RGBA_1010102_PRE:
        return CG_PIXEL_FORMAT_BGRA_1010102_PRE;
    case CG_PIXEL_FORMAT_BGRA_1010102_PRE:
        return CG_PIXEL_FORMAT_RGBA_1010102_PRE;
    case CG_PIXEL_FORMAT_ARGB_2101010_PRE:
        return CG_PIXEL_FORMAT_ABGR_2101010_PRE;
    case CG_PIXEL_FORMAT_ABGR_2101010_PRE:
        return CG_PIXEL_FORMAT_ARGB_2101010_PRE;
    case CG_PIXEL_FORMAT_RGBA_16161616U:
        return CG_PIXEL_FORMAT_BGRA_16161616U;
    case CG_PIXEL_FORMAT_BGRA_16161616U:
        return CG_PIXEL_FORMAT_RGBA_16161616U;
    case CG_PIXEL_FORMAT_RGBA_32323232U:
        return CG_PIXEL_FORMAT_BGRA_32323232U;
    case CG_PIXEL_FORMAT_BGRA_32323232U:
        return CG_PIXEL_FORMAT_RGBA_32323232U;
    case CG_PIXEL_FORMAT_RGBA_16161616F:
        return CG_PIXEL_FORMAT_BGRA_16161616F;
    case CG_PIXEL_FORMAT_BGRA_16161616F:
        return CG_PIXEL_FORMAT_RGBA_16161616F;
    case CG_PIXEL_FORMAT_RGBA_16161616F_PRE:
        return CG_PIXEL_FORMAT_BGRA_16161616F_PRE;
    case CG_PIXEL_FORMAT_BGRA_16161616F_PRE:
        return CG_PIXEL_FORMAT_RGBA_16161616F_PRE;
    case CG_PIXEL_FORMAT_RGBA_32323232F:
        return CG_PIXEL_FORMAT_BGRA_32323232F;
    case CG_PIXEL_FORMAT_BGRA_32323232F:
        return CG_PIXEL_FORMAT_RGBA_32323232F;
    case CG_PIXEL_FORMAT_RGBA_32323232F_PRE:
        return CG_PIXEL_FORMAT_BGRA_32323232F_PRE;
    case CG_PIXEL_FORMAT_BGRA_32323232F_PRE:
        return CG_PIXEL_FORMAT_RGBA_32323232F_PRE;
    }

    c_assert_not_reached();
    return format;
}

cg_pixel_format_t
_cg_pixel_format_flip_alpha_position(cg_pixel_format_t format)
{
    switch (format) {
    case CG_PIXEL_FORMAT_ANY:
    case CG_PIXEL_FORMAT_A_8:
    case CG_PIXEL_FORMAT_A_8SN:
    case CG_PIXEL_FORMAT_A_16U:
    case CG_PIXEL_FORMAT_A_16F:
    case CG_PIXEL_FORMAT_A_32U:
    case CG_PIXEL_FORMAT_A_32F:
    case CG_PIXEL_FORMAT_RG_88:
    case CG_PIXEL_FORMAT_RG_88SN:
    case CG_PIXEL_FORMAT_RG_1616U:
    case CG_PIXEL_FORMAT_RG_1616F:
    case CG_PIXEL_FORMAT_RG_3232U:
    case CG_PIXEL_FORMAT_RG_3232F:
    case CG_PIXEL_FORMAT_RGB_565:
    case CG_PIXEL_FORMAT_RGB_888:
    case CG_PIXEL_FORMAT_BGR_888:
    case CG_PIXEL_FORMAT_RGB_888SN:
    case CG_PIXEL_FORMAT_BGR_888SN:
    case CG_PIXEL_FORMAT_RGB_161616U:
    case CG_PIXEL_FORMAT_BGR_161616U:
    case CG_PIXEL_FORMAT_RGB_161616F:
    case CG_PIXEL_FORMAT_BGR_161616F:
    case CG_PIXEL_FORMAT_RGB_323232U:
    case CG_PIXEL_FORMAT_BGR_323232U:
    case CG_PIXEL_FORMAT_RGB_323232F:
    case CG_PIXEL_FORMAT_BGR_323232F:
    case CG_PIXEL_FORMAT_RGBA_4444:
    case CG_PIXEL_FORMAT_RGBA_4444_PRE:
    case CG_PIXEL_FORMAT_RGBA_5551:
    case CG_PIXEL_FORMAT_RGBA_5551_PRE:
    case CG_PIXEL_FORMAT_RGBA_8888SN:
    case CG_PIXEL_FORMAT_BGRA_8888SN:
    case CG_PIXEL_FORMAT_RGBA_16161616U:
    case CG_PIXEL_FORMAT_BGRA_16161616U:
    case CG_PIXEL_FORMAT_RGBA_32323232U:
    case CG_PIXEL_FORMAT_BGRA_32323232U:
    case CG_PIXEL_FORMAT_RGBA_16161616F:
    case CG_PIXEL_FORMAT_BGRA_16161616F:
    case CG_PIXEL_FORMAT_RGBA_16161616F_PRE:
    case CG_PIXEL_FORMAT_BGRA_16161616F_PRE:
    case CG_PIXEL_FORMAT_RGBA_32323232F:
    case CG_PIXEL_FORMAT_BGRA_32323232F:
    case CG_PIXEL_FORMAT_RGBA_32323232F_PRE:
    case CG_PIXEL_FORMAT_BGRA_32323232F_PRE:
    case CG_PIXEL_FORMAT_DEPTH_16:
    case CG_PIXEL_FORMAT_DEPTH_32:
    case CG_PIXEL_FORMAT_DEPTH_24_STENCIL_8:
        c_assert_not_reached();
        return format;

    case CG_PIXEL_FORMAT_RGBA_8888:
        return CG_PIXEL_FORMAT_ARGB_8888;
    case CG_PIXEL_FORMAT_BGRA_8888:
        return CG_PIXEL_FORMAT_ABGR_8888;
    case CG_PIXEL_FORMAT_ARGB_8888:
        return CG_PIXEL_FORMAT_RGBA_8888;
    case CG_PIXEL_FORMAT_ABGR_8888:
        return CG_PIXEL_FORMAT_BGRA_8888;
    case CG_PIXEL_FORMAT_RGBA_8888_PRE:
        return CG_PIXEL_FORMAT_ARGB_8888_PRE;
    case CG_PIXEL_FORMAT_BGRA_8888_PRE:
        return CG_PIXEL_FORMAT_ABGR_8888_PRE;
    case CG_PIXEL_FORMAT_ARGB_8888_PRE:
        return CG_PIXEL_FORMAT_RGBA_8888_PRE;
    case CG_PIXEL_FORMAT_ABGR_8888_PRE:
        return CG_PIXEL_FORMAT_BGRA_8888_PRE;
    case CG_PIXEL_FORMAT_RGBA_1010102:
        return CG_PIXEL_FORMAT_ARGB_2101010;
    case CG_PIXEL_FORMAT_BGRA_1010102:
        return CG_PIXEL_FORMAT_ABGR_2101010;
    case CG_PIXEL_FORMAT_ARGB_2101010:
        return CG_PIXEL_FORMAT_RGBA_1010102;
    case CG_PIXEL_FORMAT_ABGR_2101010:
        return CG_PIXEL_FORMAT_BGRA_1010102;
    case CG_PIXEL_FORMAT_RGBA_1010102_PRE:
        return CG_PIXEL_FORMAT_ARGB_2101010_PRE;
    case CG_PIXEL_FORMAT_BGRA_1010102_PRE:
        return CG_PIXEL_FORMAT_ABGR_2101010_PRE;
    case CG_PIXEL_FORMAT_ARGB_2101010_PRE:
        return CG_PIXEL_FORMAT_RGBA_1010102_PRE;
    case CG_PIXEL_FORMAT_ABGR_2101010_PRE:
        return CG_PIXEL_FORMAT_BGRA_1010102_PRE;
    }

    c_assert_not_reached();
    return format;
}
