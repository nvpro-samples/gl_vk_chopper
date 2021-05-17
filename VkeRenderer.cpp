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

#include "VkeRenderer.h"


VkeRenderer::VkeRenderer()
{
  m_samples = VK_SAMPLE_COUNT_8_BIT;
  m_width   = 1024;
  m_height  = 768;
}


VkeRenderer::~VkeRenderer() {}

void VkeRenderer::initLayouts()
{
  initDescriptorLayout();
  initRenderPass();
  initPipeline();
}

void VkeRenderer::initDescriptors()
{
  initDescriptorPool();
  initDescriptorSets();
}


void VkeRenderer::initDescriptorPool()
{
  m_descriptor_pool_size = uint32_t(getRequiredDescriptorCount());
}

void VkeRenderer::releaseDescriptorPool() {}

VkDescriptorPool& VkeRenderer::getDescriptorPool()
{
  return m_descriptor_pool;
}

void VkeRenderer::resize(uint32_t inWidth, uint32_t inHeight)
{
  initFramebuffer(inWidth, inHeight);
  initDescriptors();

  m_is_first_frame = true;
}