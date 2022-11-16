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

#include "VkeAnimationChannel.h"
#include "VkeSceneAnimation.h"


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

VkeAnimationKey* VkeAnimationChannel::newKey(double& inTime, nvmath::vec4f& inData)
{
  if(m_parent)
  {
    m_parent->updateDuration(inTime);
  }
  return m_keys.newKey(inTime, inData);
}

nvmath::vec4f cubicLerp(nvmath::vec4f inA, nvmath::vec4f inB, float inT)
{
  float c = (3.0 - 2.0 * inT) * inT * inT;

  inA *= (float)(1.0 - c);
  inB *= (float)c;
  nvmath::vec4f outValue = inA;
  outValue += inB;
  return outValue;
}

nvmath::quatf cubicLerp(nvmath::quatf inA, nvmath::quatf inB, float inT)
{
  float c = (3.0 - 2.0 * inT) * inT * inT;
  return nvmath::slerp_quats(c, inA, inB);
}

nvmath::quatf VkeAnimationChannel::currentQuatValue()
{


  VkeAnimationKeyPair pair;
  double              curTime = m_parent->getCurrentTime();
  m_keys.getKeys(curTime, &pair);

  //no keys found at all for this time.
  //Therefore there should not have been a channel
  //for this node in the first place.
  if(!pair.low && !pair.high)
    return nvmath::quatf();
  if(!pair.high)
  {
    nvmath::vec4f vValue = pair.low->getValue();
    nvmath::quatf qValue(vValue.x, vValue.y, vValue.z, vValue.w);
    return qValue;
  }
  if(!pair.low)
  {
    nvmath::vec4f vValue = pair.high->getValue();
    nvmath::quatf qValue(vValue.x, vValue.y, vValue.z, vValue.w);
    return qValue;
  }

  double timeDelta = pair.high->getTime() - pair.low->getTime();
  if(timeDelta == 0.0)
  {
    nvmath::vec4f vValue = pair.low->getValue();
    nvmath::quatf qValue(vValue.x, vValue.y, vValue.z, vValue.w);
    return qValue;
  }

  double durationDelta = curTime - pair.low->getTime();

  double timeScale = durationDelta / timeDelta;

  nvmath::vec4f lowVal  = pair.low->getValue();
  nvmath::vec4f highVal = pair.high->getValue();

  nvmath::quatf quatA(lowVal.x, lowVal.y, lowVal.z, lowVal.w);
  nvmath::quatf quatB(highVal.x, highVal.y, highVal.z, highVal.w);

  nvmath::quatf outQuat = cubicLerp(quatA, quatB, timeScale);

  return outQuat;
}

nvmath::vec4f VkeAnimationChannel::currentValue()
{


  VkeAnimationKeyPair pair;
  double              curTime = m_parent->getCurrentTime();
  m_keys.getKeys(curTime, &pair);

  //no keys found at all for this time.
  //Therefore there should not have been a channel
  //for this node in the first place.
  if(!pair.low && !pair.high)
    return nvmath::vec4f(0.0, 0.0, 0.0);
  if(!pair.high)
    return pair.low->getValue();
  if(!pair.low)
    return pair.high->getValue();

  double timeDelta = pair.high->getTime() - pair.low->getTime();
  if(timeDelta == 0.0)
    return pair.low->getValue();

  double durationDelta = curTime - pair.low->getTime();

  double timeScale = durationDelta / timeDelta;

  nvmath::vec4f lowVal  = pair.low->getValue();
  nvmath::vec4f highVal = pair.high->getValue();

  nvmath::vec4f outVal = cubicLerp(lowVal, highVal, timeScale);

  //	lowVal *= (float)(1.0 - timeScale);
  //	highVal *= (float)timeScale;
  //	highVal += lowVal;
  return outVal;
}

void VkeAnimationChannel::update() {}
