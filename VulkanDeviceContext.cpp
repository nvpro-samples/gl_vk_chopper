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


#include "VulkanDeviceContext.h"
#include "VkeCreateUtils.h"
#include "vkaUtils.h"
#include <iostream>
VulkanDC::Device::Queue::Queue() {}

VulkanDC::Device::Queue::Queue(VulkanDC::Device::Queue::Name& inName, VulkanDC::Device::Queue::NodeID inNodeID)
    : m_name(inName)
    , m_node_id(inNodeID)
    , m_queue(VK_NULL_HANDLE)
{
}

VulkanDC::Device::Queue::Queue(const VulkanDC::Device::Queue& inOther)
{
  m_name    = inOther.m_name;
  m_node_id = inOther.m_node_id;
  m_queue   = inOther.m_queue;
}

void VulkanDC::Device::Queue::initQueue(VulkanDC::Device* inDevice)
{
  //TODO - currently this is just queue index 0.
  //need to work out how best to use this.
  m_device        = inDevice;
  VkDevice device = m_device->getVKDevice();
  vkGetDeviceQueue(device, m_node_id, 0, &m_queue);

  VkCommandPoolCreateInfo cmdPoolInfo;
  commandPoolCreateInfo(&cmdPoolInfo, m_node_id, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
  VKA_CHECK_ERROR(vkCreateCommandPool(device, &cmdPoolInfo, NULL, &m_command_pool), "Could not create command pool.");
}

void VulkanDC::Device::Queue::createCommandBuffer(VkCommandBuffer* outBuffer, VkCommandBufferLevel inLevel)
{

  VkCommandBufferAllocateInfo cmd;
  memset(&cmd, 0, sizeof(cmd));
  cmd.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmd.commandPool        = m_command_pool;
  cmd.level              = inLevel;
  cmd.commandBufferCount = 1;

  vkAllocateCommandBuffers(m_device->getVKDevice(), &cmd, outBuffer);
}

bool VulkanDC::Device::Queue::createAndCacheCommandBuffer(const VulkanDC::Device::Queue::CommandBufferID& inID,
                                                          VkCommandBuffer*                                outBuffer,
                                                          VkCommandBufferLevel                            inLevel)
{

  bool isNew = true;

  CommandBufferMap::iterator itr = m_cached_buffers.find(inID);

  if(itr == m_cached_buffers.end() || itr->second == VK_NULL_HANDLE)
  {
    createCommandBuffer(outBuffer, inLevel);
    m_cached_buffers[inID] = *outBuffer;
  }
  else
  {
    *outBuffer = itr->second;
    isNew      = false;
  }
  return isNew;
}

void VulkanDC::Device::Queue::beginCommandBuffer(VkCommandBuffer*          inBuffer,
                                                 VkCommandBufferUsageFlags inFlags,
                                                 VkCommandBufferLevel      inLevel,
                                                 VkFramebuffer             inFBO,
                                                 VkRenderPass              inRP)
{

  if(*inBuffer == VK_NULL_HANDLE)
  {
    createCommandBuffer(inBuffer, inLevel);
  }

  VkCommandBufferBeginInfo cmdBufInfo;
  memset(&cmdBufInfo, 0, sizeof(cmdBufInfo));

  cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmdBufInfo.flags = inFlags;

  VKA_CHECK_ERROR(vkBeginCommandBuffer(*inBuffer, &cmdBufInfo), "Could not begin command buffer.");
}

void VulkanDC::Device::Queue::beginCommandBuffer(VulkanDC::Device::Queue::CommandBufferID& inID,
                                                 VkCommandBuffer*                          outBuffer,
                                                 VkCommandBufferUsageFlags                 inFlags,
                                                 VkCommandBufferLevel                      inLevel,
                                                 VkFramebuffer                             inFBO,
                                                 VkRenderPass                              inRP)
{

  if(!createAndCacheCommandBuffer(inID, outBuffer, inLevel))
    return;


  VkCommandBufferBeginInfo cmdBufInfo;
  memset(&cmdBufInfo, 0, sizeof(cmdBufInfo));

  cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmdBufInfo.flags = inFlags;

  VKA_CHECK_ERROR(vkBeginCommandBuffer(*outBuffer, &cmdBufInfo), "Could not begin command buffer.");
}

VkCommandBuffer VulkanDC::Device::Queue::getCachedCommandBuffer(VulkanDC::Device::Queue::CommandBufferID& inID)
{
  VulkanDC::Device::Queue::CommandBufferMap::iterator itr = m_cached_buffers.find(inID);
  if(itr == m_cached_buffers.end())
    return VK_NULL_HANDLE;

  return itr->second;
}

void VulkanDC::Device::Queue::resetCommandBuffer(VulkanDC::Device::Queue::CommandBufferID& inID)
{
  VkCommandBuffer cmd = getCachedCommandBuffer(inID);
  if(cmd == VK_NULL_HANDLE)
    return;

  VKA_CHECK_ERROR(vkResetCommandBuffer(cmd, 0), "Could not reset command buffer.");

  VkCommandBufferBeginInfo cmdBufInfo;
  memset(&cmdBufInfo, 0, sizeof(cmdBufInfo));

  cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmdBufInfo.flags = 0;

  VKA_CHECK_ERROR(vkBeginCommandBuffer(cmd, &cmdBufInfo), "Could not begin command buffer.");
}

void VulkanDC::Device::Queue::flushPersistentBuffer(VulkanDC::Device::Queue::CommandBufferID& inID, VkFence* inFence)
{

  VkCommandBuffer cmd = getCachedCommandBuffer(inID);
  if(cmd == VK_NULL_HANDLE)
    return;

  VKA_CHECK_ERROR(vkEndCommandBuffer(cmd), "Could not end command buffer.");
  VkCommandBuffer bufs[]   = {cmd};
  VkFence         theFence = VK_NULL_HANDLE;
  if(inFence)
  {
    theFence = *inFence;
  }

  VkSubmitInfo subInfo;
  memset(&subInfo, 0, sizeof(subInfo));
  subInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  subInfo.commandBufferCount = 1;
  subInfo.pCommandBuffers    = bufs;

  VKA_CHECK_ERROR(vkQueueSubmit(m_queue, 1, &subInfo, theFence), "Could not submit queue.");
}

void VulkanDC::Device::Queue::flushCommandBuffer(VulkanDC::Device::Queue::CommandBufferID& inID, VkFence* inFence, bool inDestroy)
{

  VkCommandBuffer cmd = getCachedCommandBuffer(inID);
  if(cmd == VK_NULL_HANDLE)
    return;

  VKA_CHECK_ERROR(vkEndCommandBuffer(cmd), "Could not end command buffer.");

  VkCommandBuffer bufs[]       = {cmd};
  VkFence         theFence     = VK_NULL_HANDLE;
  bool            createdFence = false;
  if(inFence)
  {
    theFence = *inFence;
  }
  else if(inDestroy)
  {
    VkFenceCreateInfo fenceInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    vkCreateFence(m_device->getVKDevice(), &fenceInfo, NULL, &theFence);
    createdFence = true;
  }

  VkSubmitInfo subInfo;
  memset(&subInfo, 0, sizeof(subInfo));
  subInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  subInfo.commandBufferCount = 1;
  subInfo.pCommandBuffers    = bufs;

  VKA_CHECK_ERROR(vkQueueSubmit(m_queue, 1, &subInfo, theFence), "Could not submit queue.");

  if(inDestroy)
  {
    VKA_CHECK_ERROR(vkWaitForFences(m_device->getVKDevice(), 1, &theFence, VK_TRUE, 100000000000), "Could not wait for idle queue.");

    destroyCachedCommandBuffer(inID);

    if(createdFence)
    {
      vkDestroyFence(m_device->getVKDevice(), theFence, NULL);
    }
  }
}

void VulkanDC::Device::Queue::destroyCachedCommandBuffer(VulkanDC::Device::Queue::CommandBufferID& inID)
{
  VkCommandBuffer cmd = getCachedCommandBuffer(inID);
  if(cmd == VK_NULL_HANDLE)
    return;
  vkFreeCommandBuffers(m_device->getVKDevice(), m_command_pool, 1, &cmd);

  m_cached_buffers[inID] = VK_NULL_HANDLE;
}

VulkanDC::Device::Queue::~Queue() {}


VulkanDC::Device::Device() {}
VulkanDC::Device::Device(const VkPhysicalDevice& inDevice)
    : m_physical_device(inDevice)
    , m_device(VK_NULL_HANDLE)
{
  initDevice();
}

VulkanDC::Device::Queue* VulkanDC::Device::getQueue(VulkanDC::Device::Queue::Name& inName)
{
  VulkanDC::Device::Queue::Map::iterator itr;
  itr = m_queues.find(inName);

  if(itr == m_queues.end())
    return NULL;
  return itr->second;
}

uint32_t VulkanDC::Device::getBestMemoryType(uint32_t inType, VkFlags inFlags)
{

  for(uint32_t i = 0; i < 32; ++i)
  {

    if((inType & 1) == 1)
    {
      if((m_memory_properties.memoryTypes[i].propertyFlags & inFlags) == inFlags)
      {
        return i;
      }
    }
    inType >>= 1;
  }

  return 0;
}


void VulkanDC::Device::waitIdle()
{
  VKA_CHECK_ERROR(vkDeviceWaitIdle(m_device), "Could not wait for device idle.");
}

void VulkanDC::Device::initDevice()
{

  VkLayerProperties* layerProps;
  uint32_t           propCount;
  VKA_CHECK_ERROR(vkEnumerateDeviceLayerProperties(m_physical_device, &propCount, NULL), "Could not enumerate device layer properties.");
  layerProps = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * propCount);
  VKA_CHECK_ERROR(vkEnumerateDeviceLayerProperties(m_physical_device, &propCount, layerProps),
                  "Could not collect device layer properties.");

  vkGetPhysicalDeviceProperties(m_physical_device, &m_device_properties);

  m_queue_count = 4;

  vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &m_queue_count, NULL);

  m_queue_properties.resize(m_queue_count);

  vkGetPhysicalDeviceMemoryProperties(m_physical_device, &m_memory_properties);

  vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &m_queue_count, m_queue_properties.data());

  std::vector<float>      priorities(m_queue_count, 1.0f);
  VkDeviceQueueCreateInfo queueInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
  queueInfo.queueCount              = m_queue_count;
  queueInfo.queueFamilyIndex        = 0;
  queueInfo.pQueuePriorities        = priorities.data();

  VkPhysicalDeviceFeatures requiredFeatures{};
  requiredFeatures.geometryShader     = VK_TRUE;
  requiredFeatures.tessellationShader = VK_TRUE;
  requiredFeatures.depthClamp         = VK_TRUE;
  requiredFeatures.multiDrawIndirect  = VK_TRUE;

  deviceCreate(&m_device, &m_physical_device, m_queue_count, &queueInfo, m_extension_count, m_extension_names, 0,
               nullptr, &requiredFeatures);

  LOGI("Device ID : %p\n", m_device);
}


