/*
 * Cogl
 *
 * A Low-Level GPU Graphics and Utilities API
 *
 * Copyright (C) 2011,2013 Intel Corporation.
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
 * Authors:
 *   Neil Roberts <neil@linux.intel.com>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <SDL.h>

#include "cogl-renderer-private.h"
#include "cogl-display-private.h"
#include "cogl-framebuffer-private.h"
#include "cogl-onscreen-template-private.h"
#include "cogl-context-private.h"
#include "cogl-onscreen-private.h"
#include "cogl-winsys-sdl-private.h"
#include "cogl-error-private.h"
#include "cogl-poll-private.h"

typedef struct _CoglRendererSdl
{
  CoglClosure *resize_notify_idle;
} CoglRendererSdl;

typedef struct _CoglDisplaySdl
{
  SDL_Surface *surface;
  CoglOnscreen *onscreen;
  Uint32 video_mode_flags;
} CoglDisplaySdl;

static CoglFuncPtr
_cogl_winsys_renderer_get_proc_address (CoglRenderer *renderer,
                                        const char *name,
                                        CoglBool in_core)
{
  CoglFuncPtr ptr;

  /* XXX: It's not totally clear whether it's safe to call this for
   * core functions. From the code it looks like the implementations
   * will fall back to using some form of dlsym if the winsys
   * GetProcAddress function returns NULL. Presumably this will work
   * in most cases apart from EGL platforms that return invalid
   * pointers for core functions. It's awkward for this code to get a
   * handle to the GL module that SDL has chosen to load so just
   * calling SDL_GL_GetProcAddress is probably the best we can do
   * here. */

#ifdef COGL_HAS_SDL_GLES_SUPPORT
  if (renderer->driver != COGL_DRIVER_GL)
    return SDL_GLES_GetProcAddress (name);
#endif

  return SDL_GL_GetProcAddress (name);
}

static void
_cogl_winsys_renderer_disconnect (CoglRenderer *renderer)
{
  SDL_Quit ();

  c_slice_free (CoglRendererSdl, renderer->winsys);
}

static CoglBool
_cogl_winsys_renderer_connect (CoglRenderer *renderer,
                               CoglError **error)
{
#ifdef EMSCRIPTEN
  if (renderer->driver != COGL_DRIVER_GLES2)
    {
      _cogl_set_error (error, COGL_WINSYS_ERROR,
                       COGL_WINSYS_ERROR_INIT,
                       "The SDL winsys with emscripten only supports "
                       "the GLES2 driver");
      return FALSE;
    }
#elif !defined (COGL_HAS_SDL_GLES_SUPPORT)
  if (renderer->driver != COGL_DRIVER_GL)
    {
      _cogl_set_error (error, COGL_WINSYS_ERROR,
                   COGL_WINSYS_ERROR_INIT,
                   "The SDL winsys only supports the GL driver");
      return FALSE;
    }
#endif

  if (SDL_Init (SDL_INIT_VIDEO) == -1)
    {
      _cogl_set_error (error, COGL_WINSYS_ERROR,
                   COGL_WINSYS_ERROR_INIT,
                   "SDL_Init failed: %s",
                   SDL_GetError ());
      return FALSE;
    }

  renderer->winsys = c_slice_new0 (CoglRendererSdl);

  return TRUE;
}

static void
_cogl_winsys_display_destroy (CoglDisplay *display)
{
  CoglDisplaySdl *sdl_display = display->winsys;

  _COGL_RETURN_IF_FAIL (sdl_display != NULL);

  /* No need to destroy the surface - it is freed by SDL_Quit */

  c_slice_free (CoglDisplaySdl, display->winsys);
  display->winsys = NULL;
}

