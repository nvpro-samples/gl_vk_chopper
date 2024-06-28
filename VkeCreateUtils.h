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

#include "VulkanDeviceContext.h"
#include "vkaUtils.h"
#include <vulkan/vulkan.h>

VkDevice getDefaultDevice();


uint32_t getMemoryType(uint32_t typeBits, VkFlags props);

VkResult vulkanCreateInstance(const VkInstanceCreateInfo* pCreateInfo, VkInstance* pInstance);


void imageSetLayoutBarrier(VulkanDC::Device::Queue::CommandBufferID& inCommandID,
                           VulkanDC::Device::Queue::Name&            inQueueName,
                           VkImage                                   image,
                           VkImageAspectFlags                        aspect,
                           VkImageLayout                             oldLayout,
                           VkImageLayout                             newLayout,
                           VkAccessFlagBits inSrcAccessmask  = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
                           VkAccessFlagBits inDstAccessmask  = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                           uint32_t         inArraySize      = 1,
                           uint32_t         inBaseArraySlice = 0);

void imageSetLayout(VkCommandBuffer*   inCmd,
                    VkImage            image,
                    VkImageAspectFlags aspect,
                    VkImageLayout      oldLayout,
                    VkImageLayout      newLayout,
                    uint32_t           inArraySize      = 1,
                    uint32_t           inBaseArraySlice = 0);

void commandPoolCreateInfo(VkCommandPoolCreateInfo* outPool, uint32_t inQueueFamilyIndex, VkCommandPoolCreateFlags inFlags = 0);

void commandPoolCreate(VkCommandPool* outPool, uint32_t inQueueFamilyIndex, VkCommandPoolCreateFlags inFlags = 0);

void createBufferData(uint8_t* inputData, size_t inputSize, BufferData* outData, VkBufferUsageFlagBits usage, VkMemoryPropertyFlagBits flags);

void bufferCreateInfo(VkBufferCreateInfo* outBuffer, size_t inputSize, VkBufferUsageFlags inUsage);

void bufferCreate(VkBuffer* outBuffer, size_t inputSize, VkBufferUsageFlags inUsage);

void bufferAlloc(VkBuffer* inBuffer, VkDeviceMemory* outMemory, VkMemoryPropertyFlags inFlags);

void bufferViewCreate(VkBuffer* inBuffer, VkBufferView* outView, size_t inSize, size_t inOffset = 0);

void imageCreateInfo(VkImageCreateInfo*    outInfo,
                     VkFormat              inFormat,
                     VkImageType           inType,
                     uint32_t              inWidth,
                     uint32_t              inHeight,
                     uint32_t              inDepth     = 1,
                     uint32_t              inArraySize = 1,
                     VkImageUsageFlags     inUsage     = VK_IMAGE_USAGE_SAMPLED_BIT,
                     VkImageTiling         inTiling    = VK_IMAGE_TILING_LINEAR,
                     VkSampleCountFlagBits inSamples   = VK_SAMPLE_COUNT_1_BIT,
                     uint32_t              inMipLevels = 1);

void imageCreate(VkImage*              outImage,
                 VkDeviceMemory*       outMemory,
                 VkFormat              inFormat,
                 VkImageType           inType,
                 uint32_t              inWidth,
                 uint32_t              inHeight,
                 uint32_t              inDepth     = 1,
                 uint32_t              inArraySize = 1,
                 VkFlags               inMemFlags  = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                 VkImageUsageFlags     inUsage     = VK_IMAGE_USAGE_SAMPLED_BIT,
                 VkImageTiling         inTiling    = VK_IMAGE_TILING_LINEAR,
                 VkSampleCountFlagBits inSamples   = VK_SAMPLE_COUNT_1_BIT,
                 uint32_t              inMipLevels = 1);

void imageCreateAndBind(VkImage*              outImage,
                        VkDeviceMemory*       outMemory,
                        VkFormat              inFormat,
                        VkImageType           inType,
                        uint32_t              inWidth,
                        uint32_t              inHeight,
                        uint32_t              inDepth     = 1,
                        uint32_t              inArraySize = 1,
                        VkFlags               inMemFlags  = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                        VkImageUsageFlags     inUsage     = VK_IMAGE_USAGE_SAMPLED_BIT,
                        VkImageTiling         inTiling    = VK_IMAGE_TILING_LINEAR,
                        VkSampleCountFlagBits inSamples   = VK_SAMPLE_COUNT_1_BIT,
                        uint32_t              inMipLevels = 1);


void attachmentDescriptionInit(VkAttachmentDescription* outDescription);

