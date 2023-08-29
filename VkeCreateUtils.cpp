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

#include "VkeCreateUtils.h"
#include "VulkanDeviceContext.h"
#include <iostream>
#include <memory.h>
#include <nvh/fileoperations.hpp>
#include <nvh/nvprint.hpp>
#include <nvvk/commands_vk.hpp>
#include <vulkan/vulkan.h>

VkDevice getDefaultDevice()
{
  VulkanDC* dc = VulkanDC::Get();

  if(!dc)
    return VK_NULL_HANDLE;

  VulkanDC::Device* defaultDevice = dc->getDefaultDevice();
  if(!defaultDevice)
    return VK_NULL_HANDLE;

  return defaultDevice->getVKDevice();
}


VkResult vulkanCreateInstance(const VkInstanceCreateInfo* pCreateInfo, VkInstance* pInstance)
{
  return vkCreateInstance(pCreateInfo, NULL, pInstance);
}


uint32_t getMemoryType(uint32_t typeBits, VkFlags props)
{
  VulkanDC*         dc     = VulkanDC::Get();
  VulkanDC::Device* device = dc->getDefaultDevice();

  return device->getBestMemoryType(typeBits, props);
}

void commandPoolCreateInfo(VkCommandPoolCreateInfo* outPool, uint32_t inQueueFamilyIndex, VkCommandPoolCreateFlags inFlags)
{

  memset(outPool, 0, sizeof(VkCommandPoolCreateInfo));
  outPool->sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  outPool->queueFamilyIndex = inQueueFamilyIndex;
  outPool->flags            = inFlags;
}

void commandPoolCreate(VkCommandPool* outPool, uint32_t inQueueFamilyIndex, VkCommandPoolCreateFlags inFlags)
{
  VkCommandPoolCreateInfo cmdPoolInfo;
  commandPoolCreateInfo(&cmdPoolInfo, inQueueFamilyIndex, inFlags);

  VKA_CHECK_ERROR(vkCreateCommandPool(getDefaultDevice(), &cmdPoolInfo, NULL, outPool), "Could not create command pool.\n");
}


void imageSetLayoutBarrier(VulkanDC::Device::Queue::CommandBufferID& inCommandID,
                           VulkanDC::Device::Queue::Name&            inQueueName,
                           VkImage                                   image,
                           VkImageAspectFlags                        aspect,
                           VkImageLayout                             oldLayout,
                           VkImageLayout                             newLayout,
                           VkAccessFlagBits                          inSrcAccessmask,
                           VkAccessFlagBits                          inDstAccessmask,
                           uint32_t                                  inArraySize,
                           uint32_t                                  inBaseArraySlice)
{

  VulkanDC* dc = VulkanDC::Get();
  if(!dc)
    return;
  VulkanDC::Device* device = dc->getDefaultDevice();
  if(!device)
    return;

  VulkanDC::Device::Queue* queue = device->getQueue(inQueueName);
  if(!queue)
    return;

  VkCommandBuffer cmd;
  queue->beginCommandBuffer(inCommandID, &cmd);
  VkImageSubresourceRange subRange;
  imageSubresourceRange(&subRange, aspect, inArraySize, inBaseArraySlice, 1, 0);
  imageBarrierCreate(&cmd, oldLayout, newLayout, image, subRange, inDstAccessmask, inSrcAccessmask);
}

void imageSetLayout(VkCommandBuffer*   inCmd,
                    VkImage            image,
                    VkImageAspectFlags aspect,
                    VkImageLayout      oldLayout,
                    VkImageLayout      newLayout,
                    uint32_t           inArraySize,
                    uint32_t           inBaseArraySlice)
{

  VkImageSubresourceRange subRange;
  imageSubresourceRange(&subRange, aspect, inArraySize, inBaseArraySlice, 1, 0);
  imageBarrierCreate(inCmd, oldLayout, newLayout, image, subRange);
}

void bufferCreateInfo(VkBufferCreateInfo* outBuffer, size_t inputSize, VkBufferUsageFlags inUsage)
{
  memset(outBuffer, 0, sizeof(VkBufferCreateInfo));
  outBuffer->sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  outBuffer->usage = inUsage;
  outBuffer->size  = inputSize;
}

void bufferCreate(VkBuffer* outBuffer, size_t inputSize, VkBufferUsageFlags inUsage)
{

  VkBufferCreateInfo bufInfo;
  bufferCreateInfo(&bufInfo, inputSize, inUsage);

  VKA_CHECK_ERROR(vkCreateBuffer(getDefaultDevice(), &bufInfo, NULL, outBuffer), "Could not create buffer.\n");
}

void createBufferData(uint8_t* inputData, size_t inputSize, BufferData* outData, VkBufferUsageFlagBits usage, VkMemoryPropertyFlagBits flags)
{
  VulkanDC*         dc     = VulkanDC::Get();
  VulkanDC::Device* device = dc->getDefaultDevice();

  bufferCreate(&outData->buf, inputSize, usage);
  bufferAlloc(&outData->buf, &outData->memory, flags);

  /*
	Populate the data with the vertices.
	map/copy/unmap
	*/
  uint8_t* vData;
  VKA_CHECK_ERROR(vkMapMemory(device->getVKDevice(), outData->memory, 0, VK_WHOLE_SIZE, 0, (void**)&vData),
                  "Could not map vertex buffer memory.\n");
  memcpy(vData, (const void*)&(inputData[0]), inputSize);
  vkUnmapMemory(device->getVKDevice(), outData->memory);
  bufferViewCreate(&outData->buf, &outData->view, inputSize);
}


