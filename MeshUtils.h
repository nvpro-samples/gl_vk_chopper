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

#ifndef __H_MESHUTILS_
#define __H_MESHUTILS_

#include<string>
#include"WMath.h"
#include<nv_math/nv_math.h>


namespace meshimport{

	template<typename T>
	struct MeshData{
		T *vertices;
		T *normals;
		T *uvs;
		uint32_t *indices;
		uint32_t vertex_count;
		uint32_t index_count;
		bool has_normals;
		bool has_uvs;

		int32_t materialID;

	};

	typedef MeshData<float> MeshDataf;

	enum TextureType{
		DIFFUSE = 0,
		AMBIENT,
		SPECULAR,
		OPACITY,
		NORMAL
	};

	struct TextureDataf{
		TextureType type;
		uint32_t count;
		char filePaths[8][128];
	};

	struct MaterialDataf{
		nv_math::vec4f diffuseColor;
		nv_math::vec4f ambientColor;
		nv_math::vec4f specularColor;
		float opacity;
		float shininess;
		float reflectivity;
		uint32_t textureCount;
		TextureDataf *textures;

	};

	struct OutputMaterialDataf{
		MaterialDataf *materials;
		uint32_t materialCount;
	};

	struct NodeAnimationKeyf{
		nv_math::vec4f data;
		double time;
	};

	struct NodeAnimationDataf{
		NodeAnimationKeyf *position;
		NodeAnimationKeyf *rotation;
		NodeAnimationKeyf *scale;

		uint32_t position_count;
		uint32_t rotation_count;
		uint32_t scale_count;

		std::string *name;
	};

	struct AnimationDataf{
		NodeAnimationDataf *nodes;
		uint32_t node_count;
	};

	struct OutputAnimationDataf{
		AnimationDataf *animations;
		uint32_t animation_count;
	};

	struct OutputDataf{
		MeshDataf *meshes;
		uint32_t meshCount;

	};

	struct OutputMeshIndexDataf{
		uint32_t *meshes;
		uint32_t meshCount;
	};

	struct OutputNodeDataf{
		OutputMeshIndexDataf meshData;
		nv_math::vec3f position;
		nv_math::vec3f scale;
		nv_math::quatf rotation;

		std::string name;

		OutputNodeDataf *children;
		uint32_t childCount;
	};


	struct OutputSceneDataf{
		OutputNodeDataf *rootNode;
		OutputDataf meshData;
		OutputMaterialDataf materialData;
		OutputAnimationDataf animationData;
	};



}

#endif
