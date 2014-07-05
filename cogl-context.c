/*
 * Cogl
 *
 * A Low-Level GPU Graphics and Utilities API
 *
 * Copyright (C) 2007,2008,2009,2013 Intel Corporation.
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

#include <config.h>

#include "cogl-object.h"
#include "cogl-private.h"
#include "cogl-winsys-private.h"
#include "winsys/cogl-winsys-stub-private.h"
#include "cogl-profile.h"
#include "cogl-util.h"
#include "cogl-context-private.h"
#include "cogl-util-gl-private.h"
#include "cogl-display-private.h"
#include "cogl-renderer-private.h"
#include "cogl-journal-private.h"
#include "cogl-texture-private.h"
#include "cogl-texture-2d-private.h"
#include "cogl-texture-3d-private.h"
#include "cogl-atlas-set.h"
#include "cogl-atlas-texture-private.h"
#include "cogl-pipeline-private.h"
#include "cogl-pipeline-opengl-private.h"
#include "cogl-framebuffer-private.h"
#include "cogl-onscreen-private.h"
#include "cogl-attribute-private.h"
#include "cogl-gpu-info-private.h"
#include "cogl-config-private.h"
#include "cogl-error-private.h"

#include <string.h>
#include <stdlib.h>

/* These aren't defined in the GLES headers */
#ifndef GL_POINT_SPRITE
#define GL_POINT_SPRITE 0x8861
#endif

#ifndef GL_NUM_EXTENSIONS
#define GL_NUM_EXTENSIONS 0x821D
#endif

static void _cogl_context_free (CoglContext *context);

COGL_OBJECT_DEFINE (Context, context);

extern void
_cogl_create_context_driver (CoglContext *context);

static CoglContext *_cogl_context = NULL;

static void
_cogl_init_feature_overrides (CoglContext *ctx)
{
  if (C_UNLIKELY (COGL_DEBUG_ENABLED (COGL_DEBUG_DISABLE_VBOS)))
    COGL_FLAGS_SET (ctx->private_features, COGL_PRIVATE_FEATURE_VBOS, false);

  if (C_UNLIKELY (COGL_DEBUG_ENABLED (COGL_DEBUG_DISABLE_PBOS)))
    COGL_FLAGS_SET (ctx->private_features, COGL_PRIVATE_FEATURE_PBOS, false);

  if (C_UNLIKELY (COGL_DEBUG_ENABLED (COGL_DEBUG_DISABLE_GLSL)))
    {
      COGL_FLAGS_SET (ctx->features, COGL_FEATURE_ID_GLSL, false);
      COGL_FLAGS_SET (ctx->features,
                      COGL_FEATURE_ID_PER_VERTEX_POINT_SIZE,
                      false);
    }

  if (C_UNLIKELY (COGL_DEBUG_ENABLED (COGL_DEBUG_DISABLE_NPOT_TEXTURES)))
    {
      COGL_FLAGS_SET (ctx->features, COGL_FEATURE_ID_TEXTURE_NPOT, false);
      COGL_FLAGS_SET (ctx->features,
                      COGL_FEATURE_ID_TEXTURE_NPOT_BASIC, false);
      COGL_FLAGS_SET (ctx->features,
                      COGL_FEATURE_ID_TEXTURE_NPOT_MIPMAP, false);
      COGL_FLAGS_SET (ctx->features,
                      COGL_FEATURE_ID_TEXTURE_NPOT_REPEAT, false);
    }
}

const CoglWinsysVtable *
_cogl_context_get_winsys (CoglContext *context)
{
  return context->display->renderer->winsys_vtable;
}

/* For reference: There was some deliberation over whether to have a
 * constructor that could throw an exception but looking at standard
 * practices with several high level OO languages including python, C++,
 * C# Java and Ruby they all support exceptions in constructors and the
 * general consensus appears to be that throwing an exception is neater
 * than successfully constructing with an internal error status that
 * would then have to be explicitly checked via some form of ::is_ok()
 * method.
 */
