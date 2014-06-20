/*
 * Cogl
 *
 * A Low-Level GPU Graphics and Utilities API
 *
 * Copyright (C) 2012, 2013 Intel Corporation.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "cogl-sdl.h"
#include "cogl-context-private.h"
#include "cogl-renderer-private.h"

void
cogl_sdl_renderer_set_event_type (CoglRenderer *renderer, int type)
{
  renderer->sdl_event_type_set = TRUE;
  renderer->sdl_event_type = type;
}

int
cogl_sdl_renderer_get_event_type (CoglRenderer *renderer)
{
  _COGL_RETURN_VAL_IF_FAIL (renderer->sdl_event_type_set, SDL_USEREVENT);

  return renderer->sdl_event_type;
}

CoglContext *
cogl_sdl_context_new (int type, CoglError **error)
{
  CoglRenderer *renderer = cogl_renderer_new ();
  CoglDisplay *display;

  cogl_renderer_set_winsys_id (renderer, COGL_WINSYS_ID_SDL);

  cogl_sdl_renderer_set_event_type (renderer, type);

  if (!cogl_renderer_connect (renderer, error))
    return NULL;

  display = cogl_display_new (renderer, NULL);
  if (!cogl_display_setup (display, error))
    return NULL;

  return cogl_context_new (display, error);
}

void
cogl_sdl_handle_event (CoglContext *context, SDL_Event *event)
{
  CoglRenderer *renderer;

  _COGL_RETURN_IF_FAIL (cogl_is_context (context));

  renderer = context->display->renderer;

  _cogl_renderer_handle_native_event (renderer, event);
}

static void
_cogl_sdl_push_wakeup_event (CoglContext *context)
{
  SDL_Event wakeup_event;

  wakeup_event.type = context->display->renderer->sdl_event_type;

  SDL_PushEvent (&wakeup_event);
}

void
cogl_sdl_idle (CoglContext *context)
{
  CoglRenderer *renderer = context->display->renderer;

  cogl_poll_renderer_dispatch (renderer, NULL, 0);

  /* It is expected that this will be called from the application
   * immediately before blocking in SDL_WaitEvent. However,
   * dispatching cause more work to be queued. If that happens we need
   * to make sure the blocking returns immediately. We'll post our
   * dummy event to make sure that happens
   */
  if (!_cogl_list_empty (&renderer->idle_closures))
    _cogl_sdl_push_wakeup_event (context);
}
