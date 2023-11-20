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

#include "VkeAnimationKey.h"

double& VkeAnimationKey::getTime()
{
  return m_time;
}

glm::vec4& VkeAnimationKey::getValue()
{
  return m_value;
}

VkeAnimationKey* VkeAnimationKey::List::newKey(double& inTime, glm::vec4& inData)
{
  VkeAnimationKey* outKey = new VkeAnimationKey(inTime, inData);
  m_data.push_back(outKey);
  return outKey;
}

VkeAnimationKey* VkeAnimationKey::List::getKey(const ID& inID)
{
  return m_data[inID];
}

void VkeAnimationKey::List::getKeys(double& inTime, VkeAnimationKeyPair* outPair)
{


  //search the slow way first, then we'll do a binary search.
  size_t sz = m_data.size();

  for(size_t i = 0; i < sz; ++i)
  {
    VkeAnimationKey* key     = m_data[i];
    double           keyTime = key->getTime();

    if(keyTime <= inTime)
      outPair->low = key;
    if(keyTime > inTime)
    {
      outPair->high = key;
      return;
    }
  }
}
