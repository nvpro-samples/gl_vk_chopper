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

#include "WMath.h"

Mat4x4f identity4x4(){
	Mat4x4f out = {
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	};

	return out;
}

Mat4x4f rotate4x4(const Quaternion<float> &q){

	Mat4x4f out = identity4x4();

	out.m_data[0] = 1.0 - (2.0*q.j*q.j + 2.0*q.k*q.k);
	out.m_data[1] = 2.0*(q.i*q.j + 2.0*q.k*q.r);
	out.m_data[2] = 2.0*(q.i*q.k - 2.0*q.j*q.r);
	out.m_data[4] = 2.0*(q.i*q.j - 2.0*q.k*q.r);
	out.m_data[5] = 1.0 - (2.0*q.i*q.i + 2.0*q.k*q.k);
	out.m_data[6] = 2.0*(q.j*q.k + 2.0*q.i*q.r);
	out.m_data[8] = 2.0*(q.i*q.k + 2.0*q.j*q.r);
	out.m_data[9] = 2.0*(q.j*q.k - 2.0*q.i*q.r);
	out.m_data[10] = 1.0 - (2.0*q.i*q.i + 2.0*q.j*q.j);

	return out;
}

Mat4x4f translate4x4(const Vec4f &inPosition){

	Mat4x4f out = {
		1.0, 0.0, 0.0, inPosition.m_data[0],
		0.0, 1.0, 0.0, inPosition.m_data[1],
		0.0, 0.0, 1.0, inPosition.m_data[2],
		0.0, 0.0, 0.0, 1.0
	};

	return out;

}

Mat4x4f translate4x4(float inX, float inY, float inZ){

	Mat4x4f out = {
		1.0, 0.0, 0.0, inX,
		0.0, 1.0, 0.0, inY,
		0.0, 0.0, 1.0, inZ,
		0.0, 0.0, 0.0, 1.0
	};

	return out;

}

Mat4x4f scale4x4(const Vec4f &inScale){

	Mat4x4f out = {
		inScale.m_data[0], 0.0, 0.0, 0.0,
		0.0, inScale.m_data[1], 0.0, 0.0,
		0.0, 0.0, inScale.m_data[2], 0.0,
		0.0, 0.0, 0.0, 1.0 
	};

	return out;
}

Mat4x4f scale4x4(const float inX, const float inY, const float inZ){
	Mat4x4f out = {
		inX,0.0,0.0,0.0,
		0.0,inY,0.0,0.0,
		0.0,0.0,inZ,0.0,
		0.0,0.0,0.0,1.0
	};

	return out;
}

Mat4x4f perspectiveProjection4x4f(const float inFOV, const float inAspect, const float inNear, const float inFar){
	
	float yScl = 1.0f / tan(inFOV*0.5f);
	float xScl = yScl / inAspect;

	Mat4x4f out = {
	xScl,0.0,0.0,0.0,
	0.0,yScl,0.0,0.0,
	0.0, 0.0, inFar / (inNear - inFar), (inNear * inFar) / (inNear-inFar),
	0.0,0.0,-1.0,0.0
	};

	return out;

}

namespace angle{

#define  _PI 3.141592653589793238462643383279502884f

	float d2rf(float ind){
		return  ind / (180.0f / _PI);
	}

	float r2df(float inr){
		return inr * (180.0f / _PI);
	}

}

namespace basis{
    Vec4f UP(){
		Vec4f out = {
			0.0,1.0,0.0,1.0
		};
		return out;
	}

    Vec4f LEFT(){
		Vec4f out = {
			1.0,0.0,0.0,1.0
		};
		return out;
	}

    Vec4f FWD(){
		Vec4f out = {
			0.0,0.0,1.0,0.0
		};
		return out;
	}
}
