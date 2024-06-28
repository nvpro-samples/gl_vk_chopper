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

#include <map>
#include <vector>
#include <vulkan/vulkan.h>

class VkeCubeTexture
{
public:
  typedef size_t                                        ID;
  typedef std::map<VkeCubeTexture::ID, VkeCubeTexture*> Map;
  typedef size_t                                        Count;

  VkeCubeTexture();
  VkeCubeTexture(const ID& inID);
  ~VkeCubeTexture();

  struct Data
  {
    VkSampler      sampler     = VK_NULL_HANDLE;
    VkImage        image       = VK_NULL_HANDLE;
    VkImageLayout  imageLayout = {};
    VkDeviceMemory memory      = VK_NULL_HANDLE;
    VkImageView    view        = VK_NULL_HANDLE;
  };

  class List
  {
  public:
    List();
    ~List();

    VkeCubeTexture* newTexture();
    VkeCubeTexture* newTexture(const VkeCubeTexture::ID& inID);
    void            addTexture(VkeCubeTexture* const inData);
    VkeCubeTexture* getTexture(const ID& inID);

    void update();

    ID    nextID();
    Count count();

    void getData(VkeCubeTexture::Data* outDescriptor, uint32_t offset = 0);


  private:
    VkeCubeTexture::Map             m_data;
    std::vector<VkeCubeTexture::ID> m_deleted_keys;
  };

  void loadCubeDDS(const char* inPath);

  inline bool& isReady() { return m_ready; }

  ID   getID() { return m_id; }
  void setID(const ID& inID) { m_id = inID; }

  Data& getData() { return m_data; }


private:
  ID m_id = 0;

  int32_t m_width  = 0;
  int32_t m_height = 0;
  bool    m_ready  = false;

  VkImageTiling     m_tiling       = VK_IMAGE_TILING_LINEAR;
  VkImageUsageFlags m_usage_flags  = VK_IMAGE_USAGE_SAMPLED_BIT;
  VkFlags           m_memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
  VkFormat          m_format       = VK_FORMAT_R8G8B8A8_UNORM;

  Data m_data = {};
};
