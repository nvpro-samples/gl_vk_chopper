/*
 * Copyright (c) 2014-2023, NVIDIA CORPORATION.  All rights reserved.
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

#ifndef __H_VKE_RENDERER_
#define __H_VKE_RENDERER_


#include "VkeMesh.h"
#include "VkeNodeData.h"
#include <map>
#include <nvvk/shadermodulemanager_vk.hpp>
#include <vulkan/vulkan.h>

#pragma once
class VkeRenderer
{
public:
  VkeRenderer();
  ~VkeRenderer();


  VkDescriptorPool& getDescriptorPool();

  virtual void initDescriptors();
  virtual void initLayouts();
  virtual void initDescriptorPool();
  void         releaseDescriptorPool();

  virtual void update() = 0;

  virtual void resize(uint32_t inWidth, uint32_t inHeight);

  inline VkSampleCountFlagBits getSamples() { return m_samples; }


  VkFramebuffer getFramebuffer(uint32_t inIdx) { return m_framebuffers[inIdx]; }
  VkRenderPass  getRenderPass() { return m_render_pass; }


  virtual void initDescriptorLayout() = 0;
  virtual void initDescriptorSets()   = 0;
  virtual void initPipeline()         = 0;
  virtual void initRenderPass()       = 0;

  virtual void initFramebuffer(uint32_t inWidth, uint32_t inHeight) = 0;
  virtual void releaseFramebuffer()                                 = 0;

  virtual void present()                                                     = 0;
  virtual void initShaders(nvvk::ShaderModuleManager& inShaderModuleManager) = 0;
  virtual void setCameraLookAt(nvmath::mat4f& inMat)                         = 0;

  VkPipeline getPipeline() { return m_pipeline; }

  VkPipelineLayout getPipelineLayout() { return m_pipeline_layout; }


protected:
  bool m_is_first_frame = true;

  VkDescriptorPool m_descriptor_pool      = VK_NULL_HANDLE;
  uint32_t         m_descriptor_pool_size = 0;

  VkSampleCountFlagBits m_samples = VK_SAMPLE_COUNT_8_BIT;

  VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;
  VkPipeline       m_pipeline        = VK_NULL_HANDLE;
  VkPipelineCache  m_pipeline_cache  = VK_NULL_HANDLE;
  VkRenderPass     m_render_pass     = VK_NULL_HANDLE;

  uint32_t      m_descriptor_set_count = 0;
  VkFramebuffer m_framebuffers[2]{};

  virtual size_t getRequiredDescriptorCount() = 0;

  uint32_t m_width  = 1024;
  uint32_t m_height = 768;
};


#endif
