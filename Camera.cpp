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

#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
    : Node()
    , m_fov(45.0f)
    , m_near(0.01f)
    , m_far(100.0f)
{
  initProjection();
}

Camera::Camera(Node* inParent, const ID& inID, const float& inFOV, const float& inNear, const float& inFar)
    : Node(inParent, inID)
    , m_fov(45.0f)
    , m_near(0.01f)
    , m_far(100.0f)
{

  initProjection();
}

void Camera::initProjection(const float& inFOV, const float& inNear, const float& inFar)
{

  m_fov  = inFOV;
  m_near = inNear;
  m_far  = inFar;

  m_viewport.m_data[2] = 800.0f;
  m_viewport.m_data[3] = 800.0f;

  m_projection_dirty = true;
}

void Camera::updateProjection()
{

  if(!m_projection_dirty)
    return;


  float vw = m_viewport.m_data[2] - m_viewport.m_data[0];
  float vh = m_viewport.m_data[3] - m_viewport.m_data[1];

  float aspect = vw / vh;

  m_projection = glm::perspectiveRH_ZO(glm::radians(m_fov), aspect, m_near, m_far);

  m_projection_dirty = false;
}

Camera::~Camera() {}