VulkanDC::Device::~Device() {}

VulkanDC* VulkanDC::Get()
{
  static VulkanDC* outContext = NULL;

  if(!outContext)
  {
    outContext = new VulkanDC();
  }
  return outContext;
}

VulkanDC::Device* VulkanDC::getDevice(uint32_t inDeviceID)
{
  return m_devices[inDeviceID];
}


VulkanDC::Device::Queue* VulkanDC::Device::newQueue(VulkanDC::Device::Queue::Name& inName, VulkanDC::Device::Queue::NodeID& inID)
{
  VulkanDC::Device::Queue* outQueue = new VulkanDC::Device::Queue(inName, inID);
  outQueue->initQueue(this);
  m_queues[inName] = outQueue;
  return outQueue;
}


VulkanDC::VulkanDC()
    : m_has_instance(false)
    , m_default_device(NULL)
{
}

void VulkanDC::initDC(const VkInstance& inInstance)
{
  if(inInstance == VK_NULL_HANDLE)
    return;
  m_vk_instance  = inInstance;
  m_has_instance = true;

  initDevices();
}

uint32_t VulkanDC::getQueueCount(uint32_t inDeviceID)
{
  VulkanDC::Device* device = getDevice(inDeviceID);
  if(!device)
    return 0;
  return device->getQueueCount();
}


