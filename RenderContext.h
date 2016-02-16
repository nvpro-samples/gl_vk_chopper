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

#ifndef __H_RENDERCONTEXT_
#define __H_RENDERCONTEXT_

#pragma once

#include<map>
#include "Scene.h"
#include"Mesh.h"
#include"WMath.h"
#include"Types.h"
#include "MeshUtils.h"

class RenderContext
{
public:


	~RenderContext();

	static RenderContext* Get();

	Scene *newScene(const Scene::ID &inID);
	Scene *getScene(const Scene::ID &inID);
	void deleteScene(const Scene::ID &inID);

	Mesh *newMesh(const Mesh::ID &inID, const meshimport::MeshDataf &inData);
	Mesh *newMesh(const Mesh::ID &inID);
	Mesh *getMesh(const Mesh::ID &inID);
	void deleteMesh(const Mesh::ID &inID);

	void getRenderTriangles(const Scene::ID &inID, render::TriangleList &outTriangles);

private:
	RenderContext();

	Scene::Map m_scene_map;
	Mesh::Map m_mesh_map;

};

#endif