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

#ifndef __H_VKE_ANIMATION_KEY_
#define __H_VKE_ANIMATION_KEY_

#pragma once

#include <nvmath/nvmath.h>
#include <stdint.h>
#include <vector>

struct VkeAnimationKeyPair;


class VkeAnimationKey
{
public:
  typedef uint32_t ID;
  typedef uint32_t Count;

  VkeAnimationKey() {}
  VkeAnimationKey(double& inTime, nvmath::vec4f& inData)
      : m_time(inTime)
      , m_value(inData)
  {
  }
  ~VkeAnimationKey() {}

  double&        getTime();
  nvmath::vec4f& getValue();

  class List
  {
  public:
    List() {}
    ~List() {}

    VkeAnimationKey* newKey(double& inTime, nvmath::vec4f& inData);

    VkeAnimationKey* getKey(const ID& inID);

    void getKeys(double& inTime, VkeAnimationKeyPair* outPair);

    std::vector<VkeAnimationKey*> m_data;
  };


private:
  nvmath::vec4f m_value;
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


#endif
