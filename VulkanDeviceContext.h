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

#ifndef __H_VULKAN_DEVICE_CONTEXT_
#define __H_VULKAN_DEVICE_CONTEXT_

#pragma once


#include<vulkan/vulkan.h>

#include<map>
#include<vector>
#include<string>

class VulkanDC
{
public:
    ~VulkanDC();

	static VulkanDC *Get();


	class Device{
	public:

		typedef std::string Name;


		Device();
		Device(const VkPhysicalDevice &inDevice);
		~Device();

		uint32_t getBestMemoryType(uint32_t inType, VkFlags inFlags);


		class Queue{
		public:
			typedef std::string Name;
			typedef uint32_t NodeID;
			typedef std::map<Name, Queue*> Map;
			typedef uint32_t CommandBufferID;
			typedef std::map<CommandBufferID, VkCommandBuffer> CommandBufferMap;

			Queue();
			Queue(Name &inName,NodeID inNodeID);
			Queue(const Queue& inOther);
			~Queue();

			void initQueue(VulkanDC::Device *inDevice);

			inline VulkanDC::Device *getDevice() const { return m_device; }
			inline VkCommandPool &getCommandPool() { return m_command_pool; }

			void createCommandBuffer(
				VkCommandBuffer *outBuffer,
				VkCommandBufferLevel inLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

			bool createAndCacheCommandBuffer(
				const CommandBufferID &inID,
				VkCommandBuffer *outBuffer,
				VkCommandBufferLevel inlevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY
				);

			void beginCommandBuffer(
				VkCommandBuffer *inBuffer,
				VkCommandBufferUsageFlags inFlags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
				VkCommandBufferLevel inLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				VkFramebuffer inFBO  = VK_NULL_HANDLE,
				VkRenderPass inRP = VK_NULL_HANDLE

				);

			void beginCommandBuffer(
				CommandBufferID &inID,
				VkCommandBuffer *outBuffer,
				VkCommandBufferUsageFlags inFlags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
				VkCommandBufferLevel inLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				VkFramebuffer inFBO = VK_NULL_HANDLE,
				VkRenderPass inRP = VK_NULL_HANDLE
				);
			
			void resetCommandBuffer(CommandBufferID &inID);
			void flushCommandBuffer(CommandBufferID &inID, VkFence *inFence = NULL, bool inDestroy = true);
			void flushPersistentBuffer(CommandBufferID &inID, VkFence *inFence = NULL);

			VkCommandBuffer getCachedCommandBuffer(CommandBufferID &inID);
			void destroyCachedCommandBuffer(CommandBufferID &inID);

			inline VkQueue &getVKQueue(){ return m_queue; }


		private:

			Name m_name;
			NodeID m_node_id;
			VkQueue m_queue;
			VulkanDC::Device *m_device;

			VkCommandPool m_command_pool;
			CommandBufferMap m_cached_buffers;

		};

		typedef std::vector<Device*> List;

		inline VkDevice &getVKDevice(){ return m_device; }

		inline uint32_t getQueueCount() const { return m_queue_count; }

		VulkanDC::Device::Queue *getQueue(VulkanDC::Device::Queue::Name &inName);
		
        VulkanDC::Device::Queue *createGraphicsQueue(VulkanDC::Device::Queue::Name &inName, VulkanDC::Device::Queue::NodeID inID = 0);

		Queue *newQueue(Queue::Name &inName, Queue::NodeID &inNodeID);


		void waitIdle();

	private:
		VkPhysicalDeviceProperties m_device_properties;
        VkQueueFamilyProperties *m_queue_properties;
		VkPhysicalDeviceMemoryProperties m_memory_properties;

		uint32_t m_queue_count;
		Queue::Map m_queues;
		VkPhysicalDevice m_physical_device;
		VkDevice m_device;
		Name m_name;


		char *m_extension_names[64];
		uint32_t m_extension_count;

		void initDevice();


	};

	


	void initDC(const VkInstance &inInstance);

	uint32_t getQueueCount(uint32_t inDeviceID = 0);
	inline uint32_t getDeviceCount() const { return m_device_count; }

	VulkanDC::Device *getDevice(uint32_t inDeviceID = 0);

	VulkanDC::Device *getDefaultDevice() { return m_default_device; }
	VulkanDC::Device::Queue *getDefaultQueue() { return m_default_queue; }
	void setDefaultDevice(VulkanDC::Device *inDevice) { m_default_device = inDevice; }

	void setDefaultQueue(VulkanDC::Device::Queue *inQueue) { m_default_queue = inQueue; }

    VulkanDC::Device::Queue *getQueueForGraphics(VulkanDC::Device::Queue::Name &queueName,VkFormat &outFormat);



	

private:
	VulkanDC();


	bool m_has_instance;
	VkInstance m_vk_instance;
	VkPhysicalDevice *m_physical_devices;
	uint32_t m_device_count;

	Device::List m_devices;

	Device *m_default_device;
	Device::Queue *m_default_queue;

	void initDevices();

	Device *newDevice(const VkPhysicalDevice &inDevice);


};

#endif
