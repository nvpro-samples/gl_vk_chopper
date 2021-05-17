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

#ifndef __H_RENDERABLE_
#define __H_RENDERABLE_

#pragma once
#include "Mesh.h"
#include <vector>

class Renderable
{
public:
  typedef uint32_t                 ID;
  typedef std::vector<Renderable*> List;

  Renderable();
  ~Renderable();

  Mesh* getMesh();
  void  setMesh(const Mesh::ID& inID);

private:
  Mesh::ID m_mesh_id;
  uint32_t m_surface_id;
};

#endif