static void
set_gl_attribs_from_framebuffer_config (CoglFramebufferConfig *config)
{
  SDL_GL_SetAttribute (SDL_GL_RED_SIZE, 1);
  SDL_GL_SetAttribute (SDL_GL_GREEN_SIZE, 1);
  SDL_GL_SetAttribute (SDL_GL_BLUE_SIZE, 1);
  SDL_GL_SetAttribute (SDL_GL_DEPTH_SIZE, 1);

  SDL_GL_SetAttribute (SDL_GL_STENCIL_SIZE,
                       config->need_stencil ? 1 : 0);

  SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);

  SDL_GL_SetAttribute (SDL_GL_ALPHA_SIZE,
                       config->has_alpha ? 1 : 0);
}

static CoglBool
_cogl_winsys_display_setup (CoglDisplay *display,
                            CoglError **error)
{
  CoglDisplaySdl *sdl_display;

  _COGL_RETURN_VAL_IF_FAIL (display->winsys == NULL, FALSE);

  sdl_display = c_slice_new0 (CoglDisplaySdl);
  display->winsys = sdl_display;

  set_gl_attribs_from_framebuffer_config (&display->onscreen_template->config);

  switch (display->renderer->driver)
    {
    case COGL_DRIVER_GL:
      sdl_display->video_mode_flags = SDL_OPENGL;
      break;

    case COGL_DRIVER_GL3:
      _cogl_set_error (error, COGL_WINSYS_ERROR,
                       COGL_WINSYS_ERROR_INIT,
                       "The SDL winsys does not support GL 3");
      goto error;

#ifdef COGL_HAS_SDL_GLES_SUPPORT
    case COGL_DRIVER_GLES2:
      sdl_display->video_mode_flags = SDL_OPENGLES;
      SDL_GL_SetAttribute (SDL_GL_CONTEXT_MAJOR_VERSION, 2);
      SDL_GL_SetAttribute (SDL_GL_CONTEXT_MINOR_VERSION, 0);
      break;

#elif defined (EMSCRIPTEN)
    case COGL_DRIVER_GLES2:
      sdl_display->video_mode_flags = SDL_OPENGL;
      break;
#endif

    default:
      c_assert_not_reached ();
    }

  /* There's no way to know what size the application will need until
     it creates the first onscreen but we need to set the video mode
     now so that we can get a GL context. We'll have to just guess at
     a size an resize it later */
  sdl_display->surface = SDL_SetVideoMode (640, 480, /* width/height */
                                           0, /* bitsperpixel */
                                           sdl_display->video_mode_flags);

  if (sdl_display->surface == NULL)
    {
      _cogl_set_error (error, COGL_WINSYS_ERROR,
                   COGL_WINSYS_ERROR_INIT,
                   "SDL_SetVideoMode failed: %s",
                   SDL_GetError ());
      goto error;
    }

  return TRUE;

error:
  _cogl_winsys_display_destroy (display);
  return FALSE;
}

static void
flush_pending_resize_notification_idle (void *user_data)
{
  CoglContext *context = user_data;
  CoglRenderer *renderer = context->display->renderer;
  CoglRendererSdl *sdl_renderer = renderer->winsys;
  CoglDisplaySdl *sdl_display = context->display->winsys;
  CoglOnscreen *onscreen = sdl_display->onscreen;

  /* This needs to be disconnected before invoking the callbacks in
   * case the callbacks cause it to be queued again */
  _cogl_closure_disconnect (sdl_renderer->resize_notify_idle);
  sdl_renderer->resize_notify_idle = NULL;

  _cogl_onscreen_notify_resize (onscreen);
}

