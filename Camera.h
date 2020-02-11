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

#ifndef __H_CAMERA_
#define __H_CAMERA_

#pragma once

#include "Node.h"

class Camera :
	public Node
{
public:
	Camera();
	Camera(Node *inParent, const ID &inID, const float &inFOV = 45.0f, const float &inNear = 0.01f, const float &inFar =  100.0f);
	~Camera();


	nvmath::mat4f &getProjection(){ return m_projection; }
  nvmath::mat4f getViewProjection() {
		nvmath::mat4f out;
		out.identity();
		out = m_projection * out;
		out = m_transform(out);
		return out;
	}

	inline float getFOV(){ 
		return  m_fov; 
	};
	inline void setFOV(const float &inFOV){
		m_fov = inFOV;
		m_projection_dirty = true;
	}

	inline float getNear(){
		return m_near;
	}

	inline void setNear(const float &inNear){
		m_near = inNear;
		m_projection_dirty = true;
	}

	inline float getFar(){
		return m_far;
	}

	inline void setFar(const float &inFar){
		m_far = inFar;
		m_projection_dirty = true;
	}

	void initProjection(const float &inFOV = 45.0f, const float &inNear = 0.01, const float &inFar = 100.0);
	void updateProjection();

	void setViewport(const float inMinX, const float inMinY, const float inMaxX, const float inMaxY){
		m_viewport.m_data[0] = inMinX;
		m_viewport.m_data[1] = inMinY;
		m_viewport.m_data[2] = inMaxX;
		m_viewport.m_data[3] = inMaxY;
	}

protected:

	nvmath::mat4f m_projection;

	float m_fov;
	float m_near;
	float m_far;

	Vec4f m_viewport;

	bool m_projection_dirty;


};

#endif
