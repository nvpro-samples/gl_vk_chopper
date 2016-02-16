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

#version 450 core
#extension GL_KHR_vulkan_glsl : require

layout(quads) in;

struct CameraData{
	mat4 proj_matrix;
	mat4 view_matrix;
	vec4 cameraPosition;
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

		float timeValue = camera.cameraPosition.w*scl;

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


in TES_OUT{
	vec4 pos;
	vec2 uv;
} tes_in[];

out FS_OUT{
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
	tes_out.wPos.xyzw = vPos.xyzw;
	vPos = camera.proj_matrix * vPos;
	vPos.y = -vPos.y;
	vPos.z = (vPos.z + vPos.w) / 2.0;


	tes_out.pos = vPos;
	tes_out.uv = vuv;
	tes_out.nml = nml;



	gl_Position = vPos;
}

