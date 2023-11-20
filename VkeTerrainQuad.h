/*
 * Copyright (c) 2014-2021, NVIDIA CORPORATION.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-FileCopyrightText: Copyright (c) 2014-2021 NVIDIA CORPORATION
 * SPDX-License-Identifier: Apache-2.0
 */

/* Contact chebert@nvidia.com (Chris Hebert) for feedback */

#ifndef __H_VKE_TERRAIN_QUAD_
#define __H_VKE_TERRAIN_QUAD_


#pragma once

#include "VkeBuffer.h"
#include "VkeIBO.h"
#include "VkeVBO.h"
#include <glm/glm.hpp>

typedef struct _TerrainUniform
{
  glm::mat4 view_matrix;
} TerrainUniform;

class VkeTerrainQuad : public VkeBuffer<TerrainUniform>
{
public:
  VkeTerrainQuad();
  ~VkeTerrainQuad();

  void bind(VkCommandBuffer* inBuffer);
  void initQuadData();
  void draw(VkCommandBuffer* inBuffer);

  VkeVBO m_vbo;
  VkeIBO m_ibo;
};


#endif
