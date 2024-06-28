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

#include <include_gl.h>

#include "fileformats/nv_dds.h"
#include "fileformats/texture_formats.h"
#include "nvh/nvprint.hpp"
#include "nvpwindow.hpp"
#include "vkaUtils.h"
#include "VkeCreateUtils.h"
#include "VkeTexture.h"

#ifndef INIT_COMMAND_ID
#define INIT_COMMAND_ID 1
#endif

VkeTexture::VkeTexture() {}

VkeTexture::VkeTexture(const ID& inID)
    : m_id(inID)
{
}

VkeTexture::~VkeTexture() {}

void VkeTexture::loadDDSTextureFile(const char* inFile)
{
  std::vector<std::string> searchPaths;
  searchPaths.push_back(std::string("."));
  searchPaths.push_back(std::string("./resources_" PROJECT_NAME));
  searchPaths.push_back(std::string(PROJECT_NAME) + std::string("/images"));
  searchPaths.push_back(NVPSystem::exePath() + std::string(PROJECT_RELDIRECTORY) + std::string("/images"));

  nv_dds::Image         ddsImage;
  nv_dds::ErrorWithText ddsError;

  std::string filePath;
  for(uint32_t i = 0; i < searchPaths.size(); ++i)
  {
    filePath = searchPaths[i] + "/" + std::string(inFile);
    ddsError = ddsImage.readFromFile(filePath.c_str(), {});
    if(!ddsError.has_value())
    {
      break;
    }
  }

  if(ddsError.has_value())
  {
    LOGE("Could not load texture image %s; the last error message was: %s\n", inFile, ddsError.value().c_str());
    exit(1);
  }
  else
  {
    LOGOK("Loaded texture image %s\n", filePath.c_str());
  }

  VkFormat vkFmt = texture_formats::dxgiToVulkan(ddsImage.dxgiFormat);
  if(VK_FORMAT_UNDEFINED == vkFmt)
  {
    LOGE("Could not determine a corresponding VkFormat for DXGI format %u.", ddsImage.dxgiFormat);
    exit(1);
  }

  VulkanDC::Device::Queue::Name            queueName = "DEFAULT_GRAPHICS_QUEUE";
  VulkanDC::Device::Queue::CommandBufferID cmdID     = INIT_COMMAND_ID + 200;
  VulkanDC*                                dc        = VulkanDC::Get();
  VulkanDC::Device*                        device    = dc->getDefaultDevice();
  VulkanDC::Device::Queue*                 queue     = device->getQueue(queueName);

  m_width                 = ddsImage.getWidth(0);
  m_height                = ddsImage.getHeight(0);
  m_format                = vkFmt;
  const uint32_t mipCount = ddsImage.getNumMips();

  imageCreateAndBind(&m_data.image, &m_data.memory, m_format, VK_IMAGE_TYPE_2D, m_width, m_height, 1, 1, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                     (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
                     VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, mipCount);

  if(m_memory_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
  {
    // We'll create a staging buffer, copy data to it, and then issue
    // a command to copy from the staging buffer to the device-local image.
    // Determine how much memory to allocate for the buffer:
    size_t bufferSizeBytes = 0;
    for(uint32_t mip = 0; mip < mipCount; mip++)
    {
      bufferSizeBytes += ddsImage.subresource(mip, 0, 0).data.size();
    }
    // Allocate the staging buffer:
    VkBuffer       stagingBuffer;
    VkDeviceMemory stagingMem;
    bufferCreate(&stagingBuffer, bufferSizeBytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    bufferAlloc(&stagingBuffer, &stagingMem, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    // Copy data from the DDS file to the staging texture.
    // While we're doing that, we'll also set up the copy commands:
    std::vector<VkBufferImageCopy> copyRegions(mipCount);
    char*                          stagingData = nullptr;
    VKA_CHECK_ERROR(vkMapMemory(device->getVKDevice(), stagingMem, 0, VK_WHOLE_SIZE, 0, (void**)&stagingData),
                    "Could not map staging buffer memory.\n");
    size_t stagingDataOffset = 0;
    for(uint32_t mip = 0; mip < mipCount; mip++)
    {
      const nv_dds::Subresource& ddsSubresource  = ddsImage.subresource(mip, 0, 0);
      const size_t               subresourceSize = ddsSubresource.data.size();
      memcpy(stagingData + stagingDataOffset, ddsSubresource.data.data(), subresourceSize);

      VkBufferImageCopy& region              = copyRegions[mip];
      region.bufferOffset                    = stagingDataOffset;
      region.imageExtent.width               = ddsImage.getWidth(mip);
      region.imageExtent.height              = ddsImage.getHeight(mip);
      region.imageExtent.depth               = 1;
      region.imageSubresource.mipLevel       = mip;
      region.imageSubresource.baseArrayLayer = 0;
      region.imageSubresource.layerCount     = 1;
      region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;

      stagingDataOffset += subresourceSize;
    }
    vkUnmapMemory(device->getVKDevice(), stagingMem);

    // Copy from the staging texture to the device texture:
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    queue->beginCommandBuffer(cmdID, &cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VkImageSubresourceRange allFaces;
    imageSubresourceRange(&allFaces, VK_IMAGE_ASPECT_COLOR_BIT, 1, 0, mipCount, 0);
    imageBarrierCreate(&cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_data.image, allFaces,
                       VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT);
    m_data.imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    VkFence           copyFence;
    VkFenceCreateInfo fenceInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    vkCreateFence(device->getVKDevice(), &fenceInfo, NULL, &copyFence);

    vkCmdCopyBufferToImage(cmd, stagingBuffer, m_data.image, m_data.imageLayout, uint32_t(copyRegions.size()),
                           copyRegions.data());
    imageBarrierCreate(&cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                       m_data.image, allFaces, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
    m_data.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    queue->flushCommandBuffer(cmdID, &copyFence);

    vkWaitForFences(device->getVKDevice(), 1, &copyFence, VK_TRUE, 100000000000);

    vkDestroyBuffer(device->getVKDevice(), stagingBuffer, NULL);
    vkFreeMemory(device->getVKDevice(), stagingMem, NULL);
  }

  samplerCreate(&m_data.sampler, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL,
                VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT,
                VK_SAMPLER_MIPMAP_MODE_LINEAR, 0.0f, float(mipCount));

  imageViewCreate(&m_data.view, m_data.image, VK_IMAGE_VIEW_TYPE_2D, m_format);
}

void VkeTexture::loadTextureFloatData(float* inData, uint32_t inWidth, uint32_t inHeight, uint32_t inCompCount)
{


  VulkanDC::Device::Queue::Name            queueName = "DEFAULT_GRAPHICS_QUEUE";
  VulkanDC::Device::Queue::CommandBufferID cmdID     = INIT_COMMAND_ID + 200;
  VulkanDC*                                dc        = VulkanDC::Get();
  VulkanDC::Device*                        device    = dc->getDefaultDevice();
  VulkanDC::Device::Queue*                 queue     = device->getQueue(queueName);
  device->waitIdle();

  TextureObject stagingTex{};

  m_width  = inWidth;
  m_height = inHeight;

  imageCreateAndBind(&m_data.image, &m_data.memory, m_format, VK_IMAGE_TYPE_2D, m_width, m_height, 1, 1, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                     (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), VK_IMAGE_TILING_OPTIMAL);

  imageCreateAndBind(&stagingTex.image, &stagingTex.memory, m_format, VK_IMAGE_TYPE_2D, m_width, m_height, 1, 1,
                     m_memory_flags, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_LINEAR);

  if(m_memory_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
  {
    VkImageSubresource subres{};
    subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subres.mipLevel   = 0;
    subres.arrayLayer = 0;

    VkSubresourceLayout layout;
    void*               data;

    vkGetImageSubresourceLayout(getDefaultDevice(), stagingTex.image, &subres, &layout);
    VKA_CHECK_ERROR(vkMapMemory(getDefaultDevice(), stagingTex.memory, 0, VK_WHOLE_SIZE, 0, &data), "Could not map memory for image.\n");

    memcpy(data, inData, size_t(inWidth) * size_t(inHeight) * size_t(inCompCount) * sizeof(float));

    vkUnmapMemory(getDefaultDevice(), stagingTex.memory);
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    queue->beginCommandBuffer(cmdID, &cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VkImageSubresourceRange fullImage;
    imageSubresourceRange(&fullImage, VK_IMAGE_ASPECT_COLOR_BIT, 1, 0, 1, 0);
    imageBarrierCreate(&cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingTex.image,
                       fullImage, VK_ACCESS_NONE, VK_ACCESS_TRANSFER_READ_BIT);
    imageBarrierCreate(&cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_data.image, fullImage,
                       VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT);

    VkImageCopy cpyRgn[1]{};

    cpyRgn[0].srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    cpyRgn[0].srcSubresource.baseArrayLayer = 0;
    cpyRgn[0].srcSubresource.mipLevel       = 0;
    cpyRgn[0].srcSubresource.layerCount     = 1;

    cpyRgn[0].dstSubresource = cpyRgn[0].srcSubresource;

    cpyRgn[0].extent.width  = m_width;
    cpyRgn[0].extent.height = m_height;
    cpyRgn[0].extent.depth  = 1;

    vkCmdCopyImage(cmd, stagingTex.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_data.image,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &cpyRgn[0]);
    imageBarrierCreate(&cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                       m_data.image, fullImage, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
    m_data.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    queue->flushCommandBuffer(cmdID);
  }

  samplerCreate(&m_data.sampler, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL,
                VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT);

  imageViewCreate(&m_data.view, m_data.image, VK_IMAGE_VIEW_TYPE_2D, m_format);

  LOGI("Texture Loaded.\n");
  LOGI("Floating Point Texture Loaded.\n");
}

VkeTexture::List::List() {}
VkeTexture::List::~List() {}

VkeTexture::ID VkeTexture::List::nextID()
{
  VkeTexture::ID outID;
  if(m_deleted_keys.size() == 0)
    return m_data.size();
  outID = m_deleted_keys.back();
  m_deleted_keys.pop_back();
  return outID;
}

VkeTexture::Count VkeTexture::List::count()
{
  return m_data.size();
}

VkeTexture* VkeTexture::List::newTexture()
{
  VkeTexture::ID id = nextID();
  return newTexture(id);
}

VkeTexture* VkeTexture::List::newTexture(const VkeTexture::ID& inID)
{
  VkeTexture* outMaterial = new VkeTexture(inID);
  m_data[inID]            = outMaterial;
  return outMaterial;
}


void VkeTexture::List::addTexture(VkeTexture* const inMaterial)
{
  VkeTexture::ID id = nextID();
  m_data[id]        = inMaterial;
}

VkeTexture* VkeTexture::List::getTexture(const VkeTexture::ID& inID)
{
  return m_data[inID];
}

void VkeTexture::List::getData(VkeTexture::Data* outData, size_t offset)
{
  VkeTexture::Map::iterator itr;
  size_t                    i = offset;
  for(itr = m_data.begin(); itr != m_data.end(); ++itr)
  {
    outData[i++] = itr->second->getData();
  }
}
