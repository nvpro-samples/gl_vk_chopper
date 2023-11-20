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

#ifndef __H_VKE_SCENE_ANIMATION_
#define __H_VKE_SCENE_ANIMATION_

#pragma once


#include "VkeAnimationNode.h"
#include <glm/glm.hpp>

class VkeSceneAnimation
{
public:
  VkeSceneAnimation();
  ~VkeSceneAnimation();

  double& getDuration();

  VkeAnimationNode::List& Nodes();

  void setCurrentTime(double& inTime);

  double& getCurrentTime();
  double& getStartTime();
  double& getEndTime();

  void update();
  void updateDuration(double& inTime);

  VkeAnimationNode* newNode(VkeAnimationNode::Name& inName);


private:
  double m_duration;
  double m_start_time;
  double m_end_time;

  VkeAnimationNode::List m_nodes;

  double m_current_time;
};

#endif
