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

#include <include_gl.h>

#include "VkeTexture.h"
#include "vkaUtils.h"
#include "VkeCreateUtils.h"
#include <fileformats/nv_dds.h>
#include "nvh/nvprint.hpp"
#include "nvpwindow.hpp"
#include<iostream>

#ifndef INIT_COMMAND_ID
#define INIT_COMMAND_ID 1
#endif

VkeTexture::VkeTexture():
m_width(0),
m_height(0),
m_mip_level(0),
m_id(0),
m_ready(false)
{
	initTexture();
}

VkeTexture::VkeTexture(const ID &inID) :
m_width(0),
m_height(0),
m_mip_level(0),
m_id(0),
m_ready(false)
{
	initTexture();
}


VkeTexture::~VkeTexture()
{
}

void VkeTexture::initTexture(){
	m_tiling = VK_IMAGE_TILING_LINEAR;
	m_memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	m_usage_flags = VK_IMAGE_USAGE_SAMPLED_BIT;
	m_format = VK_FORMAT_R8G8B8A8_UNORM;
}

void VkeTexture::loadDDSTextureFile(const char *inFile){



    std::vector<std::string> searchPaths;
    searchPaths.push_back(std::string("."));
    searchPaths.push_back(std::string("./resources_" PROJECT_NAME));
    searchPaths.push_back(std::string(PROJECT_NAME) + std::string("/images"));
    searchPaths.push_back(NVPSystem::exePath() + std::string(PROJECT_RELDIRECTORY) + std::string("/images"));
    //searchPaths.push_back(std::string(PROJECT_ABSDIRECTORY);

	nv_dds::CDDSImage ddsImage;

    std::string filePath;
	for (uint32_t i = 0; i < searchPaths.size(); ++i){
		filePath = searchPaths[i] + "/" + std::string(inFile);
		ddsImage.load(filePath, true);
		if (ddsImage.is_valid()) break;
	}

	if (!ddsImage.is_valid()){
        LOGE("Could not load texture image %s\n", inFile);
		exit(1);
	} else {
        LOGI("loaded texture image %s\n", filePath.c_str());
    }

	uint32_t imgW = ddsImage.get_width();
	uint32_t imgH = ddsImage.get_height();
	uint32_t comCount = ddsImage.get_components();
	uint32_t fmt = ddsImage.get_format();
	
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



	VulkanDC::Device::Queue::Name queueName = "DEFAULT_GRAPHICS_QUEUE";
	VulkanDC::Device::Queue::CommandBufferID cmdID = INIT_COMMAND_ID + 200;
	VulkanDC *dc = VulkanDC::Get();
	VulkanDC::Device *device = dc->getDefaultDevice();
	VulkanDC::Device::Queue *queue = device->getQueue(queueName);
	device->waitIdle();

	TextureObject stagingTex;

	m_width = imgW;
	m_height = imgH;
	m_format = vkFmt;

	imageCreateAndBind(
		&m_data.image,
		&m_data.memory,
		m_format, VK_IMAGE_TYPE_2D,
		m_width, m_height, 1, 1,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		(VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
		VK_IMAGE_TILING_OPTIMAL);

	imageCreateAndBind(
		&stagingTex.image,
		&stagingTex.memory,
		m_format, VK_IMAGE_TYPE_2D,
		m_width, m_height, 1, 1,
		m_memory_flags,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VK_IMAGE_TILING_LINEAR);

	if (m_memory_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT){


		VkImageSubresource subres;
		subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subres.mipLevel = m_mip_level;
		subres.arrayLayer = 0;

		VkSubresourceLayout layout;
		void *data;

		vkGetImageSubresourceLayout(getDefaultDevice(), stagingTex.image, &subres, &layout);
    VKA_CHECK_ERROR(vkMapMemory(getDefaultDevice(), stagingTex.memory, 0, VK_WHOLE_SIZE, 0, &data), "Could not map memory for image.\n");
	
		const nv_dds::CSurface &mipmap = ddsImage.get_mipmap(0);
		uint32_t sz = mipmap.get_size();

		memcpy(data, (void *)mipmap, layout.size);

		vkUnmapMemory(getDefaultDevice(), stagingTex.memory);
		VkCommandBuffer cmd = VK_NULL_HANDLE;
		queue->beginCommandBuffer(cmdID, &cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		m_data.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


		VkImageCopy cpyRgn[1];

		cpyRgn[0].srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		cpyRgn[0].srcSubresource.baseArrayLayer = 0;
		cpyRgn[0].srcSubresource.mipLevel = 0;
		cpyRgn[0].srcSubresource.layerCount = 1;

		cpyRgn[0].srcOffset.x = 0;
		cpyRgn[0].srcOffset.y = 0;
		cpyRgn[0].srcOffset.z = 0;

		cpyRgn[0].dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		cpyRgn[0].dstSubresource.baseArrayLayer = 0;
		cpyRgn[0].dstSubresource.mipLevel = 0;
		cpyRgn[0].dstSubresource.layerCount = 1;

		cpyRgn[0].dstOffset.x = 0;
		cpyRgn[0].dstOffset.y = 0;
		cpyRgn[0].dstOffset.z = 0;

		cpyRgn[0].extent.width = m_width;
		cpyRgn[0].extent.height = m_height;
		cpyRgn[0].extent.depth = 1;

		vkCmdCopyImage(cmd, stagingTex.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_data.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &cpyRgn[0]);

		queue->flushCommandBuffer(cmdID);

	}

	samplerCreate(&m_data.sampler, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL,
		VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT);

	imageViewCreate(&m_data.view, m_data.image, VK_IMAGE_VIEW_TYPE_2D, m_format);

}

void VkeTexture::loadTextureFloatData(float *inData, uint32_t inWidth, uint32_t inHeight, uint32_t inCompCount){



	VulkanDC::Device::Queue::Name queueName = "DEFAULT_GRAPHICS_QUEUE";
	VulkanDC::Device::Queue::CommandBufferID cmdID = INIT_COMMAND_ID + 200;
	VulkanDC *dc = VulkanDC::Get();
	VulkanDC::Device *device = dc->getDefaultDevice();
	VulkanDC::Device::Queue *queue = device->getQueue(queueName);
	device->waitIdle();

	TextureObject stagingTex;

	m_width = inWidth;
	m_height = inHeight;

	imageCreateAndBind(
		&m_data.image,
		&m_data.memory,
		m_format, VK_IMAGE_TYPE_2D,
		m_width, m_height, 1, 1,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		(VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
		VK_IMAGE_TILING_OPTIMAL);

	imageCreateAndBind(
		&stagingTex.image,
		&stagingTex.memory,
		m_format, VK_IMAGE_TYPE_2D,
		m_width, m_height, 1, 1,
		m_memory_flags,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VK_IMAGE_TILING_LINEAR);

	if (m_memory_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT){


		VkImageSubresource subres;
		subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subres.mipLevel = m_mip_level;
		subres.arrayLayer = 0;

		VkSubresourceLayout layout;
		void *data;

		vkGetImageSubresourceLayout(getDefaultDevice(), stagingTex.image, &subres, &layout);
    VKA_CHECK_ERROR(vkMapMemory(getDefaultDevice(), stagingTex.memory, 0, VK_WHOLE_SIZE, 0, &data), "Could not map memory for image.\n");

		memcpy(data, inData, inWidth*inHeight*inCompCount*sizeof(float));

		vkUnmapMemory(getDefaultDevice(), stagingTex.memory);
		VkCommandBuffer cmd = VK_NULL_HANDLE;
		queue->beginCommandBuffer(cmdID, &cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		m_data.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


		VkImageCopy cpyRgn[1];

		cpyRgn[0].srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		cpyRgn[0].srcSubresource.baseArrayLayer = 0;
		cpyRgn[0].srcSubresource.mipLevel = 0;
		cpyRgn[0].srcSubresource.layerCount = 1;

		cpyRgn[0].srcOffset.x = 0;
		cpyRgn[0].srcOffset.y = 0;
		cpyRgn[0].srcOffset.z = 0;

		cpyRgn[0].dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		cpyRgn[0].dstSubresource.baseArrayLayer = 0;
		cpyRgn[0].dstSubresource.mipLevel = 0;
		cpyRgn[0].dstSubresource.layerCount = 1;

		cpyRgn[0].dstOffset.x = 0;
		cpyRgn[0].dstOffset.y = 0;
		cpyRgn[0].dstOffset.z = 0;

		cpyRgn[0].extent.width = m_width;
		cpyRgn[0].extent.height = m_height;
		cpyRgn[0].extent.depth = 1;

		vkCmdCopyImage(cmd, stagingTex.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_data.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &cpyRgn[0]);

		queue->flushCommandBuffer(cmdID);

	}

	samplerCreate(&m_data.sampler, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL,
		VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT);

	imageViewCreate(&m_data.view, m_data.image,VK_IMAGE_VIEW_TYPE_2D,m_format);

	LOGI("Texture Loaded.\n");
    LOGI("Floating Point Texture Loaded.\n");

}

#ifdef USE_LIB_PNG
void VkeTexture::loadTextureFile(const char *inPath){

	std::string filePath = std::string("images/") + std::string(inPath);

	std::cout << "Texture : " << std::string(inPath) << std::endl;


	if (!loadTexture((const char *)filePath.c_str(), NULL, NULL, &m_width, &m_height)){
		VKA_ERROR_MSG("Error loading texture image.\n");
		return;
	}

	VulkanDC::Device::Queue::Name queueName = "DEFAULT_GRAPHICS_QUEUE";
	VulkanDC::Device::Queue::CommandBufferID cmdID = INIT_COMMAND_ID+200;
	VulkanDC *dc = VulkanDC::Get();
	VulkanDC::Device *device = dc->getDefaultDevice();
	VulkanDC::Device::Queue *queue = device->getQueue(queueName);
	device->waitIdle();

	TextureObject stagingTex;

	imageCreateAndBind(
		&m_data.image,
		&m_data.memory,
		m_format,VK_IMAGE_TYPE_2D,
		m_width, m_height, 1, 1,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		(VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
		VK_IMAGE_TILING_OPTIMAL);

	imageCreateAndBind(
		&stagingTex.image,
		&stagingTex.memory,
		m_format, VK_IMAGE_TYPE_2D,
		m_width, m_height, 1, 1,
		m_memory_flags,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VK_IMAGE_TILING_LINEAR);

	if (m_memory_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT){


		VkImageSubresource subres;
		subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subres.mipLevel = m_mip_level;
        subres.arrayLayer = 0;

		VkSubresourceLayout layout;
		void *data;

		vkGetImageSubresourceLayout(getDefaultDevice(),stagingTex.image,&subres,&layout);
    VKA_CHECK_ERROR(vkMapMemory(getDefaultDevice(), stagingTex.memory, 0, VK_WHOLE_SIZE, 0, &data), "Could not map memory for image.\n");

		if (!loadTexture((const char *)filePath.c_str(), (uint8_t**)&data, layout.rowPitch, &m_width, &m_height)){
			VKA_ERROR_MSG("Could not load final image.\n");
		}

        vkUnmapMemory(getDefaultDevice(), stagingTex.memory);
		VkCommandBuffer cmd = VK_NULL_HANDLE;
		queue->beginCommandBuffer(cmdID,&cmd,  VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		m_data.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	

		VkImageCopy cpyRgn[1];

		cpyRgn[0].srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        cpyRgn[0].srcSubresource.baseArrayLayer = 0;
		cpyRgn[0].srcSubresource.mipLevel = 0;
		cpyRgn[0].srcSubresource.layerCount = 1;

        cpyRgn[0].srcOffset.x = 0;
        cpyRgn[0].srcOffset.y = 0;
        cpyRgn[0].srcOffset.z = 0;

		cpyRgn[0].dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        cpyRgn[0].dstSubresource.baseArrayLayer = 0;
		cpyRgn[0].dstSubresource.mipLevel = 0;
		cpyRgn[0].dstSubresource.layerCount = 1;

        cpyRgn[0].dstOffset.x = 0;
        cpyRgn[0].dstOffset.y = 0;
        cpyRgn[0].dstOffset.z = 0;

        cpyRgn[0].extent.width = m_width;
        cpyRgn[0].extent.height = m_height;
        cpyRgn[0].extent.depth = 1;

		vkCmdCopyImage(cmd, stagingTex.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_data.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &cpyRgn[0]);

		queue->flushCommandBuffer(cmdID);

	}

	samplerCreate(&m_data.sampler, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL,
		VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT);

	imageViewCreate(&m_data.view, m_data.image);

	VKA_INFO_MSG("Texture Loaded.\n");

}
#endif

VkeTexture::List::List(){}
VkeTexture::List::~List(){}

VkeTexture::ID VkeTexture::List::nextID(){
	VkeTexture::ID outID;
	if (m_deleted_keys.size() == 0) return m_data.size();
	outID = m_deleted_keys.back();
	m_deleted_keys.pop_back();
	return outID;
}

VkeTexture::Count VkeTexture::List::count(){
	return m_data.size();
}

VkeTexture *VkeTexture::List::newTexture(){
	VkeTexture::ID id = nextID();
	return newTexture(id);
}

VkeTexture *VkeTexture::List::newTexture(const VkeTexture::ID &inID){
	VkeTexture *outMaterial = new VkeTexture(inID);
	m_data[inID] = outMaterial;
	return outMaterial;
}


void VkeTexture::List::addTexture(VkeTexture * const inMaterial){
	VkeTexture::ID id = nextID();
	m_data[id] = inMaterial;
}

VkeTexture *VkeTexture::List::getTexture(const VkeTexture::ID &inID){
	return m_data[inID];
}

void VkeTexture::List::getData(VkeTexture::Data *outData,uint32_t offset){
	VkeTexture::Map::iterator itr;
	uint32_t i = offset;
	for (itr = m_data.begin(); itr != m_data.end(); ++itr){
		outData[i++] = itr->second->getData();
	}
}
