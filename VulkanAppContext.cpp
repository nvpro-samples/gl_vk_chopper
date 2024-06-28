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


#include "VulkanAppContext.h"
#include "VKSFile.h"
#include "vkaUtils.h"

#include <string>

#include "nvpwindow.hpp"

#include "VkeCreateUtils.h"
#include "VkeRenderer.h"


#include "VkeGameRendererDynamic.h"

#include "VulkanDeviceContext.h"
#include <iostream>
#ifndef INIT_COMMAND_ID
#define INIT_COMMAND_ID 1
#endif

#define DEFAULT_SCENE_ID 1

#define RENDERER vkeGameRendererDynamic


VulkanAppContext* VulkanAppContext::GetInstance()
{
  static VulkanAppContext* instance = NULL;

  if(!instance)
  {
    instance = new VulkanAppContext();
  }

  return instance;
}

VulkanAppContext::VulkanAppContext() {}

void VulkanAppContext::loadVKSScene(const std::string& inFileName)
{
  VKSFile vkFile;
  vkFile.outputFile = inFileName;


  readVKSFile(&vkFile);

  /*
		Unpack meshes
	*/

  uint32_t meshCnt = vkFile.header.meshCount;

#if USE_SINGLE_VBO

  uint32_t vtxStoreSize = size_t(vkFile.vertexCount) * 8 * sizeof(float);
  uint32_t idxStoreSize = vkFile.indexCount * sizeof(uint32_t);

  m_global_vbo.initBackingStore(vtxStoreSize);
  m_global_ibo.initBackingStore(idxStoreSize);

  float* vData = m_global_vbo.getBackingStore();
  memcpy(vData, vkFile.vertices.data(), vtxStoreSize);
  uint32_t* iData = m_global_ibo.getBackingStore();
  memcpy(iData, vkFile.indices.data(), idxStoreSize);


  m_global_vbo.initVKBufferData();
  m_global_ibo.initVKBufferData();

#endif


  uint32_t animCount = vkFile.header.animationCount;


  if(animCount >= 1)
  {
    //importing the first animation only at the moment.
    VKSAnimationRecord animation = vkFile.animations[0];
    uint32_t           nCnt      = animation.nodecount;

    for(size_t n = 0; n < nCnt; ++n)
    {
      VKSAnimationNodeRecord nodeAnim = vkFile.animationNodes[n + animation.firstNode];

      std::string newNodeName(nodeAnim.name);

      VkeAnimationNode* vAnimNode = m_animation.newNode(newNodeName);

      uint32_t keyCount = nodeAnim.positionCount;

      for(size_t k = 0; k < keyCount; ++k)
      {
        VKSAnimationKeyRecord key = vkFile.animationKeys[k + nodeAnim.firstPosition];
        vAnimNode->Position().newKey(key.time, key.key);
      }

      keyCount = nodeAnim.rotationCount;

      for(size_t k = 0; k < keyCount; ++k)
      {
        VKSAnimationKeyRecord key = vkFile.animationKeys[k + nodeAnim.firstRotation];
        vAnimNode->Rotation().newKey(key.time, key.key);
      }

      keyCount = nodeAnim.scaleCount;

      for(size_t k = 0; k < keyCount; ++k)
      {
        VKSAnimationKeyRecord key = vkFile.animationKeys[k + nodeAnim.firstScale];
        vAnimNode->Scale().newKey(key.time, key.key);
      }
    }
  }


  for(uint32_t i = 0; i < meshCnt; ++i)
  {
    VkeMesh* theMesh = m_mesh_data.newMesh(i, &vkFile, &vkFile.meshes[i]);

#if USE_SINGLE_VBO
#else
    theMesh->initVKBuffers();
#endif


    theMesh->setFirstIndex(vkFile.meshes[i].firstIndex);
    theMesh->setFirstVertex(vkFile.meshes[i].firstVertex);
  }

  uint32_t matCnt = vkFile.header.materialCount;


  for(uint32_t i = 0; i < matCnt; ++i)
  {
    m_materials.newMaterial(i)->initFromData(&vkFile, &vkFile.materials[i]);
  }

  uint32_t nodeCnt        = vkFile.header.nodeCount;
  uint32_t nodesProcessed = 0;


  while(nodesProcessed < nodeCnt)
  {
    addVKSNode(&vkFile, nodesProcessed);
  }


  m_node_data.sortByOpacity();
}

