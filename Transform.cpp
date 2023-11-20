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
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

Transform::Transform()
{
  reset();
}


void Transform::reset()
{

  m_matrix = glm::mat4(1);
}

void Transform::update(Transform* inParent)
{

  m_transform = m_matrix;

  if(inParent)
  {
    m_transform = (inParent->getTransform()) * m_transform;
  }

  m_inverse = m_transform;

  m_inverse = glm::inverse(m_inverse);
  m_inverse = glm::transpose(m_inverse);

  m_inverse[3] = glm::vec4(0,0,0,1);
}

void Transform::translate(glm::vec4& inPosition)
{
  glm::vec3 vec = glm::vec3(inPosition);
  m_matrix = glm::translate(m_matrix, vec);
}

void Transform::translate(float inX, float inY, float inZ)
{
  glm::vec3 vec = glm::vec3(inX, inY, inZ);
  m_matrix = glm::translate(m_matrix, vec);
}

void Transform::rotate(const float inValue, glm::vec3& inBasis)
{


  m_matrix = glm::rotate(m_matrix, inValue, inBasis);
}

void Transform::rotate(glm::quat& inQuat)
{

  m_matrix = m_matrix * glm::mat4_cast(inQuat);
}

void Transform::scale(const glm::vec3& inScale)
{
  m_matrix = glm::scale(m_matrix, inScale);
}

void Transform::scale(const float inX, const float inY, const float inZ)
{
  scale(glm::vec3(inX, inY, inZ));
}


Transform::~Transform() {}
