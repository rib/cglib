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

#ifndef __COGL_CONTEXT_PRIVATE_H
#define __COGL_CONTEXT_PRIVATE_H

#include "cogl-context.h"
#include "cogl-winsys-private.h"
#include "cogl-flags.h"

#ifdef COGL_HAS_XLIB_SUPPORT
#include "cogl-xlib-renderer-private.h"
#endif

#include "cogl-display-private.h"
#include "cogl-clip-stack.h"
#include "cogl-matrix-stack.h"
#include "cogl-pipeline-private.h"
#include "cogl-buffer-private.h"
#include "cogl-bitmask.h"
#include "cogl-atlas-set.h"
#include "cogl-driver.h"
#include "cogl-texture-driver.h"
#include "cogl-pipeline-cache.h"
#include "cogl-texture-2d.h"
#include "cogl-texture-3d.h"
#include "cogl-sampler-cache-private.h"
#include "cogl-gpu-info-private.h"
#include "cogl-gl-header.h"
#include "cogl-framebuffer-private.h"
#include "cogl-onscreen-private.h"
#include "cogl-fence-private.h"
#include "cogl-poll-private.h"
#include "cogl-private.h"

typedef struct
{
  GLfloat v[3];
  GLfloat t[2];
  GLubyte c[4];
} CoglTextureGLVertex;

struct _CoglContext
{
  CoglObject _parent;

  CoglDisplay *display;

  CoglDriver driver;

  /* Information about the GPU and driver which we can use to
     determine certain workarounds */
  CoglGpuInfo gpu;

  /* vtables for the driver functions */
  const CoglDriverVtable *driver_vtable;
  const CoglTextureDriver *texture_driver;

  int glsl_major;
  int glsl_minor;

  /* This is the GLSL version that we will claim that snippets are
   * written against using the #version pragma. This will be the
   * largest version that is less than or equal to the version
   * provided by the driver without massively altering the syntax. Eg,
   * we wouldn't use version 1.3 even if it is available because that
   * removes the ‘attribute’ and ‘varying’ keywords. */
  int glsl_version_to_use;

  /* Features cache */
  unsigned long features[COGL_FLAGS_N_LONGS_FOR_SIZE (_COGL_N_FEATURE_IDS)];
  unsigned long private_features
    [COGL_FLAGS_N_LONGS_FOR_SIZE (COGL_N_PRIVATE_FEATURES)];

  bool needs_viewport_scissor_workaround;
  CoglFramebuffer *viewport_scissor_workaround_framebuffer;

  CoglPipeline *default_pipeline;
  CoglPipelineLayer *default_layer_0;
  CoglPipelineLayer *default_layer_n;
  CoglPipelineLayer *dummy_layer_dependant;

  c_hash_table_t *attribute_name_states_hash;
  c_array_t *attribute_name_index_map;
  int n_attribute_names;

  CoglBitmask       enabled_custom_attributes;

  /* A temporary bitmask that is used when enabling/disabling
   * custom attribute arrays */
  CoglBitmask       enable_custom_attributes_tmp;
  CoglBitmask       changed_bits_tmp;

  /* A few handy matrix constants */
  CoglMatrix        identity_matrix;
  CoglMatrix        y_flip_matrix;

  /* The matrix stack entries that should be flushed during the next
   * pipeline state flush */
  CoglMatrixEntry *current_projection_entry;
  CoglMatrixEntry *current_modelview_entry;

  CoglMatrixEntry identity_entry;

  /* A cache of the last (immutable) matrix stack entries that were
   * flushed to the GL matrix builtins */
  CoglMatrixEntryCache builtin_flushed_projection;
  CoglMatrixEntryCache builtin_flushed_modelview;

  c_array_t           *texture_units;
  int               active_texture_unit;

  /* Pipelines */
  CoglPipeline     *opaque_color_pipeline; /* to check for simple pipelines */
  c_string_t          *codegen_header_buffer;
  c_string_t          *codegen_source_buffer;

