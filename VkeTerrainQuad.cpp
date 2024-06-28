/*
 * Copyright (c) 2014-2024, NVIDIA CORPORATION.  All rights reserved.
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
 * SPDX-FileCopyrightText: Copyright (c) 2014-2024 NVIDIA CORPORATION
 * SPDX-License-Identifier: Apache-2.0
 */

/* Contact chebert@nvidia.com (Chris Hebert) for feedback */

#include "VkeTerrainQuad.h"


VkeTerrainQuad::VkeTerrainQuad()
    : VkeBuffer()
{
}


VkeTerrainQuad::~VkeTerrainQuad() {}

void VkeTerrainQuad::initQuadData()
{
  m_usage_flags  = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  m_memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
  initBackingStore(sizeof(TerrainUniform));

  m_backing_store->view_matrix = glm::mat4(1);

  initVKBufferData();

  // Each of these are a VertexObjectUV.
  float quadVerts[] = {-1.0, 0.0, -1.0, 1.0, 0.0, 0.0,  //
                       1.0,  0.0, -1.0, 1.0, 1.0, 0.0,  //
                       1.0,  0.0, 1.0,  1.0, 1.0, 1.0,  //
                       -1.0, 0.0, 1.0,  1.0, 0.0, 1.0};

  uint32_t quadIdxs[] = {0, 1, 3, 2};

  size_t dataSize = 4 * sizeof(VertexObjectUV);

  m_vbo.initBackingStore(dataSize);

  float* vData = m_vbo.getBackingStore();
  memcpy(vData, (const void*)quadVerts, dataSize);


  dataSize = sizeof(uint32_t) * 4;
  m_ibo.initBackingStore(dataSize);
  uint32_t* iData = m_ibo.getBackingStore();
  memcpy(iData, (const void*)quadIdxs, dataSize);

  m_vbo.initVKBufferData();
  m_ibo.initVKBufferData();
}

void VkeTerrainQuad::draw(VkCommandBuffer* inCommand)
{
  vkCmdDrawIndexed(*inCommand, 4, 256, 0, 0, 0);
}

void VkeTerrainQuad::bind(VkCommandBuffer* inCmd)
{
  m_vbo.bind(inCmd);
  m_ibo.bind(inCmd);
}
