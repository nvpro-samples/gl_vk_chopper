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

#include "VkeRenderer.h"


VkeRenderer::VkeRenderer()
{
	m_samples = VK_SAMPLE_COUNT_8_BIT;
	m_width = 1024;
	m_height = 768;
}


VkeRenderer::~VkeRenderer()
{
}

void VkeRenderer::initLayouts(){
	initDescriptorLayout();
	initRenderPass();
	initPipeline();
}

void VkeRenderer::initDescriptors(){
	initDescriptorPool();
	initDescriptorSets();

}


void VkeRenderer::initDescriptorPool(){
	m_descriptor_pool_size = getRequiredDescriptorCount();
}

void VkeRenderer::releaseDescriptorPool(){


}

VkDescriptorPool &VkeRenderer::getDescriptorPool(){
	return m_descriptor_pool;
}

VkShaderModule VkeRenderer::createVKShader(nv_helpers_gl::ProgramManager &inProgramManager, nv_helpers_gl::ProgramManager::ProgramID &inID, GLenum inStage){
	VulkanDC *dc = VulkanDC::Get();
	VulkanDC::Device *device = dc->getDefaultDevice();
	const nv_helpers_gl::ProgramManager::Program &pgm = inProgramManager.getProgram(inID);
	uint32_t sz = pgm.definitions.size();
	for (uint32_t i = 0; i < sz; ++i){
		if (pgm.definitions[i].type != inStage) continue;

		VkResult rslt;
		VkShaderModuleCreateInfo shaderModuleInfo;
		memset(&shaderModuleInfo, 0, sizeof(shaderModuleInfo));
		shaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

		shaderModuleInfo.codeSize = pgm.definitions[i].preprocessed.size();
		shaderModuleInfo.pCode = (const uint32_t*)pgm.definitions[i].preprocessed.c_str();
		
		VkShaderModule shaderModule;

		rslt = vkCreateShaderModule(device->getVKDevice(), &shaderModuleInfo, NULL, &shaderModule);

		if (rslt != VK_SUCCESS){
			return VK_NULL_HANDLE;
		}
		return shaderModule;

	}
	return VK_NULL_HANDLE;
}

void VkeRenderer::resize(uint32_t inWidth, uint32_t inHeight){
	initFramebuffer(inWidth, inHeight);
	initDescriptors();

	m_is_first_frame = true;

}