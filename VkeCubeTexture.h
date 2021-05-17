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

#ifndef __H_VKE_CUBE_TEXTURE_
#define __H_VKE_CUBE_TEXTURE_

#include <map>
#include <vector>
#include <vulkan/vulkan.h>


#pragma once


#pragma once
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
    VkSampler      sampler;
    VkImage        image;
    VkImageLayout  imageLayout;
    VkDeviceMemory memory;
    VkImageView    view;
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


  void initTexture();

  void loadCubeDDS(const char* inPath);
#ifdef USE_LIB_PNG
  void loadTextureFiles(const char** inFile);
#endif

  inline bool& isReady() { return m_ready; }

  ID   getID() { return m_id; }
  void setID(const ID& inID) { m_id = inID; }

  Data& getData() { return m_data; }


private:
  ID m_id;

  int32_t m_width;
  int32_t m_height;
  bool    m_ready;

  VkImageTiling     m_tiling;
  VkImageUsageFlags m_usage_flags;
  VkFlags           m_memory_flags;
  VkFormat          m_format;

  Data     m_data;
  uint32_t m_mip_level;
};

#endif
