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

#ifndef __H_MESH_
#define __H_MESH_

#include "Transform.h"
#include "WMath.h"
#include <map>
#include <stdio.h>

#pragma once
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

  Vec4f*&    getVertices();
  Vec4f*&    getNormals();
  uint32_t*& getIndices();
  Vec2f*&    getUVs();


  void addVertex(const Vec4f& inVertex);
  void addVertex(const Vec4f& inVertex, const Vec4f& inNormal);
  void addIndex(const uint32_t& inIndex);

  Triangle4f getTriangle(const uint32_t inIndex);
  Triangle4f getTransformedTriangle(Mat4x4f& inTransform, const uint32_t inIndex);

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

  void getTransformedTriangles(Mat4x4f& inTransform, TriangleList4f& outTriangles);

  ID      getID() { return m_id; }
  int32_t getMaterialID() { return m_material_id; }
  void    setMaterialID(const int32_t inID) { m_material_id = inID; }

private:
  ID m_id;

  Vec4f* m_vertices;
  Vec4f* m_normals;
  Vec2f* m_uvs;

  uint32_t* m_indices;

  uint32_t m_max_vertices;
  uint32_t m_max_indices;

  uint32_t m_vertex_count;
  uint32_t m_index_count;

  uint32_t m_triangle_count;

  int32_t m_material_id;
};

#endif
