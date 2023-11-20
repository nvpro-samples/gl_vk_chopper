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

#include "Node.h"
#include "glm/gtc/quaternion.hpp"


Node::Node() {}

Node::Node(Node* inParent, const ID& inID)
    : m_id(inID)
    , m_parent(inParent)
{
}

void Node::reset() {}

bool Node::update(bool inUpdateChildren)
{
  bool updated = false;

  //m_transform_needs_update = true;
  if(m_transform_needs_update || inUpdateChildren)
  {
    Transform* parentTransform = NULL;
    if(m_parent)
      parentTransform = &m_parent->GetTransform();

    m_transform.reset();
    glm::vec4 tra = glm::vec4(m_position, 1.0);
    m_transform.translate(tra);
    glm::quat q(m_rotation);
    m_transform.rotate(q);

    m_transform.update(parentTransform);
    m_transform_needs_update = false;
    updated                  = true;
  }

  size_t cnt = m_child_nodes.count();

  if(updated)
    for(size_t i = 0; i < cnt; ++i)
    {
      m_child_nodes.getNode(i)->update(updated);
    }

  return updated;
}

glm::vec4 Node::worldPosition()
{
  glm::vec4 outPosition(0.0, 0.0, 0.0, 1.0);

  return m_transform(outPosition);
}

glm::vec4 Node::worldPosition(glm::vec4& inPosition)
{
  return m_transform(inPosition);
}

void Node::draw() {}

void Node::setPosition(float inX, float inY, float inZ)
{
  m_position.x             = inX;
  m_position.y             = inY;
  m_position.z             = inZ;
  m_transform_needs_update = true;
}

void Node::setRotation(glm::quat& inQuat)
{
  glm::vec3 angles = glm::eulerAngles(inQuat);
  setRotation(angles.x, angles.y, angles.z);
}

void Node::setRotation(float inX, float inY, float inZ)
{
  m_rotation.x             = inX;
  m_rotation.y             = inY;
  m_rotation.z             = inZ;
  m_transform_needs_update = true;
}

void Node::setScale(float inX, float inY, float inZ)
{
  m_scale.x                = inX;
  m_scale.y                = inY;
  m_scale.z                = inZ;
  m_transform_needs_update = true;
}

void Node::setScale(float inScale)
{
  setScale(inScale, inScale, inScale);
}

Node* Node::newChild()
{
  return newChild(0.0, 0.0, 0.0);
}

Node* Node::newChild(const ID& inID)
{

  Node* node = new Node(this, inID);
  m_child_nodes.addNode(node, this);
  return node;
}

Node* Node::newChild(const Vec4f& inPosition)
{
  ID    id   = m_child_nodes.nextID();
  Node* node = new Node(this, id);
  m_child_nodes.addNode(node, this);
  return node;
}

Node* Node::newChild(const float inX, const float inY, const float inZ)
{
  Vec4f position = {inX, inY, inZ, 1.0};
  return newChild(position);
}

Renderable* Node::newRenderable()
{
  Renderable* renderable = new Renderable();
  m_renderables.push_back(renderable);
  return renderable;
}

Renderable* Node::getRenderable(const Renderable::ID& inID)
{
  if(inID >= m_renderables.size())
    return NULL;
  return m_renderables[inID];
}

Node::NodeList& Node::ChildNodes()
{
  return m_child_nodes;
}

Transform& Node::GetTransform()
{
  return m_transform;
}

void Node::getTriangles(render::TriangleList& outTriangles)
{
  size_t cnt = m_renderables.size();

  for(size_t i = 0; i < cnt; ++i)
  {
    Mesh* mesh = m_renderables[i]->getMesh();
    if(!mesh)
      continue;

    uint32_t triCnt = mesh->getTriangleCount();

    for(uint32_t j = 0; j < triCnt; ++j)
    {
      render::Triangle tri = {mesh->getTriangle(j), this};
      outTriangles.push_back(tri);
    }
  }
}

Node::~Node() {}

Node::NodeList::NodeList() {}
Node::NodeList::~NodeList() {}

Node::ID Node::NodeList::nextID()
{
  return m_data.size();
}

Node::Count Node::NodeList::count()
{
  return m_data.size();
}

Node* Node::NodeList::newNode(const ID& inID, Node* inParent)
{
  Node* node = new Node(inParent, inID);
  m_data.push_back(node);
  return node;
}

void Node::NodeList::addNode(Node* inNode, Node* inParent)
{

  if(!inNode)
    return;

  inNode->setParent(inParent);


  m_data.push_back(inNode);
}

Node* Node::NodeList::getNode(const ID& inID)
{
  if(inID >= count())
    return NULL;
  return m_data[inID];
}
