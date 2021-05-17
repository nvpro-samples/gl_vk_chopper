/*
 * Copyright (c) 2014-2021, NVIDIA CORPORATION.  All rights reserved.
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
 * SPDX-FileCopyrightText: Copyright (c) 2014-2021 NVIDIA CORPORATION
 * SPDX-License-Identifier: Apache-2.0
 */

/* Contact chebert@nvidia.com (Chris Hebert) for feedback */

#include "VkeNodeData.h"
#include "VulkanAppContext.h"
#include <iostream>

VkeNodeData::VkeNodeData()
    : VkeBuffer()
    , m_layer(0)
    , m_needs_buffer_update(true)
{
  //initNodeData();
  initNodeDataSubAlloc();
}

VkeNodeData::VkeNodeData(const ID& inID)
    : VkeBuffer()
    , m_layer(0)
    , m_needs_buffer_update(true)
{

  m_index = inID;
  initNodeDataSubAlloc();
  //initNodeData();
}

bool sortByMatFunc(VkeNodeData* lhs, VkeNodeData* rhs)
{
  return (lhs->getMesh()->getMaterialID() < rhs->getMesh()->getMaterialID());
}

bool sortByMeshFunc(VkeNodeData* lhs, VkeNodeData* rhs)
{
  return (lhs->getMesh()->getID() < rhs->getMesh()->getID());
}

bool sortByOpacityFunc(VkeNodeData* lhs, VkeNodeData* rhs)
{
  VulkanAppContext* ctxt = VulkanAppContext::GetInstance();
  return (ctxt->getOpacity(lhs->getMesh()->getMaterialID()) > ctxt->getOpacity(rhs->getMesh()->getMaterialID()));
}


void VkeNodeData::List::sortByMaterialID()
{
  std::sort(m_data.begin(), m_data.end(), sortByMatFunc);
  size_t sz = m_data.size();
  for(size_t i = 0; i < sz; ++i)
  {
    m_data[i]->setIndex(i);
  }
}

void VkeNodeData::List::sortByOpacity()
{
  std::sort(m_data.begin(), m_data.end(), sortByOpacityFunc);
  size_t sz = m_data.size();
  for(size_t i = 0; i < sz; ++i)
  {
    m_data[i]->setIndex(i);
  }
}

void VkeNodeData::List::sortByMeshID()
{
  std::sort(m_data.begin(), m_data.end(), sortByMeshFunc);
}

void VkeNodeData::initNodeData()
{


  m_usage_flags  = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  m_memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
  m_use_staging  = true;


  initBackingStore(sizeof(VkeNodeUniform));
  initVKBufferData();
}

void VkeNodeData::initNodeDataSubAlloc()
{
  initBackingStore(sizeof(VkeNodeUniform));
}

void VkeNodeData::bind(VkCommandBuffer* inCmd) {}

void VkeNodeData::updateVKBufferData(VkeNodeUniform* inData)
{
  uint8_t* ptr = (uint8_t*)inData + (sizeof(VkeNodeUniform) * m_index);
  memcpy(ptr, (void*)&m_backing_store->view_matrix, sizeof(VkeNodeUniform));
}

void VkeNodeData::updateFromNode(Node* const inNode, VkCommandBuffer* inBuffer)
{

  m_node = inNode;

  inNode->update();
  Transform transform = inNode->GetTransform();

  m_backing_store->view_matrix   = transform.getTransform();
  m_backing_store->normal_matrix = transform.getInverse();
}

void VkeNodeData::updateFromNode(Node* const inNode, VkeNodeUniform* inData, uint32_t inInstanceCount)
{

  m_node = inNode;

  inNode->update();
  Transform transform = inNode->GetTransform();

  m_backing_store->view_matrix   = transform.getTransform();
  m_backing_store->normal_matrix = transform.getInverse();
  m_backing_store->lookup.x      = m_mesh->getMaterialID();
  m_backing_store->lookup.y      = inInstanceCount;

  updateVKBufferData(inData);
}

void VkeNodeData::updateConstantVKBufferData(VkCommandBuffer* inBuffer)
{

  vkCmdUpdateBuffer(*inBuffer, m_data.buffer, 0, m_data_size, (const uint32_t*)&m_backing_store[0]);
}

void VkeNodeData::updateFromNode(VkeNodeUniform* inData, uint32_t inInstanceCount)
{
  updateFromNode(m_node, inData, inInstanceCount);
}

void VkeNodeData::updateFromNode()
{
  updateFromNode(m_node);
}

VkeNodeData::~VkeNodeData() {}

VkeNodeData::List::List() {}
VkeNodeData::List::~List() {}

VkeNodeData::ID VkeNodeData::List::nextID()
{
  VkeNodeData::ID outID;
  if(m_deleted_keys.size() == 0)
    return m_data.size();
  outID = m_deleted_keys.back();
  m_deleted_keys.pop_back();
  return outID;
}

VkeNodeData::Count VkeNodeData::List::count()
{
  return m_data.size();
}

VkeNodeData* VkeNodeData::List::newData()
{
  VkeNodeData::ID id = nextID();
  return newData(id);
}

VkeNodeData* VkeNodeData::List::newData(const VkeNodeData::ID& inID)
{
  VkeNodeData* outData = new VkeNodeData(inID);
  m_data.push_back(outData);
  return outData;
}


void VkeNodeData::List::addData(VkeNodeData* const inData)
{
  VkeNodeData::ID id = nextID();
  m_data[id]         = inData;
}

VkeNodeData* VkeNodeData::List::getData(const VkeNodeData::ID& inID)
{
  return m_data[inID];
}

void VkeNodeData::List::update()
{
  VkeNodeData::Map::iterator itr;

  size_t sz = m_data.size();
  for(size_t i = 0; i < sz; ++i)
  {
    m_data[i]->updateFromNode();
  }
}

void VkeNodeData::List::update(VkeNodeUniform* inData, uint32_t inInstanceCount)
{
  VkeNodeData::Map::iterator itr;

  size_t sz = m_data.size();
  for(size_t i = 0; i < sz; ++i)
  {
    m_data[i]->updateFromNode(inData, inInstanceCount);
  }
}

void VkeNodeData::List::getDescriptors(VkDescriptorBufferInfo* outDescriptors)
{
  VkeNodeData::Map::iterator itr;
  size_t                     sz = m_data.size();
  for(size_t i = 0; i < sz; ++i)
  {
    outDescriptors[i] = m_data[i]->getDescriptor();
  }
}

void VkeNodeData::List::getMeshes(VkeMesh** outMeshes)
{
  VkeNodeData::Map::iterator itr;
  size_t                     sz = m_data.size();

  for(size_t i = 0; i < sz; ++i)
  {
    outMeshes[i] = m_data[i]->getMesh();
  }
}
