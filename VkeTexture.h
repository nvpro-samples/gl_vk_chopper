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

#ifndef __H_VKE_TEXTURE_
#define __H_VKE_TEXTURE_


#include <map>
#include <vector>
#include <vulkan/vulkan.h>


#pragma once
class VkeTexture
{
public:
  typedef size_t                                ID;
  typedef std::map<VkeTexture::ID, VkeTexture*> Map;
  typedef size_t                                Count;

  VkeTexture();
  VkeTexture(const ID& inID);
  ~VkeTexture();

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

    VkeTexture* newTexture();
    VkeTexture* newTexture(const VkeTexture::ID& inID);
    void        addTexture(VkeTexture* const inData);
    VkeTexture* getTexture(const ID& inID);

    void update();

    ID    nextID();
    Count count();

    void getData(VkeTexture::Data* outDescriptor, size_t offset = 0);


  private:
    VkeTexture::Map             m_data;
    std::vector<VkeTexture::ID> m_deleted_keys;
  };


  void initTexture();

  void loadDDSTextureFile(const char* inFile);

#ifdef USE_LIB_PNG
  void loadTextureFile(const char* inFile);
#endif

  void loadTextureFloatData(float* inData, uint32_t inWidth, uint32_t inHeight, uint32_t inCompCount = 4);


  inline bool& isReady() { return m_ready; }

  ID   getID() { return m_id; }
  void setID(const ID& inID) { m_id = inID; }

  inline void      setFormat(VkFormat inFormat) { m_format = inFormat; }
  inline VkFormat& getFormat() { return m_format; }

  Data& getData() { return m_data; }

  inline int32_t getWidth() { return m_width; }
  inline int32_t getHeight() { return m_height; }


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
