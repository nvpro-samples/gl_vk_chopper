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

#ifndef __H_VKA_UTILS_
#define __H_VKA_UTILS_

#include "nvh/nvprint.hpp"
#include <glm/glm.hpp>
#include <stdio.h>
#include <vulkan/vulkan.h>
#include "glm/gtc/matrix_transform.hpp"

#define VKA_CHECK_ERROR(func, msg)                                                                                     \
  {                                                                                                                    \
    VkResult rslt = func;                                                                                              \
    if(rslt != VK_SUCCESS)                                                                                             \
    {                                                                                                                  \
      LOGE("ERROR \t [%d] : %s \n", rslt, msg);                                                                        \
    }                                                                                                                  \
  }


void dumpGlobalLayerNames(VkLayerProperties* pros, uint32_t count);

struct Projection
{
  float         nearplane;
  float         farplane;
  float         fov;
  glm::mat4 matrix;

  glm::mat4 proj;
  glm::vec3 position;
  glm::vec3 target;

  Projection()
      : nearplane(0.1f)
      , farplane(400.0f)
      , fov((40.0f))
      , position(0.0, 0.0, -15.0)
      , target(0.0, 0.0, 0.0)
  {
  }

  void update(int width, int height)
  {

    glm::mat4 camView(1);

    camView = glm::lookAt(target - position, target, glm::vec3(0, 1, 0));

    proj = glm::perspectiveRH_ZO(fov, float(width) / float(height), nearplane, farplane);

    matrix = proj * camView;
  }
};

struct ShadowProjection
{
  float         nearplane;
  float         farplane;
  float         fov;
  glm::mat4 matrix;

  ShadowProjection()
      : nearplane(0.1f)
      , farplane(400.0f)
      , fov((90.0f))
  {
  }

  void update(int width, int height)
  {

    glm::mat4 camView(1);

    glm::vec3 viewDir = glm::vec3(5 * 1.4, 20.5 * 1.4, 20 * 1.4);

    camView = glm::lookAt(viewDir, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0, 1, 0));

    matrix = glm::perspectiveRH_ZO(fov, float(width) / float(height), nearplane, farplane);
    matrix = matrix * camView;
  }
};


typedef struct _DepthImageView
{
  VkFormat       format;
  VkImage        image;
  VkDeviceMemory memory;
  VkImageView    view;
  /*VkAttachmentView view;*/
} DepthImageView;

typedef struct _ColorImageView
{
  VkFormat       format;
  VkImage        image;
  VkDeviceMemory memory;
  VkImageView    view;
  /*VkAttachmentView view;*/
} ColorImageView;

typedef struct _UBOView
{
  VkBuffer               buffer;
  VkDeviceMemory         memory;
  VkBufferView           view;
  VkDescriptorBufferInfo descriptor;
} UBOView;

typedef struct _TextureObject
{
  VkSampler sampler;
  VkImage   image;

  VkImageLayout  imageLayout;
  VkDeviceMemory memory;
  VkImageView    view;

  int32_t width;
  int32_t height;

} TextureObject;

typedef struct _vctr4
{
  float x;
  float y;
  float z;
  float w;
} Vctr4;

typedef struct _vctr2
{
  float x;
  float y;
} Vctr2;

typedef struct _VertexObject
{
  Vctr4 pos;
  Vctr4 nml;
  //Vctr2 uv;
} VertexObject;

typedef struct _VertexObjectUV
{
  Vctr4 pos;
  Vctr2 nml;
} VertexObjectUV;

typedef struct _BufferData
{
  VkBuffer               buf;
  VkDeviceMemory         memory;
  VkBufferView           view;
  VkDescriptorBufferInfo descriptor;
} BufferData;

typedef struct _UBOData
{
  glm::mat4 view_matrix;
  glm::mat4 nml_matrix;
} UBOData;

typedef struct _UBOCamera
{
  glm::mat4 proj_matrix;
  glm::mat4 inv_proj_matrix;
  glm::vec4 camera_position;
} UBOCamera;

typedef struct _UBOObject
{
  BufferData ubo;
} UBOObject;

typedef struct _MeshObject
{
  BufferData vbo;
  BufferData ibo;
  uint32_t   indexCount;
  uint32_t   vertexCount;
} MeshObject;

#endif

#ifndef ICD_SPV_H
#define ICD_SPV_H

#include <stdint.h>

#define ICD_SPV_MAGIC 0x07230203
#define ICD_SPV_VERSION 99

struct icd_spv_header
{
  uint32_t magic;
  uint32_t version;
  uint32_t gen_magic;  // Generator's magic number
};

#endif /* ICD_SPV_H */
