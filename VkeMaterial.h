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

#include "MeshUtils.h"
#include "VkeBuffer.h"
#include "VkeTexture.h"
#include <map>
#include <glm/glm.hpp>
#include <vector>

typedef struct VkeMaterialUniform
{
  //glm::vec4 diffuseColor;
  //glm::vec4 ambientColor;
  //glm::vec4 specularColor;
  float reflectivity;
  float shininess;
  float opacity;

  float padding;
  float morepadding[48];

} VkeMaterialUniform;

struct VKSFile;
struct VKSMaterialRecord;

class VkeMaterial : public VkeBuffer<VkeMaterialUniform>
{
public:
  typedef size_t                                  ID;
  typedef std::map<VkeMaterial::ID, VkeMaterial*> Map;
  typedef size_t                                  Count;

  class List
  {
  public:
    List();
    ~List();

    VkeMaterial* newMaterial();
    VkeMaterial* newMaterial(const VkeMaterial::ID& inID);
    void         addMaterial(VkeMaterial* const inMaterial);
    VkeMaterial* getMaterial(const ID& inID);

    ID    nextID();
    Count count();

    size_t getTextureCount();
    void   getTextureData(VkeTexture::Data* outData);

    VkeMaterial* getDefault()
    {
      if(!m_default)
      {

        m_default = new VkeMaterial(99999);
      }
      return m_default;
    }


  private:
    VkeMaterial::Map             m_data;
    std::vector<VkeMaterial::ID> m_deleted_keys;
    VkeMaterial*                 m_default;
  };


  VkeMaterial();
  VkeMaterial(const ID& inID);
  ~VkeMaterial();

  void initMaterialData();
  void initMaterialDataSubAlloc();

  void bind(VkCommandBuffer* inBuffer);

  void initFromData(meshimport::MaterialDataf* inData);
  void initFromData(VKSFile* inFile, VKSMaterialRecord* inMaterial);
  void initWithDefaults();

  void updateVKBufferData(VkeMaterialUniform* inData);

  VkeTexture::List& getTextures() { return m_textures; }

  ID   getID() { return m_id; }
  void setID(const ID& inID) { m_id = inID; }

private:
  ID m_id;

  VkeTexture::List m_textures;
};
