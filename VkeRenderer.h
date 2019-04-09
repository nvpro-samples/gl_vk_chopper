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

#ifndef __H_VKE_RENDERER_
#define __H_VKE_RENDERER_


#include<vulkan/vulkan.h>
#include"VkeMesh.h"
#include"VkeNodeData.h"
#include<map>
#include<nvvk/shadermodulemanager_vk.hpp>

#pragma once
class VkeRenderer
{
public:
	VkeRenderer();
	~VkeRenderer();


	VkDescriptorPool &getDescriptorPool();

	virtual void initDescriptors();
	virtual void initLayouts();
	virtual void initDescriptorPool();
	void releaseDescriptorPool();

	virtual void update() = 0;

	virtual void resize(uint32_t inWidth, uint32_t inHeight);

	inline VkSampleCountFlagBits getSamples(){ return m_samples; }


	VkFramebuffer getFramebuffer(uint32_t inIdx){ return m_framebuffers[inIdx]; }
	VkRenderPass getRenderPass(){ return m_render_pass; }


	virtual void initDescriptorLayout() = 0;
	virtual void initDescriptorSets() = 0;
	virtual void initPipeline() = 0;
	virtual void initRenderPass() = 0;

	virtual void initFramebuffer(uint32_t inWidth, uint32_t inHeight) = 0;
	virtual void releaseFramebuffer() = 0;

	virtual void present() = 0;
  virtual void initShaders(nvvk::ShaderModuleManager& inShaderModuleManager) = 0;
	virtual void setCameraLookAt(nvmath::mat4f &inMat) = 0;

	VkPipeline getPipeline() {
		return m_pipeline;
	}

	VkPipelineLayout getPipelineLayout(){
		return m_pipeline_layout;
	}


protected:
	bool							m_is_first_frame;

	VkDescriptorPool			m_descriptor_pool;
	uint32_t					m_descriptor_pool_size;

	VkSampleCountFlagBits		m_samples;

	VkPipelineLayout			m_pipeline_layout;
	VkPipeline					m_pipeline;
	VkPipelineCache				m_pipeline_cache;
	VkRenderPass				m_render_pass;

	uint32_t					m_descriptor_set_count;
	VkFramebuffer				m_framebuffers[2];

	virtual uint32_t getRequiredDescriptorCount() = 0;

	uint32_t					m_width;
	uint32_t					m_height;
  


};


#endif
