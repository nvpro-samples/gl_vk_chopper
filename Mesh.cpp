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

#include "Mesh.h"
#include<stdlib.h>

Mesh::Mesh():
m_vertices(NULL),
m_indices(NULL),
m_normals(NULL),
m_max_vertices(0),
m_max_indices(0),
m_vertex_count(0),
m_index_count(0),
m_material_id(-1)
{
}

Mesh::Mesh(const Mesh::ID &inID):
m_id(inID),
m_vertices(NULL),
m_indices(NULL),
m_normals(NULL),
m_max_vertices(0),
m_max_indices(0),
m_vertex_count(0),
m_index_count(0),
m_material_id(-1)
{
}

void Mesh::allocate(uint32_t inMaxVerts, uint32_t inMaxIdx){
	dispose();
	m_max_vertices = inMaxVerts;
	m_max_indices = inMaxIdx;
	m_vertices = new Vec4f[m_max_vertices];
	m_normals = new Vec4f[m_max_vertices];
	m_indices = new uint32_t[m_max_indices];
	m_uvs = new Vec2f[m_max_vertices];
}

Vec4f *&Mesh::getVertices(){
	return m_vertices;
}

Vec4f *&Mesh::getNormals(){
	return m_normals;
}

uint32_t *&Mesh::getIndices(){
	return m_indices;
}

Vec2f *&Mesh::getUVs(){
	return m_uvs;
}

void Mesh::addVertex(const Vec4f &inVertex){
	Vec4f normal = {0.0,0.0,0.0,1.0};
	addVertex(inVertex, normal);	
}

void Mesh::addVertex(const Vec4f &inVertex, const Vec4f &inNormal){
	if (m_vertex_count >= m_max_vertices) return;
	m_normals[m_vertex_count] = inNormal;
	m_vertices[m_vertex_count++] = inVertex;

}

void Mesh::addIndex(const uint32_t &inIndex){
	if (m_index_count >= m_max_indices) return;
	m_indices[m_index_count++] = inIndex;
	m_triangle_count = m_index_count / 3;
}

Triangle4f &Mesh::getTriangle(const uint32_t inIndex){
	uint32_t startIndex = inIndex * 3;
	Triangle4f out = {
		m_vertices[m_indices[startIndex]],
		m_vertices[m_indices[startIndex + 1 ]],
		m_vertices[m_indices[startIndex + 2]],
		m_normals[m_indices[startIndex]],
		m_normals[m_indices[startIndex + 1]],
		m_normals[m_indices[startIndex + 2]]
	};
	return out;
}

Triangle4f &Mesh::getTransformedTriangle(Mat4x4f &inTransform, const uint32_t inIndex){
	uint32_t startIndex = inIndex * 3;

	Triangle4f out = {
		inTransform(m_vertices[m_indices[startIndex]]),
		inTransform(m_vertices[m_indices[startIndex + 1]]),
		inTransform(m_vertices[m_indices[startIndex + 2]]),
		inTransform(m_normals[m_indices[startIndex]]),
		inTransform(m_normals[m_indices[startIndex + 1]]),
		inTransform(m_normals[m_indices[startIndex + 2]])
	};

	return out;
}


void Mesh::dispose(){
	if (m_vertices){
		delete[] m_vertices;
		m_vertices = NULL;
	}

	if (m_indices){
		delete[] m_indices;
		m_indices = NULL;
	}

	if (m_normals){
		delete[] m_normals;
		m_normals = NULL;
	}

	m_max_indices = 0;
	m_max_vertices = 0;
	m_index_count = 0;
	m_vertex_count = 0;
	m_triangle_count = 0;
}

void Mesh::getTransformedTriangles(Mat4x4f &inTransform,TriangleList4f &outTriangles){

	for (uint32_t i = 0; i < m_triangle_count; ++i){
		Triangle4f tri = getTransformedTriangle(inTransform,i);
		outTriangles.push_back(tri);
	}

}



Mesh::~Mesh()
{
}