void bufferAlloc(VkBuffer* inBuffer, VkDeviceMemory* outMemory, VkMemoryPropertyFlags inFlags)
{
  VkMemoryAllocateInfo allocInfo;
  memset(&allocInfo, 0, sizeof(allocInfo));
  allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize  = 0;
  allocInfo.memoryTypeIndex = 0;

  VkMemoryRequirements memReqs;

  vkGetBufferMemoryRequirements(getDefaultDevice(), *inBuffer, &memReqs);

  allocInfo.allocationSize = memReqs.size;

  allocInfo.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, inFlags);

  VKA_CHECK_ERROR(vkAllocateMemory(getDefaultDevice(), &allocInfo, NULL, outMemory), "Could not allocate memory.\n");

  VKA_CHECK_ERROR(vkBindBufferMemory(getDefaultDevice(), *inBuffer, *outMemory, 0), "Could not bind buffer memory.\n");
}

void bufferViewCreate(VkBuffer* inBuffer, VkBufferView* outView, size_t inSize, size_t inOffset)
{

  VkBufferViewCreateInfo view_info;
  memset(&view_info, 0, sizeof(view_info));
  view_info.sType  = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
  view_info.pNext  = NULL;
  view_info.buffer = *inBuffer;
  view_info.offset = inOffset;
  view_info.range  = inSize;

  VKA_CHECK_ERROR(vkCreateBufferView(getDefaultDevice(), &view_info, NULL, outView), "Could not create buffer view.\n");
}


void imageCreateInfo(VkImageCreateInfo*    outInfo,
                     VkFormat              inFormat,
                     VkImageType           inType,
                     uint32_t              inWidth,
                     uint32_t              inHeight,
                     uint32_t              inDepth,
                     uint32_t              inArraySize,
                     VkImageUsageFlags     inUsage,
                     VkImageTiling         inTiling,
                     VkSampleCountFlagBits inSamples,
                     uint32_t              inMipLevels)
{


  memset(outInfo, 0, sizeof(VkImageCreateInfo));
  outInfo->sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  outInfo->imageType     = inType;
  outInfo->format        = inFormat;
  outInfo->extent.width  = inWidth;
  outInfo->extent.height = inHeight;
  outInfo->extent.depth  = inDepth;
  outInfo->mipLevels     = inMipLevels;
  outInfo->arrayLayers   = inArraySize;
  outInfo->samples       = inSamples;
  outInfo->tiling        = inTiling;
  outInfo->usage         = inUsage;
  outInfo->flags         = 0;
  outInfo->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  if(inArraySize == 6)
    outInfo->flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
}

void imageCreate(VkImage*              outImage,
                 VkDeviceMemory*       outMemory,
                 VkFormat              inFormat,
                 VkImageType           inType,
                 uint32_t              inWidth,
                 uint32_t              inHeight,
                 uint32_t              inDepth,
                 uint32_t              inArraySize,
                 VkFlags               inMemFlags,
                 VkImageUsageFlags     inUsage,
                 VkImageTiling         inTiling,
                 VkSampleCountFlagBits inSamples,
                 uint32_t              inMipLevels)
{

  VkImageCreateInfo imageInfo;
  imageCreateInfo(&imageInfo, inFormat, inType, inWidth, inHeight, inDepth, inArraySize, inUsage, inTiling, inSamples, inMipLevels);


  VkMemoryRequirements memReqs;
  VkMemoryAllocateInfo memAlloc;
  memset(&memAlloc, 0, sizeof(memAlloc));
  memAlloc.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memAlloc.allocationSize  = 0;
  memAlloc.memoryTypeIndex = 0;


  VKA_CHECK_ERROR(vkCreateImage(getDefaultDevice(), &imageInfo, NULL, outImage), "Could not create image.\n");

  vkGetImageMemoryRequirements(getDefaultDevice(), *outImage, &memReqs);

  memAlloc.allocationSize = memReqs.size;  // *inArraySize;

  memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, inMemFlags);


  VKA_CHECK_ERROR(vkAllocateMemory(getDefaultDevice(), &memAlloc, NULL, outMemory), "Could not allocate image memory on device.\n");
}

void imageCreateAndBind(VkImage*              outImage,
                        VkDeviceMemory*       outMemory,
                        VkFormat              inFormat,
                        VkImageType           inType,
                        uint32_t              inWidth,
                        uint32_t              inHeight,
                        uint32_t              inDepth,
                        uint32_t              inArraySize,
                        VkFlags               inMemFlags,
                        VkImageUsageFlags     inUsage,
                        VkImageTiling         inTiling,
                        VkSampleCountFlagBits inSamples,
                        uint32_t              inMipLevels)
{

  imageCreate(outImage, outMemory, inFormat, inType, inWidth, inHeight, inDepth, inArraySize, inMemFlags, inUsage,
              inTiling, inSamples, inMipLevels);

  VKA_CHECK_ERROR(vkBindImageMemory(getDefaultDevice(), *outImage, *outMemory, 0), "Could not bind image memory.\n");
}


void attachmentDescriptionInit(VkAttachmentDescription* outDescription)
{

  memset(outDescription, 0, sizeof(VkAttachmentDescription));
}

