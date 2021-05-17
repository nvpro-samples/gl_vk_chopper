/*
 * Copyright (c) 2014-2021, NVIDIA CORPORATION.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-FileCopyrightText: Copyright (c) 2014-2021 NVIDIA CORPORATION
 * SPDX-License-Identifier: Apache-2.0
 */

/* Contact chebert@nvidia.com (Chris Hebert) for feedback */

#include "WMath.h"

// clang-format off
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
// clang-format on

namespace angle {

#define _PI 3.141592653589793238462643383279502884f

float d2rf(float ind)
{
  return ind / (180.0f / _PI);
}

float r2df(float inr)
{
  return inr * (180.0f / _PI);
}

}  // namespace angle

namespace basis {
Vec4f UP()
{
  Vec4f out = {0.0, 1.0, 0.0, 1.0};
  return out;
}

Vec4f LEFT()
{
  Vec4f out = {1.0, 0.0, 0.0, 1.0};
  return out;
}

Vec4f FWD()
{
  Vec4f out = {0.0, 0.0, 1.0, 0.0};
  return out;
}
}  // namespace basis
