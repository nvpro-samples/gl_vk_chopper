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

#include "VkeCreateUtils.h"
#include "VkeCubeTexture.h"
#include "nvh/nvprint.hpp"
#include "nvpwindow.hpp"
#include "vkaUtils.h"
#include <fileformats/nv_dds.h>
#include <iostream>

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

  nv_dds::CDDSImage ddsImage;

  std::string filePath;
  for(uint32_t i = 0; i < searchPaths.size(); ++i)
  {
    std::string separator = "";
    size_t      strSize   = searchPaths[i].size();
    if(searchPaths[i].substr(strSize - 1, strSize) != "/")
      separator = "/";
    filePath = searchPaths[i] + separator + std::string(inFile);
    ddsImage.load(filePath, true);
    if(ddsImage.is_valid())
      break;
  }

  if(!ddsImage.is_valid())
  {
    LOGE("Could not cube load texture image %s\n", inFile);
    exit(1);
  }
  else
  {
    LOGI("loaded texture image %s\n", filePath.c_str());
  }

  uint32_t imgW = ddsImage.get_width();
  uint32_t imgH = ddsImage.get_height();
  uint32_t fmt  = ddsImage.get_format();

  bool isCube = ddsImage.is_cubemap();
  if(!isCube)
  {
    LOGE("The texture at %s must be a cubemap.\n", inFile);
    exit(1);
  }

  VkFormat vkFmt = VK_FORMAT_R8G8B8A8_UNORM;

  switch(fmt)
  {
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
      vkFmt = VK_FORMAT_BC1_RGB_SRGB_BLOCK;
      break;

    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
      vkFmt = VK_FORMAT_BC2_UNORM_BLOCK;

      break;

    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
      vkFmt = VK_FORMAT_BC3_UNORM_BLOCK;
      break;
    default:

      break;
  }


  m_width  = imgW;
  m_height = imgH;
  m_format = vkFmt;

  VulkanDC::Device::Queue::Name            queueName = "DEFAULT_GRAPHICS_QUEUE";
  VulkanDC::Device::Queue::CommandBufferID cmdID     = INIT_COMMAND_ID;
  VulkanDC*                                dc        = VulkanDC::Get();
  VulkanDC::Device*                        device    = dc->getDefaultDevice();
  VulkanDC::Device::Queue*                 queue     = device->getQueue(queueName);
  VkCommandBuffer                          cmd       = VK_NULL_HANDLE;

  queue->beginCommandBuffer(cmdID, &cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  imageCreateAndBind(&m_data.image, &m_data.memory, m_format, VK_IMAGE_TYPE_2D, m_width, m_height, 1, 6, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                     (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), VK_IMAGE_TILING_OPTIMAL);

  // Staging buffer used to upload data to the cube map
  VkBuffer       cubeMapBuffer;
  VkDeviceMemory cubeMapMem;

  const size_t faceSizeBytes = ddsImage.get_mipmap(m_mip_level).get_size();
  bufferCreate(&cubeMapBuffer, faceSizeBytes * 6, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
  bufferAlloc(&cubeMapBuffer, &cubeMapMem, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);


  if(m_memory_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
  {
    VkImageSubresourceRange allFaces;
    imageSubresourceRange(&allFaces, VK_IMAGE_ASPECT_COLOR_BIT, 6, 0, 1, m_mip_level);
    imageBarrierCreate(&cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_data.image, allFaces,
                       VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT);
    m_data.imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    char* data = nullptr;
    VKA_CHECK_ERROR(vkMapMemory(getDefaultDevice(), cubeMapMem, 0, faceSizeBytes * 6, 0, (void**)&data),
                    "Could not map staging buffer memory.\n");
    for(uint32_t i = 0; i < 6; ++i)
    {
      const nv_dds::CTexture& mipmap = ddsImage.get_cubemap_face(i);
      memcpy(data + i * faceSizeBytes, (void*)mipmap, faceSizeBytes);
    }
    vkUnmapMemory(getDefaultDevice(), cubeMapMem);

    VkBufferImageCopy biCpyRgn[6]{};
    for(uint32_t k = 0; k < 6; ++k)
    {
      biCpyRgn[k].bufferOffset                    = k * faceSizeBytes;
      biCpyRgn[k].bufferImageHeight               = 0;
      biCpyRgn[k].bufferRowLength                 = 0;
      biCpyRgn[k].imageExtent.width               = m_width;
      biCpyRgn[k].imageExtent.height              = m_height;
      biCpyRgn[k].imageExtent.depth               = 1;
      biCpyRgn[k].imageOffset.x                   = 0;
      biCpyRgn[k].imageOffset.y                   = 0;
      biCpyRgn[k].imageOffset.z                   = 0;
      biCpyRgn[k].imageSubresource.baseArrayLayer = k;
      biCpyRgn[k].imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
      biCpyRgn[k].imageSubresource.layerCount     = 1;
      biCpyRgn[k].imageSubresource.mipLevel       = m_mip_level;
    }

    VkFence           copyFence;
    VkFenceCreateInfo fenceInfo;
    memset(&fenceInfo, 0, sizeof(fenceInfo));
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkCreateFence(device->getVKDevice(), &fenceInfo, NULL, &copyFence);

    vkCmdCopyBufferToImage(cmd, cubeMapBuffer, m_data.image, m_data.imageLayout, 6, biCpyRgn);
    imageBarrierCreate(&cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                       m_data.image, allFaces, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
    m_data.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    queue->flushCommandBuffer(cmdID, &copyFence);

    vkWaitForFences(device->getVKDevice(), 1, &copyFence, VK_TRUE, 100000000000);

    vkDestroyBuffer(device->getVKDevice(), cubeMapBuffer, NULL);
    vkFreeMemory(device->getVKDevice(), cubeMapMem, NULL);
  }


  VkSamplerCreateInfo sampler = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};

  sampler.magFilter     = VK_FILTER_NEAREST;
  sampler.minFilter     = VK_FILTER_NEAREST;
  sampler.mipmapMode    = VK_SAMPLER_MIPMAP_MODE_NEAREST;
  sampler.addressModeU  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  sampler.addressModeV  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  sampler.addressModeW  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  sampler.mipLodBias    = 0.0f;
  sampler.maxAnisotropy = 1;
  sampler.compareOp     = VK_COMPARE_OP_NEVER;
  sampler.minLod        = 0.0f;
  sampler.maxLod        = 0.0f;

  sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

  VkImageViewCreateInfo view = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  view.viewType              = VK_IMAGE_VIEW_TYPE_CUBE;
  view.format                = m_format;
  view.components.r          = VK_COMPONENT_SWIZZLE_R;
  view.components.g          = VK_COMPONENT_SWIZZLE_G;
  view.components.b          = VK_COMPONENT_SWIZZLE_B;
  view.components.a          = VK_COMPONENT_SWIZZLE_A;

  view.subresourceRange.baseArrayLayer = 0;
  view.subresourceRange.levelCount     = 1;
  view.subresourceRange.baseMipLevel   = m_mip_level;
  view.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
  view.subresourceRange.layerCount     = 6;

  VKA_CHECK_ERROR(vkCreateSampler(getDefaultDevice(), &sampler, NULL, &m_data.sampler), "Could not create sampler for image texture.\n");

  view.image = m_data.image;

  VKA_CHECK_ERROR(vkCreateImageView(getDefaultDevice(), &view, NULL, &m_data.view), "Could not create image view for texture.\n");
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
