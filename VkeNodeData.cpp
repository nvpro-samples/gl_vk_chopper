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

#include "VkeNodeData.h"
#include<iostream>
#include"VulkanAppContext.h"

VkeNodeData::VkeNodeData():
VkeBuffer(),
m_layer(0),
m_needs_buffer_update(true)
{
	//initNodeData();
	initNodeDataSubAlloc();
}

VkeNodeData::VkeNodeData(const ID &inID) :
VkeBuffer(),
m_layer(0),
m_needs_buffer_update(true){
	
	m_index = inID;
	initNodeDataSubAlloc();
	//initNodeData();
}

bool sortByMatFunc(VkeNodeData *lhs, VkeNodeData *rhs){
	return (lhs->getMesh()->getMaterialID() < rhs->getMesh()->getMaterialID());
}

bool sortByMeshFunc(VkeNodeData *lhs, VkeNodeData *rhs){
	return (lhs->getMesh()->getID() < rhs->getMesh()->getID());
}

bool sortByOpacityFunc(VkeNodeData *lhs, VkeNodeData *rhs){
	VulkanAppContext *ctxt = VulkanAppContext::GetInstance();
	return (ctxt->getOpacity(lhs->getMesh()->getMaterialID()) > ctxt->getOpacity(rhs->getMesh()->getMaterialID()));
}


void VkeNodeData::List::sortByMaterialID(){
	std::sort(m_data.begin(), m_data.end(), sortByMatFunc);
	uint32_t sz = m_data.size();
	for (uint32_t i = 0; i < sz; ++i){
		m_data[i]->setIndex(i);
	}
}

void VkeNodeData::List::sortByOpacity(){
	std::sort(m_data.begin(), m_data.end(), sortByOpacityFunc);
	uint32_t sz = m_data.size();
	for (uint32_t i = 0; i < sz; ++i){
		m_data[i]->setIndex(i);
	}
}

void VkeNodeData::List::sortByMeshID(){
	std::sort(m_data.begin(), m_data.end(), sortByMeshFunc);
}

void VkeNodeData::initNodeData(){


	m_usage_flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	m_memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	m_use_staging = true;


	initBackingStore(sizeof(VkeNodeUniform));
	initVKBufferData();
}

void VkeNodeData::initNodeDataSubAlloc(){
	initBackingStore(sizeof(VkeNodeUniform));
}

void VkeNodeData::bind(VkCommandBuffer *inCmd){}

void VkeNodeData::updateVKBufferData(VkeNodeUniform *inData){
	uint8_t *ptr = (uint8_t*)inData + (sizeof(VkeNodeUniform)*m_index);
	memcpy(ptr, (void*)&m_backing_store->view_matrix,sizeof(VkeNodeUniform));
}

void VkeNodeData::updateFromNode(Node * const inNode, VkCommandBuffer *inBuffer){

	m_node = inNode;

	inNode->update();
	Transform transform = inNode->GetTransform();

	m_backing_store->view_matrix = transform.getTransform();
	m_backing_store->normal_matrix = transform.getInverse();



}

void VkeNodeData::updateFromNode(Node * const inNode, VkeNodeUniform *inData, uint32_t inInstanceCount){

	m_node = inNode;

	inNode->update();
	Transform transform = inNode->GetTransform();

	m_backing_store->view_matrix = transform.getTransform();
	m_backing_store->normal_matrix = transform.getInverse();
	m_backing_store->lookup.x = m_mesh->getMaterialID();
	m_backing_store->lookup.y = inInstanceCount;

	updateVKBufferData(inData);
}

void VkeNodeData::updateConstantVKBufferData(VkCommandBuffer *inBuffer){

	vkCmdUpdateBuffer(*inBuffer, m_data.buffer, 0, m_data_size, (const uint32_t *)&m_backing_store[0]);

}

void VkeNodeData::updateFromNode(VkeNodeUniform *inData, uint32_t inInstanceCount){
	updateFromNode(m_node, inData, inInstanceCount);
}

void VkeNodeData::updateFromNode(){
	updateFromNode(m_node);
}

VkeNodeData::~VkeNodeData()
{
}

VkeNodeData::List::List(){}
VkeNodeData::List::~List(){}

VkeNodeData::ID VkeNodeData::List::nextID(){
	VkeNodeData::ID outID;
	if (m_deleted_keys.size() == 0) return m_data.size();
	outID = m_deleted_keys.back();
	m_deleted_keys.pop_back();
	return outID;
}

VkeNodeData::Count VkeNodeData::List::count(){
	return m_data.size();
}

VkeNodeData *VkeNodeData::List::newData(){
	VkeNodeData::ID id = nextID();
	return newData(id);
}

VkeNodeData *VkeNodeData::List::newData(const VkeNodeData::ID &inID){
	VkeNodeData *outData = new VkeNodeData(inID);
	m_data.push_back(outData);
	return outData;
}


void VkeNodeData::List::addData(VkeNodeData * const inData){
	VkeNodeData::ID id = nextID();
	m_data[id] = inData;
}

VkeNodeData *VkeNodeData::List::getData(const VkeNodeData::ID &inID){
	return m_data[inID];
}

void VkeNodeData::List::update(){
	VkeNodeData::Map::iterator itr;

	uint32_t sz = m_data.size();
	for (uint32_t i = 0; i < sz; ++i){
		m_data[i]->updateFromNode();
	}
}

void VkeNodeData::List::update(VkeNodeUniform *inData, uint32_t inInstanceCount){
	VkeNodeData::Map::iterator itr;

		uint32_t sz = m_data.size();
		for (uint32_t i = 0; i < sz; ++i){
			m_data[i]->updateFromNode(inData, inInstanceCount);
		}
}

void VkeNodeData::List::getDescriptors(VkDescriptorBufferInfo *outDescriptors){
	VkeNodeData::Map::iterator itr;
	uint32_t sz = m_data.size();
	for (uint32_t i = 0; i < sz; ++i){
		outDescriptors[i] = m_data[i]->getDescriptor();
	}
}

void VkeNodeData::List::getMeshes(VkeMesh **outMeshes){
	VkeNodeData::Map::iterator itr;
	uint32_t sz = m_data.size();

	for (uint32_t i = 0; i < sz;++i){
		outMeshes[i] = m_data[i]->getMesh();
	}
}

