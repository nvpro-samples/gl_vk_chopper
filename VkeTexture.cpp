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
#include "VkeTexture.h"
#include "nvh/nvprint.hpp"
#include "nvpwindow.hpp"
#include "vkaUtils.h"
#include <fileformats/nv_dds.h>
#include <iostream>

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

  nv_dds::CDDSImage ddsImage;

  std::string filePath;
  for(uint32_t i = 0; i < searchPaths.size(); ++i)
  {
    filePath = searchPaths[i] + "/" + std::string(inFile);
    ddsImage.load(filePath, true);
    if(ddsImage.is_valid())
      break;
  }

  if(!ddsImage.is_valid())
  {
    LOGE("Could not load texture image %s\n", inFile);
    exit(1);
  }
  else
  {
    LOGI("loaded texture image %s\n", filePath.c_str());
  }

  uint32_t imgW = ddsImage.get_width();
  uint32_t imgH = ddsImage.get_height();
  uint32_t fmt  = ddsImage.get_format();

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


  VulkanDC::Device::Queue::Name            queueName = "DEFAULT_GRAPHICS_QUEUE";
  VulkanDC::Device::Queue::CommandBufferID cmdID     = INIT_COMMAND_ID + 200;
  VulkanDC*                                dc        = VulkanDC::Get();
  VulkanDC::Device*                        device    = dc->getDefaultDevice();
  VulkanDC::Device::Queue*                 queue     = device->getQueue(queueName);
  device->waitIdle();

  TextureObject stagingTex{};

  m_width  = imgW;
  m_height = imgH;
  m_format = vkFmt;

  imageCreateAndBind(&m_data.image, &m_data.memory, m_format, VK_IMAGE_TYPE_2D, m_width, m_height, 1, 1, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                     (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), VK_IMAGE_TILING_OPTIMAL);

  imageCreateAndBind(&stagingTex.image, &stagingTex.memory, m_format, VK_IMAGE_TYPE_2D, m_width, m_height, 1, 1,
                     m_memory_flags, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_IMAGE_TILING_LINEAR);

  if(m_memory_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
  {
    VkImageSubresource subres{};
    subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subres.mipLevel   = m_mip_level;
    subres.arrayLayer = 0;

    VkSubresourceLayout layout;
    void*               data;

    vkGetImageSubresourceLayout(getDefaultDevice(), stagingTex.image, &subres, &layout);
    VKA_CHECK_ERROR(vkMapMemory(getDefaultDevice(), stagingTex.memory, 0, VK_WHOLE_SIZE, 0, &data), "Could not map memory for image.\n");

    const nv_dds::CSurface& mipmap = ddsImage.get_mipmap(0);

    memcpy(data, (void*)mipmap, layout.size);

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

  samplerCreate(&m_data.sampler, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL,
                VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT);

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
    subres.mipLevel   = m_mip_level;
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
