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

#ifndef __H_VKE_ANIMATION_NODE_
#define __H_VKE_ANIMATION_NODE_

#pragma once
#include "VkeAnimationChannel.h"
#include "VkeNodeData.h"
#include <map>

class VkeAnimationNode
{
public:
  typedef std::string                       Name;
  typedef std::map<Name, VkeAnimationNode*> Map;

  VkeAnimationNode();
  VkeAnimationNode(Name& inName, VkeSceneAnimation* inParent);

  ~VkeAnimationNode();

  VkeAnimationChannel& Position();
  VkeAnimationChannel& Rotation();
  VkeAnimationChannel& Scale();

  Name& getName();

  VkeNodeData* getNode();
  void         setNode(VkeNodeData* inNode);

  void update();

  class List
  {
  public:
    List();
    ~List();

    VkeAnimationNode* newNode(VkeAnimationNode::Name& inName, VkeSceneAnimation* inParent);
    VkeAnimationNode* getNode(VkeAnimationNode::Name& inName);

    void update();

  private:
    VkeAnimationNode::Map m_data;
  };

private:
  VkeSceneAnimation* m_parent;

  VkeAnimationChannel m_position;
  VkeAnimationChannel m_rotation;
  VkeAnimationChannel m_scale;

  Name m_name;

  VkeNodeData* m_scene_node;
};


#endif