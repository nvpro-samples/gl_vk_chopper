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

#include "VkeVBO.h"


VkeVBO::VkeVBO()
    : VkeBuffer()
{
  m_use_staging = true;

  m_usage_flags = (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
  //m_memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;#

  m_memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
}

void VkeVBO::bind(VkCommandBuffer* inCmd)
{
  VkDeviceSize ofst = 0;
  vkCmdBindVertexBuffers(*inCmd, 0, 1, &m_data.buffer, &ofst);
}


VkeVBO::~VkeVBO() {}
