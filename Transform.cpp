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

#include "Transform.h"


Transform::Transform()
{
  reset();
}


void Transform::reset()
{

  m_matrix.identity();
}

void Transform::update(Transform* inParent)
{

  m_transform = m_matrix;

  if(inParent)
  {
    m_transform = (inParent->getTransform()) * m_transform;
  }

  m_inverse = m_transform;

  m_inverse = nvmath::invert(m_inverse);
  m_inverse = nvmath::transpose(m_inverse);

  m_inverse.a30 = 0.0;
  m_inverse.a31 = 0.0;
  m_inverse.a32 = 0.0;
}

void Transform::translate(nvmath::vec4f& inPosition)
{
  nvmath::vec3f vec = nvmath::vec3f(inPosition);
  m_matrix.translate(vec);
}

void Transform::translate(float inX, float inY, float inZ)
{
  nvmath::vec3f vec = nvmath::vec3f(inX, inY, inZ);
  m_matrix.translate(vec);
}

void Transform::rotate(const float inValue, nvmath::vec3f& inBasis)
{


  m_matrix.rotate(inValue, inBasis);
}

void Transform::rotate(nvmath::quatf& inQuat)
{

  m_matrix.rotate(inQuat);
}

void Transform::scale(const nvmath::vec3f& inScale)
{
  m_matrix.scale(inScale);
}

void Transform::scale(const float inX, const float inY, const float inZ)
{
  scale(nvmath::vec3f(inX, inY, inZ));
}


Transform::~Transform() {}
