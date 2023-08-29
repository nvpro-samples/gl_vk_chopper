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

  m_projection = nvmath::perspective(m_fov, m_aspect, m_near, m_far);

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

    nvmath::vec4f tra = nvmath::vec4f(m_position, 1.0);
    nvmath::vec3f fwd(0.0, 0.0, 1.0);
    nvmath::vec3f up(0.0, 0.0, 0.0);
    nvmath::vec3f lft(0.0, 0.0, 0.0);
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
  m_backing_store->view_matrix = nvmath::invert(m_transform.getTransform());

  m_backing_store->view_matrix.a03 = 0.0;
  m_backing_store->view_matrix.a13 = 0.0;
  m_backing_store->view_matrix.a23 = 0.0;

  m_time += 0.01f;

  m_backing_store->camera_position = nvmath::vec4f(m_position, m_time);

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

void VkeCamera::setRotation(nvmath::quatf& inQuat)
{
  nvmath::vec3f angles;
  inQuat.to_euler_xyz(angles);
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

nvmath::vec4f VkeCamera::worldPosition()
{
  nvmath::vec4f zero(0.0, 0.0, 0.0, 1.0);
  return worldPosition(zero);
}

nvmath::vec4f VkeCamera::worldPosition(nvmath::vec4f& inPosition)
{
  return m_transform(inPosition);
}

void VkeCamera::lookAt(nvmath::vec4f& inPos)
{
  m_use_look_at = true;
  nvmath::mat4f camView;
  m_look_at_matrix.identity();


  m_look_at_matrix = nvmath::look_at(nvmath::vec3f(inPos) - m_position, nvmath::vec3f(inPos), nvmath::vec3f(0, 1, 0));
  nvmath::vec3f zro(0.0f);
  m_position = m_look_at_matrix.get_translation(zro);
}

void VkeCamera::setLookAtMatrix(nvmath::mat4f& inMat)
{
  m_look_at_matrix  = inMat;
  nvmath::mat4f inv = nvmath::invert(inMat);
  m_position.x      = inv.a03;
  m_position.y      = inv.a13;
  m_position.z      = inv.a23;

  m_use_look_at = true;
}