static CoglFilterReturn
sdl_event_filter_cb (SDL_Event *event, void *data)
{
  CoglContext *context = data;
  CoglDisplay *display = context->display;
  CoglDisplaySdl *sdl_display = display->winsys;
  CoglFramebuffer *framebuffer;

  if (!sdl_display->onscreen)
    return COGL_FILTER_CONTINUE;

  framebuffer = COGL_FRAMEBUFFER (sdl_display->onscreen);

  if (event->type == SDL_VIDEORESIZE)
    {
      CoglRenderer *renderer = display->renderer;
      CoglRendererSdl *sdl_renderer = renderer->winsys;
      float width = event->resize.w;
      float height = event->resize.h;

      sdl_display->surface = SDL_SetVideoMode (width, height,
                                               0, /* bitsperpixel */
                                               sdl_display->video_mode_flags);

      _cogl_framebuffer_winsys_update_size (framebuffer, width, height);

      /* We only want to notify that a resize happened when the
       * application calls cogl_context_dispatch so instead of
       * immediately notifying we queue an idle callback */
      if (!sdl_renderer->resize_notify_idle)
        {
          sdl_renderer->resize_notify_idle =
            _cogl_poll_renderer_add_idle (renderer,
                                          flush_pending_resize_notification_idle,
                                          context,
                                          NULL);
        }

      return COGL_FILTER_CONTINUE;
    }
  else if (event->type == SDL_VIDEOEXPOSE)
    {
      CoglOnscreenDirtyInfo info;

      /* Sadly SDL doesn't seem to report the rectangle of the expose
       * event so we'll just queue the whole window */
      info.x = 0;
      info.y = 0;
      info.width = framebuffer->width;
      info.height = framebuffer->height;

      _cogl_onscreen_queue_dirty (COGL_ONSCREEN (framebuffer), &info);
    }

  return COGL_FILTER_CONTINUE;
}

static CoglBool
_cogl_winsys_context_init (CoglContext *context, CoglError **error)
{
  CoglRenderer *renderer = context->display->renderer;

  if (C_UNLIKELY (renderer->sdl_event_type_set == FALSE))
    c_error ("cogl_sdl_renderer_set_event_type() or cogl_sdl_context_new() "
             "must be called during initialization");

  _cogl_renderer_add_native_filter (renderer,
                                    (CoglNativeFilterFunc)sdl_event_filter_cb,
                                    context);

  /* We'll manually handle queueing dirty events in response to
   * SDL_VIDEOEXPOSE events */
  COGL_FLAGS_SET (context->private_features,
                  COGL_PRIVATE_FEATURE_DIRTY_EVENTS,
                  TRUE);

  return _cogl_context_update_features (context, error);
}

static void
_cogl_winsys_context_deinit (CoglContext *context)
{
}

static void
_cogl_winsys_onscreen_bind (CoglOnscreen *onscreen)
{
}

static void
_cogl_winsys_onscreen_deinit (CoglOnscreen *onscreen)
{
  CoglContext *context = COGL_FRAMEBUFFER (onscreen)->context;
  CoglDisplay *display = context->display;
  CoglDisplaySdl *sdl_display = display->winsys;

  sdl_display->onscreen = NULL;
}

static CoglBool
_cogl_winsys_onscreen_init (CoglOnscreen *onscreen,
                            CoglError **error)
{
  CoglFramebuffer *framebuffer = COGL_FRAMEBUFFER (onscreen);
  CoglContext *context = framebuffer->context;
  CoglDisplay *display = context->display;
  CoglDisplaySdl *sdl_display = display->winsys;
  CoglBool flags_changed = FALSE;
  int width, height;

  if (sdl_display->onscreen)
    {
      _cogl_set_error (error, COGL_WINSYS_ERROR,
                   COGL_WINSYS_ERROR_CREATE_ONSCREEN,
                   "SDL winsys only supports a single onscreen window");
      return FALSE;
    }

  width = cogl_framebuffer_get_width (framebuffer);
  height = cogl_framebuffer_get_height (framebuffer);

  if (cogl_onscreen_get_resizable (onscreen))
    {
      sdl_display->video_mode_flags |= SDL_RESIZABLE;
      flags_changed = TRUE;
    }

  /* Try to update the video size using the onscreen size */
  if (width != sdl_display->surface->w ||
      height != sdl_display->surface->h ||
      flags_changed)
    {
      sdl_display->surface = SDL_SetVideoMode (width, height,
                                               0, /* bitsperpixel */
                                               sdl_display->video_mode_flags);

      if (sdl_display->surface == NULL)
        {
          _cogl_set_error (error, COGL_WINSYS_ERROR,
                       COGL_WINSYS_ERROR_CREATE_ONSCREEN,
                       "SDL_SetVideoMode failed: %s",
                       SDL_GetError ());
          return FALSE;
        }
    }

  _cogl_framebuffer_winsys_update_size (framebuffer,
                                        sdl_display->surface->w,
                                        sdl_display->surface->h);

  sdl_display->onscreen = onscreen;

  return TRUE;
}

