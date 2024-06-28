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

#include "Transform.h"
#include <glm/glm.hpp>
#include <map>
#include <stdio.h>\

template <typename T>
struct Triangle
{
  T m_vertices[3];
  T m_normals[3];
};

typedef Triangle<glm::vec3> Triangle3f;
typedef Triangle<glm::vec4> Triangle4f;

class Mesh
{
public:
  typedef uint32_t            ID;
  typedef std::map<ID, Mesh*> Map;

  Mesh();
  Mesh(const ID& inID);
  ~Mesh();

  void allocate(uint32_t inMaxVerts, uint32_t inMaxIdx);
  void dispose();

  glm::vec4*& getVertices();
  glm::vec4*& getNormals();
  uint32_t*&  getIndices();
  glm::vec2*& getUVs();


  void addVertex(const glm::vec4& inVertex);
  void addVertex(const glm::vec4& inVertex, const glm::vec4& inNormal);
  void addIndex(const uint32_t& inIndex);

  Triangle4f getTriangle(const uint32_t inIndex);

  uint32_t getVertexCount() { return m_vertex_count; }
  uint32_t getIndexCount() { return m_index_count; }

  uint32_t getMaxVertexCount() { return m_max_vertices; }
  uint32_t getMaxIndexCount() { return m_max_indices; }

  uint32_t getTriangleCount() { return m_triangle_count; }

  void setVertexCount(const uint32_t& inCount) { m_vertex_count = inCount; };
  void setIndexCount(const uint32_t& inCount)
  {
    m_index_count    = inCount;
    m_triangle_count = m_index_count / 3;
  };

  ID      getID() { return m_id; }
  int32_t getMaterialID() { return m_material_id; }
  void    setMaterialID(const int32_t inID) { m_material_id = inID; }

private:
  ID m_id;

  glm::vec4* m_vertices = nullptr;
  glm::vec4* m_normals  = nullptr;
  glm::vec2* m_uvs      = nullptr;

  uint32_t* m_indices = nullptr;

  uint32_t m_max_vertices = 0;
  uint32_t m_max_indices  = 0;

  uint32_t m_vertex_count = 0;
  uint32_t m_index_count  = 0;

  uint32_t m_triangle_count = 0;

  int32_t m_material_id = 0;
};
