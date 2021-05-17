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

#include "VkeAnimationNode.h"
#include "Node.h"

VkeAnimationNode::VkeAnimationNode()
    : m_scene_node(NULL)
    , m_parent(NULL)
{
}

VkeAnimationNode::VkeAnimationNode(VkeAnimationNode::Name& inName, VkeSceneAnimation* inParent)
    : m_name(inName)
    , m_scene_node(NULL)
    , m_parent(inParent)
{

  m_position.setParent(inParent);
  m_rotation.setParent(inParent);
  m_scale.setParent(inParent);
}

VkeNodeData* VkeAnimationNode::getNode()
{
  return m_scene_node;
}

void VkeAnimationNode::setNode(VkeNodeData* inNode)
{
  m_scene_node = inNode;
}


VkeAnimationChannel& VkeAnimationNode::Position()
{
  return m_position;
}

VkeAnimationChannel& VkeAnimationNode::Rotation()
{
  return m_rotation;
}

VkeAnimationChannel& VkeAnimationNode::Scale()
{
  return m_scale;
}

VkeAnimationNode::Name& VkeAnimationNode::getName()
{
  return m_name;
}

void VkeAnimationNode::update()
{
  if(!m_scene_node)
    return;
  Node* node = m_scene_node->getNode();
  if(!node)
    return;

  nvmath::vec4f curValue;

  curValue = m_position.currentValue();

  node->setPosition(curValue.x, curValue.y, curValue.z);


  nvmath::quatf curQuat = m_rotation.currentQuatValue();
  curQuat.to_euler_xyz(&curValue.x);

  node->setRotation(curValue.x, curValue.y, curValue.z);

  curValue = m_scale.currentValue();
  node->setScale(curValue.x, curValue.y, curValue.z);
}

VkeAnimationNode::List::List() {}

VkeAnimationNode::List::~List() {}

VkeAnimationNode* VkeAnimationNode::List::newNode(VkeAnimationNode::Name& inName, VkeSceneAnimation* inParent)
{
  VkeAnimationNode* outNode = new VkeAnimationNode(inName, inParent);
  m_data[inName]            = outNode;
  return outNode;
}

VkeAnimationNode* VkeAnimationNode::List::getNode(VkeAnimationNode::Name& inName)
{
  return m_data[inName];
}

void VkeAnimationNode::List::update()
{

  VkeAnimationNode::Map::iterator itr;

  for(itr = m_data.begin(); itr != m_data.end(); ++itr)
  {
    if(!itr->second)
      continue;
    itr->second->update();
  }
}


VkeAnimationNode::~VkeAnimationNode() {}