static void
_cogl_winsys_onscreen_swap_buffers_with_damage (CoglOnscreen *onscreen,
                                                const int *rectangles,
                                                int n_rectangles)
{
  SDL_GL_SwapBuffers ();
}

static void
_cogl_winsys_onscreen_update_swap_throttled (CoglOnscreen *onscreen)
{
  /* SDL doesn't appear to provide a way to set this */
}

static void
_cogl_winsys_onscreen_set_visibility (CoglOnscreen *onscreen,
                                      CoglBool visibility)
{
  /* SDL doesn't appear to provide a way to set this */
}

static void
_cogl_winsys_onscreen_set_resizable (CoglOnscreen *onscreen,
                                     CoglBool resizable)
{
  CoglFramebuffer *framebuffer = COGL_FRAMEBUFFER (onscreen);
  CoglContext *context = framebuffer->context;
  CoglDisplay *display = context->display;
  CoglDisplaySdl *sdl_display = display->winsys;
  int width, height;

  width = cogl_framebuffer_get_width (framebuffer);
  height = cogl_framebuffer_get_height (framebuffer);

  if (resizable)
    sdl_display->video_mode_flags |= SDL_RESIZABLE;
  else
    sdl_display->video_mode_flags &= ~SDL_RESIZABLE;

  sdl_display->surface = SDL_SetVideoMode (width, height,
                                           0, /* bitsperpixel */
                                           sdl_display->video_mode_flags);
}

const CoglWinsysVtable *
_cogl_winsys_sdl_get_vtable (void)
{
  static CoglBool vtable_inited = FALSE;
  static CoglWinsysVtable vtable;

  /* It would be nice if we could use C99 struct initializers here
     like the GLX backend does. However this code is more likely to be
     compiled using Visual Studio which (still!) doesn't support them
     so we initialize it in code instead */

  if (!vtable_inited)
    {
      memset (&vtable, 0, sizeof (vtable));

      vtable.id = COGL_WINSYS_ID_SDL;
      vtable.name = "SDL";
      vtable.renderer_get_proc_address = _cogl_winsys_renderer_get_proc_address;
      vtable.renderer_connect = _cogl_winsys_renderer_connect;
      vtable.renderer_disconnect = _cogl_winsys_renderer_disconnect;
      vtable.display_setup = _cogl_winsys_display_setup;
      vtable.display_destroy = _cogl_winsys_display_destroy;
      vtable.context_init = _cogl_winsys_context_init;
      vtable.context_deinit = _cogl_winsys_context_deinit;
      vtable.onscreen_init = _cogl_winsys_onscreen_init;
      vtable.onscreen_deinit = _cogl_winsys_onscreen_deinit;
      vtable.onscreen_bind = _cogl_winsys_onscreen_bind;
      vtable.onscreen_swap_buffers_with_damage =
        _cogl_winsys_onscreen_swap_buffers_with_damage;
      vtable.onscreen_update_swap_throttled =
        _cogl_winsys_onscreen_update_swap_throttled;
      vtable.onscreen_set_visibility = _cogl_winsys_onscreen_set_visibility;
      vtable.onscreen_set_resizable = _cogl_winsys_onscreen_set_resizable;

      vtable_inited = TRUE;
    }

  return &vtable;
}