CoglContext *
cogl_context_new (CoglDisplay *display,
                  CoglError **error)
{
  CoglContext *context;
  uint8_t white_pixel[] = { 0xff, 0xff, 0xff, 0xff };
  const CoglWinsysVtable *winsys;
  int i;
  CoglError *internal_error = NULL;

  _cogl_init ();

#ifdef ENABLE_PROFILE
  /* We need to be absolutely sure that uprof has been initialized
   * before calling _cogl_uprof_init. uprof_init (NULL, NULL)
   * will be a NOP if it has been initialized but it will also
   * mean subsequent parsing of the UProf GOptionGroup will have no
   * affect.
   *
   * Sadly GOptionGroup based library initialization is extremely
   * fragile by design because GOptionGroups have no notion of
   * dependencies and so the order things are initialized isn't
   * currently under tight control.
   */
  uprof_init (NULL, NULL);
  _cogl_uprof_init ();
#endif

  /* Allocate context memory */
  context = c_malloc0 (sizeof (CoglContext));

  /* Convert the context into an object immediately in case any of the
     code below wants to verify that the context pointer is a valid
     object */
  _cogl_context_object_new (context);

  /* XXX: Gross hack!
   * Currently everything in Cogl just assumes there is a default
   * context which it can access via _COGL_GET_CONTEXT() including
   * code used to construct a CoglContext. Until all of that code
   * has been updated to take an explicit context argument we have
   * to immediately make our pointer the default context.
   */
  _cogl_context = context;

  /* Init default values */
  memset (context->features, 0, sizeof (context->features));
  memset (context->private_features, 0, sizeof (context->private_features));

  context->rectangle_state = COGL_WINSYS_RECTANGLE_STATE_UNKNOWN;

  memset (context->winsys_features, 0, sizeof (context->winsys_features));

  if (!display)
    {
      CoglRenderer *renderer = cogl_renderer_new ();
      if (!cogl_renderer_connect (renderer, error))
        {
          c_free (context);
          return NULL;
        }

      display = cogl_display_new (renderer, NULL);
      cogl_object_unref(renderer);
    }
  else
    cogl_object_ref (display);

  if (!cogl_display_setup (display, error))
    {
      cogl_object_unref (display);
      c_free (context);
      return NULL;
    }

  context->display = display;

  /* This is duplicated data, but it's much more convenient to have
     the driver attached to the context and the value is accessed a
     lot throughout Cogl */
  context->driver = display->renderer->driver;

  /* Again this is duplicated data, but it convenient to be able
   * access these from the context. */
  context->driver_vtable = display->renderer->driver_vtable;
  context->texture_driver = display->renderer->texture_driver;

  for (i = 0; i < C_N_ELEMENTS (context->private_features); i++)
    context->private_features[i] |= display->renderer->private_features[i];

  winsys = _cogl_context_get_winsys (context);
  if (!winsys->context_init (context, error))
    {
      cogl_object_unref (display);
      c_free (context);
      return NULL;
    }

  context->attribute_name_states_hash =
    c_hash_table_new_full (c_str_hash, c_str_equal, c_free, c_free);
  context->attribute_name_index_map = NULL;
  context->n_attribute_names = 0;

  /* The "cogl_color_in" attribute needs a deterministic name_index
   * so we make sure it's the first attribute name we register */
  _cogl_attribute_register_attribute_name (context, "cogl_color_in");


  context->uniform_names =
    c_ptr_array_new_with_free_func ((c_destroy_func_t) c_free);
  context->uniform_name_hash = c_hash_table_new (c_str_hash, c_str_equal);
  context->n_uniform_names = 0;

  /* Initialise the driver specific state */
  _cogl_init_feature_overrides (context);

  /* XXX: ONGOING BUG: Intel viewport scissor
   *
   * Intel gen6 drivers don't currently correctly handle offset
   * viewports, since primitives aren't clipped within the bounds of
   * the viewport.  To workaround this we push our own clip for the
   * viewport that will use scissoring to ensure we clip as expected.
   *
   * TODO: file a bug upstream!
   */
  if (context->gpu.driver_package == COGL_GPU_INFO_DRIVER_PACKAGE_MESA &&
      context->gpu.architecture == COGL_GPU_INFO_ARCHITECTURE_SANDYBRIDGE &&
      !getenv ("COGL_DISABLE_INTEL_VIEWPORT_SCISSORT_WORKAROUND"))
    context->needs_viewport_scissor_workaround = true;
  else
    context->needs_viewport_scissor_workaround = false;

  context->sampler_cache = _cogl_sampler_cache_new (context);

  _cogl_pipeline_init_default_pipeline ();
  _cogl_pipeline_init_default_layers ();
  _cogl_pipeline_init_state_hash_functions ();
  _cogl_pipeline_init_layer_state_hash_functions ();

  context->current_clip_stack_valid = false;
  context->current_clip_stack = NULL;

  cogl_matrix_init_identity (&context->identity_matrix);
  cogl_matrix_init_identity (&context->y_flip_matrix);
  cogl_matrix_scale (&context->y_flip_matrix, 1, -1, 1);

  context->texture_units =
    c_array_new (false, false, sizeof (CoglTextureUnit));

  if (_cogl_has_private_feature (context, COGL_PRIVATE_FEATURE_ANY_GL))
    {
      /* See cogl-pipeline.c for more details about why we leave texture unit 1
       * active by default... */
      context->active_texture_unit = 1;
      GE (context, glActiveTexture (GL_TEXTURE1));
    }

  context->opaque_color_pipeline = cogl_pipeline_new (context);
  context->codegen_header_buffer = c_string_new ("");
  context->codegen_source_buffer = c_string_new ("");

  context->default_gl_texture_2d_tex = NULL;
  context->default_gl_texture_3d_tex = NULL;

  context->framebuffers = NULL;
  context->current_draw_buffer = NULL;
  context->current_read_buffer = NULL;
  context->current_draw_buffer_state_flushed = 0;
  context->current_draw_buffer_changes = COGL_FRAMEBUFFER_STATE_ALL;

  _cogl_list_init (&context->onscreen_events_queue);
  _cogl_list_init (&context->onscreen_dirty_queue);

  c_queue_init (&context->gles2_context_stack);

  context->journal_flush_attributes_array =
    c_array_new (true, false, sizeof (CoglAttribute *));
  context->journal_clip_bounds = NULL;

  context->current_pipeline = NULL;
  context->current_pipeline_changes_since_flush = 0;
  context->current_pipeline_with_color_attrib = false;

  _cogl_bitmask_init (&context->enabled_custom_attributes);
  _cogl_bitmask_init (&context->enable_custom_attributes_tmp);
  _cogl_bitmask_init (&context->changed_bits_tmp);

  context->max_texture_units = -1;
  context->max_activateable_texture_units = -1;

  context->current_gl_program = 0;

  context->current_gl_dither_enabled = true;
  context->current_gl_color_mask = COGL_COLOR_MASK_ALL;

  context->gl_blend_enable_cache = false;

  context->depth_test_enabled_cache = false;
  context->depth_test_function_cache = COGL_DEPTH_TEST_FUNCTION_LESS;
  context->depth_writing_enabled_cache = true;
  context->depth_range_near_cache = 0;
  context->depth_range_far_cache = 1;

  context->pipeline_cache = _cogl_pipeline_cache_new ();

  for (i = 0; i < COGL_BUFFER_BIND_TARGET_COUNT; i++)
    context->current_buffer[i] = NULL;

  context->stencil_pipeline = cogl_pipeline_new (context);

  context->rectangle_byte_indices = NULL;
  context->rectangle_short_indices = NULL;
  context->rectangle_short_indices_len = 0;

  context->texture_download_pipeline = NULL;
  context->blit_texture_pipeline = NULL;

#if defined (HAVE_COGL_GL)
  if ((context->driver == COGL_DRIVER_GL3))
    {
      GLuint vertex_array;

      /* In a forward compatible context, GL 3 doesn't support rendering
       * using the default vertex array object. Cogl doesn't use vertex
       * array objects yet so for now we just create a dummy array
       * object that we will use as our own default object. Eventually
       * it could be good to attach the vertex array objects to
       * CoglPrimitives */
      context->glGenVertexArrays (1, &vertex_array);
      context->glBindVertexArray (vertex_array);
    }
#endif

  context->current_modelview_entry = NULL;
  context->current_projection_entry = NULL;
  _cogl_matrix_entry_identity_init (&context->identity_entry);
  _cogl_matrix_entry_cache_init (&context->builtin_flushed_projection);
  _cogl_matrix_entry_cache_init (&context->builtin_flushed_modelview);

  /* Create default textures used for fall backs */
  context->default_gl_texture_2d_tex =
    cogl_texture_2d_new_from_data (context,
                                   1, 1,
                                   COGL_PIXEL_FORMAT_RGBA_8888_PRE,
                                   0, /* rowstride */
                                   white_pixel,
                                   NULL); /* abort on error */

  /* If 3D or rectangle textures aren't supported then these will
   * return errors that we can simply ignore. */
  internal_error = NULL;
  context->default_gl_texture_3d_tex =
    cogl_texture_3d_new_from_data (context,
                                   1, 1, 1, /* width, height, depth */
                                   COGL_PIXEL_FORMAT_RGBA_8888_PRE,
                                   0, /* rowstride */
                                   0, /* image stride */
                                   white_pixel,
                                   &internal_error);
  if (internal_error)
    cogl_error_free (internal_error);

  context->buffer_map_fallback_array = c_byte_array_new ();
  context->buffer_map_fallback_in_use = false;

  _cogl_list_init (&context->fences);

  context->atlas_set = cogl_atlas_set_new (context);
  cogl_atlas_set_set_components (context->atlas_set, COGL_TEXTURE_COMPONENTS_RGBA);
  cogl_atlas_set_set_premultiplied (context->atlas_set, false);
  cogl_atlas_set_add_atlas_callback (context->atlas_set,
                                     _cogl_atlas_texture_atlas_event_handler,
                                     NULL, /* user data */
                                     NULL); /* destroy */

  return context;
}