void attachmentDescription(VkAttachmentDescription* outDescription,
                           VkFormat                 inFormat,
                           VkSampleCountFlagBits    inSamples,
                           VkAttachmentLoadOp       inLoadOp,
                           VkAttachmentStoreOp      inStoreOp,
                           VkImageLayout            inInitialLayout,
                           VkImageLayout            inFinalLayout)
{

  attachmentDescriptionInit(outDescription);


  outDescription->flags          = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;
  outDescription->samples        = inSamples;
  outDescription->format         = inFormat;
  outDescription->loadOp         = inLoadOp;
  outDescription->storeOp        = inStoreOp;
  outDescription->stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  outDescription->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  outDescription->initialLayout  = inInitialLayout;
  outDescription->finalLayout    = inFinalLayout;
}

void depthStencilAttachmentDescription(VkAttachmentDescription* outDescription,
                                       VkFormat                 inFormat,
                                       VkSampleCountFlagBits    inSamples,
                                       VkAttachmentLoadOp       inLoadOp,
                                       VkAttachmentStoreOp      inStoreOp,
                                       VkImageLayout            inInitialLayout,
                                       VkImageLayout            inFinalLayout)
{

  attachmentDescriptionInit(outDescription);

  outDescription->samples        = inSamples;
  outDescription->format         = inFormat;
  outDescription->loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
  outDescription->storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
  outDescription->stencilLoadOp  = inLoadOp;
  outDescription->stencilStoreOp = inStoreOp;
  outDescription->initialLayout  = inInitialLayout;
  outDescription->finalLayout    = inFinalLayout;
}

void subpassDescriptionInit(VkSubpassDescription* outDescription)
{
  memset(outDescription, 0, sizeof(VkSubpassDescription));
}

void subpassDescription(VkSubpassDescription*     outDescription,
                        uint32_t                  inColorCount,
                        VkAttachmentReference*    inColorAttachments,
                        VkAttachmentReference*    inDepthStencilAttachment,
                        VkAttachmentReference*    inResolveAttachments,
                        uint32_t                  inInputCount,
                        VkAttachmentReference*    inInputAttachments,
                        VkSubpassDescriptionFlags inFlags,
                        VkPipelineBindPoint       inBindPoint,
                        uint32_t                  inPreserveCount,
                        VkAttachmentReference*    inPreserveAttachments)
{

  subpassDescriptionInit(outDescription);

  outDescription->pipelineBindPoint       = inBindPoint;
  outDescription->flags                   = inFlags;
  outDescription->inputAttachmentCount    = inInputCount;
  outDescription->pInputAttachments       = inInputAttachments;
  outDescription->colorAttachmentCount    = inColorCount;
  outDescription->pColorAttachments       = inColorAttachments;
  outDescription->pResolveAttachments     = inResolveAttachments;
  outDescription->pDepthStencilAttachment = inDepthStencilAttachment;
  outDescription->preserveAttachmentCount = inPreserveCount;
  outDescription->pPreserveAttachments    = 0;  //hack!!
}

void renderPassCreate(VkRenderPass*            outPass,
                      uint32_t                 inAttachmentCount,
                      VkAttachmentDescription* inAttachments,
                      uint32_t                 inSubPassCount,
                      VkSubpassDescription*    inSubPasses,
                      uint32_t                 inDependencyCount,
                      VkSubpassDependency*     inDependencies)
{

  VkRenderPassCreateInfo rpInfo;
  memset(&rpInfo, 0, sizeof(VkRenderPassCreateInfo));
  rpInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  rpInfo.attachmentCount = inAttachmentCount;
  rpInfo.pAttachments    = inAttachments;
  rpInfo.subpassCount    = inSubPassCount;
  rpInfo.pSubpasses      = inSubPasses;
  rpInfo.dependencyCount = inDependencyCount;
  rpInfo.pDependencies   = inDependencies;

  VKA_CHECK_ERROR(vkCreateRenderPass(getDefaultDevice(), &rpInfo, NULL, outPass), "Could not create render pass.\n");
}

void viewportStateInfo(VkPipelineViewportStateCreateInfo* outInfo, uint32_t inViewportCount)
{

  memset(outInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));
  outInfo->sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  outInfo->viewportCount = inViewportCount;
  outInfo->scissorCount  = inViewportCount;

  VkViewport* vp = new VkViewport;

  vp->x        = 0;
  vp->y        = 0;
  vp->width    = 1024;
  vp->height   = 768;
  vp->minDepth = 0.0f;
  vp->maxDepth = 1.0f;

  outInfo->pViewports = vp;

  VkRect2D* rct = new VkRect2D;

  rct->extent.width  = 1024;
  rct->extent.height = 768;
  rct->offset.x      = 0;
  rct->offset.y      = 0;

  outInfo->pScissors = rct;
}

