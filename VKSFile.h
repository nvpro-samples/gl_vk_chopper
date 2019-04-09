/*-----------------------------------------------------------------------
Copyright (c) 2014-2016, NVIDIA. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Neither the name of its contributors may be used to endorse
or promote products derived from this software without specific
prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------*/
/* Contact chebert@nvidia.com (Chris Hebert) for feedback */

#ifndef __H_VKS_FILE_
#define __H_VKS_FILE_


#include<vector>
#include"MeshUtils.h"

/**************************************/
/*structures***************************/
/**************************************/

struct VKSFileHeader{
	uint32_t meshCount;
	uint32_t materialCount;
	uint32_t nodeCount;
	uint32_t animationCount;
	uint32_t textureCount;
};

struct VKSNodeRecord{
	uint32_t childCount;//4
	uint32_t index;//8
	nvmath::vec3f position;//20
	nvmath::quatf rotation;//36
	nvmath::vec3f scale;//48
	char name[32];//70

	uint8_t meshCount;//71
	uint8_t meshIndices[8];//79
};

struct VKSTextureRecord{
	uint8_t type;
	uint8_t count;
	char filePaths[8][128];
};

struct VKSMeshRecord{
	uint32_t vertexCount;
	uint32_t indexCount;
	uint32_t firstVertex;
	uint32_t firstIndex;
	uint32_t materialID;
};

struct VKSMaterialRecord{
	nvmath::vec4f diffuseColor;
	nvmath::vec4f ambientColor;
	nvmath::vec4f specularColor;
	float opacity;
	float shininess;
	float reflectivity;
	uint32_t textureCount;
	uint32_t firstTexture;
};

struct VKSAnimationRecord{
	uint32_t firstNode;
	uint32_t nodecount;
};

struct VKSAnimationNodeRecord{
	uint32_t firstPosition;
	uint32_t firstRotation;
	uint32_t firstScale;

	uint32_t positionCount;
	uint32_t rotationCount;
	uint32_t scaleCount;
	char name[32];
};

struct VKSAnimationKeyRecord{
	nvmath::vec4f key;
	double time;
};


struct VKSFile{
	std::string inputFile;
	std::string outputFile;
	int fileHandle;
	VKSFileHeader header;
	std::vector<VKSNodeRecord> nodes;
	std::vector<VKSMeshRecord> meshes;
	std::vector<VKSMaterialRecord> materials;
	std::vector<VKSAnimationRecord> animations;
	std::vector<VKSTextureRecord> textures;
	std::vector<float> vertices;
	std::vector<uint32_t> indices;
	uint32_t indexCount;
	uint32_t vertexCount;

	std::vector<VKSAnimationNodeRecord> animationNodes;
	std::vector<VKSAnimationKeyRecord> animationKeys;

	uint32_t animationNodeCount;
	uint32_t animationKeyCount;

};


void readVKSFile(VKSFile *inFile);



#endif