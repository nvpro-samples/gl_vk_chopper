/*
 * Copyright (c) 2014-2024, NVIDIA CORPORATION.  All rights reserved.
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
 * SPDX-FileCopyrightText: Copyright (c) 2014-2024 NVIDIA CORPORATION
 * SPDX-License-Identifier: Apache-2.0
 */

/* Contact chebert@nvidia.com (Chris Hebert) for feedback */

#pragma once

#include "Node.h"
#include "VkeBuffer.h"
#include "VkeMesh.h"
#include <algorithm>
#include <map>

struct VkeNodeUniform
{
  glm::mat4  node_matrix;
  glm::mat4  inverse_node_matrix;
  glm::ivec4 lookup;
  glm::vec4  padding[3];
};


class VkeNodeData : public VkeBuffer<VkeNodeUniform>
{
public:
  typedef size_t                    ID;
  typedef std::vector<VkeNodeData*> Map;
  typedef size_t                    Count;

  class List
  {
  public:
    List();
    ~List();

    VkeNodeData* newData();
    VkeNodeData* newData(const VkeNodeData::ID& inID);
    void         addData(VkeNodeData* const inData);
    VkeNodeData* getData(const ID& inID);
    void         update();
    void         update(VkeNodeUniform* inData, uint32_t inInstanceCount = 1);

    ID    nextID();
    Count count();

    void getDescriptors(VkDescriptorBufferInfo* outDescriptor);
    void getMeshes(VkeMesh** outMeshes);
    void sortByMaterialID();
    void sortByOpacity();
    void sortByMeshID();


  private:
    VkeNodeData::Map             m_data;
    std::vector<VkeNodeData::ID> m_deleted_keys;
  };


  VkeNodeData();
  VkeNodeData(const ID& inID);
  ~VkeNodeData();


  void     setMesh(VkeMesh* inMesh) { m_mesh = inMesh; }
  VkeMesh* getMesh() { return m_mesh; }

  Node* getNode() { return m_node; }

  inline void setIndex(size_t inIndex) { m_index = inIndex; }

  void initNodeData();
  void initNodeDataSubAlloc();
  void updateFromNode();
  void updateFromNode(Node* const inNode, VkCommandBuffer* inBuffer = NULL);
  void updateFromNode(VkCommandBuffer* inBuffer);
  void updateFromNode(Node* const inNode, VkeNodeUniform* inData, uint32_t inInstanceCount = 1);
  void updateFromNode(VkeNodeUniform* inData, uint32_t inInstanceCount = 1);
  void updateConstantVKBufferData(VkCommandBuffer* inBuffer = NULL);

  void updateVKBufferData(VkeNodeUniform* inData);

  void bind(VkCommandBuffer* inBuffer);

  void setLayer(uint32_t inLayer) { m_layer = inLayer; }

  uint32_t getLayer() { return m_layer; }

  Node*    m_node;
  VkeMesh* m_mesh;

  uint32_t m_layer;

  bool m_needs_buffer_update;
};