static void
_cogl_context_free (CoglContext *context)
{
  const CoglWinsysVtable *winsys = _cogl_context_get_winsys (context);

  winsys->context_deinit (context);

  if (context->atlas_set)
    cogl_object_unref (context->atlas_set);

  if (context->default_gl_texture_2d_tex)
    cogl_object_unref (context->default_gl_texture_2d_tex);
  if (context->default_gl_texture_3d_tex)
    cogl_object_unref (context->default_gl_texture_3d_tex);

  if (context->opaque_color_pipeline)
    cogl_object_unref (context->opaque_color_pipeline);

  if (context->blit_texture_pipeline)
    cogl_object_unref (context->blit_texture_pipeline);

  c_warn_if_fail (context->gles2_context_stack.length == 0);

  if (context->journal_flush_attributes_array)
    c_array_free (context->journal_flush_attributes_array, true);
  if (context->journal_clip_bounds)
    c_array_free (context->journal_clip_bounds, true);

  if (context->rectangle_byte_indices)
    cogl_object_unref (context->rectangle_byte_indices);
  if (context->rectangle_short_indices)
    cogl_object_unref (context->rectangle_short_indices);

  if (context->default_pipeline)
    cogl_object_unref (context->default_pipeline);

  if (context->dummy_layer_dependant)
    cogl_object_unref (context->dummy_layer_dependant);
  if (context->default_layer_n)
    cogl_object_unref (context->default_layer_n);
  if (context->default_layer_0)
    cogl_object_unref (context->default_layer_0);

  if (context->current_clip_stack_valid)
    _cogl_clip_stack_unref (context->current_clip_stack);

  _cogl_bitmask_destroy (&context->enabled_custom_attributes);
  _cogl_bitmask_destroy (&context->enable_custom_attributes_tmp);
  _cogl_bitmask_destroy (&context->changed_bits_tmp);

  if (context->current_modelview_entry)
    cogl_matrix_entry_unref (context->current_modelview_entry);
  if (context->current_projection_entry)
    cogl_matrix_entry_unref (context->current_projection_entry);
  _cogl_matrix_entry_cache_destroy (&context->builtin_flushed_projection);
  _cogl_matrix_entry_cache_destroy (&context->builtin_flushed_modelview);

  _cogl_pipeline_cache_free (context->pipeline_cache);

  _cogl_sampler_cache_free (context->sampler_cache);

  _cogl_destroy_texture_units ();

  c_ptr_array_free (context->uniform_names, true);
  c_hash_table_destroy (context->uniform_name_hash);

  c_hash_table_destroy (context->attribute_name_states_hash);
  c_array_free (context->attribute_name_index_map, true);

  c_byte_array_free (context->buffer_map_fallback_array, true);

  cogl_object_unref (context->display);

  c_free (context);
}

