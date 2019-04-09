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

layout(vertices = 4) out;

struct CameraData{
	mat4 proj_matrix;
	mat4 view_matrix;
	vec4 cameraPosition;
};

layout(std140, binding = 1) uniform cameraBuffer{
	CameraData camera;
};


layout(location=0) in VS_OUT{
	vec4 pos;
	vec2 uv;
} tcs_in[];


layout(location=0) out VS_OUT{
	vec4 pos;
	vec2 uv;
} tcs_out[];



void main(){


	if (gl_InvocationID == 0){
		float tf = 16.0;
		gl_TessLevelOuter[0] = tf;
		gl_TessLevelOuter[1] = tf;
		gl_TessLevelOuter[2] = tf;
		gl_TessLevelOuter[3] = tf;
		gl_TessLevelInner[0] = tf;
		gl_TessLevelInner[1] = tf;

		}

	tcs_out[gl_InvocationID].pos = tcs_in[gl_InvocationID].pos;
	tcs_out[gl_InvocationID].uv = tcs_in[gl_InvocationID].uv;



}

