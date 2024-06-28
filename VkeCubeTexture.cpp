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
#include "VkeCubeTexture.h"

#ifndef INIT_COMMAND_ID
#define INIT_COMMAND_ID 1
#endif

VkeCubeTexture::VkeCubeTexture()
    : m_id(0)
{
}

VkeCubeTexture::VkeCubeTexture(const ID& inID)
    : m_id(inID)
{
}


VkeCubeTexture::~VkeCubeTexture() {}

void VkeCubeTexture::loadCubeDDS(const char* inFile)
{
  std::vector<std::string> searchPaths;
  searchPaths.push_back(std::string("."));
  searchPaths.push_back(std::string("./resources_" PROJECT_NAME));
  searchPaths.push_back(std::string(PROJECT_NAME) + std::string("/images"));
  searchPaths.push_back(NVPSystem::exePath() + std::string(PROJECT_RELDIRECTORY) + std::string("/images"));

  nv_dds::Image         ddsImage;
  nv_dds::ErrorWithText loadError;

  std::string filePath;
  for(uint32_t i = 0; i < searchPaths.size(); ++i)
  {
    std::string separator = "";
    size_t      strSize   = searchPaths[i].size();
    if(searchPaths[i].substr(strSize - 1, strSize) != "/")
      separator = "/";
    filePath  = searchPaths[i] + separator + std::string(inFile);
    loadError = ddsImage.readFromFile(filePath.c_str(), {});
    if(!loadError.has_value())
      break;
  }

  if(loadError.has_value())
  {
    LOGE("Could not cube load texture image %s; the last error message was: %s.\n", inFile, loadError.value().c_str());
    exit(1);
  }
  else
  {
    LOGI("loaded texture image %s\n", filePath.c_str());
  }

  VkFormat vkFmt = texture_formats::dxgiToVulkan(ddsImage.dxgiFormat);
  if(VK_FORMAT_UNDEFINED == vkFmt)
  {
    LOGE("Could not determine a corresponding VkFormat for DXGI format %u!", ddsImage.dxgiFormat);
    exit(1);
  }

  m_width                  = ddsImage.getWidth(0);
  m_height                 = ddsImage.getHeight(0);
  m_format                 = vkFmt;
  const uint32_t mipCount  = ddsImage.getNumMips();
  const uint32_t faceCount = ddsImage.getNumFaces();

  if(faceCount != 6)
  {
    LOGE("The texture at %s must be a complete cubemap, but it had %u faces instead of 6.\n", inFile, ddsImage.getNumFaces());
    exit(1);
  }

  VulkanDC::Device::Queue::Name            queueName = "DEFAULT_GRAPHICS_QUEUE";
  VulkanDC::Device::Queue::CommandBufferID cmdID     = INIT_COMMAND_ID;
  VulkanDC*                                dc        = VulkanDC::Get();
  VulkanDC::Device*                        device    = dc->getDefaultDevice();
  VulkanDC::Device::Queue*                 queue     = device->getQueue(queueName);

  imageCreateAndBind(&m_data.image, &m_data.memory, m_format, VK_IMAGE_TYPE_2D, m_width, m_height, 1, 6, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
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
      bufferSizeBytes += ddsImage.subresource(mip, 0, 0).data.size() * faceCount;
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
      const size_t subresourceSize = ddsImage.subresource(mip, 0, 0).data.size();
      for(uint32_t face = 0; face < faceCount; face++)
      {
        const nv_dds::Subresource& ddsSubresource = ddsImage.subresource(mip, 0, face);
        memcpy(stagingData + stagingDataOffset + face * subresourceSize, ddsSubresource.data.data(), subresourceSize);
      }

      VkBufferImageCopy& region              = copyRegions[mip];
      region.bufferOffset                    = stagingDataOffset;
      region.imageExtent.width               = ddsImage.getWidth(mip);
      region.imageExtent.height              = ddsImage.getHeight(mip);
      region.imageExtent.depth               = 1;
      region.imageSubresource.mipLevel       = mip;
      region.imageSubresource.baseArrayLayer = 0;
      region.imageSubresource.layerCount     = faceCount;
      region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;

      stagingDataOffset += subresourceSize * faceCount;
    }
    vkUnmapMemory(device->getVKDevice(), stagingMem);

    // Copy from the staging texture to the device texture:
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    queue->beginCommandBuffer(cmdID, &cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VkImageSubresourceRange allFaces;
    imageSubresourceRange(&allFaces, VK_IMAGE_ASPECT_COLOR_BIT, faceCount, 0, mipCount, 0);
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
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_MIPMAP_MODE_LINEAR, 0.0f, float(mipCount));

  imageViewCreate(&m_data.view, m_data.image, VK_IMAGE_VIEW_TYPE_CUBE, m_format, VK_IMAGE_ASPECT_COLOR_BIT, faceCount);
}

VkeCubeTexture::List::List() {}
VkeCubeTexture::List::~List() {}

VkeCubeTexture::ID VkeCubeTexture::List::nextID()
{
  VkeCubeTexture::ID outID;
  if(m_deleted_keys.size() == 0)
    return m_data.size();
  outID = m_deleted_keys.back();
  m_deleted_keys.pop_back();
  return outID;
}

VkeCubeTexture::Count VkeCubeTexture::List::count()
{
  return m_data.size();
}

VkeCubeTexture* VkeCubeTexture::List::newTexture()
{
  VkeCubeTexture::ID id = nextID();
  return newTexture(id);
}

VkeCubeTexture* VkeCubeTexture::List::newTexture(const VkeCubeTexture::ID& inID)
{
  VkeCubeTexture* outMaterial = new VkeCubeTexture(inID);
  m_data[inID]                = outMaterial;
  return outMaterial;
}


void VkeCubeTexture::List::addTexture(VkeCubeTexture* const inMaterial)
{
  VkeCubeTexture::ID id = nextID();
  m_data[id]            = inMaterial;
}

VkeCubeTexture* VkeCubeTexture::List::getTexture(const VkeCubeTexture::ID& inID)
{
  return m_data[inID];
}

void VkeCubeTexture::List::getData(VkeCubeTexture::Data* outData, uint32_t offset)
{
  VkeCubeTexture::Map::iterator itr;
  uint32_t                      i = offset;
  for(itr = m_data.begin(); itr != m_data.end(); ++itr)
  {
    outData[i++] = itr->second->getData();
  }
}
