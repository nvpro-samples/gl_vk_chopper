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

#ifndef __H_CAMERA_
#define __H_CAMERA_

#pragma once

#include "Node.h"

class Camera : public Node
{
public:
  Camera();
  Camera(Node* inParent, const ID& inID, const float& inFOV = 45.0f, const float& inNear = 0.01f, const float& inFar = 100.0f);
  ~Camera();


  glm::mat4& getProjection() { return m_projection; }
  glm::mat4  getViewProjection()
  {
    glm::mat4 out(1);
    out = m_projection * out;
    out = m_transform(out);
    return out;
  }

  inline float getFOV() { return m_fov; };
  inline void  setFOV(const float& inFOV)
  {
    m_fov              = inFOV;
    m_projection_dirty = true;
  }

  inline float getNear() { return m_near; }

  inline void setNear(const float& inNear)
  {
    m_near             = inNear;
    m_projection_dirty = true;
  }

  inline float getFar() { return m_far; }

  inline void setFar(const float& inFar)
  {
    m_far              = inFar;
    m_projection_dirty = true;
  }

  void initProjection(const float& inFOV = 45.0f, const float& inNear = 0.01, const float& inFar = 100.0);
  void updateProjection();

  void setViewport(const float inMinX, const float inMinY, const float inMaxX, const float inMaxY)
  {
    m_viewport.m_data[0] = inMinX;
    m_viewport.m_data[1] = inMinY;
    m_viewport.m_data[2] = inMaxX;
    m_viewport.m_data[3] = inMaxY;
  }

protected:
  glm::mat4 m_projection;

  float m_fov;
  float m_near;
  float m_far;

  Vec4f m_viewport;

  bool m_projection_dirty;
};

#endif
