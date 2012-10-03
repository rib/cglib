/*
 * Rut
 *
 * Copyright (C) 2012  Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __RUT_DIAMOND_H__
#define __RUT_DIAMOND_H__

#include "rut-entity.h"

typedef struct _RutDiamondSlice RutDiamondSlice;
#define RUT_DIAMOND_SLICE(X) ((RutDiamondSlice *)X)
extern RutType _rut_diamond_slice_type;

typedef struct _RutDiamondSlice
{
  RutObjectProps _parent;
  int ref_count;

  CoglMatrix rotate_matrix;

  CoglTexture *texture;

  float size;

  CoglPipeline *pipeline;
  CoglPrimitive *primitive;

} RutDiamondSlice;

void
_rut_diamond_slice_init_type (void);

CoglPipeline *
rut_diamond_slice_get_pipeline_template (RutDiamondSlice *slice);

typedef struct _RutDiamond RutDiamond;
#define RUT_DIAMOND(p) ((RutDiamond *)(p))
extern RutType rut_diamond_type;

struct _RutDiamond
{
  RutObjectProps _parent;
  RutComponentableProps component;

  RutContext *ctx;

  RutDiamondSlice *slice;

  CoglVertexP3 pick_vertices[6];

  int size;
};

void
_rut_diamond_init_type (void);

RutDiamond *
rut_diamond_new (RutContext *ctx,
                 float size,
                 int tex_width,
                 int tex_height);

RutDiamondSlice *
rut_diamond_get_slice (RutDiamond *diamond);

float
rut_diamond_get_size (RutDiamond *diamond);

CoglPrimitive *
rut_diamond_get_primitive (RutObject *object);

void
rut_diamond_apply_mask (RutDiamond *diamond,
                        CoglPipeline *pipeline);

void *
rut_diamond_get_vertex_data (RutDiamond *diamond,
                             size_t *stride,
                             int *n_vertices);

#endif /* __RUT_DIAMOND_H__ */