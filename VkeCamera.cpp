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

#include "VkeCamera.h"
#include "glm/gtc/quaternion.hpp"


VkeCamera::VkeCamera()
    : VkeBuffer()
{
  initCameraData();
}

VkeCamera::VkeCamera(const VkeCamera::ID& inID)
    : VkeBuffer()
    , m_id(inID)
{
  initCameraData();
}

VkeCamera::VkeCamera(const VkeCamera::ID& inID, const float inX, const float inY, const float inZ)
    : VkeBuffer()
    , m_id(inID)
    , m_position(inX, inY, inZ)
{
  initCameraData();
}

VkeCamera::~VkeCamera() {}

void VkeCamera::initCameraData()
{
  m_projection_needs_update      = true;
  m_transform_needs_update       = true;
  m_view_projection_needs_update = true;
  m_usage_flags                  = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  m_memory_flags                 = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

  initBackingStore(sizeof(VkeCameraUniform));
  initVKBufferData(false);
}

void VkeCamera::updateProjection()
{
  if(!m_projection_needs_update)
    return;
  m_aspect = m_viewport.z / m_viewport.w;

  m_projection = glm::perspectiveRH_ZO(m_fov, m_aspect, m_near, m_far);

  m_projection_needs_update      = false;
  m_view_projection_needs_update = true;
}

void VkeCamera::updateTransform()
{
  if(!m_transform_needs_update && !m_use_look_at)
    return;

  m_transform.reset();

  if(!m_use_look_at)
  {

    glm::vec4 tra = glm::vec4(m_position, 1.0);
    glm::vec3 fwd(0.0, 0.0, 1.0);
    glm::vec3 up(0.0, 0.0, 0.0);
    glm::vec3 lft(0.0, 0.0, 0.0);
    m_transform.translate(tra);
    m_transform.rotate(m_rotation.z, fwd);
    m_transform.rotate(m_rotation.y, up);
    m_transform.rotate(m_rotation.x, lft);
  }
  else
  {
    m_transform.setMatrix(m_look_at_matrix);
  }

  m_transform.update();

  m_transform_needs_update       = false;
  m_view_projection_needs_update = true;
}

void VkeCamera::updateViewProjection()
{
  if(!m_view_projection_needs_update && !m_use_look_at)
    return;

  m_backing_store->view_proj_matrix = m_projection;
  m_backing_store->view_proj_matrix *= m_transform.getTransform();
  m_backing_store->view_matrix = glm::inverse(m_transform.getTransform());

  m_backing_store->view_matrix[0][3] = 0.0;
  m_backing_store->view_matrix[1][3] = 0.0;
  m_backing_store->view_matrix[2][3] = 0.0;

  m_time += 0.01f;

  m_backing_store->camera_position = glm::vec4(m_position, m_time);

  m_view_projection_needs_update = false;
}

void VkeCamera::updateCameraCmd(VkCommandBuffer inCommand)
{

  vkCmdUpdateBuffer(inCommand, m_data.buffer, 0, sizeof(VkeCameraUniform), (const uint32_t*)&(m_backing_store[0]));
}

void VkeCamera::update()
{
  updateTransform();
  updateProjection();
  updateViewProjection();
}

void VkeCamera::bind(VkCommandBuffer* inBuffer) {}

void VkeCamera::setViewport(float inX, float inY, float inW, float inH)
{

  if((m_viewport.z == inW) && (m_viewport.w == inH))
    return;

  m_viewport.x              = inX;
  m_viewport.y              = inY;
  m_viewport.z              = inW;
  m_viewport.w              = inH;
  m_projection_needs_update = true;
}

void VkeCamera::setPosition(float inX, float inY, float inZ)
{
  m_position.x = inX;
  m_position.y = inY;
  m_position.z = inZ;

  m_transform_needs_update = true;
}
void VkeCamera::setRotation(float inX, float inY, float inZ)
{
  m_rotation.x = inX;
  m_rotation.y = inY;
  m_rotation.z = inZ;

  m_transform_needs_update = true;
}

void VkeCamera::setRotation(glm::quat& inQuat)
{
  glm::vec3 angles = glm::eulerAngles(inQuat);
  setRotation(angles.x, angles.y, angles.z);

  m_transform_needs_update = true;
}

void VkeCamera::setNear(float inNear)
{
  m_near                    = inNear;
  m_projection_needs_update = true;
}

void VkeCamera::setFar(float inFar)
{
  m_far                     = inFar;
  m_projection_needs_update = true;
}

void VkeCamera::setFOV(float inFOV)
{
  m_fov                     = inFOV;
  m_projection_needs_update = true;
}

float VkeCamera::getNear()
{
  return m_near;
}

float VkeCamera::getFar()
{
  return m_far;
}

float VkeCamera::getFOV()
{
  return m_fov;
}

glm::vec4 VkeCamera::worldPosition()
{
  glm::vec4 zero(0.0, 0.0, 0.0, 1.0);
  return worldPosition(zero);
}

glm::vec4 VkeCamera::worldPosition(glm::vec4& inPosition)
{
  return m_transform(inPosition);
}

void VkeCamera::lookAt(glm::vec4& inPos)
{
  m_use_look_at = true;
  glm::mat4 camView;
  m_look_at_matrix = glm::mat4(1);


  m_look_at_matrix = glm::lookAt(glm::vec3(inPos) - m_position, glm::vec3(inPos), glm::vec3(0, 1, 0));
  glm::vec3 zro(0.0f);
  m_position = m_look_at_matrix[3];
}

void VkeCamera::setLookAtMatrix(glm::mat4& inMat)
{
  m_look_at_matrix  = inMat;
  glm::mat4 inv = glm::inverse(inMat);
  m_position.x      = inv[0][3];
  m_position.y      = inv[1][3];
  m_position.z      = inv[2][3];

  m_use_look_at = true;
}
