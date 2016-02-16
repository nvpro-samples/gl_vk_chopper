/*-----------------------------------------------------------------------
Copyright (c) 2014-2016, NVIDIA. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Neither the name of its contributors may be used to endorse
or promote products derived from this software without specific
prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------*/
/* Contact chebert@nvidia.com (Chris Hebert) for feedback */

#include "VkeCubeTexture.h"
#include "vkaUtils.h"
#include "VkeCreateUtils.h"
#include <iostream>
#include<main.h>
#include <nv_helpers_gl/nv_dds.h>

#ifndef INIT_COMMAND_ID
#define INIT_COMMAND_ID 1
#endif

VkeCubeTexture::VkeCubeTexture() :
m_width(0),
m_height(0),
m_mip_level(0),
m_id(0),
m_ready(false)
{
	initTexture();
}

VkeCubeTexture::VkeCubeTexture(const ID &inID) :
m_width(0),
m_height(0),
m_mip_level(0),
m_id(0),
m_ready(false)
{
	initTexture();
}


VkeCubeTexture::~VkeCubeTexture()
{
}

void VkeCubeTexture::initTexture(){
	m_tiling = VK_IMAGE_TILING_LINEAR;
	m_memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	m_usage_flags = VK_IMAGE_USAGE_SAMPLED_BIT;
	m_format = VK_FORMAT_R8G8B8A8_UNORM;
}

