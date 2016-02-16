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

#ifndef __H_VKE_BUFFER_
#define __H_VKE_BUFFER_

#pragma once


#include<vulkan/vulkan.h>
#include"VkeCreateUtils.h"
#include"vkaUtils.h"


#ifndef INIT_COMMAND_ID
#define INIT_COMMAND_ID 1
#endif

template<typename T>
class VkeBuffer
{
protected:

	uint32_t m_index;


public:
	VkeBuffer() :
		m_backing_store(NULL),
		m_use_staging(false){}

	~VkeBuffer(){}

	virtual void bind(VkCommandBuffer *inBuffer) = 0;

	struct Data{
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkBufferView view;	
		VkDescriptorBufferInfo descriptor;
	};

	void deleteBackingStore(){
		if (m_backing_store == NULL) return;
		free(m_backing_store);
	}

	void initBackingStore(size_t inSize){
		m_data_size = inSize;
		m_backing_store = (T*)malloc(m_data_size);
	}

	T *&getBackingStore(){
		return m_backing_store;
	}

	Data &getData(){
		return m_data;
	}

	VkDescriptorBufferInfo &getDescriptor(){ return m_data.descriptor;  }

	virtual void updateVKBufferData(VkCommandBuffer *inBuffer = NULL,bool doStage = true){
		if (m_backing_store == NULL) return;
		uint8_t *vData = NULL;

		Data &useData = (m_use_staging) ? m_staging : m_data;

		VKA_CHECK_ERROR(vkMapMemory(getDefaultDevice(), useData.memory, 0, 0, 0, (void **)&vData), "Could not map buffer memory.\n");

		memcpy(vData, (const void *)&(m_backing_store[0]), m_data_size);

		vkUnmapMemory(getDefaultDevice(), useData.memory);

		if (m_use_staging){

			
			if(doStage) stageCopy(inBuffer);

		}

	}

	virtual void stageCopy(VkCommandBuffer *inBuffer = NULL){
		VulkanDC::Device::Queue::Name queueName = "DEFAULT_GRAPHICS_QUEUE";
		VulkanDC::Device::Queue::CommandBufferID cmdID = INIT_COMMAND_ID + 300;
		VulkanDC *dc = VulkanDC::Get();
		VulkanDC::Device *device = dc->getDefaultDevice();
		VulkanDC::Device::Queue *queue = device->getQueue(queueName);
		VkCommandBuffer cmd = VK_NULL_HANDLE;

		VkBufferCopy bufCpy;
		bufCpy.srcOffset = 0;
		bufCpy.dstOffset = 0;
		bufCpy.size = m_data_size;

		if (inBuffer){
			cmd = *inBuffer;
		}
		else{

			queue->beginCommandBuffer(cmdID, &cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		}


		vkCmdCopyBuffer(cmd, m_staging.buffer, m_data.buffer, 1, &bufCpy);


	}

	virtual void initVKBufferData(VkBuffer &inBuffer, size_t inCount = 1){

		VkDescriptorBufferInfo bufInfo;
		bufInfo.buffer = inBuffer;
		bufInfo.range = m_data_size*inCount;
		bufInfo.offset = 0;// m_index*m_data_size;
		
		m_data.descriptor = bufInfo;
	}

	virtual void initVKBufferData(){
		if (m_backing_store == NULL) return;

		if (m_use_staging){
			m_memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			VkMemoryPropertyFlagBits stageMemFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			VkBufferUsageFlagBits stageBufFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

			bufferCreate(&m_staging.buffer, m_data_size, stageBufFlags);
			bufferAlloc(&m_staging.buffer, &m_staging.memory, stageMemFlags);

		}


		bufferCreate(&m_data.buffer, m_data_size, m_usage_flags);
		bufferAlloc(&m_data.buffer, &m_data.memory, m_memory_flags);



		updateVKBufferData();


		bufferViewCreate(&m_data.buffer, &m_data.view, m_data_size);

		VkDescriptorBufferInfo bufInfo;
		bufInfo.buffer = m_data.buffer;
		bufInfo.range = m_data_size;
		bufInfo.offset = 0;


		m_data.descriptor = bufInfo;
	

	}

	

protected:

	size_t m_data_size;
	T *m_backing_store;
	Data m_data;
	Data m_staging;

	bool m_use_staging;

	VkBufferUsageFlagBits m_usage_flags;
	VkMemoryPropertyFlagBits m_memory_flags;

};

#endif
