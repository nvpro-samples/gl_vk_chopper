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

#include <cstring>
#include "RenderContext.h"

RenderContext* RenderContext::Get()
{
  static RenderContext* Instance(NULL);
  if(!Instance)
  {
    Instance = new RenderContext();
  }
  return Instance;
}

RenderContext::RenderContext() {}


Scene* RenderContext::newScene(const Scene::ID& inID)
{
  deleteScene(inID);
  Scene* scene      = new Scene(inID);
  m_scene_map[inID] = scene;
  return scene;
}

Scene* RenderContext::getScene(const Scene::ID& inID)
{
  return m_scene_map[inID];
}

void RenderContext::deleteScene(const Scene::ID& inID)
{
  Scene* scene = m_scene_map[inID];
  if(scene)
  {
    m_scene_map.erase(inID);
    delete scene;
    scene = NULL;
  }
}

Mesh* RenderContext::newMesh(const Mesh::ID& inID, const meshimport::MeshDataf& inData)
{

  deleteMesh(inID);

  Mesh* mesh = new Mesh(inID);

  mesh->allocate(inData.vertex_count, inData.index_count);

  glm::vec4* nmls = mesh->getNormals();
  glm::vec4* vtxs = mesh->getVertices();
  uint32_t*  idxs = mesh->getIndices();
  glm::vec2* uvs  = mesh->getUVs();

  mesh->setMaterialID(inData.materialID);

  memcpy(vtxs, inData.vertices, sizeof(float) * 4 * inData.vertex_count);
  memcpy(nmls, inData.normals, sizeof(float) * 4 * inData.vertex_count);
  memcpy(idxs, inData.indices, sizeof(uint32_t) * inData.index_count);
  memcpy(uvs, inData.uvs, sizeof(float) * 2 * inData.vertex_count);

  mesh->setIndexCount(inData.index_count);
  mesh->setVertexCount(inData.vertex_count);

  return mesh;
}

Mesh* RenderContext::newMesh(const Mesh::ID& inID)
{
  deleteMesh(inID);
  Mesh* mesh       = new Mesh(inID);
  m_mesh_map[inID] = mesh;
  return mesh;
}

Mesh* RenderContext::getMesh(const Mesh::ID& inID)
{
  return m_mesh_map[inID];
}

void RenderContext::deleteMesh(const Mesh::ID& inID)
{
  Mesh* mesh = m_mesh_map[inID];
  if(mesh)
  {
    m_mesh_map.erase(inID);
    delete mesh;
    mesh = NULL;
  }
}

RenderContext::~RenderContext() {}