void VkeCubeTexture::loadCubeDDS(const char *inFile){


	std::string searchPaths[] = {
		std::string(PROJECT_NAME),
		NVPWindow::sysExePath() + std::string(PROJECT_RELDIRECTORY),
		std::string(PROJECT_ABSDIRECTORY)
	};

	nv_dds::CDDSImage ddsImage;

	for (uint32_t i = 0; i < 3; ++i){
        std::string separator = "";
        uint32_t strSize = searchPaths[i].size();
        if(searchPaths[i].substr(strSize-1,strSize) != "/") separator = "/";
        std::string filePath = searchPaths[i] + separator + std::string("images/") + std::string(inFile);
        ddsImage.load(filePath, true);
		if (ddsImage.is_valid()) break;
	}

	if (!ddsImage.is_valid()){
		perror("Could not cube load texture image.\n");
		exit(1);
	}

	uint32_t imgW = ddsImage.get_width();
	uint32_t imgH = ddsImage.get_height();
	uint32_t comCount = ddsImage.get_components();
	uint32_t fmt = ddsImage.get_format();

	bool isCube = ddsImage.is_cubemap();
	bool isComp = ddsImage.is_compressed();

	VkFormat vkFmt = VK_FORMAT_R8G8B8A8_UNORM;

	switch (fmt){
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


	m_width = imgW;
	m_height = imgH;
	m_format = vkFmt;

	VulkanDC::Device::Queue::Name queueName = "DEFAULT_GRAPHICS_QUEUE";
	VulkanDC::Device::Queue::CommandBufferID cmdID = INIT_COMMAND_ID;
	VulkanDC *dc = VulkanDC::Get();
	VulkanDC::Device *device = dc->getDefaultDevice();
	VulkanDC::Device::Queue *queue = device->getQueue(queueName);
	VkCommandBuffer cmd = VK_NULL_HANDLE;

	queue->beginCommandBuffer(cmdID, &cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	imageCreateAndBind(
		&m_data.image,
		&m_data.memory,
		m_format, VK_IMAGE_TYPE_2D,
		m_width, m_height, 1, 6,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		(VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
		VK_IMAGE_TILING_OPTIMAL);

	VkBuffer cubeMapBuffer;
	VkDeviceMemory cubeMapMem;

	bufferCreate(&cubeMapBuffer, m_width*m_height * 3 * 6, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	bufferAlloc(&cubeMapBuffer, &cubeMapMem, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);


	if (m_memory_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT){
		imageSetLayoutBarrier(cmdID, queueName, m_data.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_GENERAL);

		for (uint32_t i = 0; i < 6; ++i){

			void *data = NULL;
			VkSubresourceLayout layout;
			VkImageSubresource subres;
			subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subres.mipLevel = m_mip_level;
			subres.arrayLayer = i;
			vkGetImageSubresourceLayout(getDefaultDevice(), m_data.image, &subres, &layout);


			VKA_CHECK_ERROR(vkMapMemory(getDefaultDevice(), cubeMapMem, layout.offset, layout.size, 0, &data), "Could not map memory for image.\n");

			const nv_dds::CTexture &mipmap = ddsImage.get_cubemap_face(i);

			memcpy(data, (void *)mipmap, layout.size);



			vkUnmapMemory(getDefaultDevice(), cubeMapMem);
		}

		VkBufferImageCopy biCpyRgn[6];


		for (uint32_t k = 0; k < 6; ++k){
			VkSubresourceLayout layout;
			VkImageSubresource subres;
			subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subres.mipLevel = m_mip_level;
			subres.arrayLayer = k;
			vkGetImageSubresourceLayout(getDefaultDevice(), m_data.image, &subres, &layout);

			biCpyRgn[k].bufferOffset = layout.offset;
			biCpyRgn[k].bufferImageHeight = 0;
			biCpyRgn[k].bufferRowLength = 0;
			biCpyRgn[k].imageExtent.width = m_width;
			biCpyRgn[k].imageExtent.height = m_height;
			biCpyRgn[k].imageExtent.depth = 1;
			biCpyRgn[k].imageOffset.x = 0;
			biCpyRgn[k].imageOffset.y = 0;
			biCpyRgn[k].imageOffset.z = 0;
			biCpyRgn[k].imageSubresource.baseArrayLayer = k;
			biCpyRgn[k].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			biCpyRgn[k].imageSubresource.layerCount = 1;
			biCpyRgn[k].imageSubresource.mipLevel = 0;

		}

		VkFence copyFence;
		VkFenceCreateInfo fenceInfo;
		memset(&fenceInfo, 0, sizeof(fenceInfo));
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;


		vkCreateFence(device->getVKDevice(), &fenceInfo, NULL, &copyFence);

		vkCmdCopyBufferToImage(cmd, cubeMapBuffer, m_data.image, m_data.imageLayout, 6, biCpyRgn);
		queue->flushCommandBuffer(cmdID, &copyFence);

		vkWaitForFences(device->getVKDevice(), 1, &copyFence, VK_TRUE, 100000000000);

		vkDestroyBuffer(device->getVKDevice(), cubeMapBuffer, NULL);
		vkFreeMemory(device->getVKDevice(), cubeMapMem, NULL);

	}


	VkSamplerCreateInfo sampler;

	sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler.pNext = NULL;
	sampler.magFilter = VK_FILTER_NEAREST;
	sampler.minFilter = VK_FILTER_NEAREST;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.mipLodBias = 0.0f;
	sampler.maxAnisotropy = 1;
	sampler.compareOp = VK_COMPARE_OP_NEVER;
	sampler.minLod = 0.0f;
	sampler.maxLod = 0.0f;

	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

	VkImageViewCreateInfo view;
	view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view.pNext = NULL;
	view.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	view.format = m_format;
	view.components.r = VK_COMPONENT_SWIZZLE_R;
	view.components.g = VK_COMPONENT_SWIZZLE_G;
	view.components.b = VK_COMPONENT_SWIZZLE_B;
	view.components.a = VK_COMPONENT_SWIZZLE_A;

	view.subresourceRange.baseArrayLayer = 0;
	view.subresourceRange.levelCount = 1;
	view.subresourceRange.baseMipLevel = 0;
	view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	view.subresourceRange.layerCount = 1;

	VKA_CHECK_ERROR(vkCreateSampler(getDefaultDevice(), &sampler, NULL, &m_data.sampler), "Could not create sampler for image texture.\n");

	view.image = m_data.image;

	VKA_CHECK_ERROR(vkCreateImageView(getDefaultDevice(), &view, NULL, &m_data.view), "Could not create image view for texture.\n");





}

#ifdef USE_LIB_PNG
void VkeCubeTexture::loadTextureFiles(const char **inPath){

	bool imagesOK = true;
	VKA_INFO_MSG("Loading Cube Texture.\n");
	for (uint32_t i = 0; i < 6; ++i){
		if (!loadTexture(inPath[i], NULL, NULL, &m_width, &m_height)){
			VKA_ERROR_MSG("Error loading texture image.\n");
			printf("Texture : %d not available (%s).\n", i, inPath[i]);
			return;
		}
	}

	VulkanDC::Device::Queue::Name queueName = "DEFAULT_GRAPHICS_QUEUE";
	VulkanDC::Device::Queue::CommandBufferID cmdID = INIT_COMMAND_ID;
	VulkanDC *dc = VulkanDC::Get();
	VulkanDC::Device *device = dc->getDefaultDevice();
	VulkanDC::Device::Queue *queue = device->getQueue(queueName);
	VkCommandBuffer cmd = VK_NULL_HANDLE;

	queue->beginCommandBuffer(cmdID, &cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	imageCreateAndBind(
		&m_data.image,
		&m_data.memory,
		m_format, VK_IMAGE_TYPE_2D,
		m_width, m_height, 1, 6,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		(VkImageUsageFlagBits)( VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT ),
		VK_IMAGE_TILING_OPTIMAL);

	VkBuffer cubeMapBuffer;
	VkDeviceMemory cubeMapMem;

	bufferCreate(&cubeMapBuffer, m_width*m_height * 4 * 6, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	bufferAlloc(&cubeMapBuffer, &cubeMapMem, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	VkDeviceSize dSize = m_width * m_height * 4;
	uint32_t rowPitch = m_width * 4;

	if (m_memory_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT){
		imageSetLayoutBarrier(cmdID, queueName, m_data.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_GENERAL);

		for (uint32_t i = 0; i < 6; ++i){

			void *data = NULL;
			VkDeviceSize ofst = dSize*i;

			VKA_CHECK_ERROR(vkMapMemory(getDefaultDevice(),cubeMapMem, ofst, dSize, 0, &data), "Could not map memory for image.\n");

			if (!loadTexture(inPath[i], (uint8_t**)&data, rowPitch, &m_width, &m_height)){
				VKA_ERROR_MSG("Could not load final image.\n");
			}

			vkUnmapMemory(getDefaultDevice(), cubeMapMem);
		}

		VkBufferImageCopy biCpyRgn[6];
			

		for (uint32_t k = 0; k < 6; ++k){
			VkDeviceSize ofst = dSize*k;

			biCpyRgn[k].bufferOffset = ofst;
			biCpyRgn[k].bufferImageHeight = 0;
			biCpyRgn[k].bufferRowLength = 0;
			biCpyRgn[k].imageExtent.width = m_width;
			biCpyRgn[k].imageExtent.height = m_height;
			biCpyRgn[k].imageExtent.depth = 1;
			biCpyRgn[k].imageOffset.x = 0;
			biCpyRgn[k].imageOffset.y = 0;
			biCpyRgn[k].imageOffset.z = 0;
			biCpyRgn[k].imageSubresource.baseArrayLayer = k;
			biCpyRgn[k].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			biCpyRgn[k].imageSubresource.layerCount = 1;
			biCpyRgn[k].imageSubresource.mipLevel = 0;

		}

		VkFence copyFence;
		VkFenceCreateInfo fenceInfo;
		memset(&fenceInfo, 0, sizeof(fenceInfo));
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		

		vkCreateFence(device->getVKDevice(), &fenceInfo,NULL , &copyFence);

		vkCmdCopyBufferToImage(cmd, cubeMapBuffer, m_data.image, m_data.imageLayout, 6, biCpyRgn);
		queue->flushCommandBuffer(cmdID , &copyFence);

		vkWaitForFences(device->getVKDevice(), 1, &copyFence, VK_TRUE, 100000000000);
		
		vkDestroyBuffer(device->getVKDevice(), cubeMapBuffer, NULL);
		vkFreeMemory(device->getVKDevice(), cubeMapMem, NULL);

	}


	VkSamplerCreateInfo sampler;
	
	sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler.pNext = NULL;
	sampler.magFilter = VK_FILTER_NEAREST;
	sampler.minFilter = VK_FILTER_NEAREST;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.mipLodBias = 0.0f;
	sampler.maxAnisotropy = 1;
	sampler.compareOp = VK_COMPARE_OP_NEVER;
	sampler.minLod = 0.0f;
	sampler.maxLod = 0.0f;

	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

	VkImageViewCreateInfo view;
	view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view.pNext = NULL;
	view.viewType = VK_IMAGE_VIEW_TYPE_CUBE; 
	view.format = m_format;
	view.components = {
		
		VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A
	};
	
	view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 0 };
    view.subresourceRange.baseArrayLayer = 0;
	view.subresourceRange.levelCount = 1;
	view.subresourceRange.baseMipLevel = 0;
    view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	view.subresourceRange.layerCount = 1;

	VKA_CHECK_ERROR(vkCreateSampler(getDefaultDevice(), &sampler,NULL, &m_data.sampler), "Could not create sampler for image texture.\n");

	view.image = m_data.image;

	VKA_CHECK_ERROR(vkCreateImageView(getDefaultDevice(), &view,NULL, &m_data.view), "Could not create image view for texture.\n");


	VKA_INFO_MSG("Created CUBE Image Texture.\n");

}

#endif

VkeCubeTexture::List::List(){}
VkeCubeTexture::List::~List(){}

VkeCubeTexture::ID VkeCubeTexture::List::nextID(){
	VkeCubeTexture::ID outID;
	if (m_deleted_keys.size() == 0) return m_data.size();
	outID = m_deleted_keys.back();
	m_deleted_keys.pop_back();
	return outID;
}

VkeCubeTexture::Count VkeCubeTexture::List::count(){
	return m_data.size();
}

VkeCubeTexture *VkeCubeTexture::List::newTexture(){
	VkeCubeTexture::ID id = nextID();
	return newTexture(id);
}

VkeCubeTexture *VkeCubeTexture::List::newTexture(const VkeCubeTexture::ID &inID){
	VkeCubeTexture *outMaterial = new VkeCubeTexture(inID);
	m_data[inID] = outMaterial;
	return outMaterial;
}


void VkeCubeTexture::List::addTexture(VkeCubeTexture * const inMaterial){
	VkeCubeTexture::ID id = nextID();
	m_data[id] = inMaterial;
}

VkeCubeTexture *VkeCubeTexture::List::getTexture(const VkeCubeTexture::ID &inID){
	return m_data[inID];
}

void VkeCubeTexture::List::getData(VkeCubeTexture::Data *outData, uint32_t offset){
	VkeCubeTexture::Map::iterator itr;
	uint32_t i = offset;
	for (itr = m_data.begin(); itr != m_data.end(); ++itr){
		outData[i++] = itr->second->getData();
	}
}
