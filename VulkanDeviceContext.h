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

#pragma once

#include <vulkan/vulkan.h>

#include <map>
#include <string>
#include <vector>

class VulkanDC
{
public:
  ~VulkanDC();

  static VulkanDC* Get();


  class Device
  {
  public:
    typedef std::string Name;


    Device();
    Device(const VkPhysicalDevice& inDevice);
    ~Device();

    uint32_t getBestMemoryType(uint32_t inType, VkFlags inFlags);


    class Queue
    {
    public:
      typedef std::string                                Name;
      typedef uint32_t                                   NodeID;
      typedef std::map<Name, Queue*>                     Map;
      typedef uint32_t                                   CommandBufferID;
      typedef std::map<CommandBufferID, VkCommandBuffer> CommandBufferMap;

      Queue();
      Queue(Name& inName, NodeID inNodeID);
      Queue(const Queue& inOther);
      ~Queue();

      void initQueue(VulkanDC::Device* inDevice);

      inline VulkanDC::Device* getDevice() const { return m_device; }
      inline VkCommandPool&    getCommandPool() { return m_command_pool; }

      void createCommandBuffer(VkCommandBuffer* outBuffer, VkCommandBufferLevel inLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

      bool createAndCacheCommandBuffer(const CommandBufferID& inID,
                                       VkCommandBuffer*       outBuffer,
                                       VkCommandBufferLevel   inlevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

      void beginCommandBuffer(VkCommandBuffer*          inBuffer,
                              VkCommandBufferUsageFlags inFlags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                              VkCommandBufferLevel      inLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                              VkFramebuffer             inFBO   = VK_NULL_HANDLE,
                              VkRenderPass              inRP    = VK_NULL_HANDLE

      );

      void beginCommandBuffer(CommandBufferID&          inID,
                              VkCommandBuffer*          outBuffer,
                              VkCommandBufferUsageFlags inFlags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                              VkCommandBufferLevel      inLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                              VkFramebuffer             inFBO   = VK_NULL_HANDLE,
                              VkRenderPass              inRP    = VK_NULL_HANDLE);

      void resetCommandBuffer(CommandBufferID& inID);
      void flushCommandBuffer(CommandBufferID& inID, VkFence* inFence = NULL, bool inDestroy = true);
      void flushPersistentBuffer(CommandBufferID& inID, VkFence* inFence = NULL);

      VkCommandBuffer getCachedCommandBuffer(CommandBufferID& inID);
      void            destroyCachedCommandBuffer(CommandBufferID& inID);

      inline VkQueue& getVKQueue() { return m_queue; }


    private:
      Name              m_name;
      NodeID            m_node_id = 0;
      VkQueue           m_queue   = nullptr;
      VulkanDC::Device* m_device  = nullptr;

      VkCommandPool    m_command_pool = VK_NULL_HANDLE;
      CommandBufferMap m_cached_buffers;
    };

    typedef std::vector<Device*> List;

    inline VkDevice& getVKDevice() { return m_device; }

    inline uint32_t getQueueCount() const { return m_queue_count; }

    VulkanDC::Device::Queue* getQueue(VulkanDC::Device::Queue::Name& inName);

    VulkanDC::Device::Queue* createGraphicsQueue(VulkanDC::Device::Queue::Name& inName, VulkanDC::Device::Queue::NodeID inID = 0);

    Queue* newQueue(Queue::Name& inName, Queue::NodeID& inNodeID);


    void waitIdle();

  private:
    VkPhysicalDeviceProperties           m_device_properties{};
    std::vector<VkQueueFamilyProperties> m_queue_properties;
    VkPhysicalDeviceMemoryProperties     m_memory_properties{};

    uint32_t         m_queue_count = 0;
    Queue::Map       m_queues;
    VkPhysicalDevice m_physical_device = nullptr;
    VkDevice         m_device          = nullptr;
    Name             m_name;


    char*    m_extension_names[64]{};
    uint32_t m_extension_count = 0;

    void initDevice();
  };


  void initDC(const VkInstance& inInstance);

  uint32_t        getQueueCount(uint32_t inDeviceID = 0);
  inline uint32_t getDeviceCount() const { return m_device_count; }

  VulkanDC::Device* getDevice(uint32_t inDeviceID = 0);

  VulkanDC::Device*        getDefaultDevice() { return m_default_device; }
  VulkanDC::Device::Queue* getDefaultQueue() { return m_default_queue; }
  void                     setDefaultDevice(VulkanDC::Device* inDevice) { m_default_device = inDevice; }

  void setDefaultQueue(VulkanDC::Device::Queue* inQueue) { m_default_queue = inQueue; }

  VulkanDC::Device::Queue* getQueueForGraphics(VulkanDC::Device::Queue::Name& queueName, VkFormat& outFormat);


private:
  VulkanDC();


  bool                          m_has_instance = false;
  VkInstance                    m_vk_instance  = nullptr;
  std::vector<VkPhysicalDevice> m_physical_devices;
  uint32_t                      m_device_count = 0;

  Device::List m_devices;

  Device*        m_default_device = nullptr;
  Device::Queue* m_default_queue  = nullptr;

  void initDevices();

  Device* newDevice(const VkPhysicalDevice& inDevice);
};
