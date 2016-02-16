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

#include "VkeTerrainQuad.h"


VkeTerrainQuad::VkeTerrainQuad():
VkeBuffer()
{
}


VkeTerrainQuad::~VkeTerrainQuad()
{
}

void VkeTerrainQuad::initQuadData(){
	m_usage_flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	m_memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	initBackingStore(sizeof(TerrainUniform));

	float ww = 1800.0;
	float hh = 1000.0;

	m_backing_store->view_matrix.identity();

	initVKBufferData();




	float quadVerts[] = {
	-1.0,0.0,-1.0,1.0,	0.0,0.0,
	1.0, 0.0, -1.0, 1.0, 1.0, 0.0,
	1.0, 0.0, 1.0, 1.0, 1.0, 1.0,
	-1.0, 0.0, 1.0, 1.0, 0.0, 1.0
	};

	uint32_t quadIdxs[] = {
		0,1,3,2
	};

	size_t dataSize = 4 * sizeof(VertexObjectUV);

	m_vbo.initBackingStore(dataSize);

	float *vData = m_vbo.getBackingStore();
	memcpy(vData, (const void*)quadVerts, dataSize);


	dataSize = sizeof(uint32_t) * 4;
	m_ibo.initBackingStore(dataSize);
	uint32_t *iData = m_ibo.getBackingStore();
	memcpy(iData, (const void *)quadIdxs, dataSize);

	m_vbo.initVKBufferData();
	m_ibo.initVKBufferData();


}

void VkeTerrainQuad::draw(VkCommandBuffer *inCommand){
	vkCmdDrawIndexed(*inCommand, 4, 256, 0,0,0);
}

void VkeTerrainQuad::bind(VkCommandBuffer *inCmd){
	m_vbo.bind(inCmd);
	m_ibo.bind(inCmd);
}