CoglContext *
_cogl_context_get_default (void)
{
  _COGL_RETURN_VAL_IF_FAIL (_cogl_context != NULL, NULL);
  return _cogl_context;
}

CoglDisplay *
cogl_context_get_display (CoglContext *context)
{
  return context->display;
}

CoglRenderer *
cogl_context_get_renderer (CoglContext *context)
{
  return context->display->renderer;
}

#ifdef COGL_HAS_EGL_SUPPORT
EGLDisplay
cogl_egl_context_get_egl_display (CoglContext *context)
{
  const CoglWinsysVtable *winsys = _cogl_context_get_winsys (context);

  /* This should only be called for EGL contexts */
  _COGL_RETURN_VAL_IF_FAIL (winsys->context_egl_get_egl_display != NULL, NULL);

  return winsys->context_egl_get_egl_display (context);
}
#endif

bool
_cogl_context_update_features (CoglContext *context,
                               CoglError **error)
{
  return context->driver_vtable->update_features (context, error);
}

void
_cogl_context_set_current_projection_entry (CoglContext *context,
                                            CoglMatrixEntry *entry)
{
  cogl_matrix_entry_ref (entry);
  if (context->current_projection_entry)
    cogl_matrix_entry_unref (context->current_projection_entry);
  context->current_projection_entry = entry;
}

