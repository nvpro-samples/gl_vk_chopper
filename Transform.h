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

class Transform
{
public:
  Transform();
  ~Transform();

  void reset();
  void update(Transform* inParent = NULL);
  void translate(glm::vec4& inPosition);
  void translate(float inX, float inY, float inZ);

  void rotate(const float inValue, glm::vec3& inBasis);
  void rotate(glm::quat& inQuaterion);

  void scale(const glm::vec3& inScale);
  void scale(const float inX, const float inY, const float inZ);

  glm::vec4 operator()(glm::vec4& rhs)
  {
    glm::vec4 out = m_transform * rhs;
    return out;
  }

  glm::mat4 operator()(glm::mat4& rhs)
  {
    glm::mat4 out = rhs * m_transform;
    return out;
  }

  glm::vec4 operator[](glm::vec4& rhs)
  {
    glm::vec4 out = m_inverse * rhs;
    return out;
  }

  glm::mat4& getTransform() { return m_transform; }
  glm::mat4& getInverse() { return m_inverse; }

  void setMatrix(glm::mat4& inMatrix) { m_matrix = inMatrix; }

private:
  glm::mat4 m_transform;
  glm::mat4 m_inverse;
  glm::mat4 m_matrix;
};
