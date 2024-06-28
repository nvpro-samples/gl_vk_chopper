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

#include "VkeAnimationKey.h"
class VkeSceneAnimation;

class VkeAnimationChannel
{
public:
  VkeAnimationChannel()
      : m_parent(NULL)
  {
  }
  VkeAnimationChannel(VkeSceneAnimation* inParent)
      : m_parent(inParent)
  {
  }
  ~VkeAnimationChannel() {}

  glm::vec4 currentValue();

  glm::quat currentQuatValue();

  VkeAnimationKey::List& Keys();

  VkeAnimationKey* newKey(double& inTime, glm::vec4& inData);

  double& getDuration();

  void setParent(VkeSceneAnimation* m_parent);

  void update();

private:
  VkeAnimationKey::List m_keys;
  VkeSceneAnimation*    m_parent;
};
