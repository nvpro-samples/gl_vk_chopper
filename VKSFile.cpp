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

#include "VKSFile.h"
#include "nvh/nvprint.hpp"
#include "nvpwindow.hpp"
#include <iostream>

#if defined(WIN32)
#else

void fopen_s(FILE **inFile,const char *inPath, const char *inPermissions){
    *inFile = fopen(inPath,inPermissions);
}

void fread_s(void* inBuffer, size_t inBufSize, size_t inSize, size_t inCount, FILE *inFP){
    fread(inBuffer,inSize,inCount,inFP);
}

#endif

void readVKSFile(VKSFile *inFile){

	FILE *fp;

	std::vector<std::string> searchPaths;
    searchPaths.push_back(std::string("."));
    searchPaths.push_back(std::string("./resources_" PROJECT_NAME));
    searchPaths.push_back(std::string(PROJECT_NAME));
    searchPaths.push_back(NVPSystem::exePath() + std::string(PROJECT_RELDIRECTORY));

    std::string filePath;
	for (uint32_t i = 0; i < searchPaths.size(); ++i){
		filePath = searchPaths[i] + "/" + inFile->outputFile;

		fopen_s(&fp, filePath.c_str(), "rb");
		if (fp) break;
	}

	if (!fp){
		LOGE("Could not load vks file %s\n", filePath.c_str());
		exit(1);
    }
    else {
        LOGOK("Loaded model %s\n", filePath.c_str())
    }

	fread_s((void *)&inFile->header, sizeof(VKSFileHeader), sizeof(VKSFileHeader), 1, fp);


	VKSNodeRecord *nodes = (VKSNodeRecord*)malloc(sizeof(VKSNodeRecord) * inFile->header.nodeCount);

	fread_s(nodes, sizeof(VKSNodeRecord)*inFile->header.nodeCount, sizeof(VKSNodeRecord), inFile->header.nodeCount, fp);

	inFile->nodes.reserve(inFile->header.nodeCount);
	inFile->nodes.assign(nodes, nodes + inFile->header.nodeCount);

	VKSMeshRecord *meshes = (VKSMeshRecord *)malloc(sizeof(VKSMeshRecord) * inFile->header.meshCount);

	fread_s(meshes, sizeof(VKSMeshRecord)*inFile->header.meshCount, sizeof(VKSMeshRecord), inFile->header.meshCount, fp);
	inFile->meshes.reserve(inFile->header.meshCount);
	inFile->meshes.assign(meshes,meshes+inFile->header.meshCount);

	VKSMaterialRecord *materials = (VKSMaterialRecord*)malloc(sizeof(VKSMaterialRecord) * inFile->header.materialCount);

	fread_s(materials, sizeof(VKSMaterialRecord)*inFile->header.materialCount, sizeof(VKSMaterialRecord), inFile->header.materialCount, fp);

	inFile->materials.reserve(inFile->header.materialCount);
	inFile->materials.assign(materials, materials + inFile->header.materialCount);

	VKSAnimationRecord *animations = (VKSAnimationRecord*)malloc(sizeof(VKSAnimationRecord) * inFile->header.animationCount);
	fread_s(animations, sizeof(VKSAnimationRecord) * inFile->header.animationCount, sizeof(VKSAnimationRecord), inFile->header.animationCount, fp);

	inFile->animations.reserve(inFile->header.animationCount);
	inFile->animations.assign(animations, animations + inFile->header.animationCount);

	uint32_t vertexElementCount;
	uint32_t indexCount;

	fread_s(&vertexElementCount, sizeof(uint32_t), sizeof(uint32_t), 1, fp);

	uint32_t vtxSize = 8;

	inFile->vertexCount = vertexElementCount / vtxSize;
	

	if (vertexElementCount > 0){
		float *vertexElements = (float*)malloc(sizeof(float)*vertexElementCount);
		fread_s(vertexElements, sizeof(float)*vertexElementCount, sizeof(float), vertexElementCount, fp);
		inFile->vertices.reserve(vertexElementCount);
		inFile->vertices.assign(vertexElements, vertexElements+vertexElementCount);
	}

	fread_s(&indexCount, sizeof(uint32_t), sizeof(uint32_t), 1, fp);

	inFile->indexCount = indexCount;

	if (indexCount > 0){
		uint32_t *indices = (uint32_t*)malloc(sizeof(uint32_t) * indexCount);
		fread_s(indices, sizeof(uint32_t) * indexCount, sizeof(uint32_t), indexCount, fp);

		inFile->indices.reserve(indexCount);
		inFile->indices.assign(indices, indices + indexCount);
	}

	uint32_t textureCount = inFile->header.textureCount;

	if (textureCount > 0){
		VKSTextureRecord *textures = (VKSTextureRecord*)malloc(sizeof(VKSTextureRecord) * textureCount);
		fread_s(textures, sizeof(VKSTextureRecord) * textureCount, sizeof(VKSTextureRecord), textureCount, fp);

		inFile->textures.reserve(textureCount);
		inFile->textures.assign(textures, textures + textureCount);
	}

	fread_s(&inFile->animationNodeCount, sizeof(uint32_t), sizeof(uint32_t), 1, fp);

	VKSAnimationNodeRecord *animNodes = (VKSAnimationNodeRecord*)malloc(sizeof(VKSAnimationNodeRecord) * inFile->animationNodeCount);
	fread_s(animNodes, sizeof(VKSAnimationNodeRecord) * inFile->animationNodeCount, sizeof(VKSAnimationNodeRecord), inFile->animationNodeCount, fp);

	inFile->animationNodes.reserve(inFile->animationNodeCount);
	inFile->animationNodes.assign(animNodes, animNodes + inFile->animationNodeCount);

	fread_s(&inFile->animationKeyCount, sizeof(uint32_t), sizeof(uint32_t), 1, fp);
	
	VKSAnimationKeyRecord *animKeys = (VKSAnimationKeyRecord*)malloc(sizeof(VKSAnimationKeyRecord) * inFile->animationKeyCount);
	fread_s(animKeys, sizeof(VKSAnimationKeyRecord) * inFile->animationKeyCount, sizeof(VKSAnimationKeyRecord), inFile->animationKeyCount, fp);

	inFile->animationKeys.reserve(inFile->animationKeyCount);
	inFile->animationKeys.assign(animKeys, animKeys + inFile->animationKeyCount);


	fclose(fp);

}
