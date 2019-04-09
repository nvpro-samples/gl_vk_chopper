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

#ifndef __H_VKE_NODE_DATA_
#define __H_VKE_NODE_DATA_

#pragma once

#include"WMath.h"
#include"VkeBuffer.h"
#include"Node.h"
#include"VkeMesh.h"
#include<map>
#include<algorithm>


typedef struct _VkeNodeUniform{
	nvmath::mat4f view_matrix;
	nvmath::mat4f normal_matrix;
	nvmath::vec4i lookup;
	nvmath::vec4f p1[3];
} VkeNodeUniform;


class VkeNodeData : public VkeBuffer<VkeNodeUniform>
{
public:
	typedef uint32_t ID;
	typedef std::vector<VkeNodeData*> Map;
	typedef uint32_t Count;

	class List{
	public:
		List();
		~List();

		VkeNodeData *newData();
		VkeNodeData *newData(const VkeNodeData::ID &inID);
		void addData(VkeNodeData * const inData);
		VkeNodeData *getData(const ID &inID);
		void update();
		void update(VkeNodeUniform *inData, uint32_t inInstanceCount = 1);

		ID nextID();
		Count count();

		void getDescriptors(VkDescriptorBufferInfo *outDescriptor);
		void getMeshes(VkeMesh **outMeshes);
		void sortByMaterialID();
		void sortByOpacity();
		void sortByMeshID();


	private:
		VkeNodeData::Map m_data;
		std::vector<VkeNodeData::ID> m_deleted_keys;
	};


	VkeNodeData();
	VkeNodeData(const ID &inID);
	~VkeNodeData();


	void setMesh(VkeMesh *inMesh)  { m_mesh = inMesh; }
	VkeMesh *getMesh(){ return m_mesh; }

	Node* getNode(){ return m_node; }

	inline void setIndex(uint32_t inIndex){
		m_index = inIndex;
	}

	void initNodeData();
	void initNodeDataSubAlloc();
	void updateFromNode();
	void updateFromNode(Node * const inNode, VkCommandBuffer *inBuffer = NULL);
	void updateFromNode(VkCommandBuffer *inBuffer);
	void updateFromNode(Node *const inNode, VkeNodeUniform *inData, uint32_t inInstanceCount = 1);
	void updateFromNode(VkeNodeUniform *inData, uint32_t inInstanceCount = 1);
	void updateConstantVKBufferData(VkCommandBuffer *inBuffer = NULL);

	void updateVKBufferData(VkeNodeUniform *inData);

	void bind(VkCommandBuffer *inBuffer);

	void setLayer(uint32_t inLayer){
		m_layer = inLayer;
	}

	uint32_t getLayer(){
		return m_layer;
	}

	Node *m_node;
	VkeMesh *m_mesh;

	uint32_t m_layer;

	bool m_needs_buffer_update;


};



#endif