  CoglPipelineCache *pipeline_cache;

  /* Textures */
  CoglTexture2D *default_gl_texture_2d_tex;
  CoglTexture3D *default_gl_texture_3d_tex;

  /* Central list of all framebuffers so all journals can be flushed
   * at any time. */
  c_list_t            *framebuffers;

  /* Global journal buffers */
  c_array_t           *journal_flush_attributes_array;
  c_array_t           *journal_clip_bounds;

  /* Some simple caching, to minimize state changes... */
  CoglPipeline     *current_pipeline;
  unsigned long     current_pipeline_changes_since_flush;
  bool          current_pipeline_with_color_attrib;
  bool          current_pipeline_unknown_color_alpha;
  unsigned long     current_pipeline_age;

  bool          gl_blend_enable_cache;

  bool              depth_test_enabled_cache;
  CoglDepthTestFunction depth_test_function_cache;
  bool              depth_writing_enabled_cache;
  float                 depth_range_near_cache;
  float                 depth_range_far_cache;

  CoglBuffer       *current_buffer[COGL_BUFFER_BIND_TARGET_COUNT];

  /* Framebuffers */
  unsigned long     current_draw_buffer_state_flushed;
  unsigned long     current_draw_buffer_changes;
  CoglFramebuffer  *current_draw_buffer;
  CoglFramebuffer  *current_read_buffer;

  bool have_last_offscreen_allocate_flags;
  CoglOffscreenAllocateFlags last_offscreen_allocate_flags;

  CoglList onscreen_events_queue;
  CoglList onscreen_dirty_queue;
  CoglClosure *onscreen_dispatch_idle;

  CoglGLES2Context *current_gles2_context;
  c_queue_t gles2_context_stack;

  /* This becomes true the first time the context is bound to an
   * onscreen buffer. This is used by cogl-framebuffer-gl to determine
   * when to initialise the glDrawBuffer state */
  bool was_bound_to_onscreen;

  /* Primitives */
  CoglPipeline     *stencil_pipeline;

  /* Pre-generated VBOs containing indices to generate GL_TRIANGLES
     out of a vertex array of quads */
  CoglIndices      *rectangle_byte_indices;
  CoglIndices      *rectangle_short_indices;
  int               rectangle_short_indices_len;

  CoglPipeline     *texture_download_pipeline;
  CoglPipeline     *blit_texture_pipeline;

  CoglAtlasSet     *atlas_set;

  /* This debugging variable is used to pick a colour for visually
     displaying the quad batches. It needs to be global so that it can
     be reset by cogl_clear. It needs to be reset to increase the
     chances of getting the same colour during an animation */
  uint8_t            journal_rectangles_color;

  /* Cached values for GL_MAX_TEXTURE_[IMAGE_]UNITS to avoid calling
     glGetInteger too often */
  GLint             max_texture_units;
  GLint             max_activateable_texture_units;

  GLuint current_gl_program;

  bool current_gl_dither_enabled;
  CoglColorMask current_gl_color_mask;

  /* Clipping */
  /* true if we have a valid clipping stack flushed. In that case
     current_clip_stack will describe what the current state is. If
     this is false then the current clip stack is completely unknown
     so it will need to be reflushed. In that case current_clip_stack
     doesn't need to be a valid pointer. We can't just use NULL in
     current_clip_stack to mark a dirty state because NULL is a valid
     stack (meaning no clipping) */
  bool          current_clip_stack_valid;
  /* The clip state that was flushed. This isn't intended to be used
     as a stack to push and pop new entries. Instead the current stack
     that the user wants is part of the framebuffer state. This is
     just used to record the flush state so we can avoid flushing the
     same state multiple times. When the clip state is flushed this
     will hold a reference */
  CoglClipStack    *current_clip_stack;

