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

#include "Scene.h"


Scene::Scene()
    : m_id(0)
{
}

Scene::Scene(const Scene::ID& inID)
    : m_id(inID)
{
}

void Scene::update()
{
  size_t cnt = m_root_nodes.count();

  for(size_t i = 0; i < cnt; ++i)
  {
    m_root_nodes.getNode(i)->update();
  }

  cnt = m_camera_list.count();

  for(size_t i = 0; i < cnt; ++i)
  {
    m_camera_list.getNode(i)->update();
  }
}

Node::NodeList& Scene::Nodes()
{
  return m_root_nodes;
}

Node::NodeList& Scene::Cameras()
{
  return m_camera_list;
}

Node::NodeList& Scene::Lights()
{
  return m_light_list;
}

void Scene::CurrentCamera(Camera* inCamera)
{
  m_current_camera = inCamera;
}

void Scene::CurrentCamera(const Camera::ID& inCamera)
{
  m_current_camera = (Camera*)m_camera_list.getNode(inCamera);
}

Camera* Scene::CurrentCamera()
{
  return m_current_camera;
}

Camera* Scene::newCamera(const float& inX, const float& inY, const float& inZ)
{
  m_current_camera = m_camera_list.newNodeClass<Camera>(NULL);
  m_current_camera->GetTransform().translate(inX, inY, inZ);
  return m_current_camera;
}

Camera* Scene::getCamera(const Camera::ID& inID)
{
  return (Camera*)m_camera_list.getNode(inID);
}

void Scene::getTriangles(render::TriangleList& outTriangles)
{

  size_t cnt = m_root_nodes.count();
  for(size_t i = 0; i < cnt; ++i)
  {
    m_root_nodes.getNode(i)->getTriangles(outTriangles);
  }
}

Scene::~Scene() {}
