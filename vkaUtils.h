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

#ifndef __H_VKA_UTILS_
#define __H_VKA_UTILS_

#include <vulkan/vulkan.h>
#include <stdio.h>
#include <nv_math/nv_math.h>
#include "nv_helpers/nvprint.hpp"

#define VKA_CHECK_ERROR(func,msg)\
{VkResult rslt = func;\
    if(rslt != VK_SUCCESS){\
    LOGE("ERROR \t [%d] : %s \n",rslt,msg);\
    }}





void dumpGlobalLayerNames(VkLayerProperties *pros, uint32_t count);

#ifdef USE_LIB_PNG
bool loadTexture(const char *filename, uint8_t **rgba_data,
	uint32_t inRowpitch,
	int32_t *width, int32_t *height, bool doAlloc = false);
#endif

bool loadTextFile(const char *filename, char **buffer, size_t &outSize);

struct Projection {
	float nearplane;
	float farplane;
	float fov;
	nv_math::mat4f  matrix;

	nv_math::mat4f proj;
	nv_math::vec3f position;
	nv_math::vec3f target;

	Projection()
		: nearplane(0.1f)
		, farplane(400.0f)
		, fov((40.0f))
		, position(0.0,0.0,-15.0)
		, target(0.0,0.0,0.0)
	{

	}

	void update(int width, int height){

		nv_math::mat4f camView;
		camView.identity();

		camView = nv_math::look_at(target - position, target, nv_math::vec3f(0, 1, 0));

		proj = nv_math::perspective(fov, float(width) / float(height), nearplane, farplane);

		matrix = proj * camView;
	}
};

struct ShadowProjection {
	float nearplane;
	float farplane;
	float fov;
	nv_math::mat4f  matrix;

	ShadowProjection()
		: nearplane(0.1f)
		, farplane(400.0f)
		, fov((90.0f))
	{

	}

	void update(int width, int height){

		nv_math::mat4f camView;
		camView.identity();
		nv_math::vec3f viewDir = nv_math::vec3f(5*1.4, 20.5*1.4, 20*1.4);

		camView = nv_math::look_at(viewDir, nv_math::vec3f(0.0, 0.0, 0.0), nv_math::vec3f(0, 1, 0));

		matrix = nv_math::perspective(fov, float(width) / float(height), nearplane, farplane);
		matrix = matrix * camView;
	}
};


typedef struct _DepthImageView{
	VkFormat format;
	VkImage image;
	VkDeviceMemory memory;
    VkImageView view;
    /*VkAttachmentView view;*/
} DepthImageView;

typedef struct _ColorImageView{
	VkFormat format;
	VkImage image;
	VkDeviceMemory memory;
    VkImageView view;
    /*VkAttachmentView view;*/
} ColorImageView;

typedef struct _UBOView{
	VkBuffer buffer;
	VkDeviceMemory memory;
	VkBufferView view;
	VkDescriptorBufferInfo descriptor;
};

typedef struct _TextureObject{
	VkSampler sampler;
	VkImage image;

	VkImageLayout imageLayout;
	VkDeviceMemory memory;
	VkImageView view;

	int32_t width;
	int32_t height;

}TextureObject;

typedef struct _vctr4{
	float x;
	float y;
	float z;
	float w;
}Vctr4;

typedef struct _vctr2{
	float x;
	float y;
}Vctr2;

typedef struct _VertexObject{
	Vctr4 pos;
	Vctr4 nml;
	//Vctr2 uv;
} VertexObject;

typedef struct _VertexObjectUV{
	Vctr4 pos;
	Vctr2 nml;
}VertexObjectUV;

typedef struct _BufferData{
	VkBuffer buf;
	VkDeviceMemory memory;
	VkBufferView view;
	VkDescriptorBufferInfo descriptor;
} BufferData;

typedef struct _UBOData{
	nv_math::mat4f view_matrix;
	nv_math::mat4f nml_matrix;
} UBOData;

typedef struct _UBOCamera{
	nv_math::mat4f proj_matrix;
	nv_math::mat4f inv_proj_matrix;
	nv_math::vec4f camera_position;
} UBOCamera;

typedef struct _UBOObject{
	BufferData ubo;
}UBOObject;

typedef struct _MeshObject{
	BufferData vbo;
	BufferData ibo;
	uint32_t indexCount;
	uint32_t vertexCount;
} MeshObject;

#endif

#ifndef ICD_SPV_H
#define ICD_SPV_H

#include <stdint.h>

#define ICD_SPV_MAGIC   0x07230203
#define ICD_SPV_VERSION 99

struct icd_spv_header {
	uint32_t magic;
	uint32_t version;
	uint32_t gen_magic;  // Generator's magic number
};

#endif /* ICD_SPV_H */
