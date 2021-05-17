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

#ifndef __H_TRANSFORM_
#define __H_TRANSFORM_

#pragma once

#include <nvmath/nvmath.h>

class Transform
{
public:
  Transform();
  ~Transform();

  void reset();
  void update(Transform* inParent = NULL);
  void translate(nvmath::vec4f& inPosition);
  void translate(float inX, float inY, float inZ);

  void rotate(const float inValue, nvmath::vec3f& inBasis);
  void rotate(nvmath::quatf& inQuaterion);

  void scale(const nvmath::vec3f& inScale);
  void scale(const float inX, const float inY, const float inZ);

  nvmath::vec4f operator()(nvmath::vec4f& rhs)
  {
    nvmath::vec4f out = m_transform * rhs;
    return out;
  }

  nvmath::mat4f operator()(nvmath::mat4f& rhs)
  {
    nvmath::mat4f out = rhs * m_transform;
    return out;
  }

  nvmath::vec4f operator[](nvmath::vec4f& rhs)
  {
    nvmath::vec4f out = m_inverse * rhs;
    return out;
  }

  nvmath::mat4f& getTransform() { return m_transform; }
  nvmath::mat4f& getInverse() { return m_inverse; }

  void setMatrix(nvmath::mat4f& inMatrix) { m_matrix = inMatrix; }

private:
  nvmath::mat4f m_transform;
  nvmath::mat4f m_inverse;
  nvmath::mat4f m_matrix;
};

#endif
