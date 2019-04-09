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

#ifndef __H_TRANSFORM_
#define __H_TRANSFORM_

#pragma once

#include<nvmath/nvmath.h>

class Transform
{
public:
	Transform();
	~Transform();

	void reset();
	void update(Transform *inParent = NULL);
	void translate(nvmath::vec4f &inPosition);
	void translate(float inX, float inY, float inZ);

	void rotate(const float inValue, nvmath::vec3f& inBasis);
  void rotate(nvmath::quatf& inQuaterion);

	void scale(const nvmath::vec3f& inScale);
	void scale(const float inX, const float inY, const float inZ);

	nvmath::vec4f operator()(nvmath::vec4f& rhs)
  {
    nvmath::vec4f out = m_transform * rhs;
		return out;
	}

	nvmath::mat4f operator()(nvmath::mat4f& rhs)
  {
    nvmath::mat4f out = rhs * m_transform;
		return out;
	}

	nvmath::vec4f operator[](nvmath::vec4f& rhs)
  {
    nvmath::vec4f out = m_inverse * rhs;
		return out;
	}

	nvmath::mat4f&  getTransform() { return m_transform; }
  nvmath::mat4f& getInverse() { return m_inverse; }

	void setMatrix(nvmath::mat4f& inMatrix) { m_matrix = inMatrix; }

private:

	nvmath::mat4f  m_transform;
  nvmath::mat4f  m_inverse;
  nvmath::mat4f  m_matrix;

};

#endif
