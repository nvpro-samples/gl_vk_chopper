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

struct SceneData{
	mat4 view_matrix;
};

struct CameraData{
	mat4 proj_matrix;
	mat4 view_matrix;
	vec4 cameraPosition;
};

layout(std140, binding = 0) uniform sceneBuffer{
	SceneData scene;
};

layout(std140, binding = 2) uniform cameraBuffer{
	CameraData camera;
};


in layout(location = 0) vec4 pos;
in layout(location = 1) vec2 uv;

layout(location=0) out VS_OUT{
	vec4 pos;
	vec3 uv;
} vs_out;

void main(){

	vec4 uvPos = camera.view_matrix * vec4(uv*2.0 - 1.0, -1.0, 1.0);

	vs_out.uv = uvPos.xyz;
	vs_out.uv.y *= -1;

	vs_out.pos =  vec4(pos.xyz,1.0);
	vs_out.pos = scene.view_matrix * vs_out.pos;
	vs_out.pos.y = -vs_out.pos.y;
	vs_out.pos.z = (vs_out.pos.z + vs_out.pos.w) / 2.0;
	gl_Position = vs_out.pos;
}

