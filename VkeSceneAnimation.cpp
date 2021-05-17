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

#include "VkeSceneAnimation.h"
#include <float.h>

VkeSceneAnimation::VkeSceneAnimation()
    : m_current_time(0.0)
    , m_duration(0.0)
    , m_start_time(DBL_MAX)
    , m_end_time(DBL_MIN)
{
}


VkeSceneAnimation::~VkeSceneAnimation() {}


double& VkeSceneAnimation::getDuration()
{
  return m_duration;
}

double& VkeSceneAnimation::getStartTime()
{
  return m_start_time;
}

double& VkeSceneAnimation::getEndTime()
{
  return m_end_time;
}

VkeAnimationNode::List& VkeSceneAnimation::Nodes()
{
  return m_nodes;
}

void VkeSceneAnimation::setCurrentTime(double& inTime)
{
  m_current_time = inTime;
}

double& VkeSceneAnimation::getCurrentTime()
{
  return m_current_time;
}

void VkeSceneAnimation::update()
{
  m_nodes.update();
}

VkeAnimationNode* VkeSceneAnimation::newNode(VkeAnimationNode::Name& inName)
{
  return m_nodes.newNode(inName, this);
}

void VkeSceneAnimation::updateDuration(double& inTime)
{
  m_start_time = std::min(m_start_time, inTime);
  m_end_time   = std::max(m_end_time, inTime);
  m_duration   = m_end_time - m_start_time;
}
