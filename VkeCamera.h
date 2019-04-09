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

#ifndef __H_VKE_CAMERA_
#define __H_VKE_CAMERA_

#pragma once

#include<nvmath/nvmath.h>
#include"VkeBuffer.h"
#include"Transform.h"
#include<map>


#ifndef VKE_DEFAULT_CAMERA_VIEWPORT
#define VKE_DEFAULT_CAMERA_VIEWPORT 0,0,1024,768
#endif

#ifndef VKE_DEFAULT_CAMERA_NEAR_PLANE
#define VKE_DEFAULT_CAMERA_NEAR_PLANE 0.001
#endif

#ifndef VKE_DEFAULT_CAMERA_FAR_PLANE
#define VKE_DEFAULT_CAMERA_FAR_PLANE 600.0
#endif

#ifndef VKE_DEFAULT_CAMERA_FOV
#define VKE_DEFAULT_CAMERA_FOV 45.0
#endif

typedef struct _VkeCameraUniform{
	nvmath::mat4f view_proj_matrix;
	nvmath::mat4f view_matrix;
	nvmath::vec4f camera_position;
}VkeCameraUniform;

class VkeCamera : public VkeBuffer<VkeCameraUniform>
{
public:

	typedef uint32_t ID;
	typedef std::map<VkeCamera::ID, VkeCamera*> Map;
	typedef uint32_t Count;

	class List{
	public:
		List();
		~List();

		VkeCamera *newCamera();
		VkeCamera *newCamera(const VkeCamera::ID &inID);
		void addCamera(VkeCamera * const inData);
		VkeCamera *getCamera(const ID &inID);

		void update();

		ID nextID();
		Count count();

		void getDescriptors(VkDescriptorBufferInfo *outDescriptor);

	

	private:
		VkeCamera::Map m_data;
		std::vector<VkeCamera::ID> m_deleted_keys;
	};

	VkeCamera();
	VkeCamera(const ID &inID);
	VkeCamera(const ID &inID, const float inX, const float inY, const float inZ);
	~VkeCamera();

	void initCameraData();
	void updateCameraCmd(VkCommandBuffer inCommand);
	void update();

	void bind(VkCommandBuffer *inBuffer);

	void setViewport(float inX, float inY, float inW, float inH);

	void setPosition(float inX, float inY, float inZ);
	void setRotation(float inX, float inY, float inZ);
	void setRotation(nvmath::quatf &inQuat);

	void setNear(float inNear);
	void setFar(float inFar);
	void setFOV(float inFOV);

	float getNear();
	float getFar();
	float getFOV();

	nvmath::vec4f worldPosition();
	nvmath::vec4f worldPosition(nvmath::vec4f &inPosition);

	void lookAt(nvmath::vec4f &inPosition);
	void setLookAtMatrix(nvmath::mat4f &inMat);

private:

	void updateProjection();
	void updateTransform();
	void updateViewProjection();

	ID m_id;

	nvmath::vec4f m_viewport;

	float m_near;
	float m_far;
	float m_fov;
	float m_aspect;

	nvmath::vec3f m_position;
	nvmath::vec3f m_rotation;

	Transform m_transform;

	nvmath::mat4f m_projection;
	bool m_projection_needs_update;
	bool m_transform_needs_update;

	bool m_view_projection_needs_update;

	nvmath::mat4f m_look_at_matrix;
	bool m_use_look_at;

	float m_time;
};

#endif