void VulkanAppContext::addVKSNode(VKSFile* inFile, uint32_t& inNodesProcessed, Node* parentNode)
{

  uint32_t       nodeID   = inNodesProcessed;
  VKSNodeRecord* fileNode = &inFile->nodes[inNodesProcessed++];
  Node*          node     = nullptr;

  uint32_t mshCount = fileNode->meshCount;

  for(uint32_t i = 0; i < mshCount; ++i)
  {

    if(parentNode)
    {
      node = parentNode->newChild(nodeID);
    }
    else
    {
      node = m_scene_graph->Nodes().newNode(nodeID);
    }

    node->setPosition(fileNode->position.x, fileNode->position.y, fileNode->position.z);

    node->setRotation(fileNode->rotation);
    node->setScale(fileNode->scale.x, fileNode->scale.y, fileNode->scale.z);


    m_node_data.newData(node->getID())->updateFromNode(node);
    m_node_data.getData(node->getID())->setMesh(m_mesh_data.getMesh(fileNode->meshIndices[i]));
  }

  std::string nameStr = std::string(fileNode->name);

  VkeAnimationNode* animNode = m_animation.Nodes().getNode(nameStr);
  if(animNode)
  {
    animNode->setNode(m_node_data.getData(node->getID()));
  }

  if(nameStr == "main_rotor_parts02")
  {
    m_rotor_node = m_node_data.getData(node->getID());
  }

  uint32_t childCount = fileNode->childCount;
  for(uint32_t i = 0; i < childCount; ++i)
  {
    addVKSNode(inFile, inNodesProcessed, node);
  }
}


void VulkanAppContext::initRenderer()
{

  RenderContext* rctxt = RenderContext::Get();
  if(!rctxt)
    return;


  /*
		Used to manage the rotation of the 
		Gazelle's rotors.
	*/

  m_rot_y          = 0.0f;
  m_clock_at_start = std::chrono::high_resolution_clock::now();

  /*
		Create the renderer.
	*/

  m_renderer = new RENDERER();

  m_scene_graph = rctxt->newScene(DEFAULT_SCENE_ID);

  loadVKSScene("chopper_pack32.vks");


  ((RENDERER*)m_renderer)->setNodeData(&m_node_data);
  ((RENDERER*)m_renderer)->setMaterialData(&m_materials);
  ((RENDERER*)m_renderer)->initIndirectCommands();
  m_renderer->initShaders(m_shaderModuleManager);

  m_renderer->initLayouts();

  resize(m_width, m_height);

  VkCommandBuffer                          cmd   = VK_NULL_HANDLE;
  VulkanDC::Device::Queue::CommandBufferID cmdID = INIT_COMMAND_ID + 300;
  VulkanDC*                                dc    = VulkanDC::Get();

  dc->getDefaultQueue()->beginCommandBuffer(cmdID, &cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  dc->getDefaultQueue()->flushCommandBuffer(cmdID, NULL);
  m_ready = true;
}

void VulkanAppContext::resize(uint32_t inWidth, uint32_t inHeight)
{
  VulkanDC*                                dc     = VulkanDC::Get();
  VulkanDC::Device::Queue::CommandBufferID cmdID  = INIT_COMMAND_ID;
  VulkanDC::Device*                        device = dc->getDefaultDevice();
  device->waitIdle();
  m_renderer->resize(inWidth, inHeight);

  dc->getDefaultQueue()->flushCommandBuffer(cmdID);
}

void VulkanAppContext::render()
{
  if(!m_ready)
    return;

  double nanoseconds_since_start = double(
      std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_clock_at_start).count());
  double seconds_since_start = nanoseconds_since_start / 1e9f;
  m_rot_y                    = float(0.75 * seconds_since_start);

  if(m_rotor_node)
  {
    m_rotor_node->getNode()->setRotation(0.0, 0.0, -m_rot_y * 32.f);
  }

  m_renderer->update();
}

// This is a simple message callback to capture debug messages.
// For a more complex callback with message filtering, see
// Context::debugMessengerCallback in nvpro_core/context_vk.cpp.
VKAPI_ATTR VkBool32 VKAPI_CALL debugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                      VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                                      const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
                                                      void*                                       userData)
{
  if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
  {
    nvprintfLevel(LOGLEVEL_DEBUG, "VERBOSE: %s \n --> %s\n", callbackData->pMessageIdName, callbackData->pMessage);
  }
  else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
  {
    nvprintfLevel(LOGLEVEL_INFO, "INFO: %s \n --> %s\n", callbackData->pMessageIdName, callbackData->pMessage);
  }
  else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
  {
    nvprintfLevel(LOGLEVEL_WARNING, "WARNING: %s \n --> %s\n", callbackData->pMessageIdName, callbackData->pMessage);
  }
  else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
  {
    nvprintfLevel(LOGLEVEL_ERROR, "ERROR: %s \n --> %s\n", callbackData->pMessageIdName, callbackData->pMessage);
  }
  else
  {
    nvprintfLevel(LOGLEVEL_INFO, "%s \n --> %s\n", callbackData->pMessageIdName, callbackData->pMessage);
  }
  // Don't bail out, but keep going.
  return VK_FALSE;
}

