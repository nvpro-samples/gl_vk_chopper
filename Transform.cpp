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

#include "Transform.h"


Transform::Transform()
{
	reset();
}


void Transform::reset(){

	m_matrix.identity();

}

void Transform::update(Transform *inParent){

	m_transform = m_matrix;

	if (inParent){
		m_transform = (inParent->getTransform()) * m_transform;
	}

	m_inverse = m_transform;
	
	m_inverse = nv_math::invert(m_inverse);
	m_inverse = nv_math::transpose(m_inverse);
	
	m_inverse.a30 = 0.0;
	m_inverse.a31 = 0.0;
	m_inverse.a32 = 0.0;
}

void Transform::translate(nv_math::vec4f &inPosition){
	nv_math::vec3f vec = nv_math::vec3f(inPosition);
	m_matrix.translate(vec);
}

void Transform::translate(float inX, float inY, float inZ){
	nv_math::vec3f vec = nv_math::vec3f(inX,inY,inZ);
	m_matrix.translate(vec);
}

void Transform::rotate(const float inValue,  nv_math::vec3f &inBasis){


	m_matrix.rotate(inValue, inBasis);
}

void Transform::rotate(nv_math::quatf &inQuat){

	m_matrix.rotate(inQuat);
}

void Transform::scale(const nv_math::vec3f &inScale){
	m_matrix.scale(inScale);
}   

void Transform::scale(const float inX, const float inY, const float inZ){
	scale(nv_math::vec3f(inX, inY, inZ));
}



Transform::~Transform()
{
}
