/*
 * Copyright (c) 2014-2024, NVIDIA CORPORATION.  All rights reserved.
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
 * SPDX-FileCopyrightText: Copyright (c) 2014-2024 NVIDIA CORPORATION
 * SPDX-License-Identifier: Apache-2.0
 */


#version 450 core

layout(quads) in;

struct CameraData{
	mat4 proj_view_matrix;
	mat4 inverse_proj_view_matrix;
	vec4 camera_position;
};


layout(std140, binding = 1) uniform cameraBuffer{
	CameraData camera;
};

layout(binding = 2) uniform sampler2D lookup;

float cubicLerp(float in0, float in1, float inT){
	float c = (3.0 - 2.0 * inT) * inT * inT;
	return (1.0 - c) * in0 + in1 * c;
}

ivec2 offsetUV(ivec2 inUV, float inOffset, int inSize){

	int lVal = (inSize*inUV.y + inUV.x) + int(inOffset);

	ivec2 outV;

	outV.x = lVal % inSize;
	outV.y = lVal / inSize;

	return outV;
}

vec2 offsetUVf(vec2 inUV, float inOffset){

	ivec2 iUV = ivec2(inUV*vec2(1000.0));
	iUV = offsetUV(iUV, inOffset, 1000);

	vec2 outUV = vec2(iUV)*(1.0 / 1000.0);

	return outUV;
}

vec2 rotV2(vec2 inVec, float inAngle){
	vec2 outV;
	float c = cos(inAngle);
	float s = sin(inAngle);
	outV.x = inVec.x * c - inVec.y *s;
	outV.y = inVec.x * s + inVec.y * c;

	return outV;
}

float perlin(vec2 inUV){

	float scl = 1.5;
	float decay = 0.5;
	int iterations = 8;
	float value = 0.0;

	ivec2 tsize = textureSize(lookup, 0);
	if (inUV.x < 0.0 || inUV.x > 1.0) return 1.0;
	if (inUV.y < 0.0 || inUV.y > 1.0) return 1.0;

	for (int i = 0; i < iterations; ++i){

		vec2 fUV = vec2(tsize*scl) * inUV;

		ivec2 iXY0;
		ivec2 iXY1;
		iXY0.x = (fUV.x <= 0.0) ? int(fUV.x - 1.0) : int(fUV.x);
		iXY1.x = iXY0.x + 1;
		iXY0.y = (fUV.y <= 0.0) ? int(fUV.y - 1.0) : int(fUV.y);
		iXY1.y = iXY0.y + 1;

		float sx = fUV.x - float(iXY0.x);
		float sy = fUV.y - float(iXY0.y);

		ivec2 uv00 = iXY0;
		ivec2 uv10 = ivec2(iXY1.x, iXY0.y);
		ivec2 uv01 = ivec2(iXY0.x, iXY1.y);
		ivec2 uv11 = iXY1;

		float timeValue = camera.camera_position.w*scl;

		vec2 v00 = rotV2(texelFetch(lookup, uv00%tsize, 0).xy, timeValue);
		vec2 v10 = rotV2(texelFetch(lookup, uv10%tsize, 0).xy, timeValue);
		vec2 v01 = rotV2(texelFetch(lookup, uv01%tsize, 0).xy, timeValue);
		vec2 v11 = rotV2(texelFetch(lookup, uv11%tsize, 0).xy, timeValue);



		float n0, n1, l0, l1;
		n0 = dot(v00, fUV - vec2(uv00));
		n1 = dot(v10, fUV - vec2(uv10));
		l0 = cubicLerp(n0, n1, sx);

		n0 = dot(v01, fUV - vec2(uv01));
		n1 = dot(v11, fUV - vec2(uv11));
		l1 = cubicLerp(n0, n1, sx);

		value += cubicLerp(l0, l1, sy);// *(1.2 - scl);
		scl *= decay;
		decay *= 0.5;

	}

	return clamp(value * 0.5 + 0.5, 0, 1);

}


layout(location=0) in TES_OUT{
	vec4 pos;
	vec2 uv;
} tes_in[];

layout(location=0) out FS_OUT{
	vec4 pos;
	vec4 wPos;
	vec3 nml;
	vec2 uv;
} tes_out;



void main(){

	vec4 p0 = mix(tes_in[0].pos, tes_in[1].pos, gl_TessCoord.x);
	vec4 p1 = mix(tes_in[2].pos, tes_in[3].pos, gl_TessCoord.x);
	vec4 vPos = mix(p0, p1, gl_TessCoord.y);

	vec2 uv0 = mix(tes_in[0].uv, tes_in[1].uv, gl_TessCoord.x);
	vec2 uv1 = mix(tes_in[2].uv, tes_in[3].uv, gl_TessCoord.x);
	vec2 vuv = mix(uv0, uv1, gl_TessCoord.y);



	float nx = perlin(vuv + vec2(0.001, 0.0)) - perlin(vuv - vec2(0.001, 0.0));
	float ny = perlin(vuv + vec2(0.0, 0.001)) - perlin(vuv - vec2(0.0, 0.001));


	vec3 nml = normalize(vec3(nx, 1.85, ny));

	float tHeight =  perlin(vuv)*1.25;

	vPos.y += tHeight;
	tes_out.wPos = vPos;
	vPos = camera.proj_view_matrix * vPos;


	tes_out.pos = vPos;
	tes_out.uv = vuv;
	tes_out.nml = nml;



	gl_Position = vPos;
}