void VulkanAppContext::initAppContext()
{

  /*
		Set up the application create info.
	*/
  VkApplicationInfo appInfo  = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
  appInfo.pApplicationName   = "gl_vk_chopper";
  appInfo.applicationVersion = 1;
  appInfo.pEngineName        = "gl_vk_chopper";
  appInfo.engineVersion      = 1;
  appInfo.apiVersion         = VK_API_VERSION_1_0;
  /*
		Set up the instance create info.
	*/
  VkInstanceCreateInfo createInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
  createInfo.pApplicationInfo     = &appInfo;
  std::vector<const char*> enabledLayers, enabledExtensions;
  /* Validation: You can enable this and the #if below to enable the validation
  layers. Note that this sample currently crashes when validation is on, but
  this can be useful for development. */
#if 0 && !defined(NDEBUG)
  nvprintSetBreakpoints(true, LOGBITS_WARNINGS);
  enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
  enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
  createInfo.enabledLayerCount       = uint32_t(enabledLayers.size());
  createInfo.ppEnabledLayerNames     = enabledLayers.data();
  createInfo.enabledExtensionCount   = uint32_t(enabledExtensions.size());
  createInfo.ppEnabledExtensionNames = enabledExtensions.data();

  /*
		Create the Vulkan Instance
	*/

  VKA_CHECK_ERROR(vulkanCreateInstance(&createInfo, &m_vk_instance), "Could not create Vulkan Instance");

  /* Validation: You can enable this and the #if above to enable the validation
  layers. Note that this sample currently crashes when validation is on, but 
  this can be useful for development. */
#if 0 && !defined(NDEBUG)
  {
    PFN_vkCreateDebugUtilsMessengerEXT createDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(m_vk_instance, "vkCreateDebugUtilsMessengerEXT"));
    if(nullptr != createDebugUtilsMessengerEXT)
    {
      VkDebugUtilsMessengerCreateInfoEXT dbgCreateInfo{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
      dbgCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT       // For debug printf
                                      | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT  // GPU info, bug
                                      | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;   // Invalid usage
      dbgCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT            // Other
                                  | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT       // Violation of spec
                                  | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;     // Non-optimal use
      dbgCreateInfo.pfnUserCallback = debugMessengerCallback;
      dbgCreateInfo.pUserData       = nullptr;
      VKA_CHECK_ERROR(createDebugUtilsMessengerEXT(m_vk_instance, &dbgCreateInfo, nullptr, &m_debug_messenger),
                      "Could not initialize debug utils messenger");
    }
  }
#endif

  /*
		Set up and initialise the Vulkan Device Context.
	*/
  VulkanDC* dc = VulkanDC::Get();
  dc->initDC(m_vk_instance);

  /*
		Get the default graphics queue.
	*/
  VulkanDC::Device::Queue::Name queueName = "DEFAULT_GRAPHICS_QUEUE";
  VulkanDC::Device::Queue*      queue     = dc->getQueueForGraphics(queueName, m_surface_format);
  dc->setDefaultDevice(queue->getDevice());
  dc->setDefaultQueue(queue);
}

VulkanAppContext::~VulkanAppContext() {}


bool VulkanAppContext::initPrograms()
{

  m_shaderModuleManager.init(VulkanDC::Get()->getDevice()->getVKDevice());
  /*
		Initialise the shaders used for the demo.
	*/
  m_shaderModuleManager.addDirectory(std::string("./GLSL_" PROJECT_NAME));
  m_shaderModuleManager.addDirectory(std::string(PROJECT_NAME) + std::string("shaders"));
  m_shaderModuleManager.addDirectory(NVPSystem::exePath() + std::string(PROJECT_RELDIRECTORY) + std::string("shaders"));

  m_program_ids.scene_vs = m_shaderModuleManager.createShaderModule(VK_SHADER_STAGE_VERTEX_BIT, "std_vertex.glsl");
  m_program_ids.scene_fs = m_shaderModuleManager.createShaderModule(VK_SHADER_STAGE_FRAGMENT_BIT, "std_fragment.glsl");

  m_program_ids.scene_quad_vs = m_shaderModuleManager.createShaderModule(VK_SHADER_STAGE_VERTEX_BIT, "vertexQuad.glsl");
  m_program_ids.scene_quad_fs = m_shaderModuleManager.createShaderModule(VK_SHADER_STAGE_FRAGMENT_BIT, "fragmentQuad.glsl");

  m_program_ids.scene_terrain_vs = m_shaderModuleManager.createShaderModule(VK_SHADER_STAGE_VERTEX_BIT, "vertexTerrain.glsl");
  m_program_ids.scene_terrain_fs = m_shaderModuleManager.createShaderModule(VK_SHADER_STAGE_FRAGMENT_BIT, "fragmentTerrain.glsl");
  m_program_ids.scene_terrain_tcs =
      m_shaderModuleManager.createShaderModule(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, "tcsTerrain.glsl");
  m_program_ids.scene_terrain_tes =
      m_shaderModuleManager.createShaderModule(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, "tesTerrain.glsl");


  /*
		Check that the programs are valid.
	*/
  bool bRes = m_shaderModuleManager.areShaderModulesValid();
  if(bRes)
  {
    LOGOK("GLSL programs validated\n")
  }
  else
  {
    LOGE("GLSL programs validation failed\n")
  }
  return bRes;
}

VkeMaterial* VulkanAppContext::getMaterial(VkeMaterial::ID inID)
{
  return m_materials.getMaterial(inID);
}

void VulkanAppContext::setCameraMatrix(glm::mat4& inMat)
{
  ((RENDERER*)m_renderer)->setCameraLookAt(inMat);
}