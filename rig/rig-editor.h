/*
 * Rig
 *
 * UI Engine & Editor
 *
 * Copyright (C) 2013  Intel Corporation
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
 */

#ifndef _RIG_EDITOR_H_
#define _RIG_EDITOR_H_

#include <rut.h>

#include "rig-types.h"

#include "rig.pb-c.h"

typedef struct _RigEditor RigEditor;

extern RutType rig_editor_type;

RigEditor *
rig_editor_new (const char *filename);

void
rig_editor_run (RigEditor *editor);

void
rig_editor_load_file (RigEditor *editor,
                      const char *filename);

/* FIXME: move necessary state to RigEditor and update this
 * api to take a RigEditor pointer */
void
rig_editor_create_ui (RigEngine *engine);

void
rig_editor_apply_last_op (RigEngine *engine);

/* XXX: This rather esoteric prototype is used as a 'read_callback' to
 * rig_asset_thumbnail and is called whenever an asset's thumnail has
 * been updated.
 *
 * It would probably be better to just have a
 * rig_editor_reload_thumbnails(RigEditor *editor) considering that
 * all this function does is trigger an asset search to refresh the
 * assets view.
 */
void
rig_editor_refresh_thumbnails (RigAsset *video, void *user_data);

/* FIXME: move necessary state to RigEditor and update this
 * api to take a RigEditor pointer */
void
rig_editor_clear_search_results (RigEngine *engine);

void
rig_editor_free_result_input_closures (RigEngine *engine);

#endif /* _RIG_EDITOR_H_ */