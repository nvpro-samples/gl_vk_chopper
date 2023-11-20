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

#include "VkeAnimationChannel.h"
#include "VkeSceneAnimation.h"
#include "glm/gtc/quaternion.hpp"



double& VkeAnimationChannel::getDuration()
{
  return m_parent->getDuration();
}

VkeAnimationKey::List& VkeAnimationChannel::Keys()
{
  return m_keys;
}

void VkeAnimationChannel::setParent(VkeSceneAnimation* inParent)
{
  m_parent = inParent;
}

VkeAnimationKey* VkeAnimationChannel::newKey(double& inTime, glm::vec4& inData)
{
  if(m_parent)
  {
    m_parent->updateDuration(inTime);
  }
  return m_keys.newKey(inTime, inData);
}

glm::vec4 cubicLerp(glm::vec4 inA, glm::vec4 inB, float inT)
{
  float c = (3.0f - 2.0f * inT) * inT * inT;

  inA *= (float)(1.0f - c);
  inB *= (float)c;
  glm::vec4 outValue = inA;
  outValue += inB;
  return outValue;
}

glm::quat cubicLerp(glm::quat inA, glm::quat inB, float inT)
{
  float c = (3.0f - 2.0f * inT) * inT * inT;
  return glm::slerp(inA, inB, c);
}

glm::quat VkeAnimationChannel::currentQuatValue()
{


  VkeAnimationKeyPair pair;
  double              curTime = m_parent->getCurrentTime();
  m_keys.getKeys(curTime, &pair);

  //no keys found at all for this time.
  //Therefore there should not have been a channel
  //for this node in the first place.
  if(!pair.low && !pair.high)
    return glm::quat();
  if(!pair.high)
  {
    glm::vec4 vValue = pair.low->getValue();
    glm::quat qValue(vValue.x, vValue.y, vValue.z, vValue.w);
    return qValue;
  }
  if(!pair.low)
  {
    glm::vec4 vValue = pair.high->getValue();
    glm::quat qValue(vValue.x, vValue.y, vValue.z, vValue.w);
    return qValue;
  }

  float timeDelta = float(pair.high->getTime() - pair.low->getTime());
  if(timeDelta == 0.0)
  {
    glm::vec4 vValue = pair.low->getValue();
    glm::quat qValue(vValue.x, vValue.y, vValue.z, vValue.w);
    return qValue;
  }

  float durationDelta = float(curTime - pair.low->getTime());

  float timeScale = durationDelta / timeDelta;

  glm::vec4 lowVal  = pair.low->getValue();
  glm::vec4 highVal = pair.high->getValue();

  glm::quat quatA(lowVal.x, lowVal.y, lowVal.z, lowVal.w);
  glm::quat quatB(highVal.x, highVal.y, highVal.z, highVal.w);

  glm::quat outQuat = cubicLerp(quatA, quatB, timeScale);

  return outQuat;
}

glm::vec4 VkeAnimationChannel::currentValue()
{


  VkeAnimationKeyPair pair;
  double              curTime = m_parent->getCurrentTime();
  m_keys.getKeys(curTime, &pair);

  //no keys found at all for this time.
  //Therefore there should not have been a channel
  //for this node in the first place.
  if(!pair.low && !pair.high)
    return glm::vec4(0.0f);
  if(!pair.high)
    return pair.low->getValue();
  if(!pair.low)
    return pair.high->getValue();

  float timeDelta = float(pair.high->getTime() - pair.low->getTime());
  if(timeDelta == 0.0)
    return pair.low->getValue();

  float durationDelta = float(curTime - pair.low->getTime());

  float timeScale = durationDelta / timeDelta;

  glm::vec4 lowVal  = pair.low->getValue();
  glm::vec4 highVal = pair.high->getValue();

  glm::vec4 outVal = cubicLerp(lowVal, highVal, timeScale);

  //	lowVal *= (float)(1.0 - timeScale);
  //	highVal *= (float)timeScale;
  //	highVal += lowVal;
  return outVal;
}

void VkeAnimationChannel::update() {}
