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

#include "Transform.h"
#include "VkeBuffer.h"
#include <map>
#include <glm/glm.hpp>


#ifndef VKE_DEFAULT_CAMERA_VIEWPORT
#define VKE_DEFAULT_CAMERA_VIEWPORT 0, 0, 1024, 768
#endif

#ifndef VKE_DEFAULT_CAMERA_NEAR_PLANE
#define VKE_DEFAULT_CAMERA_NEAR_PLANE 0.001f
#endif

#ifndef VKE_DEFAULT_CAMERA_FAR_PLANE
#define VKE_DEFAULT_CAMERA_FAR_PLANE 600.0f
#endif

#ifndef VKE_DEFAULT_CAMERA_FOV
#define VKE_DEFAULT_CAMERA_FOV 45.0f
#endif

struct VkeCameraUniform
{
  glm::mat4 proj_view_matrix;          // proj * view
  glm::mat4 inverse_proj_view_matrix;  // inverse(proj * view)
  glm::vec4 camera_position;           // .xyz = camera position, .w = time
};

class VkeCamera : public VkeBuffer<VkeCameraUniform>
{
public:
  typedef uint32_t                            ID;
  typedef std::map<VkeCamera::ID, VkeCamera*> Map;
  typedef uint32_t                            Count;

  class List
  {
  public:
    List();
    ~List();

    void update();

    ID    nextID();
    Count count();

    void getDescriptors(VkDescriptorBufferInfo* outDescriptor);


  private:
    VkeCamera::Map             m_data;
    std::vector<VkeCamera::ID> m_deleted_keys;
  };

  VkeCamera();
  VkeCamera(const ID& inID);
  VkeCamera(const ID& inID, const float inX, const float inY, const float inZ);
  ~VkeCamera();

  void initCameraData();
  void updateCameraCmd(VkCommandBuffer inCommand);
  void update(float time);

  void bind(VkCommandBuffer* inBuffer);

  void setViewport(float inX, float inY, float inW, float inH);

  void setPosition(float inX, float inY, float inZ);
  void setRotation(float inX, float inY, float inZ);
  void setRotation(glm::quat& inQuat);

  void setNear(float inNear);
  void setFar(float inFar);
  void setFOV(float inFOV);

  float getNear();
  float getFar();
  float getFOV();

  glm::vec4 worldPosition();
  glm::vec4 worldPosition(glm::vec4& inPosition);

  void lookAt(glm::vec4& inPosition);
  void setLookAtMatrix(glm::mat4& inMat);

private:
  void updateProjection();
  void updateTransform();
  void updateViewProjection(float time);

  ID m_id = 0;

  glm::vec4 m_viewport{VKE_DEFAULT_CAMERA_VIEWPORT};

  float m_near   = VKE_DEFAULT_CAMERA_NEAR_PLANE;
  float m_far    = VKE_DEFAULT_CAMERA_FAR_PLANE;
  float m_fov    = VKE_DEFAULT_CAMERA_FOV;
  float m_aspect = 1.0f;

  glm::vec3 m_position{0.0f, 0.0f, 0.0f};
  glm::vec3 m_rotation{0.0f, 0.0f, 0.0f};

  Transform m_transform;

  glm::mat4 m_projection;
  bool      m_projection_needs_update = true;
  bool      m_transform_needs_update  = true;

  bool m_view_projection_needs_update = true;

  glm::mat4 m_look_at_matrix;
  bool      m_use_look_at = false;
};