void
_cogl_context_set_current_modelview_entry (CoglContext *context,
                                           CoglMatrixEntry *entry)
{
  cogl_matrix_entry_ref (entry);
  if (context->current_modelview_entry)
    cogl_matrix_entry_unref (context->current_modelview_entry);
  context->current_modelview_entry = entry;
}

char **
_cogl_context_get_gl_extensions (CoglContext *context)
{
  const char *env_disabled_extensions;
  char **ret;

  /* In GL 3, querying GL_EXTENSIONS is deprecated so we have to build
   * the array using glGetStringi instead */
#ifdef HAVE_COGL_GL
  if (context->driver == COGL_DRIVER_GL3)
    {
      int num_extensions, i;

      context->glGetIntegerv (GL_NUM_EXTENSIONS, &num_extensions);

      ret = c_malloc (sizeof (char *) * (num_extensions + 1));

      for (i = 0; i < num_extensions; i++)
        {
          const char *ext =
            (const char *) context->glGetStringi (GL_EXTENSIONS, i);
          ret[i] = c_strdup (ext);
        }

      ret[num_extensions] = NULL;
    }
  else
#endif
    {
      const char *all_extensions =
        (const char *) context->glGetString (GL_EXTENSIONS);

      ret = c_strsplit (all_extensions, " ", 0 /* max tokens */);
    }

  if ((env_disabled_extensions = c_getenv ("COGL_DISABLE_GL_EXTENSIONS"))
      || _cogl_config_disable_gl_extensions)
    {
      char **split_env_disabled_extensions;
      char **split_conf_disabled_extensions;
      char **src, **dst;

      if (env_disabled_extensions)
        split_env_disabled_extensions =
          c_strsplit (env_disabled_extensions,
                      ",",
                      0 /* no max tokens */);
      else
        split_env_disabled_extensions = NULL;

      if (_cogl_config_disable_gl_extensions)
        split_conf_disabled_extensions =
          c_strsplit (_cogl_config_disable_gl_extensions,
                      ",",
                      0 /* no max tokens */);
      else
        split_conf_disabled_extensions = NULL;

      for (dst = ret, src = ret;
           *src;
           src++)
        {
          char **d;

          if (split_env_disabled_extensions)
            for (d = split_env_disabled_extensions; *d; d++)
              if (!strcmp (*src, *d))
                goto disabled;
          if (split_conf_disabled_extensions)
            for (d = split_conf_disabled_extensions; *d; d++)
              if (!strcmp (*src, *d))
                goto disabled;

          *(dst++) = *src;
          continue;

        disabled:
          c_free (*src);
          continue;
        }

      *dst = NULL;

      if (split_env_disabled_extensions)
        c_strfreev (split_env_disabled_extensions);
      if (split_conf_disabled_extensions)
        c_strfreev (split_conf_disabled_extensions);
    }

  return ret;
}

const char *
_cogl_context_get_gl_version (CoglContext *context)
{
  const char *version_override;

  if ((version_override = c_getenv ("COGL_OVERRIDE_GL_VERSION")))
    return version_override;
  else if (_cogl_config_override_gl_version)
    return _cogl_config_override_gl_version;
  else
    return (const char *) context->glGetString (GL_VERSION);

}

int64_t
cogl_get_clock_time (CoglContext *context)
{
  const CoglWinsysVtable *winsys = _cogl_context_get_winsys (context);

  if (winsys->context_get_clock_time)
    return winsys->context_get_clock_time (context);
  else
    return 0;
}

CoglAtlasSet *
_cogl_get_atlas_set (CoglContext *context)
{
  return context->atlas_set;
}
