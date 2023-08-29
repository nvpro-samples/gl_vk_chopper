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

#include <include_gl.h>

#include "VkeCamera.h"
#include "VkeGameRendererDynamic.h"
#include "VkeIBO.h"
#include "VkeMaterial.h"
#include "VkeTexture.h"
#include "VkeVBO.h"
#include "VulkanAppContext.h"
#include <algorithm>
#ifndef INIT_COMMAND_ID
#define INIT_COMMAND_ID 1
#endif

#ifndef GL_NV_draw_vulkan_image
#define GL_NV_draw_vulkan_image 1
//typedef GLVULKANPROCNV (GLAPIENTRY* PFNGLGETVKINSTANCEPROCADDRNVPROC) (const GLchar *name);
typedef void(GLAPIENTRY* PFNGLWAITVKSEMAPHORENVPROC)(GLuint64 vkSemaphore);
typedef void(GLAPIENTRY* PFNGLSIGNALVKSEMAPHORENVPROC)(GLuint64 vkSemaphore);
typedef void(GLAPIENTRY* PFNGLSIGNALVKFENCENVPROC)(GLuint64 vkFence);
typedef void(GLAPIENTRY* PFNGLDRAWVKIMAGENVPROC)(GLuint64 vkImage,
                                                 GLuint   sampler,
                                                 GLfloat  x0,
                                                 GLfloat  y0,
                                                 GLfloat  x1,
                                                 GLfloat  y1,
                                                 GLfloat  z,
                                                 GLfloat  s0,
                                                 GLfloat  t0,
                                                 GLfloat  s1,
                                                 GLfloat  t1);
//PFNGLGETVKINSTANCEPROCADDRNVPROC    glGetVkInstanceProcAddrNV;
PFNGLWAITVKSEMAPHORENVPROC   glWaitVkSemaphoreNV;
PFNGLSIGNALVKSEMAPHORENVPROC glSignalVkSemaphoreNV;
PFNGLSIGNALVKFENCENVPROC     glSignalVkFenceNV;
PFNGLDRAWVKIMAGENVPROC       glDrawVkImageNV;
#endif

#ifdef GL_GLEXT_PROTOTYPES
//GLAPI GLVULKANPROCNV GLAPIENTRY glGetVkInstanceProcAddrNV (const GLchar *name);
//GLAPI void GLAPIENTRY glWaitVkSemaphoreNV(GLuint64 vkSemaphore);
//GLAPI void GLAPIENTRY glSignalVkSemaphoreNV(GLuint64 vkSemaphore);
//GLAPI void GLAPIENTRY glSignalVkFenceNV(GLuint64 vkFence);
//GLAPI void GLAPIENTRY glDrawVkImageNV(GLuint64 vkImage, GLuint sampler, GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, GLfloat z, GLfloat s0, GLfloat t0, GLfloat s1, GLfloat t1);
#endif /* GL_GLEXT_PROTOTYPES */

static void setDefaultViewportAndScissor(VkCommandBuffer cmd, uint32_t width, uint32_t height)
{
  VkViewport vp{};
  VkRect2D   sc{};
  vp.x        = 0;
  vp.y        = 0;
  vp.height   = (float)(height);
  vp.width    = (float)(width);
  vp.minDepth = 0.0f;
  vp.maxDepth = 1.0f;

  sc.offset.x      = 0;
  sc.offset.y      = 0;
  sc.extent.width  = width;
  sc.extent.height = height;

  vkCmdSetViewport(cmd, 0, 1, &vp);
  vkCmdSetScissor(cmd, 0, 1, &sc);
}

VkeDrawCall::VkeDrawCall(vkeGameRendererDynamic* inRenderer)
    : m_renderer(inRenderer)
    , m_buffer_ready(false)
{
  m_draw_command[0] = VK_NULL_HANDLE;
  m_draw_command[1] = VK_NULL_HANDLE;

  initCommandPool();
  initDescriptorPool();
}

VkeDrawCall::~VkeDrawCall() {}

VkCommandBuffer VkeDrawCall::getDrawCommand(const uint32_t inFrameIndex)
{
  return m_draw_command[inFrameIndex];
}

void VkeDrawCall::initCommandPool()
{

  VulkanDC*         dc     = VulkanDC::Get();
  VulkanDC::Device* device = dc->getDefaultDevice();

  VkCommandPoolCreateInfo cmdPoolInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
  cmdPoolInfo.queueFamilyIndex        = 0;
  cmdPoolInfo.flags                   = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  VKA_CHECK_ERROR(vkCreateCommandPool(device->getVKDevice(), &cmdPoolInfo, NULL, &m_command_pool), "Could not create command pool.\n");

  VkCommandBufferAllocateInfo cmdBufInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  cmdBufInfo.commandBufferCount          = 2;
  cmdBufInfo.level                       = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
  cmdBufInfo.commandPool                 = m_command_pool;

  VKA_CHECK_ERROR(vkAllocateCommandBuffers(device->getVKDevice(), &cmdBufInfo, m_draw_command),
                  "Could not allocate secondary command buffers.\n");
}

void VkeDrawCall::initDescriptorPool()
{
  VulkanDC*         dc     = VulkanDC::Get();
  VulkanDC::Device* device = dc->getDefaultDevice();

  VkDescriptorPoolSize poolSize = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1};

  VkDescriptorPoolCreateInfo poolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
  poolInfo.maxSets                    = 1;
  poolInfo.poolSizeCount              = 1;
  poolInfo.pPoolSizes                 = &poolSize;
  poolInfo.flags                      = 0;

  VKA_CHECK_ERROR(vkCreateDescriptorPool(device->getVKDevice(), &poolInfo, NULL, &m_descriptor_pool),
                  "Could not create descriptor pool.\n");
}

void VkeDrawCall::initDescriptor()
{

  VulkanDC*            dc     = VulkanDC::Get();
  VulkanDC::Device*    device = dc->getDefaultDevice();
  VkWriteDescriptorSet writes[1];

  vkResetDescriptorPool(device->getVKDevice(), m_descriptor_pool, 0);

  VkDescriptorSetAllocateInfo descAlloc = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};

  descAlloc.descriptorPool     = m_descriptor_pool;
  descAlloc.pSetLayouts        = (m_renderer->getTransformDescriptorLayout());
  descAlloc.descriptorSetCount = 1;


  VKA_CHECK_ERROR(vkAllocateDescriptorSets(device->getVKDevice(), &descAlloc, &m_transform_descriptor_set),
                  "Could not allocate descriptor sets.\n");

  descriptorSetWrite(&writes[0], 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, (m_renderer->getTransformsDescriptor()),
                     VK_NULL_HANDLE, 0, m_transform_descriptor_set);  //transform
  vkUpdateDescriptorSets(device->getVKDevice(), 1, writes, 0, NULL);
}


