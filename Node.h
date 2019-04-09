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

#ifndef __H_NODE_
#define __H_NODE_

#include"Transform.h"
#include"Renderable.h"
#include"Types.h"
#include<map>
#include<vector>



#pragma once
class Node
{
public:
	typedef uint32_t ID;
	typedef uint32_t Count;
	typedef std::map<ID, Node*> Map;
	typedef std::vector<Node*> List;

	Node();
	Node(Node   *inParent, const ID &inID);
	~Node();

	void reset();
	bool update(bool inUpdateChildren = false);
	void draw();

	class NodeList{
	public:
		NodeList();
		~NodeList();

		Node *newNode(const ID &inID,Node *inParent = NULL);
		void addNode(Node *inNode, Node *inParent = NULL);
		Node *getNode(const ID &inID);

		Node::List &getData(){ return m_data; }

		template<typename T>
		T* newNodeClass(Node *inParent=NULL){

            typename T::ID id = nextID();

            T* node = new T(inParent, id);
			
			addNode(node);

			return node;
		}

		ID nextID();
		Count count();
	private:
		Node::List m_data;
	};

	void setPosition(float inX, float inY, float inZ);
	void setRotation(float inX, float inY, float inZ);
	void setRotation(nvmath::quatf &inQuat);
	void setScale(float inX, float inY, float inZ);
	void setScale(float inScale);

	nvmath::vec4f worldPosition();
	nvmath::vec4f worldPosition(nvmath::vec4f &inPosition);

	Node *newChild();
	Node *newChild(const ID &inID);
	Node *newChild(const Vec4f &inPosition);
	Node *newChild(const float inX, const float inY, const float inZ);

	Renderable *newRenderable();
	Renderable *getRenderable(const Renderable::ID &inID);

	NodeList &ChildNodes();
	Transform &GetTransform();

	Node *getParent() { return m_parent; }
	void setParent(Node *inParent) { m_parent = inParent; }

	void getTriangles(render::TriangleList &outTriangles);

	ID getID(){ return m_id; }

protected:
	ID m_id;
	Node	*m_parent;
	Transform m_transform;

	NodeList m_child_nodes;
	Renderable::List m_renderables;

	nvmath::vec3f m_position;
	nvmath::vec3f m_rotation;
	nvmath::vec3f m_scale;

	bool m_transform_needs_update;


};


#endif
