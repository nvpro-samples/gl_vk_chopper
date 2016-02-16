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

#include "RenderContext.h"

RenderContext *RenderContext::Get(){
	static RenderContext *Instance(NULL);
	if (!Instance){
		Instance = new RenderContext();
	}
	return Instance;
}

RenderContext::RenderContext()
{
}


Scene *RenderContext::newScene(const Scene::ID &inID){
	deleteScene(inID);
	Scene *scene = new Scene(inID);
	m_scene_map[inID] = scene;
	return scene;
}

Scene *RenderContext::getScene(const Scene::ID &inID){
	return m_scene_map[inID];
}

void RenderContext::deleteScene(const Scene::ID &inID){
	Scene *scene = m_scene_map[inID];
	if (scene){
		m_scene_map.erase(inID);
		delete scene;
		scene = NULL;
	}
}

Mesh *RenderContext::newMesh(const Mesh::ID &inID, const meshimport::MeshDataf &inData){

	deleteMesh(inID);

	Mesh *mesh = new Mesh(inID);

	mesh->allocate(inData.vertex_count, inData.index_count);

	Vec4f *nmls = mesh->getNormals();
	Vec4f *vtxs = mesh->getVertices();
	uint32_t *idxs = mesh->getIndices();
	Vec2f *uvs = mesh->getUVs();

	mesh->setMaterialID(inData.materialID);

	memcpy(vtxs, inData.vertices, sizeof(float) * 4 * inData.vertex_count);
	memcpy(nmls, inData.normals, sizeof(float) * 4 * inData.vertex_count);
	memcpy(idxs, inData.indices, sizeof(uint32_t) * inData.index_count);
	memcpy(uvs, inData.uvs, sizeof(float) * 2 * inData.vertex_count);

	mesh->setIndexCount(inData.index_count);
	mesh->setVertexCount(inData.vertex_count);

	return mesh;
}

Mesh *RenderContext::newMesh(const Mesh::ID &inID){
	deleteMesh(inID);
	Mesh *mesh = new Mesh(inID);
	m_mesh_map[inID] = mesh;
	return mesh;
}

Mesh *RenderContext::getMesh(const Mesh::ID &inID){
	return m_mesh_map[inID];
}

void RenderContext::deleteMesh(const Mesh::ID &inID){
	Mesh *mesh = m_mesh_map[inID];
	if (mesh){
		m_mesh_map.erase(inID);
		delete mesh;
		mesh = NULL;
	}
}


void RenderContext::getRenderTriangles(const Scene::ID &inID, render::TriangleList &outTriangles){
	Scene *scene = getScene(inID);
	if (!scene) return;
	
	scene->getTriangles(outTriangles);

}

RenderContext::~RenderContext()
{
}