void depthStateInfo(VkPipelineDepthStencilStateCreateInfo* outInfo,
                    VkBool32                               inDepthTestEnable,
                    VkBool32                               inDepthWriteEnable,
                    VkCompareOp                            inDepthCompareOp,
                    VkBool32                               inDepthBoundsEnable,
                    VkStencilOp                            inStencilFailOp,
                    VkStencilOp                            inStencilPassOp,
                    VkCompareOp                            inStencilCompareOp,
                    VkBool32                               inStencilTestEnable)
{


  memset(outInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
  outInfo->sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  outInfo->depthTestEnable       = inDepthTestEnable;
  outInfo->depthWriteEnable      = inDepthWriteEnable;
  outInfo->depthCompareOp        = inDepthCompareOp;
  outInfo->depthBoundsTestEnable = inDepthBoundsEnable;
  outInfo->back.failOp           = inStencilFailOp;
  outInfo->back.passOp           = inStencilPassOp;
  outInfo->back.compareOp        = inStencilCompareOp;
  outInfo->stencilTestEnable     = inStencilTestEnable;
  outInfo->front                 = outInfo->back;
  outInfo->minDepthBounds        = 0.0f;
  outInfo->maxDepthBounds        = 1.0f;
  outInfo->front.reference       = 0;
  outInfo->back.reference        = 0;
  outInfo->front.compareMask     = 0xff;
  outInfo->back.compareMask      = 0xff;
  outInfo->front.writeMask       = 0xff;
  outInfo->back.writeMask        = 0xff;
}

void multisampleStateInfo(VkPipelineMultisampleStateCreateInfo* outInfo,
                          VkSampleCountFlagBits                 inSamples,
                          VkSampleMask*                         inSampleMask,
                          VkBool32                              inSampleShadingEnable,
                          float                                 inMinSampleShading)
{

  memset(outInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));
  outInfo->sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  outInfo->rasterizationSamples = inSamples;
  outInfo->pSampleMask          = inSampleMask;
  outInfo->sampleShadingEnable  = inSampleShadingEnable;
  outInfo->minSampleShading     = inMinSampleShading;
}

void blendStateInfo(VkPipelineColorBlendStateCreateInfo* outInfo,
                    uint32_t                             inAttachmentCount,
                    VkPipelineColorBlendAttachmentState* inAttachments,
                    VkBool32                             inAlphaToCoverageEnable)
{

  memset(outInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));
  outInfo->sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  outInfo->attachmentCount   = inAttachmentCount;
  outInfo->pAttachments      = inAttachments;
  outInfo->logicOpEnable     = VK_FALSE;
  outInfo->blendConstants[0] = 1.0f;
  outInfo->blendConstants[1] = 1.0f;
  outInfo->blendConstants[2] = 1.0f;
  outInfo->blendConstants[3] = 1.0f;
}


void blendAttachmentState(VkPipelineColorBlendAttachmentState* outState,
                          VkBool32                             inBlendEnable,
                          VkBlendOp                            inBlendOpAlpha,
                          VkBlendOp                            inBlendOpColor,
                          VkBlendFactor                        inSrcAlpha,
                          VkBlendFactor                        inDestAlpha,
                          VkBlendFactor                        inSrcColor,
                          VkBlendFactor                        inDestColor,
                          VkColorComponentFlags                inWriteMask)
{


  memset(outState, 0, sizeof(VkPipelineColorBlendAttachmentState));
  outState->colorWriteMask      = inWriteMask;
  outState->blendEnable         = inBlendEnable;
  outState->alphaBlendOp        = inBlendOpAlpha;
  outState->colorBlendOp        = inBlendOpColor;
  outState->srcAlphaBlendFactor = inSrcAlpha;
  outState->dstAlphaBlendFactor = inDestAlpha;
  outState->srcColorBlendFactor = inSrcColor;
  outState->dstColorBlendFactor = inDestColor;
}

void blendAttachmentStateN(uint32_t                             inCount,
                           VkPipelineColorBlendAttachmentState* outState,
                           VkBool32                             inBlendEnable,
                           VkBlendOp                            inBlendOpAlpha,
                           VkBlendOp                            inBlendOpColor,
                           VkBlendFactor                        inSrcAlpha,
                           VkBlendFactor                        inDestAlpha,
                           VkBlendFactor                        inSrcColor,
                           VkBlendFactor                        inDestColor,
                           VkColorComponentFlags                inWriteMask)
{

  for(uint32_t i = 0; i < inCount; ++i)
  {
    blendAttachmentState(&outState[i], inBlendEnable, inBlendOpAlpha, inBlendOpColor, inSrcAlpha, inDestAlpha,
                         inSrcColor, inDestColor, inWriteMask);
  }
}

