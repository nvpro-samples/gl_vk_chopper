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

#ifndef __H_VKS_FILE_
#define __H_VKS_FILE_


#include "MeshUtils.h"
#include <vector>

/**************************************/
/*structures***************************/
/**************************************/

struct VKSFileHeader
{
  uint32_t meshCount;
  uint32_t materialCount;
  uint32_t nodeCount;
  uint32_t animationCount;
  uint32_t textureCount;
};

struct VKSNodeRecord
{
  uint32_t      childCount;  //4
  uint32_t      index;       //8
  nvmath::vec3f position;    //20
  nvmath::quatf rotation;    //36
  nvmath::vec3f scale;       //48
  char          name[32];    //70

  uint8_t meshCount;       //71
  uint8_t meshIndices[8];  //79
};

struct VKSTextureRecord
{
  uint8_t type;
  uint8_t count;
  char    filePaths[8][128];
};

struct VKSMeshRecord
{
  uint32_t vertexCount;
  uint32_t indexCount;
  uint32_t firstVertex;
  uint32_t firstIndex;
  uint32_t materialID;
};

struct VKSMaterialRecord
{
  nvmath::vec4f diffuseColor;
  nvmath::vec4f ambientColor;
  nvmath::vec4f specularColor;
  float         opacity;
  float         shininess;
  float         reflectivity;
  uint32_t      textureCount;
  uint32_t      firstTexture;
};

struct VKSAnimationRecord
{
  uint32_t firstNode;
  uint32_t nodecount;
};

struct VKSAnimationNodeRecord
{
  uint32_t firstPosition;
  uint32_t firstRotation;
  uint32_t firstScale;

  uint32_t positionCount;
  uint32_t rotationCount;
  uint32_t scaleCount;
  char     name[32];
};

struct VKSAnimationKeyRecord
{
  nvmath::vec4f key{0.f, 0.f, 0.f, 0.f};
  double        time = 0.;
};


struct VKSFile
{
  std::string                     inputFile;
  std::string                     outputFile;
  int                             fileHandle = 0;
  VKSFileHeader                   header{};
  std::vector<VKSNodeRecord>      nodes;
  std::vector<VKSMeshRecord>      meshes;
  std::vector<VKSMaterialRecord>  materials;
  std::vector<VKSAnimationRecord> animations;
  std::vector<VKSTextureRecord>   textures;
  std::vector<float>              vertices;
  std::vector<uint32_t>           indices;
  uint32_t                        indexCount  = 0;
  uint32_t                        vertexCount = 0;

  std::vector<VKSAnimationNodeRecord> animationNodes;
  std::vector<VKSAnimationKeyRecord>  animationKeys;

  uint32_t animationNodeCount = 0;
  uint32_t animationKeyCount  = 0;
};


void readVKSFile(VKSFile* inFile);


#endif