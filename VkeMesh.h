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

#ifndef __H_VKE_MESH_
#define __H_VKE_MESH_


#ifndef USE_SINGLE_VBO
#define USE_SINGLE_VBO 1
#endif


#pragma once

#include"VkeVBO.h"
#include"VkeIBO.h"
#include"Mesh.h"
#include<map>
#include<queue>
class VKSMeshRecord;
class VKSFile;
class VkeMesh
{
public:
	typedef uint32_t ID;
	typedef std::map<VkeMesh::ID, VkeMesh*> Map;
	typedef uint32_t Count;


	class List{
	public:
		List();
		~List();
		
		VkeMesh *newMesh();
		VkeMesh *newMesh(const VkeMesh::ID &inID);
		VkeMesh *newMesh(const VkeMesh::ID &inID, VKSFile *inFile, VKSMeshRecord *inData);
		VkeMesh *newMesh(const VkeMesh::ID &inID, Mesh * const inMesh);
		void addMesh(VkeMesh * const inMesh);
		VkeMesh *getMesh(const ID &inID);

		ID nextID();
		Count count();
	private:
		VkeMesh::Map m_data;
		std::vector<VkeMesh::ID> m_deleted_keys;
	};


	VkeMesh();
	VkeMesh(const ID &inID);
	~VkeMesh();

	void initFromMesh(Mesh  * const inMesh);
	void initFromMesh(VKSFile *inFile,VKSMeshRecord *inMesh);

	void initVKBuffers();

	void bind(VkCommandBuffer *inCmd);

	void initBindCommand(VkCommandPool *inPool, VkQueue *inQueue);
	void initDrawCommand(VkCommandBuffer *inCommand);
	void initDrawCommand(VkCommandBuffer *inCommand, VkBuffer &inIndirectBuffer, uint32_t inIndex);

	void draw(VkCommandBuffer *inCommand);

	ID getID() { return m_id; }


	int32_t getMaterialID(){ return m_material_id; }
	void setFirstIndex(const uint32_t inFirstIndex){ m_first_index = inFirstIndex; }
	void setFirstVertex(const uint32_t inFirstvertex){ m_first_vertex = inFirstvertex; }

	const uint32_t getFirstIndex(){ return m_first_index; }
	const uint32_t getFirstVertex(){ return m_first_vertex; }

	const uint32_t getIndexCount(){ return m_index_count; }


protected:

	ID m_id;

	/*
		Vertex Buffer
	*/
	VkeVBO m_vbo;

	/*
		Index Buffer
	*/
	VkeIBO m_ibo;

	VertexObject *m_vertices;
	uint32_t *m_indices;

	uint32_t m_vertex_count;
	uint32_t m_index_count;

	uint32_t m_first_index;
	uint32_t m_first_vertex;

	VkCommandBuffer m_draw_cmd;
	VkCommandBuffer m_bind_cmd;

	int32_t m_material_id;

};

#endif