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

#include "Mesh.h"
#include <stdlib.h>

Mesh::Mesh()
    : m_vertices(NULL)
    , m_indices(NULL)
    , m_normals(NULL)
    , m_max_vertices(0)
    , m_max_indices(0)
    , m_vertex_count(0)
    , m_index_count(0)
    , m_material_id(-1)
{
}

Mesh::Mesh(const Mesh::ID& inID)
    : m_id(inID)
    , m_vertices(NULL)
    , m_indices(NULL)
    , m_normals(NULL)
    , m_max_vertices(0)
    , m_max_indices(0)
    , m_vertex_count(0)
    , m_index_count(0)
    , m_material_id(-1)
{
}

void Mesh::allocate(uint32_t inMaxVerts, uint32_t inMaxIdx)
{
  dispose();
  m_max_vertices = inMaxVerts;
  m_max_indices  = inMaxIdx;
  m_vertices     = new Vec4f[m_max_vertices];
  m_normals      = new Vec4f[m_max_vertices];
  m_indices      = new uint32_t[m_max_indices];
  m_uvs          = new Vec2f[m_max_vertices];
}

Vec4f*& Mesh::getVertices()
{
  return m_vertices;
}

Vec4f*& Mesh::getNormals()
{
  return m_normals;
}

uint32_t*& Mesh::getIndices()
{
  return m_indices;
}

Vec2f*& Mesh::getUVs()
{
  return m_uvs;
}

void Mesh::addVertex(const Vec4f& inVertex)
{
  Vec4f normal = {0.0, 0.0, 0.0, 1.0};
  addVertex(inVertex, normal);
}

void Mesh::addVertex(const Vec4f& inVertex, const Vec4f& inNormal)
{
  if(m_vertex_count >= m_max_vertices)
    return;
  m_normals[m_vertex_count]    = inNormal;
  m_vertices[m_vertex_count++] = inVertex;
}

void Mesh::addIndex(const uint32_t& inIndex)
{
  if(m_index_count >= m_max_indices)
    return;
  m_indices[m_index_count++] = inIndex;
  m_triangle_count           = m_index_count / 3;
}

Triangle4f Mesh::getTriangle(const uint32_t inIndex)
{
  uint32_t   startIndex = inIndex * 3;
  Triangle4f out        = {m_vertices[m_indices[startIndex]],     m_vertices[m_indices[startIndex + 1]],
                    m_vertices[m_indices[startIndex + 2]], m_normals[m_indices[startIndex]],
                    m_normals[m_indices[startIndex + 1]],  m_normals[m_indices[startIndex + 2]]};
  return out;
}

Triangle4f Mesh::getTransformedTriangle(Mat4x4f& inTransform, const uint32_t inIndex)
{
  uint32_t startIndex = inIndex * 3;

  Triangle4f out = {
      inTransform(m_vertices[m_indices[startIndex]]),     inTransform(m_vertices[m_indices[startIndex + 1]]),
      inTransform(m_vertices[m_indices[startIndex + 2]]), inTransform(m_normals[m_indices[startIndex]]),
      inTransform(m_normals[m_indices[startIndex + 1]]),  inTransform(m_normals[m_indices[startIndex + 2]])};

  return out;
}


void Mesh::dispose()
{
  if(m_vertices)
  {
    delete[] m_vertices;
    m_vertices = NULL;
  }

  if(m_indices)
  {
    delete[] m_indices;
    m_indices = NULL;
  }

  if(m_normals)
  {
    delete[] m_normals;
    m_normals = NULL;
  }

  m_max_indices    = 0;
  m_max_vertices   = 0;
  m_index_count    = 0;
  m_vertex_count   = 0;
  m_triangle_count = 0;
}

void Mesh::getTransformedTriangles(Mat4x4f& inTransform, TriangleList4f& outTriangles)
{

  for(uint32_t i = 0; i < m_triangle_count; ++i)
  {
    Triangle4f tri = getTransformedTriangle(inTransform, i);
    outTriangles.push_back(tri);
  }
}


Mesh::~Mesh() {}
