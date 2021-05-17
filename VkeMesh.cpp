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

#include "VkeMesh.h"
#include "VKSFile.h"

VkeMesh::VkeMesh()
    : m_id(0)
    , m_material_id(-1)
{
}

VkeMesh::VkeMesh(const ID& inID)
    : m_id(inID)
    , m_material_id(-1)
{
}

VkeMesh::~VkeMesh() {}

void VkeMesh::initVKBuffers()
{

  m_vbo.initVKBufferData();
  m_ibo.initVKBufferData();
}

void VkeMesh::initFromMesh(VKSFile* inFile, VKSMeshRecord* inMesh)
{
  m_vertex_count = inMesh->vertexCount;
  m_index_count  = inMesh->indexCount;
  m_material_id  = inMesh->materialID;
}

void VkeMesh::initFromMesh(Mesh* const inMesh)
{

  if(!inMesh)
    return;

  m_vertex_count = inMesh->getVertexCount();
  m_index_count  = inMesh->getIndexCount();
  m_material_id  = inMesh->getMaterialID();

  Vec4f*    nmls = inMesh->getNormals();
  Vec4f*    vtxs = inMesh->getVertices();
  uint32_t* idxs = inMesh->getIndices();
  Vec2f*    uvs  = inMesh->getUVs();

  size_t dataSize = m_vertex_count * sizeof(VertexObject);

  m_vbo.initBackingStore(dataSize);

  float* vData = m_vbo.getBackingStore();

  uint32_t cmpCount = 0;
  for(uint32_t i = 0; i < m_vertex_count; ++i)
  {
    vData[cmpCount++] = vtxs[i].m_data[0];
    vData[cmpCount++] = vtxs[i].m_data[1];
    vData[cmpCount++] = vtxs[i].m_data[2];
    vData[cmpCount++] = vtxs[i].m_data[3];
    vData[cmpCount++] = nmls[i].m_data[0];
    vData[cmpCount++] = nmls[i].m_data[1];
    vData[cmpCount++] = nmls[i].m_data[2];
    vData[cmpCount++] = nmls[i].m_data[3];
    vData[cmpCount++] = uvs[i].m_data[0];
    vData[cmpCount++] = uvs[i].m_data[1];
  }

  dataSize = m_index_count * sizeof(uint32_t);

  m_ibo.initBackingStore(dataSize);

  uint32_t* iData = m_ibo.getBackingStore();
  for(uint32_t i = 0; i < m_index_count; ++i)
  {
    iData[i] = idxs[i];
  }
}

void VkeMesh::bind(VkCommandBuffer* inCmd)
{
  m_vbo.bind(inCmd);
  m_ibo.bind(inCmd);
}

void VkeMesh::initBindCommand(VkCommandPool* inPool, VkQueue* inQueue)
{
  VkCommandBufferAllocateInfo cmdBufInfo;
  VkCommandBuffer             cmdBuf;
  VkFence                     nullFence = VK_NULL_HANDLE;


  memset(&cmdBufInfo, 0, sizeof(cmdBufInfo));
  cmdBufInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmdBufInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  cmdBufInfo.commandPool        = *inPool;
  cmdBufInfo.commandBufferCount = 1;

  VKA_CHECK_ERROR(vkAllocateCommandBuffers(getDefaultDevice(), &cmdBufInfo, &cmdBuf),
                  "Could not create command buffer for init.\n");


  VkCommandBufferBeginInfo cmdBegin;
  memset(&cmdBegin, 0, sizeof(cmdBegin));
  cmdBegin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmdBegin.flags = 0;

  VkDeviceSize ofst = 0;
  VKA_CHECK_ERROR(vkBeginCommandBuffer(cmdBuf, &cmdBegin), "Could not begin command buffer.\n");
  m_vbo.bind(&cmdBuf);
  m_ibo.bind(&cmdBuf);
  VKA_CHECK_ERROR(vkEndCommandBuffer(cmdBuf), "Could not end command buffer for draw command.\n");

  VkSubmitInfo subInfo;
  memset(&subInfo, 0, sizeof(subInfo));
  subInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  subInfo.commandBufferCount = 1;
  subInfo.pCommandBuffers    = &cmdBuf;

  VKA_CHECK_ERROR(vkQueueSubmit(*inQueue, 1, &subInfo, nullFence), "Could not submit bind command buffer.\n");

  VKA_CHECK_ERROR(vkQueueWaitIdle(*inQueue), "Could not wait for idle graphics queue.\n");
}

void VkeMesh::initDrawCommand(VkCommandBuffer* inCommand)
{

  uint32_t firstVert = 0;
  uint32_t firstIdx  = 0;

#if USE_SINGLE_VBO
  firstVert = m_first_vertex;
  firstIdx  = m_first_index;
#endif

  VkDrawIndexedIndirectCommand testCmd;
  testCmd.firstIndex    = firstIdx;
  testCmd.firstInstance = 0;
  testCmd.indexCount    = m_index_count;
  testCmd.instanceCount = 1;
  testCmd.vertexOffset  = firstVert;

  vkCmdDrawIndexed(*inCommand, m_index_count, 1, firstVert, firstIdx, 0);
  //vkCmdDrawIndexedIndirect(*inCommand,)
}

void VkeMesh::initDrawCommand(VkCommandBuffer* inCommand, VkBuffer& inIndirectBuffer, uint32_t inIndex)
{
  VkDeviceSize sz   = sizeof(VkDrawIndexedIndirectCommand);
  VkDeviceSize ofst = sz * inIndex;

  vkCmdDrawIndexedIndirect(*inCommand, inIndirectBuffer, ofst, 1, sz);
}


void VkeMesh::draw(VkCommandBuffer* inCommand)
{
  vkCmdDrawIndexed(*inCommand, 0, m_index_count, 0, 0, 1);
}

VkeMesh::List::List() {}
VkeMesh::List::~List() {}

VkeMesh::ID VkeMesh::List::nextID()
{
  VkeMesh::ID outID;
  if(m_deleted_keys.size() == 0)
    return m_data.size();
  outID = m_deleted_keys.back();
  m_deleted_keys.pop_back();
  return outID;
}

VkeMesh::Count VkeMesh::List::count()
{
  return m_data.size();
}

VkeMesh* VkeMesh::List::newMesh()
{
  VkeMesh::ID id = nextID();
  return newMesh(id);
}

VkeMesh* VkeMesh::List::newMesh(const VkeMesh::ID& inID)
{
  VkeMesh* outMesh = new VkeMesh(inID);
  m_data[inID]     = outMesh;
  return outMesh;
}

VkeMesh* VkeMesh::List::newMesh(const VkeMesh::ID& inID, VKSFile* inFile, VKSMeshRecord* inData)
{
  VkeMesh* outMesh = newMesh(inID);
  if(!outMesh)
    return NULL;
  outMesh->initFromMesh(inFile, inData);
  return outMesh;
}

VkeMesh* VkeMesh::List::newMesh(const VkeMesh::ID& inID, Mesh* const inMesh)
{
  VkeMesh* outMesh = newMesh(inID);
  if(!outMesh)
    return NULL;
  outMesh->initFromMesh(inMesh);
  return outMesh;
}

void VkeMesh::List::addMesh(VkeMesh* const inMesh)
{
  VkeMesh::ID id = nextID();
  m_data[id]     = inMesh;
}

VkeMesh* VkeMesh::List::getMesh(const VkeMesh::ID& inID)
{
  return m_data[inID];
}
