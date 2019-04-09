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

#include "VkeAnimationNode.h"
#include "Node.h"

VkeAnimationNode::VkeAnimationNode():
m_scene_node(NULL),
m_parent(NULL)
{
}

VkeAnimationNode::VkeAnimationNode(VkeAnimationNode::Name &inName,VkeSceneAnimation *inParent) :
m_name(inName),
m_scene_node(NULL),
m_parent(inParent){

	m_position.setParent(inParent);
	m_rotation.setParent(inParent);
	m_scale.setParent(inParent);
}

VkeNodeData *VkeAnimationNode::getNode(){
	return m_scene_node;
}

void VkeAnimationNode::setNode(VkeNodeData *inNode){
	m_scene_node = inNode;
}


VkeAnimationChannel &VkeAnimationNode::Position(){
	return m_position;
}

VkeAnimationChannel &VkeAnimationNode::Rotation(){
	return m_rotation;
}

VkeAnimationChannel &VkeAnimationNode::Scale(){
	return m_scale;
}

VkeAnimationNode::Name &VkeAnimationNode::getName(){
	return m_name;
}

void VkeAnimationNode::update(){
	if (!m_scene_node) return;
	Node *node = m_scene_node->getNode();
	if (!node) return;

	nvmath::vec4f curValue;

	curValue = m_position.currentValue();

	node->setPosition(curValue.x, curValue.y, curValue.z);


	nvmath::quatf curQuat = m_rotation.currentQuatValue();
	curQuat.to_euler_xyz(&curValue.x);

	node->setRotation(curValue.x, curValue.y, curValue.z);

	curValue = m_scale.currentValue();
	node->setScale(curValue.x, curValue.y, curValue.z);

}

VkeAnimationNode::List::List(){
}

VkeAnimationNode::List::~List(){

}

VkeAnimationNode *VkeAnimationNode::List::newNode(VkeAnimationNode::Name &inName,VkeSceneAnimation *inParent){
	VkeAnimationNode *outNode = new VkeAnimationNode(inName,inParent);
	m_data[inName] = outNode;
	return outNode;
}

VkeAnimationNode *VkeAnimationNode::List::getNode(VkeAnimationNode::Name &inName){
	return m_data[inName];
}

void VkeAnimationNode::List::update(){

	VkeAnimationNode::Map::iterator itr;

	for (itr = m_data.begin(); itr != m_data.end(); ++itr){
		if (!itr->second) continue;
		itr->second->update();
	}

}


VkeAnimationNode::~VkeAnimationNode()
{
}
