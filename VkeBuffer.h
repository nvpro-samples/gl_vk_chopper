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

#ifndef __H_VKE_BUFFER_
#define __H_VKE_BUFFER_

#pragma once


#include "VkeCreateUtils.h"
#include "vkaUtils.h"
#include <vulkan/vulkan.h>


#ifndef INIT_COMMAND_ID
#define INIT_COMMAND_ID 1
#endif

template <typename T>
class VkeBuffer
{
protected:
  size_t m_index;


public:
  VkeBuffer()
      : m_backing_store(NULL)
      , m_use_staging(false)
  {
  }

  ~VkeBuffer() {}

  virtual void bind(VkCommandBuffer* inBuffer) = 0;

  struct Data
  {
    VkBuffer               buffer;
    VkDeviceMemory         memory;
    VkBufferView           view;
    VkDescriptorBufferInfo descriptor;
  };

  void deleteBackingStore()
  {
    if(m_backing_store == NULL)
      return;
    free(m_backing_store);
  }

  void initBackingStore(size_t inSize)
  {
    m_data_size     = inSize;
    m_backing_store = (T*)malloc(m_data_size);
  }

  T*& getBackingStore() { return m_backing_store; }

  Data& getData() { return m_data; }

  VkDescriptorBufferInfo& getDescriptor() { return m_data.descriptor; }

  virtual void updateVKBufferData(VkCommandBuffer* inBuffer = NULL, bool doStage = true)
  {
    if(m_backing_store == NULL)
      return;
    uint8_t* vData = NULL;

    Data& useData = (m_use_staging) ? m_staging : m_data;

    VKA_CHECK_ERROR(vkMapMemory(getDefaultDevice(), useData.memory, 0, VK_WHOLE_SIZE, 0, (void**)&vData),
                    "Could not map buffer memory.\n");

    memcpy(vData, (const void*)&(m_backing_store[0]), m_data_size);

    vkUnmapMemory(getDefaultDevice(), useData.memory);

    if(m_use_staging)
    {


      if(doStage)
        stageCopy(inBuffer);
    }
  }

  virtual void stageCopy(VkCommandBuffer* inBuffer = NULL)
  {
    VulkanDC::Device::Queue::Name            queueName = "DEFAULT_GRAPHICS_QUEUE";
    VulkanDC::Device::Queue::CommandBufferID cmdID     = INIT_COMMAND_ID + 300;
    VulkanDC*                                dc        = VulkanDC::Get();
    VulkanDC::Device*                        device    = dc->getDefaultDevice();
    VulkanDC::Device::Queue*                 queue     = device->getQueue(queueName);
    VkCommandBuffer                          cmd       = VK_NULL_HANDLE;

    VkBufferCopy bufCpy;
    bufCpy.srcOffset = 0;
    bufCpy.dstOffset = 0;
    bufCpy.size      = m_data_size;

    if(inBuffer)
    {
      cmd = *inBuffer;
    }
    else
    {

      queue->beginCommandBuffer(cmdID, &cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    }


    vkCmdCopyBuffer(cmd, m_staging.buffer, m_data.buffer, 1, &bufCpy);
  }

  virtual void initVKBufferData(VkBuffer& inBuffer, size_t inCount = 1)
  {

    VkDescriptorBufferInfo bufInfo;
    bufInfo.buffer = inBuffer;
    bufInfo.range  = m_data_size * inCount;
    bufInfo.offset = 0;  // m_index*m_data_size;

    m_data.descriptor = bufInfo;
  }

  virtual void initVKBufferData(bool doUpdate = true)
  {
    if(m_backing_store == NULL)
      return;

    if(m_use_staging)
    {
      m_memory_flags                         = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      VkMemoryPropertyFlagBits stageMemFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
      VkBufferUsageFlagBits    stageBufFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

      bufferCreate(&m_staging.buffer, m_data_size, stageBufFlags);
      bufferAlloc(&m_staging.buffer, &m_staging.memory, stageMemFlags);
    }


    bufferCreate(&m_data.buffer, m_data_size, m_usage_flags);
    bufferAlloc(&m_data.buffer, &m_data.memory, m_memory_flags);


    if(doUpdate)
      updateVKBufferData();


    // If not used as uniform texel buffer or storage texel buffer ,no view is necessary
    // bufferViewCreate(&m_data.buffer, &m_data.view, m_data_size);

    VkDescriptorBufferInfo bufInfo;
    bufInfo.buffer = m_data.buffer;
    bufInfo.range  = m_data_size;
    bufInfo.offset = 0;


    m_data.descriptor = bufInfo;
  }


protected:
  size_t m_data_size;
  T*     m_backing_store;
  Data   m_data;
  Data   m_staging;

  bool m_use_staging;

  VkBufferUsageFlagBits    m_usage_flags;
  VkMemoryPropertyFlagBits m_memory_flags;
};

#endif
