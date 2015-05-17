/*
 * Cogl
 *
 * A Low-Level GPU Graphics and Utilities API
 *
 * Copyright (C) 2008,2009 Intel Corporation.
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

#ifndef __CG_H__
#define __CG_H__

#ifndef CG_COMPILATION
#define __CG_H_INSIDE__
#endif

#include <cogl/cogl-defines.h>
#include <cogl/cogl-error.h>

#include <cogl/cogl-object.h>
#include <cogl/cogl-bitmap.h>
#include <cogl/cogl-color.h>
#include <cogl/cogl-matrix.h>
#include <cogl/cogl-matrix-stack.h>
#include <cogl/cogl-offscreen.h>
#include <cogl/cogl-texture.h>
#include <cogl/cogl-types.h>
#include <cogl/cogl-version.h>

#ifdef CG_HAS_GTYPE_SUPPORT
/* GType integration */
#include <cogl/cogl-enum-types.h>
#endif

#include <cogl/cogl-renderer.h>
#include <cogl/cogl-output.h>
#include <cogl/cogl-display.h>
#include <cogl/cogl-device.h>
#include <cogl/cogl-buffer.h>
#include <cogl/cogl-pixel-buffer.h>
#include <cogl/cogl-vector.h>
#include <cogl/cogl-euler.h>
#include <cogl/cogl-quaternion.h>
#include <cogl/cogl-texture-2d.h>
#include <cogl/cogl-texture-2d-gl.h>
#include <cogl/cogl-texture-3d.h>
#include <cogl/cogl-texture-2d-sliced.h>
#include <cogl/cogl-sub-texture.h>
#include <cogl/cogl-atlas-set.h>
#include <cogl/cogl-atlas.h>
#include <cogl/cogl-atlas-texture.h>
#include <cogl/cogl-meta-texture.h>
#include <cogl/cogl-primitive-texture.h>
#include <cogl/cogl-index-buffer.h>
#include <cogl/cogl-attribute-buffer.h>
#include <cogl/cogl-indices.h>
#include <cogl/cogl-attribute.h>
#include <cogl/cogl-primitive.h>
#include <cogl/cogl-depth-state.h>
#include <cogl/cogl-pipeline.h>
#include <cogl/cogl-pipeline-state.h>
#include <cogl/cogl-pipeline-layer-state.h>
#include <cogl/cogl-snippet.h>
#include <cogl/cogl-framebuffer.h>
#include <cogl/cogl-onscreen.h>
#include <cogl/cogl-frame-info.h>
#include <cogl/cogl-loop.h>
#include <cogl/cogl-fence.h>
#if defined(CG_HAS_EGL_PLATFORM_KMS_SUPPORT)
#include <cogl/cogl-kms-renderer.h>
#include <cogl/cogl-kms-display.h>
#endif
#ifdef CG_HAS_WIN32_SUPPORT
#include <cogl/cogl-win32-renderer.h>
#endif
#ifdef CG_HAS_GLIB_SUPPORT
#include <cogl/cogl-glib-source.h>
#endif
#ifdef CG_HAS_UV_SUPPORT
#include <cogl/cogl-uv.h>
#endif

#ifndef CG_COMPILATION
#undef __CG_H_INSIDE__
#endif

#endif /* __CG_H__ */