void rasterStateInfo(VkPipelineRasterizationStateCreateInfo* outInfo,
                     VkPolygonMode                           inFillMode,
                     VkCullModeFlags                         inCullMode,
                     VkFrontFace                             inFrontFace,
                     VkBool32                                inDepthClipEnable,
                     VkBool32                                inDiscardEnable)
{

  memset(outInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
  outInfo->sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  outInfo->polygonMode             = inFillMode;
  outInfo->cullMode                = inCullMode;
  outInfo->frontFace               = inFrontFace;
  outInfo->depthClampEnable        = inDepthClipEnable;
  outInfo->rasterizerDiscardEnable = inDiscardEnable;
  outInfo->lineWidth               = 1.0f;
  outInfo->depthBiasConstantFactor = 0.0f;
  outInfo->depthBiasClamp          = 0.0f;
  outInfo->depthBiasEnable         = VK_FALSE;
}

void inputAssemblyStateInfo(VkPipelineInputAssemblyStateCreateInfo* outInfo, VkPrimitiveTopology inTopology, VkBool32 inRestartEnable)
{

  memset(outInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
  outInfo->sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  outInfo->topology               = inTopology;
  outInfo->primitiveRestartEnable = inRestartEnable;
}


void vertexStateInfo(VkPipelineVertexInputStateCreateInfo* outInfo,
                     uint32_t                              inBindingCount,
                     uint32_t                              inAttributeCount,
                     VkVertexInputBindingDescription*      inBindings,
                     VkVertexInputAttributeDescription*    inAttributes)
{

  memset(outInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
  outInfo->sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  outInfo->vertexAttributeDescriptionCount = inAttributeCount;
  outInfo->vertexBindingDescriptionCount   = inBindingCount;
  outInfo->pVertexAttributeDescriptions    = inAttributes;
  outInfo->pVertexBindingDescriptions      = inBindings;
}

void vertexAttributef(VkVertexInputAttributeDescription* outDescription, uint32_t inLocation, uint32_t inBinding, VkFormat inFormat, uint32_t inOffset)
{

  memset(outDescription, 0, sizeof(VkVertexInputAttributeDescription));
  outDescription->location = inLocation;
  outDescription->binding  = inBinding;
  outDescription->format   = inFormat;
  outDescription->offset   = inOffset * sizeof(float);
}

void vertexBinding(VkVertexInputBindingDescription* outDescription, uint32_t inBinding, uint32_t inStride, VkVertexInputRate inStepRate)
{

  memset(outDescription, 0, sizeof(VkVertexInputBindingDescription));
  outDescription->binding   = inBinding;
  outDescription->stride    = inStride;
  outDescription->inputRate = inStepRate;
}

void imageBarrierCreate(VkCommandBuffer*         inCmd,
                        VkImageLayout            inOldLayout,
                        VkImageLayout            inNewLayout,
                        VkImage                  inImage,
                        VkImageSubresourceRange& inSubRange,
                        VkAccessFlagBits         inSrcAccessMask,
                        VkAccessFlagBits         inDstAccessMask)
{
  VkImageMemoryBarrier image_memory_barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
  image_memory_barrier.dstAccessMask       = inDstAccessMask;
  image_memory_barrier.srcAccessMask       = inSrcAccessMask;
  image_memory_barrier.oldLayout           = inOldLayout;
  image_memory_barrier.newLayout           = inNewLayout;
  image_memory_barrier.image               = inImage;
  image_memory_barrier.subresourceRange    = inSubRange;
  image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

  VkPipelineStageFlags srcStages = nvvk::makeAccessMaskPipelineStageFlags(inSrcAccessMask);
  VkPipelineStageFlags dstStages = nvvk::makeAccessMaskPipelineStageFlags(inDstAccessMask);

  vkCmdPipelineBarrier(*inCmd, srcStages, dstStages, false, 0, NULL, 0, NULL, 1, &image_memory_barrier);
}

void imageSubresourceRange(VkImageSubresourceRange* outRange,
                           VkImageAspectFlags       inAspect,
                           uint32_t                 inArraySize,
                           uint32_t                 inBaseArraySlice,
                           uint32_t                 inMipLevels,
                           uint32_t                 inBaseMipLevel)
{

  memset(outRange, 0, sizeof(VkImageSubresourceRange));
  outRange->aspectMask     = inAspect;
  outRange->layerCount     = inArraySize;
  outRange->baseArrayLayer = inBaseArraySlice;
  outRange->levelCount     = inMipLevels;
  outRange->baseMipLevel   = inBaseMipLevel;
}

void colorClearValues(VkClearValue* outValue, float inR, float inG, float inB, float inA)
{

  memset(outValue, 0, sizeof(VkClearValue));
  outValue->color.float32[0] = inR;
  outValue->color.float32[1] = inG;
  outValue->color.float32[2] = inB;
  outValue->color.float32[3] = inA;
}

void depthStencilClearValues(VkClearValue* outValue, float inD, uint32_t inS)
{

  memset(outValue, 0, sizeof(VkClearValue));
  outValue->depthStencil.depth   = inD;
  outValue->depthStencil.stencil = inS;
}

void renderPassBegin(VkCommandBuffer*  inCmd,
                     VkRenderPass&     inRenderPass,
                     VkFramebuffer&    inFrameBuffer,
                     uint32_t          inX,
                     uint32_t          inY,
                     uint32_t          inWidth,
                     uint32_t          inHeight,
                     VkClearValue*     inClearValues,
                     uint32_t          inClearCount,
                     VkSubpassContents inContents)
{

  VkRenderPassBeginInfo renderBegin;
  memset(&renderBegin, 0, sizeof(renderBegin));
  renderBegin.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderBegin.renderPass               = inRenderPass;
  renderBegin.framebuffer              = inFrameBuffer;
  renderBegin.renderArea.offset.x      = inX;
  renderBegin.renderArea.offset.y      = inY;
  renderBegin.renderArea.extent.width  = inWidth;
  renderBegin.renderArea.extent.height = inHeight;
  renderBegin.clearValueCount          = inClearCount;
  renderBegin.pClearValues             = inClearValues;

  vkCmdBeginRenderPass(*inCmd, &renderBegin, inContents);
}

void descriptorSetWrite(VkWriteDescriptorSet*   outWrite,
                        uint32_t                inDestBinding,
                        VkDescriptorType        inType,
                        uint32_t                inCount,
                        VkDescriptorBufferInfo* inDescriptorBufferInfo,
                        VkDescriptorImageInfo*  inDescriptorImageInfo,
                        uint32_t                inDestElement,
                        VkDescriptorSet         inDestSet)
{

  memset(outWrite, 0, sizeof(VkWriteDescriptorSet));

  outWrite->sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  outWrite->descriptorType  = inType;
  outWrite->descriptorCount = inCount;
  outWrite->dstArrayElement = inDestElement;
  outWrite->dstBinding      = inDestBinding;
  outWrite->dstSet          = inDestSet;
  outWrite->pBufferInfo     = inDescriptorBufferInfo;
  outWrite->pImageInfo      = inDescriptorImageInfo;
}

void graphicsPipelineCreate(VkPipeline*                             outPipeline,
                            VkPipelineLayout&                       inLayout,
                            uint32_t                                inStageCount,
                            VkPipelineShaderStageCreateInfo*        inStages,
                            VkPipelineVertexInputStateCreateInfo*   inVertexInput,
                            VkPipelineInputAssemblyStateCreateInfo* inAssembly,
                            VkPipelineRasterizationStateCreateInfo* inRaster,
                            VkPipelineColorBlendStateCreateInfo*    inBlend,
                            VkPipelineMultisampleStateCreateInfo*   inMultisample,
                            VkPipelineViewportStateCreateInfo*      inViewport,
                            VkPipelineDepthStencilStateCreateInfo*  inDepthStencil,
                            VkPipelineCache*                        inCache)
{


  VkGraphicsPipelineCreateInfo pipeline;
  memset(&pipeline, 0, sizeof(pipeline));

  pipeline.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline.layout              = inLayout;
  pipeline.stageCount          = inStageCount;
  pipeline.pStages             = inStages;
  pipeline.pVertexInputState   = inVertexInput;
  pipeline.pInputAssemblyState = inAssembly;
  pipeline.pRasterizationState = inRaster;
  pipeline.pColorBlendState    = inBlend;
  pipeline.pMultisampleState   = inMultisample;
  pipeline.pViewportState      = inViewport;
  pipeline.pDepthStencilState  = inDepthStencil;


  VkPipelineDynamicStateCreateInfo dynStateInfo;
  VkDynamicState                   dynStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  memset(&dynStateInfo, 0, sizeof(dynStateInfo));
  dynStateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynStateInfo.dynamicStateCount = 2;
  dynStateInfo.pDynamicStates    = dynStates;

  VkPipelineTessellationStateCreateInfo tessInfo;
  memset(&tessInfo, 0, sizeof(tessInfo));
  tessInfo.sType              = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
  tessInfo.patchControlPoints = 0;

  pipeline.pDynamicState      = &dynStateInfo;
  pipeline.pTessellationState = &tessInfo;
  pipeline.renderPass         = VK_NULL_HANDLE;


  VkResult rslt = vkCreateGraphicsPipelines(getDefaultDevice(), *inCache, 1, &pipeline, NULL, outPipeline);

  if(rslt != VK_SUCCESS)
  {
    LOGE("Could not create graphics pipeline. %d\n", rslt);
  }
}

void graphicsPipelineCreate(VkPipeline*                             outPipeline,
                            VkPipelineCache*                        inCache,
                            VkPipelineLayout&                       inLayout,
                            uint32_t                                inStageCount,
                            VkPipelineShaderStageCreateInfo*        inStages,
                            VkPipelineVertexInputStateCreateInfo*   inVertexInput,
                            VkPipelineInputAssemblyStateCreateInfo* inAssembly,
                            VkPipelineRasterizationStateCreateInfo* inRaster,
                            VkPipelineColorBlendStateCreateInfo*    inBlend,
                            VkPipelineMultisampleStateCreateInfo*   inMultisample,
                            VkPipelineViewportStateCreateInfo*      inViewport,
                            VkPipelineDepthStencilStateCreateInfo*  inDepthStencil,
                            VkRenderPass*                           inRenderPass,
                            uint32_t                                inSubPass,
                            VkPipelineCreateFlags                   inFlags,
                            VkPipeline                              inBasePipeline)
{


  VkGraphicsPipelineCreateInfo pipeline;
  memset(&pipeline, 0, sizeof(pipeline));

  pipeline.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline.layout              = inLayout;
  pipeline.stageCount          = inStageCount;
  pipeline.pStages             = inStages;
  pipeline.pVertexInputState   = inVertexInput;
  pipeline.pInputAssemblyState = inAssembly;
  pipeline.pRasterizationState = inRaster;
  pipeline.pColorBlendState    = inBlend;
  pipeline.pMultisampleState   = inMultisample;
  pipeline.pViewportState      = inViewport;
  pipeline.pDepthStencilState  = inDepthStencil;
  pipeline.renderPass          = *inRenderPass;
  pipeline.subpass             = inSubPass;
  pipeline.flags               = inFlags;
  if(inBasePipeline)
  {
    pipeline.basePipelineHandle = inBasePipeline;
    pipeline.basePipelineIndex  = -1;
  }

  VkPipelineDynamicStateCreateInfo dynStateInfo;
  VkDynamicState                   dynStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  memset(&dynStateInfo, 0, sizeof(dynStateInfo));
  dynStateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynStateInfo.dynamicStateCount = 2;
  dynStateInfo.pDynamicStates    = dynStates;

  VkPipelineTessellationStateCreateInfo tessInfo;
  memset(&tessInfo, 0, sizeof(tessInfo));
  tessInfo.sType              = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
  tessInfo.patchControlPoints = 4;

  pipeline.pDynamicState      = &dynStateInfo;
  pipeline.pTessellationState = &tessInfo;

  VkResult rslt = vkCreateGraphicsPipelines(getDefaultDevice(), *inCache, 1, &pipeline, NULL, outPipeline);

  if(rslt != VK_SUCCESS)
  {
    LOGE("Could not create graphics pipeline. %d\n", rslt);
  }
}

void createShader(const char* fileName, VkShaderStageFlagBits inStage, VkShaderModule* outModule)
{
  VkShaderModuleCreateInfo moduleInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};

  std::string buf = nvh::loadFile(fileName, true);

  LOGI("Shader Source : %s\n", buf.c_str());

  moduleInfo.codeSize = buf.size();
  moduleInfo.pCode    = reinterpret_cast<const uint32_t*>(buf.data());
  moduleInfo.flags    = 0;


  VkResult err = vkCreateShaderModule(getDefaultDevice(), &moduleInfo, NULL, outModule);
  if(err != VK_SUCCESS)
  {
    LOGE("Could not create shader module.\n");
    return;
  }
}

void createShaderStage(VkPipelineShaderStageCreateInfo* outStage, VkShaderStageFlagBits inStage, VkShaderModule inShader)
{

  memset(outStage, 0, sizeof(VkPipelineShaderStageCreateInfo));
  outStage->sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  outStage->stage  = inStage;
  outStage->module = inShader;
  outStage->pName  = "main";
}

void layoutBinding(VkDescriptorSetLayoutBinding* outBinding,
                   uint32_t                      binding,
                   VkDescriptorType              intype,
                   VkShaderStageFlags            inFlags,
                   uint32_t                      inArraySize,
                   VkSampler*                    inImmutableSamplers)
{

  outBinding->descriptorCount    = inArraySize;
  outBinding->descriptorType     = intype;
  outBinding->stageFlags         = inFlags;
  outBinding->pImmutableSamplers = inImmutableSamplers;
  outBinding->binding            = binding;
}

void descriptorSetLayoutCreateInfo(VkDescriptorSetLayoutCreateInfo* outInfo, uint32_t inCount, VkDescriptorSetLayoutBinding* inBindings)
{

  memset(outInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
  outInfo->sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  outInfo->bindingCount = inCount;
  outInfo->pBindings    = inBindings;
}

void descriptorSetLayoutCreate(VkDescriptorSetLayout* outLayout, uint32_t inCount, VkDescriptorSetLayoutBinding* inBindings)
{

  VkDescriptorSetLayoutCreateInfo descInfo;
  descriptorSetLayoutCreateInfo(&descInfo, inCount, inBindings);

  VKA_CHECK_ERROR(vkCreateDescriptorSetLayout(getDefaultDevice(), &descInfo, NULL, outLayout),
                  "Could not create descriptor set layout.\n");
}

void pipelineLayoutCreateInfo(VkPipelineLayoutCreateInfo* outInfo,
                              uint32_t                    inCount,
                              VkDescriptorSetLayout*      inLayouts,
                              uint32_t                    inPushConstantCount,
                              VkPushConstantRange*        inPushConstantRanges)
{

  memset(outInfo, 0, sizeof(VkPipelineLayoutCreateInfo));
  outInfo->sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  outInfo->setLayoutCount         = inCount;
  outInfo->pSetLayouts            = inLayouts;
  outInfo->pushConstantRangeCount = inPushConstantCount;
  outInfo->pPushConstantRanges    = inPushConstantRanges;
}

void pipelineLayoutCreate(VkPipelineLayout*      outLayout,
                          uint32_t               inCount,
                          VkDescriptorSetLayout* inLayouts,
                          uint32_t               inPushConstantCount,
                          VkPushConstantRange*   inPushConstantRanges)
{

  VkPipelineLayoutCreateInfo layoutInfo;
  pipelineLayoutCreateInfo(&layoutInfo, inCount, inLayouts, inPushConstantCount, inPushConstantRanges);
  VKA_CHECK_ERROR(vkCreatePipelineLayout(getDefaultDevice(), &layoutInfo, NULL, outLayout), "Could not create pipeline layout.\n");
}

void imageViewCreateInfo(VkImageViewCreateInfo* outInfo,
                         VkImage                inImage,
                         VkImageViewType        inType,
                         VkFormat               inFormat,
                         VkImageAspectFlags     inAspect,
                         uint32_t               inArraySizse,
                         uint32_t               inBaseArraySlice,
                         VkComponentSwizzle     inR,
                         VkComponentSwizzle     inG,
                         VkComponentSwizzle     inB,
                         VkComponentSwizzle     inA)
{

  memset(outInfo, 0, sizeof(VkImageViewCreateInfo));
  outInfo->sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  outInfo->image                           = inImage;
  outInfo->viewType                        = inType;
  outInfo->format                          = inFormat;
  outInfo->components.r                    = inR;
  outInfo->components.g                    = inG;
  outInfo->components.b                    = inB;
  outInfo->components.a                    = inA;
  outInfo->subresourceRange.aspectMask     = inAspect;
  outInfo->subresourceRange.layerCount     = inArraySizse;
  outInfo->subresourceRange.baseArrayLayer = inBaseArraySlice;
  outInfo->subresourceRange.levelCount     = 1;
  outInfo->subresourceRange.baseMipLevel   = 0;
  outInfo->flags                           = 0;
}

void imageViewCreate(VkImageView*       outView,
                     VkImage            inImage,
                     VkImageViewType    inType,
                     VkFormat           inFormat,
                     VkImageAspectFlags inAspect,
                     uint32_t           inArraySizse,
                     uint32_t           inBaseArraySlice,
                     VkComponentSwizzle inR,
                     VkComponentSwizzle inG,
                     VkComponentSwizzle inB,
                     VkComponentSwizzle inA)
{

  VkImageViewCreateInfo imageInfo;

  imageViewCreateInfo(&imageInfo, inImage, inType, inFormat, inAspect, inArraySizse, inBaseArraySlice, inR, inG, inB, inA);


  VKA_CHECK_ERROR(vkCreateImageView(getDefaultDevice(), &imageInfo, NULL, outView), "Could not create image view for texture.\n");
}

void samplerCreateInfo(VkSamplerCreateInfo* outInfo,
                       VkFilter             inMagFilter,
                       VkFilter             inMinFilter,
                       VkBool32             inCompareEnable,
                       VkCompareOp          inCompareOp,
                       VkSamplerAddressMode inAddressU,
                       VkSamplerAddressMode inAddressV,
                       VkSamplerAddressMode inAddressW,
                       VkSamplerMipmapMode  inMipMode,
                       float                inMinLod,
                       float                inMaxLod,
                       float                inMipLodBias,
                       float                inMaxAnsisotropy,
                       VkBorderColor        inBorderColor)
{

  memset(outInfo, 0, sizeof(VkSamplerCreateInfo));
  outInfo->sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  outInfo->minFilter               = inMinFilter;
  outInfo->magFilter               = inMagFilter;
  outInfo->compareEnable           = inCompareEnable;
  outInfo->compareOp               = inCompareOp;
  outInfo->addressModeU            = inAddressU;
  outInfo->addressModeV            = inAddressV;
  outInfo->addressModeW            = inAddressW;
  outInfo->maxAnisotropy           = inMaxAnsisotropy;
  outInfo->maxLod                  = inMaxLod;
  outInfo->minLod                  = inMinLod;
  outInfo->mipLodBias              = inMipLodBias;
  outInfo->mipmapMode              = inMipMode;
  outInfo->borderColor             = inBorderColor;
  outInfo->unnormalizedCoordinates = VK_FALSE;
  outInfo->anisotropyEnable        = VK_FALSE;
}

void samplerCreate(VkSampler*           outSampler,
                   VkFilter             inMagFilter,
                   VkFilter             inMinFilter,
                   VkBool32             inCompareEnable,
                   VkCompareOp          inCompareOp,
                   VkSamplerAddressMode inAddressU,
                   VkSamplerAddressMode inAddressV,
                   VkSamplerAddressMode inAddressW,
                   VkSamplerMipmapMode  inMipMode,
                   float                inMinLod,
                   float                inMaxLod,
                   float                inMipLodBias,
                   float                inMaxAnsisotropy,
                   VkBorderColor        inBorderColor)
{

  VkSamplerCreateInfo samplerInfo;
  samplerCreateInfo(&samplerInfo, inMagFilter, inMinFilter, inCompareEnable, inCompareOp, inAddressU, inAddressV,
                    inAddressW, inMipMode, inMinLod, inMaxLod, inMipLodBias, inMaxAnsisotropy, inBorderColor);

  VKA_CHECK_ERROR(vkCreateSampler(getDefaultDevice(), &samplerInfo, NULL, outSampler), "Could not create sampler for image texture.\n");
}

void deviceCreateInfo(VkDeviceCreateInfo*       outInfo,
                      uint32_t                  inQueueCount,
                      VkDeviceQueueCreateInfo*  inQueues,
                      uint32_t                  inExtensionCount,
                      const char* const*        inExtensionNames,
                      uint32_t                  inLayerCount,
                      const char* const*        inLayerNames,
                      VkPhysicalDeviceFeatures* inFeatures)
{

  memset(outInfo, 0, sizeof(VkDeviceCreateInfo));
  outInfo->sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  outInfo->queueCreateInfoCount    = inQueueCount;
  outInfo->pQueueCreateInfos       = inQueues;
  outInfo->enabledLayerCount       = inLayerCount;
  outInfo->ppEnabledLayerNames     = inLayerNames;
  outInfo->enabledExtensionCount   = inExtensionCount;
  outInfo->ppEnabledExtensionNames = inExtensionNames;
  outInfo->pEnabledFeatures        = inFeatures;
}

void deviceCreate(VkDevice*                 outDevice,
                  VkPhysicalDevice*         inPhysicalDevice,
                  uint32_t                  inQueueCount,
                  VkDeviceQueueCreateInfo*  inQueues,
                  uint32_t                  inExtensionCount,
                  const char* const*        inExtensionNames,
                  uint32_t                  inLayerCount,
                  const char* const*        inLayerNames,
                  VkPhysicalDeviceFeatures* inFeatures)
{

  VkDeviceCreateInfo devInfo;

  deviceCreateInfo(&devInfo, 1, inQueues, inExtensionCount, NULL, inLayerCount, inLayerNames, inFeatures);

  VKA_CHECK_ERROR(vkCreateDevice(*inPhysicalDevice, &devInfo, NULL, outDevice), "Could not create logical device.\n");
}
