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

#include "VkeMaterial.h"
#include "VKSFile.h"
#include <iostream>

VkeMaterial::VkeMaterial()
    : VkeBuffer()
    , m_id(0)
{
  initMaterialData();
}

VkeMaterial::VkeMaterial(const ID& inID)
    : VkeBuffer()
    , m_id(inID)
{
  initMaterialDataSubAlloc();
}

void VkeMaterial::initMaterialData()
{
  m_usage_flags  = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  m_memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

  initBackingStore(sizeof(VkeMaterialUniform));
  initVKBufferData();
}

void VkeMaterial::initMaterialDataSubAlloc()
{
  initBackingStore(sizeof(VkeMaterialUniform));
}

void VkeMaterial::updateVKBufferData(VkeMaterialUniform* inData)
{
  uint8_t* ptr = (uint8_t*)inData + (sizeof(VkeMaterialUniform) * m_id);
  memcpy(ptr, (void*)&m_backing_store->reflectivity, sizeof(VkeMaterialUniform));
}

void VkeMaterial::bind(VkCommandBuffer* inBuffer) {}

void VkeMaterial::initFromData(VKSFile* inFile, VKSMaterialRecord* inMaterial)
{
  m_backing_store->reflectivity = inMaterial->reflectivity;
  m_backing_store->opacity      = inMaterial->opacity;
  m_backing_store->shininess    = inMaterial->shininess;

  //updateVKBufferData();

  uint32_t texCount = inMaterial->textureCount;

  for(uint32_t i = 0; i < texCount; ++i)
  {
    VKSTextureRecord& tex = inFile->textures.data()[i + inMaterial->firstTexture];
    if(tex.type == meshimport::DIFFUSE)
    {

      std::string fPath      = std::string(tex.filePaths[0]);
      size_t      stringSize = fPath.size();
      std::string newPath    = fPath.substr(0, stringSize - 3) + "dds";
      m_textures.newTexture()->loadDDSTextureFile(newPath.c_str());
    }
  }
}

void VkeMaterial::initFromData(meshimport::MaterialDataf* inData)
{

  m_backing_store->reflectivity = inData->reflectivity;
  m_backing_store->opacity      = inData->opacity;
  m_backing_store->shininess    = inData->shininess;

  uint32_t texCount = inData->textureCount;

  for(uint32_t i = 0; i < texCount; ++i)
  {
    meshimport::TextureDataf texData = inData->textures[i];

    /*
			Just deal with diffuse textures for now.
		*/
    if(texData.type == meshimport::DIFFUSE)
    {

      std::string fPath      = std::string(texData.filePaths[0]);
      size_t      stringSize = fPath.size();
      std::string newPath    = fPath.substr(0, stringSize - 3) + "dds";
      m_textures.newTexture()->loadDDSTextureFile(newPath.c_str());
    }
  }
}

void VkeMaterial::initWithDefaults()
{
  m_backing_store->opacity   = 1.0;
  m_backing_store->shininess = 1.0;
}

VkeMaterial::~VkeMaterial() {}

VkeMaterial::List::List()
    : m_default(NULL)
{
}
VkeMaterial::List::~List() {}

VkeMaterial::ID VkeMaterial::List::nextID()
{
  VkeMaterial::ID outID;
  if(m_deleted_keys.size() == 0)
    return m_data.size();
  outID = m_deleted_keys.back();
  m_deleted_keys.pop_back();
  return outID;
}

VkeMaterial::Count VkeMaterial::List::count()
{
  return m_data.size();
}

VkeMaterial* VkeMaterial::List::newMaterial()
{
  VkeMaterial::ID id = nextID();
  return newMaterial(id);
}

VkeMaterial* VkeMaterial::List::newMaterial(const VkeMaterial::ID& inID)
{
  VkeMaterial* outMaterial = new VkeMaterial(inID);
  m_data[inID]             = outMaterial;
  return outMaterial;
}


void VkeMaterial::List::addMaterial(VkeMaterial* const inMaterial)
{
  VkeMaterial::ID id = nextID();
  m_data[id]         = inMaterial;
}

VkeMaterial* VkeMaterial::List::getMaterial(const VkeMaterial::ID& inID)
{
  return m_data[inID];
}

size_t VkeMaterial::List::getTextureCount()
{
  size_t                     outCount = 0;
  VkeMaterial::Map::iterator itr;

  for(itr = m_data.begin(); itr != m_data.end(); ++itr)
  {
    outCount += itr->second->getTextures().count();
  }

  return outCount;
}

void VkeMaterial::List::getTextureData(VkeTexture::Data* outData)
{
  if(!outData)
    return;

  VkeMaterial::Map::iterator itr;

  /*
		For each material..
	*/
  size_t offset = 0;
  for(itr = m_data.begin(); itr != m_data.end(); ++itr)
  {
    itr->second->getTextures().getData(outData, offset);
    offset += itr->second->getTextures().count();
  }
}
