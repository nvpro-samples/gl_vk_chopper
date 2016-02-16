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

#include "Node.h"


Node::Node():
m_parent(0),
m_position(0.0,0.0,0.0),
m_rotation(0.0,0.0,0.0),
m_scale(1.0,1.0,1.0),
m_transform_needs_update(true)
{
}

Node::Node(Node  *inParent, const ID &inID) :
m_parent(inParent),
m_id(inID),
m_position(0.0, 0.0, 0.0),
m_rotation(0.0, 0.0, 0.0),
m_scale(1.0, 1.0, 1.0),
m_transform_needs_update(true){

}

void Node::reset(){

}

bool Node::update(bool inUpdateChildren){
	bool updated = false;

	//m_transform_needs_update = true;
	if (m_transform_needs_update || inUpdateChildren){
		Transform *parentTransform = NULL;
		if (m_parent) parentTransform = &m_parent->GetTransform();

		m_transform.reset();
        nv_math::vec4f tra = nv_math::vec4f(m_position,1.0);
        m_transform.translate(tra);
		nv_math::quatf q;
		q.from_euler_xyz(m_rotation);
		m_transform.rotate(q);

		m_transform.update(parentTransform);
		m_transform_needs_update = false;
		updated = true;
	}

	uint32_t cnt = m_child_nodes.count();

	if (updated)
	for (uint32_t i = 0; i < cnt; ++i){
		m_child_nodes.getNode(i)->update(updated);

	}

	return updated;
}

nv_math::vec4f Node::worldPosition(){
	nv_math::vec4f outPosition(0.0, 0.0, 0.0, 1.0);

	return m_transform(outPosition);
}

nv_math::vec4f Node::worldPosition(nv_math::vec4f &inPosition){
	return m_transform(inPosition);
}

void Node::draw(){


}

void Node::setPosition(float inX, float inY, float inZ){
	m_position.x = inX;
	m_position.y = inY;
	m_position.z = inZ;
	m_transform_needs_update = true;
}

void Node::setRotation(nv_math::quatf &inQuat){
	nv_math::vec3f angles;
	inQuat.to_euler_xyz(angles);
	setRotation(angles.x, angles.y, angles.z);
}

void Node::setRotation(float inX, float inY, float inZ){
	m_rotation.x = inX;
	m_rotation.y = inY;
	m_rotation.z = inZ;
	m_transform_needs_update = true;
}

void Node::setScale(float inX, float inY, float inZ){
	m_scale.x = inX;
	m_scale.y = inY;
	m_scale.z = inZ;
	m_transform_needs_update = true;
}

void Node::setScale(float inScale){
	setScale(inScale, inScale, inScale);
}

Node *Node::newChild(){
	return newChild(0.0, 0.0, 0.0);
}

Node *Node::newChild(const ID &inID){

	Node *node = new Node(this, inID);
	m_child_nodes.addNode(node,this);
	return node;

}

Node *Node::newChild(const Vec4f &inPosition){
	ID id = m_child_nodes.nextID();
	Node *node = new Node(this, id);
	m_child_nodes.addNode(node, this);
	return node;
}

Node *Node::newChild(const float inX, const float inY, const float inZ){
	Vec4f position = {inX,inY,inZ,1.0};
	return newChild(position);
}

Renderable *Node::newRenderable(){
	Renderable *renderable = new Renderable();
	m_renderables.push_back(renderable);
	return renderable;
}

Renderable *Node::getRenderable(const Renderable::ID &inID){
	if (inID >= m_renderables.size()) return NULL;
	return m_renderables[inID];
}

Node::NodeList &Node::ChildNodes(){
	return m_child_nodes;
}

Transform &Node::GetTransform(){
	return m_transform;
}

void Node::getTriangles(render::TriangleList &outTriangles){
    uint32_t cnt = m_renderables.size();

	for (uint32_t i = 0; i < cnt; ++i){
		Mesh *mesh = m_renderables[i]->getMesh();
		if (!mesh) continue;

		uint32_t triCnt = mesh->getTriangleCount();

		for (uint32_t j = 0; j < triCnt; ++j){
			render::Triangle tri = {
				mesh->getTriangle(j),
				this
			};
			outTriangles.push_back(tri);
		}
    }
}

Node::~Node()
{
}

Node::NodeList::NodeList(){}
Node::NodeList::~NodeList(){}

Node::ID Node::NodeList::nextID(){
	return m_data.size();
}

Node::Count Node::NodeList::count(){
	return m_data.size();
}

Node *Node::NodeList::newNode(const ID &inID,Node *inParent){
	Node *node = new Node(inParent, inID);
	m_data.push_back(node);
	return node;
}

void Node::NodeList::addNode(Node *inNode, Node *inParent){

	if (!inNode) return;

	inNode->setParent(inParent);


	m_data.push_back(inNode);
}

Node *Node::NodeList::getNode(const ID &inID){
	if (inID >= count()) return NULL;
	return m_data[inID];
}