void VulkanDC::initDevices()
{
  if(!m_has_instance)
    return;

  LOGI("Initialising Devices\n");

  m_device_count = 0;

  VKA_CHECK_ERROR(vkEnumeratePhysicalDevices(m_vk_instance, &m_device_count, NULL), "Could not get physical device count.");

  LOGI("Found %u Devices\n", m_device_count);

  m_physical_devices.resize(m_device_count);

  VKA_CHECK_ERROR(vkEnumeratePhysicalDevices(m_vk_instance, &m_device_count, m_physical_devices.data()),
                  "Could not get physical devices.");

  for(uint32_t i = 0; i < m_device_count; ++i)
  {
    newDevice(m_physical_devices[i]);
  }
}

VulkanDC::Device::Queue* VulkanDC::Device::createGraphicsQueue(VulkanDC::Device::Queue::Name&  inName,
                                                               VulkanDC::Device::Queue::NodeID inID)
{

  return newQueue(inName, inID);
}

VulkanDC::Device::Queue* VulkanDC::getQueueForGraphics(VulkanDC::Device::Queue::Name& queueName, VkFormat& outFormat)
{
  VulkanDC::Device::Queue* queue = NULL;

  for(uint32_t dc = 0; dc < m_device_count; ++dc)
  {
    VulkanDC::Device* device = getDevice(dc);

    queue = device->createGraphicsQueue(queueName);
    if(queue)
    {

      outFormat = VK_FORMAT_B8G8R8A8_UNORM;
      return queue;
    }
  }

  return NULL;
}

VulkanDC::Device* VulkanDC::newDevice(const VkPhysicalDevice& inDevice)
{

  VulkanDC::Device* outDevice = new VulkanDC::Device(inDevice);
  m_default_device            = outDevice;
  m_devices.push_back(outDevice);
  return outDevice;
}

VulkanDC::~VulkanDC() {}
