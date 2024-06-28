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

#include "Mesh.h"
#include "MeshUtils.h"
#include "Scene.h"
#include "Types.h"
#include <map>

class RenderContext
{
public:
  ~RenderContext();

  static RenderContext* Get();

  Scene* newScene(const Scene::ID& inID);
  Scene* getScene(const Scene::ID& inID);
  void   deleteScene(const Scene::ID& inID);

  Mesh* newMesh(const Mesh::ID& inID, const meshimport::MeshDataf& inData);
  Mesh* newMesh(const Mesh::ID& inID);
  Mesh* getMesh(const Mesh::ID& inID);
  void  deleteMesh(const Mesh::ID& inID);

private:
  RenderContext();

  Scene::Map m_scene_map;
  Mesh::Map  m_mesh_map;
};
