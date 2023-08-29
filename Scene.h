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

#ifndef __H_SCENE_
#define __H_SCENE_

#include "Camera.h"
#include "Node.h"
#include "Types.h"
#include <map>
#include <stdint.h>


#pragma once
class Scene
{
public:
  typedef uint32_t             ID;
  typedef std::map<ID, Scene*> Map;


  Scene();
  Scene(const ID& inID);
  ~Scene();

  void update();

  Node::NodeList& Nodes();
  Node::NodeList& Cameras();
  Node::NodeList& Lights();

  void    CurrentCamera(Camera* inCamera);
  void    CurrentCamera(const Camera::ID& inID);
  Camera* CurrentCamera();

  Camera* newCamera(const float& inX = 0.0f, const float& inY = 0.0f, const float& inZ = -5.0f);
  Camera* getCamera(const Camera::ID& inID);

  void getTriangles(render::TriangleList& outTriangles);

private:
  ID m_id;

  Node::NodeList m_root_nodes;
  Node::NodeList m_camera_list;
  Node::NodeList m_light_list;

  Camera* m_current_camera = nullptr;
};


#endif