/*
 * Cogl
 *
 * A Low-Level GPU Graphics and Utilities API
 *
 * Copyright (C) 2012 Intel Corporation.
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

#ifndef __CG_SDL_H__
#define __CG_SDL_H__

/* NB: this is a top-level header that can be included directly but we
 * want to be careful not to define __CG_H_INSIDE__ when this is
 * included internally while building Cogl itself since
 * __CG_H_INSIDE__ is used in headers to guard public vs private api
 * definitions
 */
#ifndef CG_COMPILATION

/* Note: When building Cogl .gir we explicitly define
 * __CG_H_INSIDE__ */
#ifndef __CG_H_INSIDE__
#define __CG_H_INSIDE__
#define __CG_SDL_H_MUST_UNDEF_CG_H_INSIDE__
#endif

#endif /* CG_COMPILATION */

#include <cogl/cogl-device.h>
#include <cogl/cogl-onscreen.h>
#include <SDL.h>

#ifdef _MSC_VER
/* We need to link to SDL.lib/SDLmain.lib
 * if we are using Cogl
 * that uses the SDL winsys
 */
#pragma comment(lib, "SDL.lib")
#pragma comment(lib, "SDLmain.lib")
#endif

CG_BEGIN_DECLS

/**
 * SECTION:cogl-sdl
 * @short_description: Integration api for the Simple DirectMedia
 *                     Layer library.
 *
 * Cogl is a portable graphics api that can either be used standalone
 * or alternatively integrated with certain existing frameworks. This
 * api enables Cogl to be used in conjunction with the Simple
 * DirectMedia Layer library.
 *
 * Using this API a typical SDL application would look something like
 * this:
 * |[
 * MyAppData data;
 * cg_error_t *error = NULL;
 *
 * data.ctx = cg_sdl_context_new (SDL_USEREVENT, &error);
 * if (!data.ctx)
 *   {
 *     fprintf (stderr, "Failed to create context: %s\n",
 *              error->message);
 *     return 1;
 *   }
 *
 * my_application_setup (&data);
 *
 * data.redraw_queued = true;
 * while (!data.quit)
 *   {
 *     while (!data.quit)
 *       {
 *         if (!SDL_PollEvent (&event))
 *           {
 *             if (data.redraw_queued)
 *               break;
 *
 *             cg_sdl_idle (ctx);
 *             if (!SDL_WaitEvent (&event))
 *               {
 *                 fprintf (stderr, "Error waiting for SDL events");
 *                 return 1;
 *               }
 *           }
 *
 *          handle_event (&data, &event);
 *          cg_sdl_handle_event (ctx, &event);
 *        }
 *
 *     data.redraw_queued = redraw (&data);
 *   }
 * ]|
 */

/**
 * cg_sdl_context_new:
 * @type: An SDL user event type between <constant>SDL_USEREVENT</constant> and
 *        <constant>SDL_NUMEVENTS</constant> - 1
 * @error: A cg_error_t return location.
 *
 * This is a convenience function for creating a new #cg_device_t for
 * use with SDL and specifying what SDL user event type Cogl can use
 * as a way to interrupt SDL_WaitEvent().
 *
 * This function is equivalent to the following code:
 * |[
 * cg_renderer_t *renderer = cg_renderer_new ();
 * cg_display_t *display;
 *
 * cg_renderer_set_winsys_id (renderer, CG_WINSYS_ID_SDL);
 *
 * cg_sdl_renderer_set_event_type (renderer, type);
 *
 * if (!cg_renderer_connect (renderer, error))
 *   return NULL;
 *
 * display = cg_display_new (renderer, NULL);
 * if (!cg_display_setup (display, error))
 *   return NULL;
 *
 * return cg_device_new (display, error);
 * ]|
 *
 * <note>SDL applications are required to either use this API or
 * to manually create a #cg_renderer_t and call
 * cg_sdl_renderer_set_event_type().</note>
 *
 * Stability: unstable
 */
cg_device_t *cg_sdl_context_new(int type, cg_error_t **error);

/**
 * cg_sdl_renderer_set_event_type:
 * @renderer: A #cg_renderer_t
 * @type: An SDL user event type between <constant>SDL_USEREVENT</constant> and
 *        <constant>SDL_NUMEVENTS</constant> - 1
 *
 * Tells Cogl what SDL user event type it can use as a way to
 * interrupt SDL_WaitEvent() to ensure that cg_sdl_handle_event()
 * will be called in a finite amount of time.
 *
 * <note>This should only be called on an un-connected
 * @renderer.</note>
 *
 * <note>For convenience most simple applications can use
 * cg_sdl_context_new() if they don't want to manually create
 * #cg_renderer_t and #cg_display_t objects during
 * initialization.</note>
 *
 * Stability: unstable
 */
void cg_sdl_renderer_set_event_type(cg_renderer_t *renderer, int type);

/**
 * cg_sdl_renderer_get_event_type:
 * @renderer: A #cg_renderer_t
 *
 * Queries what SDL user event type Cogl is using as a way to
 * interrupt SDL_WaitEvent(). This is set either using
 * cg_sdl_context_new or by using
 * cg_sdl_renderer_set_event_type().
 *
 * Stability: unstable
 */
int cg_sdl_renderer_get_event_type(cg_renderer_t *renderer);

/**
 * cg_sdl_handle_event:
 * @dev: A #cg_device_t
 * @event: An SDL event
 *
 * Passes control to Cogl so that it may dispatch any internal event
 * callbacks in response to the given SDL @event. This function must
 * be called for every SDL event.
 *
 * Stability: unstable
 */
void cg_sdl_handle_event(cg_device_t *dev, SDL_Event *event);

/**
 * cg_sdl_idle:
 * @dev: A #cg_device_t
 *
 * Notifies Cogl that the application is idle and about to call
 * SDL_WaitEvent(). Cogl may use this to run low priority book keeping
 * tasks.
 *
 * Stability: unstable
 */
void cg_sdl_idle(cg_device_t *dev);

#if SDL_MAJOR_VERSION >= 2

/**
 * cg_sdl_onscreen_get_window:
 * @onscreen: A #cg_onscreen_t
 *
 * Returns: the underlying SDL_Window associated with an onscreen framebuffer.
 *
 * Stability: unstable
 */
SDL_Window *cg_sdl_onscreen_get_window(cg_onscreen_t *onscreen);

#endif /* SDL_MAJOR_VERSION */

CG_END_DECLS

/* The gobject introspection scanner seems to parse public headers in
 * isolation which means we need to be extra careful about how we
 * define and undefine __CG_H_INSIDE__ used to detect when internal
 * headers are incorrectly included by developers. In the gobject
 * introspection case we have to manually define __CG_H_INSIDE__ as
 * a commandline argument for the scanner which means we must be
 * careful not to undefine it in a header...
 */
#ifdef __CG_SDL_H_MUST_UNDEF_CG_H_INSIDE__
#undef __CG_H_INSIDE__
#undef __CG_SDL_H_MUST_UNDEF_CG_H_INSIDE__
#endif

#endif /* __CG_SDL_H__ */
