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

#include "Camera.h"


Camera::Camera():
Node(),
m_fov(45.0f),
m_near(0.01f),
m_far(100.0f)
{
	initProjection();
}

Camera::Camera(Node *inParent, const ID &inID,const float &inFOV, const float &inNear, const float &inFar) :
Node(inParent, inID),
m_fov(45.0f),
m_near(0.01f),
m_far(100.0f){

	initProjection();

}

void Camera::initProjection(const float &inFOV, const float &inNear, const float &inFar){

	m_fov = inFOV;
	m_near = inNear;
	m_far = inFar;

	m_viewport.m_data[2] = 800.0f;
	m_viewport.m_data[3] = 800.0f;

	m_projection_dirty = true;


}

void Camera::updateProjection(){

	if (!m_projection_dirty) return;


	float vw = m_viewport.m_data[2] - m_viewport.m_data[0];
	float vh = m_viewport.m_data[3] - m_viewport.m_data[1];

	float aspect = vw / vh;

	m_projection = nvmath::perspective(m_fov, aspect, m_near, m_far);

	m_projection_dirty = false;

}

Camera::~Camera()
{
}
