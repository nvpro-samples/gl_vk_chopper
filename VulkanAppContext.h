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

#ifndef __H_VULKAN_APP_CONTEXT_
#define __H_VULKAN_APP_CONTEXT_

#include <chrono>
#include <nvmath/nvmath.h>
#include <nvvk/shadermodulemanager_vk.hpp>
#include <vulkan/vulkan.h>

#include "RenderContext.h"
#include "VkeMaterial.h"
#include "VkeMesh.h"
#include "VkeNodeData.h"
#include "VkeSceneAnimation.h"
#include "vkaUtils.h"


#define TEXTURE_COUNT 2

class VkeMesh;
class VkeDeferredRenderer;
class VkeRenderer;


#pragma once
class VulkanAppContext
{
public:
  VulkanAppContext();
  ~VulkanAppContext();

  static VulkanAppContext* GetInstance();

  void initAppContext();

  void initRenderer();

  void loadVKSScene(std::string& inFileName);
  void addVKSNode(VKSFile* inFile, uint32_t& inNodesProcessed, Node* parentNode = NULL);

  void render();

  bool initPrograms();

  void         resize(uint32_t inWidth, uint32_t inHeight);
  VkeMaterial* getMaterial(VkeMaterial::ID inID);

  nvmath::vec4f getRotorPos()
  {
    nvmath::vec4f pos(0.0f, 0.0f, -1.0f, 1.0f);
    return m_rotor_node->getNode()->worldPosition(pos);
  }

  VkeVBO* getVBO() { return &m_global_vbo; }
  VkeIBO* getIBO() { return &m_global_ibo; }

  void setCameraMatrix(nvmath::mat4f& inMat);

  float getOpacity(uint32_t inMatID) { return m_materials.getMaterial(inMatID)->getBackingStore()->opacity; }

private:
  VkInstance m_vk_instance = nullptr;
#ifndef NDEBUG
  VkDebugUtilsMessengerEXT m_debug_messenger = VK_NULL_HANDLE;
#endif

  uint32_t m_width  = 1024;
  uint32_t m_height = 768;


  VkFormat          m_surface_format = VK_FORMAT_UNDEFINED;
  VkeRenderer*      m_renderer       = nullptr;
  VkeNodeData::List m_node_data;
  VkeMesh::List     m_mesh_data;
  Scene*            m_scene_graph = nullptr;
  VkeMaterial::List m_materials;
  VkeNodeData*      m_rotor_node = nullptr;

  float                                          m_rot_y = 0.0f;
  std::chrono::high_resolution_clock::time_point m_clock_at_start{};

  bool              m_ready = false;
  VkeSceneAnimation m_animation;

  VkeVBO m_global_vbo;
  VkeIBO m_global_ibo;

  nvvk::ShaderModuleManager m_shaderModuleManager;


  struct ModuleIDs
  {
    nvvk::ShaderModuleID scene_vs;
    nvvk::ShaderModuleID scene_fs;
    nvvk::ShaderModuleID scene_quad_vs;
    nvvk::ShaderModuleID scene_quad_fs;
    nvvk::ShaderModuleID scene_terrain_vs;
    nvvk::ShaderModuleID scene_terrain_fs;
    nvvk::ShaderModuleID scene_terrain_tcs;
    nvvk::ShaderModuleID scene_terrain_tes;
  } m_program_ids;

public:
  ModuleIDs& getModuleIDs() { return m_program_ids; }
};


#endif