  /* This is used as a temporary buffer to fill a CoglBuffer when
     cogl_buffer_map fails and we only want to map to fill it with new
     data */
  c_byte_array_t       *buffer_map_fallback_array;
  bool          buffer_map_fallback_in_use;
  size_t            buffer_map_fallback_offset;

  CoglWinsysRectangleState rectangle_state;

  CoglSamplerCache *sampler_cache;

  /* FIXME: remove these when we remove the last xlib based clutter
   * backend. they should be tracked as part of the renderer but e.g.
   * the eglx backend doesn't yet have a corresponding Cogl winsys
   * and so we wont have a renderer in that case. */
#ifdef COGL_HAS_XLIB_SUPPORT
  int damage_base;
  /* List of callback functions that will be given every Xlib event */
  c_slist_t *event_filters;
  /* Current top of the XError trap state stack. The actual memory for
     these is expected to be allocated on the stack by the caller */
  CoglXlibTrapState *trap_state;
#endif

  unsigned long winsys_features
    [COGL_FLAGS_N_LONGS_FOR_SIZE (COGL_WINSYS_FEATURE_N_FEATURES)];
  void *winsys;

  /* Array of names of uniforms. These are used like quarks to give a
     unique number to each uniform name except that we ensure that
     they increase sequentially so that we can use the id as an index
     into a bitfield representing the uniforms that a pipeline
     overrides from its parent. */
  c_ptr_array_t *uniform_names;
  /* A hash table to quickly get an index given an existing name. The
     name strings are owned by the uniform_names array. The values are
     the uniform location cast to a pointer. */
  c_hash_table_t *uniform_name_hash;
  int n_uniform_names;

  CoglPollSource *fences_poll_source;
  CoglList fences;

  /* This defines a list of function pointers that Cogl uses from
     either GL or GLES. All functions are accessed indirectly through
     these pointers rather than linking to them directly */
#ifndef APIENTRY
#define APIENTRY
#endif

#define COGL_EXT_BEGIN(name, \
                       min_gl_major, min_gl_minor, \
                       gles_availability, \
                       extension_suffixes, extension_names)
#define COGL_EXT_FUNCTION(ret, name, args) \
  ret (APIENTRY * name) args;
#define COGL_EXT_END()

#include "gl-prototypes/cogl-all-functions.h"

#undef COGL_EXT_BEGIN
#undef COGL_EXT_FUNCTION
#undef COGL_EXT_END
};

CoglContext *
_cogl_context_get_default ();

const CoglWinsysVtable *
_cogl_context_get_winsys (CoglContext *context);

/* Query the GL extensions and lookup the corresponding function
 * pointers. Theoretically the list of extensions can change for
 * different GL contexts so it is the winsys backend's responsiblity
 * to know when to re-query the GL extensions. The backend should also
 * check whether the GL context is supported by Cogl. If not it should
 * return false and set @error */
bool
_cogl_context_update_features (CoglContext *context,
                               CoglError **error);

/* Obtains the context and returns retval if NULL */
#define _COGL_GET_CONTEXT(ctxvar, retval) \
CoglContext *ctxvar = _cogl_context_get_default (); \
if (ctxvar == NULL) return retval;

#define NO_RETVAL

void
_cogl_context_set_current_projection_entry (CoglContext *context,
                                            CoglMatrixEntry *entry);

void
_cogl_context_set_current_modelview_entry (CoglContext *context,
                                           CoglMatrixEntry *entry);

/*
 * _cogl_context_get_gl_extensions:
 * @context: A CoglContext
 *
 * Return value: a NULL-terminated array of strings representing the
 *   supported extensions by the current driver. This array is owned
 *   by the caller and should be freed with c_strfreev().
 */
char **
_cogl_context_get_gl_extensions (CoglContext *context);

const char *
_cogl_context_get_gl_version (CoglContext *context);

CoglAtlasSet *
_cogl_get_atlas_set (CoglContext *context);

#endif /* __COGL_CONTEXT_PRIVATE_H */
