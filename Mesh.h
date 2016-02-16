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

#ifndef __H_MESH_
#define __H_MESH_

#include"WMath.h"
#include"Transform.h"
#include<map>
#include<stdio.h>

#pragma once
class Mesh
{
public:
	typedef uint32_t ID;
	typedef std::map<ID, Mesh*> Map;

	Mesh();
	Mesh(const ID &inID);
	~Mesh();

	void allocate(uint32_t inMaxVerts, uint32_t inMaxIdx);
	void dispose();

	Vec4f *&getVertices();
	Vec4f *&getNormals();
	uint32_t *&getIndices();
	Vec2f *&getUVs();


	void addVertex(const Vec4f &inVertex);
	void addVertex(const Vec4f &inVertex, const Vec4f &inNormal);
	void addIndex(const uint32_t &inIndex);

	Triangle4f &getTriangle(const uint32_t inIndex);
	Triangle4f &getTransformedTriangle(Mat4x4f &inTransform, const uint32_t inIndex);

	uint32_t getVertexCount(){ return m_vertex_count; }
	uint32_t getIndexCount(){ return m_index_count; }

	uint32_t getMaxVertexCount(){ return m_max_vertices; }
	uint32_t getMaxIndexCount(){ return m_max_indices; }

	uint32_t getTriangleCount(){ return m_triangle_count; }

	void setVertexCount(const uint32_t &inCount){ m_vertex_count = inCount; };
	void setIndexCount(const uint32_t &inCount){
		m_index_count = inCount;
		m_triangle_count = m_index_count / 3;
	};

	void getTransformedTriangles(Mat4x4f &inTransform,TriangleList4f &outTriangles);

	ID getID(){ return m_id; }
	int32_t getMaterialID() { return m_material_id; }
	void setMaterialID(const int32_t inID) { 
		
		m_material_id = inID; 
	
	}

private:

	ID m_id;

	Vec4f *m_vertices;
	Vec4f *m_normals;
	Vec2f *m_uvs;

	uint32_t *m_indices;

	uint32_t m_max_vertices;
	uint32_t m_max_indices;

	uint32_t m_vertex_count;
	uint32_t m_index_count;

	uint32_t m_triangle_count;

	int32_t m_material_id;
	


};

#endif