void VkeDrawCall::initDrawCommands(const uint32_t inCount,
                                   const uint32_t inCommandIndex,
                                   VkRenderPass   parentRenderPass,
                                   uint32_t       viewportWidth,
                                   uint32_t       viewportHeight)
{

  VkPipelineLayout layout              = m_renderer->getPipelineLayout();
  VkPipeline       pipeline            = m_renderer->getPipeline();
  VkDescriptorSet  sceneDescriptor     = m_renderer->getSceneDescriptorSet();
  VkDescriptorSet* textureDescriptors  = m_renderer->getTextureDescriptorSets();
  VkBuffer         sceneIndirectBuffer = m_renderer->getSceneIndirectBuffer();


  VulkanAppContext* ctxt = VulkanAppContext::GetInstance();


  vkResetCommandBuffer(m_draw_command[inCommandIndex], 0);

  VkCommandBufferInheritanceInfo cmdInheritanceInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO};
  cmdInheritanceInfo.renderPass                     = parentRenderPass;
  cmdInheritanceInfo.subpass                        = 0;
  VkCommandBufferBeginInfo cmdBeginInfo             = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
  cmdBeginInfo.pInheritanceInfo = &cmdInheritanceInfo;

  VKA_CHECK_ERROR(vkBeginCommandBuffer(m_draw_command[inCommandIndex], &cmdBeginInfo), "Could not begin command buffer.\n");

  // TODO: Switch to using inherited viewport and scissor
  setDefaultViewportAndScissor(m_draw_command[inCommandIndex], viewportWidth, viewportHeight);
  vkCmdBindPipeline(m_draw_command[inCommandIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

  VkeVBO* theVBO = ctxt->getVBO();
  VkeIBO* theIBO = ctxt->getIBO();

  theVBO->bind(&m_draw_command[inCommandIndex]);
  theIBO->bind(&m_draw_command[inCommandIndex]);

  VkDescriptorSet sets[3] = {sceneDescriptor, textureDescriptors[0], m_transform_descriptor_set};
  vkCmdBindDescriptorSets(m_draw_command[inCommandIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 3, sets, 0, NULL);

  vkCmdDrawIndexedIndirect(m_draw_command[inCommandIndex], sceneIndirectBuffer, 0, inCount, sizeof(VkDrawIndexedIndirectCommand));
  vkCmdDraw(m_draw_command[inCommandIndex], 1, 1, 0, 0);
  vkEndCommandBuffer(m_draw_command[inCommandIndex]);

  /*
	Lock mutex to update generated call count.
	*/
  //std::lock_guard<std::mutex> lk(m_renderer->getSecondaryCmdBufferMutex());

  /*
	Increment the generated call count
	*/
  m_renderer->incrementDrawCallsGenerated();
}

vkeGameRendererDynamic::vkeGameRendererDynamic()
    : VkeRenderer()
    , m_node_data(NULL)
{
  initRenderer();
}


vkeGameRendererDynamic::~vkeGameRendererDynamic() {}


void vkeGameRendererDynamic::initIndirectCommands()
{

  if(!m_node_data)
    return;

  VulkanDC*                dc     = VulkanDC::Get();
  VulkanDC::Device*        device = dc->getDefaultDevice();
  VulkanDC::Device::Queue* queue  = dc->getDefaultQueue();

  size_t cnt = m_node_data->count();
  size_t sz  = sizeof(VkDrawIndexedIndirectCommand) * cnt;

  VkBuffer       sceneIndirectStaging;
  VkDeviceMemory sceneIndirectMemStaging;

  VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

  bufferCreate(&m_scene_indirect_buffer, sz, (VkBufferUsageFlagBits)usageFlags);
  bufferAlloc(&m_scene_indirect_buffer, &m_scene_indirect_memory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

  bufferCreate(&sceneIndirectStaging, sz, (VkBufferUsageFlagBits)usageFlags);
  bufferAlloc(&sceneIndirectStaging, &sceneIndirectMemStaging, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

  VkDrawIndexedIndirectCommand* commands = NULL;

  VKA_CHECK_ERROR(vkMapMemory(device->getVKDevice(), sceneIndirectMemStaging, 0, sz, 0, (void**)&commands),
                  "Could not map indirect buffer memory.\n");

  for(size_t i = 0; i < cnt; ++i)
  {
    VkeMesh* mesh             = m_node_data->getData(i)->getMesh();
    commands[i].firstIndex    = mesh->getFirstIndex();
    commands[i].firstInstance = uint32_t(i * m_instance_count);
    commands[i].vertexOffset  = mesh->getFirstVertex();
    commands[i].indexCount    = mesh->getIndexCount();
    commands[i].instanceCount = uint32_t(m_instance_count);
  }

  vkUnmapMemory(device->getVKDevice(), sceneIndirectMemStaging);

  VkBufferCopy bufCpy;
  bufCpy.dstOffset = 0;
  bufCpy.srcOffset = 0;
  bufCpy.size      = sz;

  VkCommandBuffer             copyCmd;
  VkCommandBufferAllocateInfo cmdBufInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  cmdBufInfo.commandBufferCount          = 1;
  cmdBufInfo.commandPool                 = queue->getCommandPool();
  cmdBufInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

  VKA_CHECK_ERROR(vkAllocateCommandBuffers(device->getVKDevice(), &cmdBufInfo, &copyCmd), "Could not allocate command buffers.\n");

  VkCommandBufferBeginInfo cmdBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  cmdBeginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  VKA_CHECK_ERROR(vkBeginCommandBuffer(copyCmd, &cmdBeginInfo), "Could not begin commmand buffer.\n");

  vkCmdCopyBuffer(copyCmd, sceneIndirectStaging, m_scene_indirect_buffer, 1, &bufCpy);

  VKA_CHECK_ERROR(vkEndCommandBuffer(copyCmd), "Could not end command buffer.\n");

  VkSubmitInfo subInfo       = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  subInfo.commandBufferCount = 1;
  subInfo.pCommandBuffers    = &copyCmd;

  VkFence           theFence;
  VkFenceCreateInfo fenceInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};

  VKA_CHECK_ERROR(vkCreateFence(device->getVKDevice(), &fenceInfo, NULL, &theFence), "Could not create fence.\n");
  VKA_CHECK_ERROR(vkQueueSubmit(queue->getVKQueue(), 1, &subInfo, theFence), "Could not submit queue for indirect buffer copy.\n");
  VKA_CHECK_ERROR(vkWaitForFences(device->getVKDevice(), 1, &theFence, VK_TRUE, UINT_MAX), "Could not wait for fence.\n");

  vkFreeCommandBuffers(device->getVKDevice(), queue->getCommandPool(), 1, &copyCmd);
  vkDestroyFence(device->getVKDevice(), theFence, NULL);
}

void vkeGameRendererDynamic::initDescriptorPool()
{
  VkeRenderer::initDescriptorPool();

  VkDescriptorPoolSize typeCounts[] = {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
                                       {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 2},
                                       {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}};

  VulkanDC* dc = VulkanDC::Get();
  if(!dc)
    return;

  VulkanDC::Device* device = dc->getDefaultDevice();

  VkDescriptorPoolCreateInfo descriptorPoolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
  descriptorPoolInfo.poolSizeCount              = 3;
  descriptorPoolInfo.pPoolSizes                 = typeCounts;
  descriptorPoolInfo.maxSets                    = (m_descriptor_pool_size * 2) + 3;
  VKA_CHECK_ERROR(vkCreateDescriptorPool(device->getVKDevice(), &descriptorPoolInfo, NULL, &m_descriptor_pool),
                  "Could not create descriptor pool.\n");
}


float quickRandomUVD()
{
  return ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
}


void vkeGameRendererDynamic::initRenderer()
{
  VulkanDC*         dc     = VulkanDC::Get();
  VulkanDC::Device* device = dc->getDevice();

  m_instance_count = 128;

  //glWaitVkSemaphoreNV = (PFNGLWAITVKSEMAPHORENVPROC)NVPSystem::GetProcAddressGL("glWaitVkSemaphoreNV");
  //glSignalVkSemaphoreNV = (PFNGLSIGNALVKSEMAPHORENVPROC)NVPSystem::GetProcAddressGL("glSignalVkSemaphoreNV");
  //glSignalVkFenceNV = (PFNGLSIGNALVKFENCENVPROC)NVPSystem::GetProcAddressGL("glSignalVkFenceNV");
  //	glDrawVkImageNV = (PFNGLDRAWVKIMAGENVPROC)NVPSystem::GetProcAddressGL("glDrawVkImageNV");

  VkSemaphoreCreateInfo semInfo   = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
  VkFenceCreateInfo     fenceInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};

  VKA_CHECK_ERROR(vkCreateSemaphore(device->getVKDevice(), &semInfo, NULL, &m_present_done[0]),
                  "Could not create present done semaphore.\n");
  VKA_CHECK_ERROR(vkCreateSemaphore(device->getVKDevice(), &semInfo, NULL, &m_render_done[0]),
                  "Could not create render done semaphore.\n");
  VKA_CHECK_ERROR(vkCreateSemaphore(device->getVKDevice(), &semInfo, NULL, &m_present_done[1]),
                  "Could not create present done semaphore.\n");
  VKA_CHECK_ERROR(vkCreateSemaphore(device->getVKDevice(), &semInfo, NULL, &m_render_done[1]),
                  "Could not create render done semaphore.\n");

  VKA_CHECK_ERROR(vkCreateFence(device->getVKDevice(), &fenceInfo, NULL, &m_update_fence[0]), "Could not create update fence.\n");
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  VKA_CHECK_ERROR(vkCreateFence(device->getVKDevice(), &fenceInfo, NULL, &m_update_fence[1]), "Could not create update fence.\n");

  m_terrain_command[0] = VK_NULL_HANDLE;
  m_terrain_command[1] = VK_NULL_HANDLE;
  m_framebuffers[0]    = VK_NULL_HANDLE;
  m_framebuffers[1]    = VK_NULL_HANDLE;
  m_update_commands[0] = VK_NULL_HANDLE;
  m_update_commands[1] = VK_NULL_HANDLE;

  m_is_first_frame = true;


  nvmath::vec4f table[128][128];

  for(int v = 0; v < 128; ++v)
  {
    for(int u = 0; u < 128; ++u)
    {
      nvmath::vec2f vctr(quickRandomUVD(), quickRandomUVD());
      vctr = nvmath::normalize(vctr);
      // VK_FORMAT_R32G32B32_SFLOAT isn't so widely supported, so we use
      // VK_FORMAT_R32G32B32A32_SFLOAT instead.
      table[u][v] = nvmath::vec4f(vctr.x, vctr.y, vctr.x, 0.f);
    }
  }

  m_cube_textures.newTexture(1)->loadCubeDDS("environ.dds");
  m_screen_quad.initQuadData();
  m_terrain_quad.initQuadData();

  m_textures.newTexture(0)->setFormat(VK_FORMAT_R32G32B32A32_SFLOAT);
  m_textures.getTexture(0)->loadTextureFloatData((float*)&(table[0][0].x), 128, 128, 4);

  m_flight_paths.resize(m_instance_count);

  for(uint32_t i = 0; i < m_instance_count; ++i)
  {
    nvmath::vec2f initPos(quickRandomUVD() * 100.0f, -200.f + (quickRandomUVD() * 20.f));
    nvmath::vec2f endPos(quickRandomUVD() * 100.0f, 200.f + (quickRandomUVD() * 20.f));
    m_flight_paths[i] =
        std::make_unique<FlightPath>(initPos, endPos, quickRandomUVD() * 0.5f + 0.5f, quickRandomUVD() * 4.f + 10.f);
  }

  /*
	Just initialises the draw call objects
	not the threads. They store thread local
	data for the threaded cmd buffer builds.
	*/
  initDrawCalls();


  /*
	Create primary command pool for the
	*/
  VkCommandPoolCreateInfo cmdPoolInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
  cmdPoolInfo.queueFamilyIndex        = 0;
  cmdPoolInfo.flags                   = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  VKA_CHECK_ERROR(vkCreateCommandPool(device->getVKDevice(), &cmdPoolInfo, NULL, &m_primary_buffer_cmd_pool),
                  "Could not create primary command pool.\n");

  m_primary_commands[0] = VK_NULL_HANDLE;
  m_primary_commands[1] = VK_NULL_HANDLE;

  VkCommandBufferAllocateInfo cmdBufInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  cmdBufInfo.commandBufferCount          = 2;
  cmdBufInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  cmdBufInfo.commandPool                 = m_primary_buffer_cmd_pool;


  VKA_CHECK_ERROR(vkAllocateCommandBuffers(device->getVKDevice(), &cmdBufInfo, m_primary_commands),
                  "Could not allocate primary command buffers.\n");
  VKA_CHECK_ERROR(vkAllocateCommandBuffers(device->getVKDevice(), &cmdBufInfo, m_update_commands),
                  "Could not allocate primary command buffers.\n");

  m_current_buffer_index = 0;
}


void vkeGameRendererDynamic::setNodeData(VkeNodeData::List* inData)
{
  m_node_data = inData;
  if(m_node_data != NULL)
  {

    size_t cnt            = m_node_data->count();
    size_t transformsSize = 64 * m_instance_count;

    size_t sz = sizeof(VkeNodeUniform) * cnt;
    sz += (transformsSize);

    m_uniforms_local = (float*)malloc(sz);

    bufferCreate(&m_uniforms_buffer, sz, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    bufferAlloc(&m_uniforms_buffer, &m_uniforms_memory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


    m_uniforms_descriptor.buffer = m_uniforms_buffer;
    m_uniforms_descriptor.offset = 0;
    m_uniforms_descriptor.range  = sizeof(VkeNodeUniform) * cnt;

    m_transforms_descriptor.buffer = m_uniforms_buffer;
    m_transforms_descriptor.offset = sizeof(VkeNodeUniform) * cnt;
    m_transforms_descriptor.range  = transformsSize;
  }
}

void vkeGameRendererDynamic::setMaterialData(VkeMaterial::List* inData)
{
  m_materials = inData;

  if(m_materials != NULL)
  {

    size_t cnt = m_materials->count();
    size_t sz  = sizeof(VkeMaterialUniform) * cnt;

    VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    bufferCreate(&m_material_buffer_staging, sz, (VkBufferUsageFlagBits)usageFlags);
    bufferAlloc(&m_material_buffer_staging, &m_material_staging, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT);

    VkeMaterialUniform* uniforms = NULL;

    VKA_CHECK_ERROR(vkMapMemory(getDefaultDevice(), m_material_staging, 0, sz, 0, (void**)&uniforms), "Could not map buffer memory.\n");

    for(size_t i = 0; i < cnt; ++i)
    {
      VkeMaterial* mat = m_materials->getMaterial(i);
      mat->initVKBufferData(m_material_buffer_staging);
      mat->updateVKBufferData(uniforms);
    }

    vkUnmapMemory(getDefaultDevice(), m_material_staging);
  }
}

size_t vkeGameRendererDynamic::getRequiredDescriptorCount()
{
  if(!m_node_data)
    return 0;
  return m_node_data->count();
}


void vkeGameRendererDynamic::update()
{
  VulkanDC*         dc     = VulkanDC::Get();
  VulkanDC::Device* device = dc->getDefaultDevice();

  static float sTime = 0.0f;

  size_t cnt = m_node_data->count();

  nvmath::mat4f tempMatrix;


  for(size_t i = 0; i < m_instance_count; ++i)
  {
    size_t         pointerOffset = (sizeof(VkeNodeUniform) * cnt) + (64 * i);
    nvmath::mat4f* matPtr        = (nvmath::mat4f*)(((uint8_t*)m_uniforms_local) + pointerOffset);
    m_flight_paths[i]->update(matPtr, sTime);
  }

  m_node_data->update((VkeNodeUniform*)m_uniforms_local, m_instance_count);

  m_camera->setViewport(0, 0, (float)m_width, (float)m_height);
  m_camera->update();

  generateDrawCommands();

  if(!m_is_first_frame)
  {
    vkResetFences(device->getVKDevice(), 1, &m_update_fence[m_current_buffer_index]);

    const VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo               subInfo    = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    subInfo.commandBufferCount            = 1;
    subInfo.pCommandBuffers               = &m_primary_commands[m_current_buffer_index];

#if defined(WIN32)
    subInfo.waitSemaphoreCount   = 1;
    subInfo.pWaitSemaphores      = &m_present_done[m_current_buffer_index];
    subInfo.pWaitDstStageMask    = &waitStages;
    subInfo.signalSemaphoreCount = 1;
    subInfo.pSignalSemaphores    = &m_render_done[m_current_buffer_index];
#endif
    vkQueueSubmit(dc->getDefaultQueue()->getVKQueue(), 1, &subInfo, m_update_fence[m_current_buffer_index]);

    /*
				Synchronise the next buffer. 
				This prevents the buffer from being reset when still in use
				when we have more than 2 frames in flight.
			*/
    uint32_t nextBufferIndex = (m_current_buffer_index + 1) % 2;
    present();
    vkWaitForFences(device->getVKDevice(), 1, &m_update_fence[nextBufferIndex], VK_TRUE, 1000000);
  }
  else
  {
    m_is_first_frame = false;
  }

  m_current_buffer_index++;
  m_current_buffer_index %= 2;

  sTime += 0.16f;
}


void vkeGameRendererDynamic::present()
{
  glDisable(GL_DEPTH_TEST);

#if defined(WIN32)
  glWaitVkSemaphoreNV((GLuint64)m_render_done[m_current_buffer_index]);
#endif

  glDrawVkImageNV((GLuint64)m_resolve_attachment[m_current_buffer_index].image, 0, 0, 0, float(m_width),
                  float(m_height), 0, 0, 1, 1, 0);

  glEnable(GL_DEPTH_TEST);
#if defined(WIN32)
  glSignalVkSemaphoreNV((GLuint64)m_present_done[m_current_buffer_index]);
#endif
}


void vkeGameRendererDynamic::initDescriptorLayout()
{
  VkDescriptorSetLayoutBinding sceneLayoutBindings[4];
  VkDescriptorSetLayoutBinding textureLayoutBindings[1];

  VkDescriptorSetLayoutBinding transformLayoutBinding;
  VkDescriptorSetLayoutBinding quadBinding[3];
  VkDescriptorSetLayoutBinding terrainBinding[5];


  /*----------------------------------------------------------
	Scene (Gazelle) objects descriptor and pipeline layout.
	----------------------------------------------------------*/

  /*
	Scene layout bindings (set 0)
	Binding 0:		Environment Cube Map
	Binding 1:		Camera Matrix
	Binding 2:		Model Matrix
	Binding 3:		Material
	*/

  layoutBinding(&sceneLayoutBindings[0], 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
  layoutBinding(&sceneLayoutBindings[1], 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
  layoutBinding(&sceneLayoutBindings[2], 2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1);
  layoutBinding(&sceneLayoutBindings[3], 3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);


  /*
	Transform layout binding (set 2)
	Binding 1:	Transform Matrix
	*/
  layoutBinding(&transformLayoutBinding, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1);


  /*
	Texture Layout Bindings (Set 1)
	Binding 0:		Diffuse Texture.
	*/
  assert(m_materials->count() == 6);  // We include 6 slots for textures in the shader
  layoutBinding(&textureLayoutBindings[0], 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 6);


  descriptorSetLayoutCreate(&m_transform_descriptor_layout, 1, &transformLayoutBinding);
  descriptorSetLayoutCreate(&m_scene_descriptor_layout, 4, sceneLayoutBindings);
  descriptorSetLayoutCreate(&m_texture_descriptor_set_layout, 1, textureLayoutBindings);

  VkDescriptorSetLayout layouts[3] = {m_scene_descriptor_layout, m_texture_descriptor_set_layout, m_transform_descriptor_layout};

  /*
	Create pipeline layout for the scene.
	*/
  pipelineLayoutCreate(&m_pipeline_layout, 3, layouts);


  /*----------------------------------------------------------
	Skybox descriptor and pipeline layout.
	----------------------------------------------------------*/


  layoutBinding(&quadBinding[0], 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1);
  layoutBinding(&quadBinding[1], 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
  layoutBinding(&quadBinding[2], 2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1);


  //descriptor layout describing the bindings.
  descriptorSetLayoutCreate(&m_quad_descriptor_set_layout, 3, quadBinding);

  //create the pipeline layout
  pipelineLayoutCreate(&m_quad_pipeline_layout, 1, &m_quad_descriptor_set_layout);


  /*----------------------------------------------------------
	Terrain descriptor and pipeline layout.
	----------------------------------------------------------*/

  layoutBinding(&terrainBinding[0], 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1);
  layoutBinding(&terrainBinding[1], 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT
                    | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                1);
  layoutBinding(&terrainBinding[2], 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
  layoutBinding(&terrainBinding[3], 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);

  //descriptor layout describing the bindings.
  descriptorSetLayoutCreate(&m_terrain_descriptor_set_layout, 4, terrainBinding);

  //create the pipeline layout
  pipelineLayoutCreate(&m_terrain_pipeline_layout, 1, &m_terrain_descriptor_set_layout);
}

void vkeGameRendererDynamic::initDescriptorSets()
{
  VulkanDC* dc = VulkanDC::Get();
  if(!dc)
    return;
  VulkanDC::Device* device = dc->getDefaultDevice();

  initCamera();

  vkResetDescriptorPool(device->getVKDevice(), getDescriptorPool(), 0);

  VkWriteDescriptorSet writes[6]{};

  /*----------------------------------------------------------
	Get the resource data for the bindings.
	----------------------------------------------------------*/


  /*
	Terrain textures.
	*/
  VkeTexture::Data terrain = m_textures.getTexture(0)->getData();

  /*
	Camera uniform
	*/
  VkDescriptorBufferInfo camInfo = m_camera->getDescriptor();

  /*
	Cube map texture.
	*/
  VkeCubeTexture::Data cube = m_cube_textures.getTexture(1)->getData();

  /*
	Create descriptor image info array
	for the terrain textures.
	*/
  VkDescriptorImageInfo fpSampler = {terrain.sampler, terrain.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};


  VkeMaterial* material = m_materials->getMaterial(0);


  /*
	Create descriptor image info array
	for the scene textures.
	*/
  size_t texCount = m_materials->count();

  VkDescriptorImageInfo* texImageInfo = (VkDescriptorImageInfo*)malloc(texCount * sizeof(VkDescriptorImageInfo));

  for(size_t i = 0; i < texCount; ++i)
  {
    VkeMaterial*     mtrl       = m_materials->getMaterial(i);
    VkeTexture::Data texData    = mtrl->getTextures().getTexture(0)->getData();
    texImageInfo[i].imageView   = texData.view;
    texImageInfo[i].sampler     = texData.sampler;
    texImageInfo[i].imageLayout = texData.imageLayout;
  }

  /*
	Create descriptor image info for
	the cube map texture.
	*/

  VkDescriptorImageInfo cubeTexture;
  cubeTexture.sampler     = cube.sampler;
  cubeTexture.imageView   = cube.view;
  cubeTexture.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  /*
	Allocate storage for the scene and
	texture descriptor sets.
	*/


  /*----------------------------------------------------------
	Allocate the descriptor sets.
	----------------------------------------------------------*/
  m_texture_descriptor_sets = (VkDescriptorSet*)malloc(sizeof(VkDescriptorSet));

  /*
	Set up the alocate info structure for
	the descriptor sets.
	*/
  VkDescriptorSetAllocateInfo descAlloc;
  memset(&descAlloc, 0, sizeof(descAlloc));
  descAlloc.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  descAlloc.descriptorPool     = getDescriptorPool();
  descAlloc.pSetLayouts        = &m_scene_descriptor_layout;
  descAlloc.descriptorSetCount = 1;


  VKA_CHECK_ERROR(vkAllocateDescriptorSets(device->getVKDevice(), &descAlloc, &m_scene_descriptor_set),
                  "Could not allocate descriptor sets.\n");

  /*
	Set up the texture descriptor sets.
	*/
  descAlloc.pSetLayouts        = &m_texture_descriptor_set_layout;
  descAlloc.descriptorSetCount = 1;

  VKA_CHECK_ERROR(vkAllocateDescriptorSets(device->getVKDevice(), &descAlloc, m_texture_descriptor_sets),
                  "Could not allocate texture descriptor sets.\n");

  /*
	Set up the skybox descriptor sets.
	*/
  descAlloc.pSetLayouts        = &m_quad_descriptor_set_layout;
  descAlloc.descriptorSetCount = 1;

  VKA_CHECK_ERROR(vkAllocateDescriptorSets(device->getVKDevice(), &descAlloc, &m_quad_descriptor_set),
                  "Could not allocate descriptor sets.\n");

  /*
	Set up the terrian descriptor sets.
	*/
  descAlloc.pSetLayouts        = &m_terrain_descriptor_set_layout;
  descAlloc.descriptorSetCount = 1;

  VKA_CHECK_ERROR(vkAllocateDescriptorSets(device->getVKDevice(), &descAlloc, &m_terrain_descriptor_set),
                  "Could not allocate descriptor sets.\n");


  /*
	Set up the transforms descriptor sets.
	*/
  descAlloc.pSetLayouts        = &m_transform_descriptor_layout;
  descAlloc.descriptorSetCount = 1;

  VKA_CHECK_ERROR(vkAllocateDescriptorSets(device->getVKDevice(), &descAlloc, &m_transform_descriptor_set),
                  "Could not allocate descriptor sets.\n");


  /*----------------------------------------------------------
	Update the descriptor sets with resource bindings.
	----------------------------------------------------------*/

  /*
	Scene layout bindings (set 0)
	Binding 0:		Environment Cube Map
	Binding 1:		Camera Matrix
	Binding 2:		Model Matrix
	Binding 3:		Material
	*/

  descriptorSetWrite(&writes[0], 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_NULL_HANDLE, &cubeTexture, 0,
                     m_scene_descriptor_set);  //cubemap
  descriptorSetWrite(&writes[1], 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &camInfo, VK_NULL_HANDLE, 0, m_scene_descriptor_set);  //Camera
  descriptorSetWrite(&writes[2], 2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &m_uniforms_descriptor, VK_NULL_HANDLE, 0,
                     m_scene_descriptor_set);  //modelview
  descriptorSetWrite(&writes[3], 3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &material->getDescriptor(), VK_NULL_HANDLE, 0,
                     m_scene_descriptor_set);  //material

  vkUpdateDescriptorSets(device->getVKDevice(), 4, writes, 0, NULL);

  /*
	Transform layout bindings (set 0)
	Binding 0:		Transform
	*/

  descriptorSetWrite(&writes[4], 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &m_transforms_descriptor, VK_NULL_HANDLE, 0,
                     m_transform_descriptor_set);  //transform
  vkUpdateDescriptorSets(device->getVKDevice(), 1, writes, 0, NULL);


  /*
	Scene layout bindings (set 1)
	Binding 0:		Scene texture array
	*/
  descriptorSetWrite(&writes[5], 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, uint32_t(texCount), VK_NULL_HANDLE,
                     texImageInfo, 0, m_texture_descriptor_sets[0]);  //cubemap

  vkUpdateDescriptorSets(device->getVKDevice(), 6, writes, 0, NULL);
  //Free the texture image info allocated earlier.
  free(texImageInfo);


  /*
	Skybox layout bindings (set 0)
	Binding 0:		Skybox Uniforms
	Binding 1:		Skybox Textures
	Binding 2:		Camera uniforms
	*/

  descriptorSetWrite(&writes[0], 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &m_screen_quad.getData().descriptor,
                     VK_NULL_HANDLE, 0, m_quad_descriptor_set);
  descriptorSetWrite(&writes[1], 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_NULL_HANDLE, &cubeTexture, 0, m_quad_descriptor_set);
  descriptorSetWrite(&writes[2], 2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &camInfo, VK_NULL_HANDLE, 0, m_quad_descriptor_set);

  vkUpdateDescriptorSets(device->getVKDevice(), 3, writes, 0, NULL);


  /*
	Terrain layout bindings (set 0)
	Binding 0:		Terrain Uniforms
	Binding 1:		Camera Uniforms
	Binding 2:		Terrain texture array
	*/

  descriptorSetWrite(&writes[0], 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &m_terrain_quad.getData().descriptor,
                     VK_NULL_HANDLE, 0, m_terrain_descriptor_set);
  descriptorSetWrite(&writes[1], 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &camInfo, VK_NULL_HANDLE, 0, m_terrain_descriptor_set);
  descriptorSetWrite(&writes[2], 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_NULL_HANDLE, &fpSampler, 0, m_terrain_descriptor_set);
  descriptorSetWrite(&writes[3], 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_NULL_HANDLE, &cubeTexture, 0,
                     m_terrain_descriptor_set);

  vkUpdateDescriptorSets(device->getVKDevice(), 4, writes, 0, NULL);


  /*----------------------------------------------------------
	Initialise the terrain and scene command buffers.
	----------------------------------------------------------*/
  initTerrainCommand();

  for(uint32_t i = 0; i < m_max_draw_calls; ++i)
  {
    m_draw_calls[i]->initDescriptor();
  }

  //m_test_drawcall->initDrawCommands(m_node_data->count());

  //this needs to happen in the thread.
  //m_draw_calls[0]->initDrawCommands(m_node_data->count());
}


void vkeGameRendererDynamic::initPipeline()
{
  VulkanDC*         dc     = VulkanDC::Get();
  VulkanDC::Device* device = dc->getDefaultDevice();

  VkPipelineCacheCreateInfo              pipelineCache = {VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO};
  VkPipelineVertexInputStateCreateInfo   vertexState;
  VkPipelineInputAssemblyStateCreateInfo inputState;
  VkPipelineRasterizationStateCreateInfo rasterState;
  VkPipelineColorBlendStateCreateInfo    blendState;
  VkPipelineDepthStencilStateCreateInfo  depthState;
  VkPipelineViewportStateCreateInfo      viewportState;
  VkPipelineMultisampleStateCreateInfo   multisampleState;
  VkVertexInputBindingDescription        binding;
  VkVertexInputAttributeDescription      attrs[3];

  /*----------------------------------------------------------
	Create the pipeline cache.
	----------------------------------------------------------*/
  VKA_CHECK_ERROR(vkCreatePipelineCache(device->getVKDevice(), &pipelineCache, NULL, &m_pipeline_cache),
                  "Couldn not create pipeline cache.\n");


  /*----------------------------------------------------------
	Create the scene pipeline.
	----------------------------------------------------------*/

  /*
	Create the vertex input state.
	Binding at location 0.
	3 attributes:
	1 : Vertex Position :	vec4
	2 : Vertex Normal	:	vec4
	3 : UV				:	vec2
	*/
  vertexBinding(&binding, 0, sizeof(VertexObject), VK_VERTEX_INPUT_RATE_VERTEX);
  vertexAttributef(&attrs[0], 0, binding.binding, VK_FORMAT_R32G32B32A32_SFLOAT, 0);
  vertexAttributef(&attrs[1], 1, binding.binding, VK_FORMAT_R32G32B32A32_SFLOAT, 4);
  vertexStateInfo(&vertexState, 1, 2, &binding, attrs);

  /*
	Create the input assembly state
	Topology: Triangle List
	Primitive restart enable : false
	*/
  inputAssemblyStateInfo(&inputState, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);

  /*
	Create the raster state
	*/
  rasterStateInfo(&rasterState, VK_POLYGON_MODE_FILL);

  /*
	Create the color blend state.
	*/
  VkPipelineColorBlendAttachmentState attState[1];
  uint32_t                            sampleMask = 0xFF;
  blendAttachmentStateN(1, attState);
  blendStateInfo(&blendState, 1, attState);

  /*
	Create the viewport state
	*/
  viewportStateInfo(&viewportState, 1);

  /*
	Create the depth stencil state
	*/
  depthStateInfo(&depthState);

  /*
	Create the multisample state
	*/
  multisampleStateInfo(&multisampleState, getSamples(), &sampleMask);


  /*
	Create the pipeline shaders.
	*/
  VkPipelineShaderStageCreateInfo shaderStages[4];
  createShaderStage(&shaderStages[0], VK_SHADER_STAGE_VERTEX_BIT, m_shaders.scene_vertex);
  createShaderStage(&shaderStages[1], VK_SHADER_STAGE_FRAGMENT_BIT, m_shaders.scene_fragment);


  /*
	Create the graphics pipeline.
	*/
  graphicsPipelineCreate(&m_pipeline, &m_pipeline_cache, m_pipeline_layout, 2, shaderStages, &vertexState, &inputState,
                         &rasterState, &blendState, &multisampleState, &viewportState, &depthState, &m_render_pass, 0,
                         VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT);

  /*----------------------------------------------------------
	Create the skybox pipeline.
	----------------------------------------------------------*/

  /*
	Reusing the create info struct from
	the scene pipeline, just update what
	we need.
	*/
  vertexBinding(&binding, 0, sizeof(VertexObjectUV), VK_VERTEX_INPUT_RATE_VERTEX);
  vertexAttributef(&attrs[0], 0, binding.binding, VK_FORMAT_R32G32B32A32_SFLOAT, 0);
  vertexAttributef(&attrs[1], 1, binding.binding, VK_FORMAT_R32G32_SFLOAT, 4);
  vertexStateInfo(&vertexState, 1, 2, &binding, attrs);
  depthStateInfo(&depthState, VK_FALSE);

  /*
	Create the pipeline shaders.
	*/
  createShaderStage(&shaderStages[0], VK_SHADER_STAGE_VERTEX_BIT, m_shaders.quad_vertex);
  createShaderStage(&shaderStages[1], VK_SHADER_STAGE_FRAGMENT_BIT, m_shaders.quad_fragment);

  /*
	create the graphics pipeline.
	*/
  graphicsPipelineCreate(&m_quad_pipeline, &m_pipeline_cache, m_quad_pipeline_layout, 2, shaderStages, &vertexState,
                         &inputState, &rasterState, &blendState, &multisampleState, &viewportState, &depthState,
                         &m_render_pass, 0, VK_PIPELINE_CREATE_DERIVATIVE_BIT, m_pipeline);

  /*----------------------------------------------------------
	Create the terrain pipeline.
	----------------------------------------------------------*/

  /*
	Reusing the create info struct from
	the scene pipeline, just update what
	we need.
	*/
  inputAssemblyStateInfo(&inputState, VK_PRIMITIVE_TOPOLOGY_PATCH_LIST);
  depthStateInfo(&depthState);
  rasterStateInfo(&rasterState, VK_POLYGON_MODE_FILL);

  /*
	create the graphics pipeline.
	*/
  createShaderStage(&shaderStages[0], VK_SHADER_STAGE_VERTEX_BIT, m_shaders.terrain_vertex);
  createShaderStage(&shaderStages[1], VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, m_shaders.terrain_tcs);
  createShaderStage(&shaderStages[2], VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, m_shaders.terrain_tes);
  createShaderStage(&shaderStages[3], VK_SHADER_STAGE_FRAGMENT_BIT, m_shaders.terrain_fragment);

  /*
	create the graphics pipeline.
	*/
  graphicsPipelineCreate(&m_terrain_pipeline, &m_pipeline_cache, m_terrain_pipeline_layout, 4, shaderStages,
                         &vertexState, &inputState, &rasterState, &blendState, &multisampleState, &viewportState,
                         &depthState, &m_render_pass, 0, VK_PIPELINE_CREATE_DERIVATIVE_BIT, m_pipeline);
}


void vkeGameRendererDynamic::initRenderPass()
{

  /*----------------------------------------------------------
	Define the render pass
	----------------------------------------------------------*/

  VkAttachmentDescription attachments[3];

  /*
	3 attachments:
	1 : Color attachment
	R8G8B8A8 UNORM
	4 samples per pixel
	2 : Depth stencil attachment
	D24S8_UNORM
	4 samples per pixel
	3 : Resolve attachment
	R8G8B8A8 UNORM
	1 sample per pixel
	*/
  attachmentDescription(&attachments[0], VK_FORMAT_R8G8B8A8_UNORM, getSamples(), VK_ATTACHMENT_LOAD_OP_CLEAR,
                        VK_ATTACHMENT_STORE_OP_STORE);
  depthStencilAttachmentDescription(&attachments[1], VK_FORMAT_D24_UNORM_S8_UINT, getSamples());
  attachmentDescription(&attachments[2], VK_FORMAT_R8G8B8A8_UNORM, VK_SAMPLE_COUNT_1_BIT,
                        VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE);

  /*
	Define the attachment referrences
	*/

  VkAttachmentReference color_reference   = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
  VkAttachmentReference resolve_reference = {2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
  VkAttachmentReference depth_reference   = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

  /*
	Define the only subpass
	*/
  VkSubpassDescription subpass[1];
  subpassDescription(&subpass[0], 1, &color_reference, &depth_reference, NULL);  // &resolve_reference);

  /*
	Create the render pass.
	*/
  renderPassCreate(&m_render_pass, 3, attachments, 1, subpass);  // , 1, &dep);
}

void vkeGameRendererDynamic::initFramebuffer(uint32_t inWidth, uint32_t inHeight)
{

  /*----------------------------------------------------------
	Create the framebuffer
	----------------------------------------------------------*/


  VulkanDC*                dc     = VulkanDC::Get();
  VulkanDC::Device*        device = dc->getDefaultDevice();
  VulkanDC::Device::Queue* queue  = dc->getDefaultQueue();

  const VkFormat depthFmt = VK_FORMAT_D24_UNORM_S8_UINT;
  const VkFormat colorFmt = VK_FORMAT_R8G8B8A8_UNORM;

  /*
	If framebuffers already exist, release them.
	*/
  if(m_framebuffers[0] != VK_NULL_HANDLE)
  {
    releaseFramebuffer();
  }

  /*
	Update the frame dimensions.
	*/
  m_width  = inWidth;
  m_height = inHeight;

  /*
	Specify usage for the frame buffer attachments.
	*/
  VkImageUsageFlags gBufferUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  /*
	Create the depth attachment image and image view.
	*/
  m_depth_attachment.format = depthFmt;
  imageCreateAndBind(&m_depth_attachment.image, &m_depth_attachment.memory, depthFmt, VK_IMAGE_TYPE_2D, m_width,
                     m_height, 1, 1, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                     VK_IMAGE_TILING_OPTIMAL, getSamples());
  imageViewCreate(&m_depth_attachment.view, m_depth_attachment.image, VK_IMAGE_VIEW_TYPE_2D, depthFmt, VK_IMAGE_ASPECT_DEPTH_BIT);

  /*
	Create the color attachment image and image view.
	*/
  m_color_attachment.format = colorFmt;
  imageCreateAndBind(&m_color_attachment.image, &m_color_attachment.memory, colorFmt, VK_IMAGE_TYPE_2D, m_width,
                     m_height, 1, 1, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                     gBufferUsage | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_OPTIMAL, getSamples());
  imageViewCreate(&m_color_attachment.view, m_color_attachment.image, VK_IMAGE_VIEW_TYPE_2D, colorFmt);

  /*
	Create the resolve attachment image and image view.
	*/
  for(uint32_t i = 0; i < 2; ++i)
  {
    m_resolve_attachment[i].format = colorFmt;
    imageCreateAndBind(&m_resolve_attachment[i].image, &m_resolve_attachment[i].memory, colorFmt, VK_IMAGE_TYPE_2D,
                       m_width, m_height, 1, 1, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                       gBufferUsage | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT);
    imageViewCreate(&m_resolve_attachment[i].view, m_resolve_attachment[i].image, VK_IMAGE_VIEW_TYPE_2D, colorFmt);
  }

  /* Start a small command buffer to set the layouts of the images to their correct values. */
  {
    VkCommandBufferAllocateInfo cmdAllocateInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    cmdAllocateInfo.commandBufferCount = 1;
    cmdAllocateInfo.commandPool        = m_primary_buffer_cmd_pool;
    cmdAllocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    VkCommandBuffer cmd                = VK_NULL_HANDLE;
    VKA_CHECK_ERROR(vkAllocateCommandBuffers(getDefaultDevice(), &cmdAllocateInfo, &cmd),
                    "Could not allocate temporary command buffer to set attachment layouts");
    VkCommandBufferBeginInfo cmdBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    cmdBeginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VKA_CHECK_ERROR(vkBeginCommandBuffer(cmd, &cmdBeginInfo), "Could not begin command buffer.");
    imageSetLayout(&cmd, m_depth_attachment.image, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                   VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    imageSetLayout(&cmd, m_color_attachment.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    imageSetLayout(&cmd, m_resolve_attachment[0].image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    imageSetLayout(&cmd, m_resolve_attachment[1].image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VKA_CHECK_ERROR(vkEndCommandBuffer(cmd), "Could not end command buffer.");
    VkSubmitInfo subInfo       = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    subInfo.commandBufferCount = 1;
    subInfo.pCommandBuffers    = &cmd;
    VkFence           fence;
    VkFenceCreateInfo fenceInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    VKA_CHECK_ERROR(vkCreateFence(getDefaultDevice(), &fenceInfo, NULL, &fence), "Could not create fence.\n");
    vkQueueSubmit(queue->getVKQueue(), 1, &subInfo, fence);
    vkWaitForFences(getDefaultDevice(), 1, &fence, VK_TRUE, ~0ULL);
    vkDestroyFence(getDefaultDevice(), fence, nullptr);
    vkFreeCommandBuffers(getDefaultDevice(), m_primary_buffer_cmd_pool, 1, &cmd);
  }

  /*
	Setup the framebuffer create info struct.
	*/
  for(uint32_t i = 0; i < 2; ++i)
  {
    VkImageView             views[] = {m_color_attachment.view, m_depth_attachment.view, m_resolve_attachment[i].view};
    VkFramebufferCreateInfo fbInfo  = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    fbInfo.renderPass               = m_render_pass;
    fbInfo.attachmentCount          = 3;
    fbInfo.pAttachments             = views;
    fbInfo.width                    = m_width;
    fbInfo.height                   = m_height;
    fbInfo.layers                   = 1;

    /*
		Create 2 framebuffers for ping pong.
		*/
    VKA_CHECK_ERROR(vkCreateFramebuffer(device->getVKDevice(), &fbInfo, NULL, &m_framebuffers[i]), "Could not create framebuffer.\n");
  }
}


void vkeGameRendererDynamic::initTerrainCommand()
{
  VulkanDC*                dc     = VulkanDC::Get();
  VulkanDC::Device*        device = dc->getDefaultDevice();
  VulkanDC::Device::Queue* queue  = dc->getDefaultQueue();

  for(uint32_t i = 0; i < 2; ++i)
  {

    if(m_terrain_command[i] != VK_NULL_HANDLE)
    {
      vkFreeCommandBuffers(device->getVKDevice(), queue->getCommandPool(), 1, &m_terrain_command[i]);
      m_terrain_command[i] = VK_NULL_HANDLE;
    }

    {

      VkCommandBufferAllocateInfo cmdBufInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
      cmdBufInfo.commandBufferCount          = 1;
      cmdBufInfo.commandPool                 = queue->getCommandPool();
      cmdBufInfo.level                       = VK_COMMAND_BUFFER_LEVEL_SECONDARY;

      VKA_CHECK_ERROR(vkAllocateCommandBuffers(device->getVKDevice(), &cmdBufInfo, &m_terrain_command[i]),
                      "vkAllocateCommandBuffers failed");

      VkCommandBufferInheritanceInfo cmdInheritanceInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO};
      cmdInheritanceInfo.renderPass                     = m_render_pass;
      cmdInheritanceInfo.subpass                        = 0;
      VkCommandBufferBeginInfo cmdBeginInfo             = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
      cmdBeginInfo.flags                                = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
      cmdBeginInfo.pInheritanceInfo                     = &cmdInheritanceInfo;

      VKA_CHECK_ERROR(vkBeginCommandBuffer(m_terrain_command[i], &cmdBeginInfo), "vkBeginCommandBuffer failed");

      // TODO: Switch to using VkCommandBufferInheritanceViewportScissorInfoNV here
      setDefaultViewportAndScissor(m_terrain_command[i], m_width, m_height);
      vkCmdBindPipeline(m_terrain_command[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_quad_pipeline);
      vkCmdBindDescriptorSets(m_terrain_command[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_quad_pipeline_layout, 0, 1,
                              &m_quad_descriptor_set, 0, NULL);
      m_screen_quad.bind(&m_terrain_command[i]);
      m_screen_quad.draw(&m_terrain_command[i]);

      vkCmdBindPipeline(m_terrain_command[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_terrain_pipeline);

      vkCmdBindDescriptorSets(m_terrain_command[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_terrain_pipeline_layout, 0, 1,
                              &m_terrain_descriptor_set, 0, NULL);

      m_terrain_quad.bind(&m_terrain_command[i]);
      m_terrain_quad.draw(&m_terrain_command[i]); /**/

      vkEndCommandBuffer(m_terrain_command[i]);
    }
  }
}

void vkeGameRendererDynamic::initCamera()
{
  nvmath::vec4f zp(0.0, 0.0, 0.0, 1.0);

  m_camera = new VkeCamera(1, -20.0, -1.0, -8.0);
  m_camera->lookAt(zp);
  m_camera->update();
  m_light = new VkeCamera(2, -6, -6, -10);
  m_light->lookAt(zp);
  m_light->update();
}

void vkeGameRendererDynamic::initDrawCalls()
{
  m_max_draw_calls = 1;
  m_draw_calls.resize(m_max_draw_calls);
  for(uint32_t i = 0; i < m_max_draw_calls; ++i)
  {
    m_draw_calls[i] = std::make_unique<VkeDrawCall>(this);
  }
}

void vkeGameRendererDynamic::generateDrawCommands()
{

  //Start generating draw commands.

  VkClearValue clearValues[3];

  colorClearValues(&clearValues[0], 1.0, 1.0, 1.0);
  depthStencilClearValues(&clearValues[1]);  //default#
  colorClearValues(&clearValues[2], 0.0, 0.0, 0.0);

  /*
	Dispatch threads to create the secondary
	command buffers.
	*/
  m_calls_generated = 0;
  for(uint32_t i = 0; i < m_max_draw_calls; ++i)
  {
    m_draw_calls[i]->initDrawCommands(uint32_t(m_node_data->count()), m_current_buffer_index, m_render_pass, m_width, m_height);
  }

  /*
	Begin setting up the primary command buffer.
	*/
  VkCommandBufferBeginInfo cmdBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  cmdBeginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  VkCommandBuffer cmd = m_primary_commands[m_current_buffer_index];
  VKA_CHECK_ERROR(vkResetCommandBuffer(cmd, 0), "Could not reset primary command buffer");
  VKA_CHECK_ERROR(vkBeginCommandBuffer(cmd, &cmdBeginInfo), "Could not begin primary command buffer.\n");

  size_t       cnt = m_node_data->count();
  VkDeviceSize sz  = (sizeof(VkeNodeUniform) * cnt) + (m_instance_count * 64);
  vkCmdUpdateBuffer(cmd, m_uniforms_buffer, 0, sz, (const uint32_t*)m_uniforms_local);
  m_camera->updateCameraCmd(cmd);


  renderPassBegin(&cmd, m_render_pass, m_framebuffers[m_current_buffer_index], 0, 0, m_width, m_height, clearValues, 3,
                  VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);


  /*
	Wait here until the secondary commands are ready.
	*/

  VkCommandBuffer secondaryCommands[11];
  secondaryCommands[0] = m_terrain_command[m_current_buffer_index];
  for(uint32_t i = 0; i < m_max_draw_calls; ++i)
  {
    secondaryCommands[i + 1] = m_draw_calls[i]->getDrawCommand(m_current_buffer_index);
  }


  vkCmdExecuteCommands(m_primary_commands[m_current_buffer_index], 1 + m_max_draw_calls, secondaryCommands);

  vkCmdEndRenderPass(m_primary_commands[m_current_buffer_index]);

  VkImageResolve blitInfo;
  blitInfo.srcOffset.x                   = 0;
  blitInfo.srcOffset.y                   = 0;
  blitInfo.srcOffset.z                   = 0;
  blitInfo.dstOffset.x                   = 0;
  blitInfo.dstOffset.y                   = 0;
  blitInfo.dstOffset.z                   = 0;
  blitInfo.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
  blitInfo.srcSubresource.mipLevel       = 0;
  blitInfo.srcSubresource.baseArrayLayer = 0;
  blitInfo.srcSubresource.layerCount     = 1;
  blitInfo.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
  blitInfo.dstSubresource.mipLevel       = 0;
  blitInfo.dstSubresource.baseArrayLayer = 0;
  blitInfo.dstSubresource.layerCount     = 1;
  blitInfo.extent.width                  = m_width;
  blitInfo.extent.height                 = m_height;
  blitInfo.extent.depth                  = 1;

  imageSetLayout(&cmd, m_color_attachment.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  imageSetLayout(&cmd, m_resolve_attachment[m_current_buffer_index].image, VK_IMAGE_ASPECT_COLOR_BIT,
                 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  vkCmdResolveImage(m_primary_commands[m_current_buffer_index], m_color_attachment.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    m_resolve_attachment[m_current_buffer_index].image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blitInfo);

  imageSetLayout(&cmd, m_color_attachment.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  imageSetLayout(&cmd, m_resolve_attachment[m_current_buffer_index].image, VK_IMAGE_ASPECT_COLOR_BIT,
                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  VKA_CHECK_ERROR(vkEndCommandBuffer(m_primary_commands[m_current_buffer_index]), "Could not end command buffer for draw command.\n");
}


void vkeGameRendererDynamic::initShaders(nvvk::ShaderModuleManager& inShaderModuleManager)
{
  VulkanAppContext* ctxt = VulkanAppContext::GetInstance();

  m_shaders.scene_vertex   = inShaderModuleManager.get(ctxt->getModuleIDs().scene_vs);
  m_shaders.scene_fragment = inShaderModuleManager.get(ctxt->getModuleIDs().scene_fs);

  m_shaders.quad_vertex   = inShaderModuleManager.get(ctxt->getModuleIDs().scene_quad_vs);
  m_shaders.quad_fragment = inShaderModuleManager.get(ctxt->getModuleIDs().scene_quad_fs);

  m_shaders.terrain_vertex   = inShaderModuleManager.get(ctxt->getModuleIDs().scene_terrain_vs);
  m_shaders.terrain_fragment = inShaderModuleManager.get(ctxt->getModuleIDs().scene_terrain_fs);
  m_shaders.terrain_tcs      = inShaderModuleManager.get(ctxt->getModuleIDs().scene_terrain_tcs);
  m_shaders.terrain_tes      = inShaderModuleManager.get(ctxt->getModuleIDs().scene_terrain_tes);
}


void vkeGameRendererDynamic::releaseFramebuffer()
{


  VulkanDC*         dc     = VulkanDC::Get();
  VulkanDC::Device* device = dc->getDefaultDevice();
  device->waitIdle();

  vkDestroyImageView(device->getVKDevice(), m_depth_attachment.view, NULL);
  vkDestroyImageView(device->getVKDevice(), m_color_attachment.view, NULL);
  vkDestroyImageView(device->getVKDevice(), m_resolve_attachment[0].view, NULL);
  vkDestroyImageView(device->getVKDevice(), m_resolve_attachment[1].view, NULL);

  m_depth_attachment.view      = NULL;
  m_color_attachment.view      = NULL;
  m_resolve_attachment[0].view = NULL;
  m_resolve_attachment[1].view = NULL;

  vkDestroyImage(device->getVKDevice(), m_depth_attachment.image, NULL);
  vkDestroyImage(device->getVKDevice(), m_color_attachment.image, NULL);
  vkDestroyImage(device->getVKDevice(), m_resolve_attachment[0].image, NULL);
  vkDestroyImage(device->getVKDevice(), m_resolve_attachment[1].image, NULL);

  m_depth_attachment.image      = NULL;
  m_color_attachment.image      = NULL;
  m_resolve_attachment[0].image = NULL;
  m_resolve_attachment[1].image = NULL;

  vkFreeMemory(device->getVKDevice(), m_depth_attachment.memory, NULL);
  vkFreeMemory(device->getVKDevice(), m_color_attachment.memory, NULL);
  vkFreeMemory(device->getVKDevice(), m_resolve_attachment[0].memory, NULL);
  vkFreeMemory(device->getVKDevice(), m_resolve_attachment[1].memory, NULL);

  m_depth_attachment.memory      = NULL;
  m_color_attachment.memory      = NULL;
  m_resolve_attachment[0].memory = NULL;
  m_resolve_attachment[1].memory = NULL;

  vkDestroyFramebuffer(device->getVKDevice(), m_framebuffers[0], NULL);
  vkDestroyFramebuffer(device->getVKDevice(), m_framebuffers[1], NULL);
  m_framebuffers[0] = NULL;
  m_framebuffers[1] = NULL;
}

void vkeGameRendererDynamic::setCameraLookAt(nvmath::mat4f& inMat)
{
  m_camera->setLookAtMatrix(inMat);
}
