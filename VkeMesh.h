/*
 * Copyright (c) 2014-2023, NVIDIA CORPORATION.  All rights reserved.
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

#ifndef __H_VKE_MESH_
#define __H_VKE_MESH_


#ifndef USE_SINGLE_VBO
#define USE_SINGLE_VBO 1
#endif


#pragma once

#include "Mesh.h"
#include "VkeIBO.h"
#include "VkeVBO.h"
#include <map>
#include <queue>
struct VKSMeshRecord;
struct VKSFile;
class VkeMesh
{
public:
  typedef size_t                          ID;
  typedef std::map<VkeMesh::ID, VkeMesh*> Map;
  typedef size_t                          Count;


  class List
  {
  public:
    List();
    ~List();

    VkeMesh* newMesh();
    VkeMesh* newMesh(const VkeMesh::ID& inID);
    VkeMesh* newMesh(const VkeMesh::ID& inID, VKSFile* inFile, VKSMeshRecord* inData);
    VkeMesh* newMesh(const VkeMesh::ID& inID, Mesh* const inMesh);
    void     addMesh(VkeMesh* const inMesh);
    VkeMesh* getMesh(const ID& inID);

    ID    nextID();
    Count count();

  private:
    VkeMesh::Map             m_data;
    std::vector<VkeMesh::ID> m_deleted_keys;
  };


  VkeMesh();
  VkeMesh(const ID& inID);
  ~VkeMesh();

  void initFromMesh(Mesh* const inMesh);
  void initFromMesh(VKSFile* inFile, VKSMeshRecord* inMesh);

  void initVKBuffers();

  void bind(VkCommandBuffer* inCmd);

  void initBindCommand(VkCommandPool* inPool, VkQueue* inQueue);
  void initDrawCommand(VkCommandBuffer* inCommand);
  void initDrawCommand(VkCommandBuffer* inCommand, VkBuffer& inIndirectBuffer, uint32_t inIndex);

  void draw(VkCommandBuffer* inCommand);

  ID getID() { return m_id; }


  int32_t getMaterialID() { return m_material_id; }
  void    setFirstIndex(const uint32_t inFirstIndex) { m_first_index = inFirstIndex; }
  void    setFirstVertex(const uint32_t inFirstvertex) { m_first_vertex = inFirstvertex; }

  const uint32_t getFirstIndex() { return m_first_index; }
  const uint32_t getFirstVertex() { return m_first_vertex; }

  const uint32_t getIndexCount() { return m_index_count; }


protected:
  ID m_id;

  /*
		Vertex Buffer
	*/
  VkeVBO m_vbo;

  /*
		Index Buffer
	*/
  VkeIBO m_ibo;

  VertexObject* m_vertices = nullptr;
  uint32_t*     m_indices  = nullptr;

  uint32_t m_vertex_count = 0;
  uint32_t m_index_count  = 0;

  uint32_t m_first_index  = 0;
  uint32_t m_first_vertex = 0;

  VkCommandBuffer m_draw_cmd = nullptr;
  VkCommandBuffer m_bind_cmd = nullptr;

  int32_t m_material_id = -1;
};

#endif