void attachmentDescription(VkAttachmentDescription* outDescription,
                           VkFormat                 inFormat        = VK_FORMAT_R8G8B8A8_UNORM,
                           VkSampleCountFlagBits    inSamples       = VK_SAMPLE_COUNT_1_BIT,
                           VkAttachmentLoadOp       inLoadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR,
                           VkAttachmentStoreOp      inStoreOp       = VK_ATTACHMENT_STORE_OP_STORE,
                           VkImageLayout            inInitialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                           VkImageLayout            inFinalLayout   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

void depthStencilAttachmentDescription(VkAttachmentDescription* outDescription,
                                       VkFormat                 inFormat  = VK_FORMAT_D32_SFLOAT,
                                       VkSampleCountFlagBits    inSamples = VK_SAMPLE_COUNT_1_BIT,
                                       VkAttachmentLoadOp       inLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                       VkAttachmentStoreOp      inStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                       VkImageLayout inInitialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                       VkImageLayout inFinalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

void subpassDescriptionInit(VkSubpassDescription* outDescription);

void subpassDescription(VkSubpassDescription*     outDescription,
                        uint32_t                  inColorCount,
                        VkAttachmentReference*    inColorAttachments,
                        VkAttachmentReference*    inDepthStencilAttachment,
                        VkAttachmentReference*    inResolveAttachments  = NULL,
                        uint32_t                  inInputCount          = 0,
                        VkAttachmentReference*    inInputAttachments    = NULL,
                        VkSubpassDescriptionFlags inFlags               = 0,
                        VkPipelineBindPoint       inBindPoint           = VK_PIPELINE_BIND_POINT_GRAPHICS,
                        uint32_t                  inPreserveCount       = 0,
                        VkAttachmentReference*    inPreserveAttachments = NULL);

void renderPassCreate(VkRenderPass*            outPass,
                      uint32_t                 inAttachmentCount,
                      VkAttachmentDescription* inAttachments,
                      uint32_t                 inSubPassCount,
                      VkSubpassDescription*    inSubPasses,
                      uint32_t                 inDependencyCount = 0,
                      VkSubpassDependency*     inDependencies    = NULL);

void viewportStateInfo(VkPipelineViewportStateCreateInfo* outInfo, uint32_t inViewportCount = 1);

void depthStateInfo(VkPipelineDepthStencilStateCreateInfo* outInfo,
                    VkBool32                               inDepthTestEnable   = VK_TRUE,
                    VkBool32                               inDepthWriteEnable  = VK_TRUE,
                    VkCompareOp                            inDepthCompareOp    = VK_COMPARE_OP_LESS_OR_EQUAL,
                    VkBool32                               inDepthBoundsEnable = VK_FALSE,
                    VkStencilOp                            inStencilFailOp     = VK_STENCIL_OP_KEEP,
                    VkStencilOp                            inStencilPassOp     = VK_STENCIL_OP_KEEP,
                    VkCompareOp                            inStencilCompareOp  = VK_COMPARE_OP_ALWAYS,
                    VkBool32                               inStencilTestEnable = VK_FALSE);

void multisampleStateInfo(VkPipelineMultisampleStateCreateInfo* outInfo,
                          VkSampleCountFlagBits                 inSamples             = VK_SAMPLE_COUNT_1_BIT,
                          VkSampleMask*                         inSampleMask          = NULL,
                          VkBool32                              inSampleShadingEnable = VK_FALSE,
                          float                                 inMinSampleShading    = 1.0);

void blendStateInfo(VkPipelineColorBlendStateCreateInfo* outInfo,
                    uint32_t                             inAttachmentCount,
                    VkPipelineColorBlendAttachmentState* inAttachments,
                    VkBool32                             inAlphaToCoverageEnable = VK_FALSE);

void blendAttachmentState(VkPipelineColorBlendAttachmentState* outState,
                          VkBool32                             inBlendEnable  = VK_TRUE,
                          VkBlendOp                            inBlendOpAlpha = VK_BLEND_OP_ADD,
                          VkBlendOp                            inBlendOpColor = VK_BLEND_OP_ADD,
                          VkBlendFactor                        inSrcAlpha     = VK_BLEND_FACTOR_SRC_ALPHA,
                          VkBlendFactor                        inDestAlpha    = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                          VkBlendFactor                        inSrcColor     = VK_BLEND_FACTOR_SRC_ALPHA,
                          VkBlendFactor                        inDestColor    = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                          VkColorComponentFlags                inWriteMask    = 0xf);

void blendAttachmentStateN(uint32_t                             inCount,
                           VkPipelineColorBlendAttachmentState* outState,
                           VkBool32                             inBlendEnable  = VK_TRUE,
                           VkBlendOp                            inBlendOpAlpha = VK_BLEND_OP_ADD,
                           VkBlendOp                            inBlendOpColor = VK_BLEND_OP_ADD,
                           VkBlendFactor                        inSrcAlpha     = VK_BLEND_FACTOR_SRC_ALPHA,
                           VkBlendFactor                        inDestAlpha    = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                           VkBlendFactor                        inSrcColor     = VK_BLEND_FACTOR_SRC_ALPHA,
                           VkBlendFactor                        inDestColor    = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                           VkColorComponentFlags                inWriteMask    = 0xf);

void rasterStateInfo(VkPipelineRasterizationStateCreateInfo* outInfo,
                     VkPolygonMode                           inFillMode        = VK_POLYGON_MODE_FILL,
                     VkCullModeFlags                         inCullMode        = VK_CULL_MODE_BACK_BIT,
                     VkFrontFace                             inFrontFace       = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                     VkBool32                                inDepthClipEnable = VK_TRUE,
                     VkBool32                                inDiscardEnable   = VK_FALSE);

void inputAssemblyStateInfo(VkPipelineInputAssemblyStateCreateInfo* outInfo,
                            VkPrimitiveTopology                     inTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                            VkBool32                                inRestartEnable = VK_FALSE);

void vertexStateInfo(VkPipelineVertexInputStateCreateInfo* outInfo,
                     uint32_t                              inBindingCount,
                     uint32_t                              inAttributeCount,
                     VkVertexInputBindingDescription*      inBindings,
                     VkVertexInputAttributeDescription*    inAttributes);

void vertexAttributef(VkVertexInputAttributeDescription* outDescription, uint32_t inLocation, uint32_t inBinding, VkFormat inFormat, uint32_t inOffset);

void vertexBinding(VkVertexInputBindingDescription* outDescription, uint32_t inBinding, uint32_t inStride, VkVertexInputRate inStepRate);

void imageBarrierCreate(VkCommandBuffer*         inCmd,
                        VkImageLayout            inOldLayout,
                        VkImageLayout            inNewLayout,
                        VkImage                  inImage,
                        VkImageSubresourceRange& inSubRange,
                        VkAccessFlagBits         inSrcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
                        VkAccessFlagBits         inDstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

void imageSubresourceRange(VkImageSubresourceRange* outRange,
                           VkImageAspectFlags       inAspect,
                           uint32_t                 inArraySize,
                           uint32_t                 inBaseArraySlice,
                           uint32_t                 inMipLevels    = 1,
                           uint32_t                 inBaseMipLevel = 0);


void colorClearValues(VkClearValue* outValue, float inR = 0.0, float inG = 0.0, float inB = 0.0, float inA = 1.0);

void depthStencilClearValues(VkClearValue* outValue, float inD = 1.0, uint32_t inS = 0);

void renderPassBegin(VkCommandBuffer*  inCmd,
                     VkRenderPass&     inRenderPass,
                     VkFramebuffer&    inFrameBuffer,
                     uint32_t          inX,
                     uint32_t          inY,
                     uint32_t          inWidth,
                     uint32_t          inHeight,
                     VkClearValue*     inClearValues,
                     uint32_t          inClearCount,
                     VkSubpassContents inContents = VK_SUBPASS_CONTENTS_INLINE);

void descriptorSetWrite(VkWriteDescriptorSet*   outWrite,
                        uint32_t                inDestBinding,
                        VkDescriptorType        inType,
                        uint32_t                inCount,
                        VkDescriptorBufferInfo* inDescriptorBufferInfo,
                        VkDescriptorImageInfo*  inDescriptorImageInfo,
                        uint32_t                inDestElement = 0,
                        VkDescriptorSet         inDestSet     = 0);

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
                            VkPipelineCache*                        inCache);

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
                            uint32_t                                inSubPass    = 0,
                            VkPipelineCreateFlags                   inFlags      = 0,
                            VkPipeline                              basePipeline = VK_NULL_HANDLE);


void createShader(const char* fileName, VkShaderStageFlagBits inStage, VkShaderModule* outModule);

void createShaderStage(VkPipelineShaderStageCreateInfo* outStage, VkShaderStageFlagBits inStage, VkShaderModule inShader);

void layoutBinding(VkDescriptorSetLayoutBinding* outBinding,
                   uint32_t                      binding,
                   VkDescriptorType              intype,
                   VkShaderStageFlags            inFlags,
                   uint32_t                      inArraySize         = 1,
                   VkSampler*                    inImmutableSamplers = NULL);

void descriptorSetLayoutCreateInfo(VkDescriptorSetLayoutCreateInfo* outInfo, uint32_t inCount, VkDescriptorSetLayoutBinding* inBindings);

void descriptorSetLayoutCreate(VkDescriptorSetLayout* outLayout, uint32_t inCount, VkDescriptorSetLayoutBinding* inBindings);

void pipelineLayoutCreateInfo(VkPipelineLayoutCreateInfo* outInfo,
                              uint32_t                    inCount,
                              VkDescriptorSetLayout*      inLayouts,
                              uint32_t                    inPushConstantCount  = 0,
                              VkPushConstantRange*        inPushConstantRanges = NULL);

void pipelineLayoutCreate(VkPipelineLayout*      outLayout,
                          uint32_t               inCount,
                          VkDescriptorSetLayout* inLayouts,
                          uint32_t               inPushConstantCount  = 0,
                          VkPushConstantRange*   inPushConstantRanges = NULL);

void imageViewCreateInfo(VkImageViewCreateInfo* outInfo,
                         VkImage                inImage,
                         VkImageViewType        inType           = VK_IMAGE_VIEW_TYPE_2D,
                         VkFormat               inFormat         = VK_FORMAT_R8G8B8A8_UNORM,
                         VkImageAspectFlagBits  inAspect         = VK_IMAGE_ASPECT_COLOR_BIT,
                         uint32_t               inArraySize     = 1,
                         uint32_t               inBaseArraySlice = 0,
                         VkComponentSwizzle     inR              = VK_COMPONENT_SWIZZLE_R,
                         VkComponentSwizzle     inG              = VK_COMPONENT_SWIZZLE_G,
                         VkComponentSwizzle     inB              = VK_COMPONENT_SWIZZLE_B,
                         VkComponentSwizzle     inA              = VK_COMPONENT_SWIZZLE_A);

void imageViewCreate(VkImageView*       outView,
                     VkImage            inImage,
                     VkImageViewType    inType           = VK_IMAGE_VIEW_TYPE_2D,
                     VkFormat           inFormat         = VK_FORMAT_R8G8B8A8_UNORM,
                     VkImageAspectFlags inAspect         = VK_IMAGE_ASPECT_COLOR_BIT,
                     uint32_t           inArraySize     = 1,
                     uint32_t           inBaseArraySlice = 0,
                     VkComponentSwizzle inR              = VK_COMPONENT_SWIZZLE_R,
                     VkComponentSwizzle inG              = VK_COMPONENT_SWIZZLE_G,
                     VkComponentSwizzle inB              = VK_COMPONENT_SWIZZLE_B,
                     VkComponentSwizzle inA              = VK_COMPONENT_SWIZZLE_A);

void samplerCreateInfo(VkSamplerCreateInfo* outInfo,
                       VkFilter             inMagFilter      = VK_FILTER_NEAREST,
                       VkFilter             inMinFilter      = VK_FILTER_NEAREST,
                       VkBool32             inCompareEnable  = VK_FALSE,
                       VkCompareOp          inCompareOp      = VK_COMPARE_OP_LESS_OR_EQUAL,
                       VkSamplerAddressMode inAddressU       = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                       VkSamplerAddressMode inAddressV       = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                       VkSamplerAddressMode inAddressW       = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                       VkSamplerMipmapMode  inMipMode        = VK_SAMPLER_MIPMAP_MODE_NEAREST,
                       float                inMinLod         = 0.0f,
                       float                inMaxLod         = 0.0f,
                       float                inMipLodBias     = 0.0f,
                       float                inMaxAnsisotropy = 1.0f,
                       VkBorderColor        inBorderColor    = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);

void samplerCreate(VkSampler*           outSampler,
                   VkFilter             inMagFilter      = VK_FILTER_NEAREST,
                   VkFilter             inMinFilter      = VK_FILTER_NEAREST,
                   VkBool32             inCompareEnable  = VK_FALSE,
                   VkCompareOp          inCompareOp      = VK_COMPARE_OP_LESS_OR_EQUAL,
                   VkSamplerAddressMode inAddressU       = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                   VkSamplerAddressMode inAddressV       = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                   VkSamplerAddressMode inAddressW       = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                   VkSamplerMipmapMode  inMipMode        = VK_SAMPLER_MIPMAP_MODE_NEAREST,
                   float                inMinLod         = 0.0f,
                   float                inMaxLod         = 0.0f,
                   float                inMipLodBias     = 0.0f,
                   float                inMaxAnsisotropy = 1.0f,
                   VkBorderColor        inBorderColor    = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE);


void deviceCreateInfo(VkDeviceCreateInfo*       outInfo,
                      uint32_t                  inDeviceCount,
                      VkDeviceQueueCreateInfo*  inQueues,
                      uint32_t                  inExtensionCount,
                      const char* const*        inExtensionNames,
                      uint32_t                  inLayerCount = 0,
                      const char* const*        inLayerNames = NULL,
                      VkPhysicalDeviceFeatures* inFeatures   = NULL);

void deviceCreate(VkDevice*                 outDevice,
                  VkPhysicalDevice*         inPhysicalDevice,
                  uint32_t                  inDeviceCount,
                  VkDeviceQueueCreateInfo*  inQueues,
                  uint32_t                  inExtensionCount,
                  const char* const*        inExtensionNames,
                  uint32_t                  inLayerCount = 0,
                  const char* const*        inLayerNames = NULL,
                  VkPhysicalDeviceFeatures* inFeatures   = NULL);
