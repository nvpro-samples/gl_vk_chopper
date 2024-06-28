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

#include <glm/glm.hpp>
#include <stdint.h>
#include <vector>

struct VkeAnimationKeyPair;


class VkeAnimationKey
{
public:
  typedef uint32_t ID;
  typedef uint32_t Count;

  VkeAnimationKey()
      : m_value(0.f, 0.f, 0.f, 0.f)
      , m_time(0.)
  {
  }
  VkeAnimationKey(double& inTime, glm::vec4& inData)
      : m_value(inData)
      , m_time(inTime)
  {
  }
  ~VkeAnimationKey() {}

  double&        getTime();
  glm::vec4& getValue();

  class List
  {
  public:
    List() {}
    ~List() {}

    VkeAnimationKey* newKey(double& inTime, glm::vec4& inData);

    VkeAnimationKey* getKey(const ID& inID);

    void getKeys(double& inTime, VkeAnimationKeyPair* outPair);

    std::vector<VkeAnimationKey*> m_data;
  };


private:
  glm::vec4 m_value;
  double        m_time;
};


struct VkeAnimationKeyPair
{
  VkeAnimationKey* low;
  VkeAnimationKey* high;

  VkeAnimationKeyPair()
      : low(NULL)
      , high(NULL)
  {
  }
};
