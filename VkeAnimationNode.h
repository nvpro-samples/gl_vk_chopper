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

#ifndef __H_VKE_ANIMATION_NODE_
#define __H_VKE_ANIMATION_NODE_

#pragma once
#include"VkeAnimationChannel.h"
#include"VkeNodeData.h"
#include<map>

class VkeAnimationNode
{
public:

	typedef std::string Name;
	typedef std::map<Name, VkeAnimationNode*> Map;

	VkeAnimationNode();
	VkeAnimationNode(Name &inName,VkeSceneAnimation *inParent);
	
	~VkeAnimationNode();

	VkeAnimationChannel &Position();
	VkeAnimationChannel &Rotation();
	VkeAnimationChannel &Scale();

	Name &getName();

	VkeNodeData *getNode();
	void setNode(VkeNodeData *inNode);

	void update();

	class List{
	public:
		List();
		~List();

		VkeAnimationNode *newNode(VkeAnimationNode::Name &inName,VkeSceneAnimation *inParent);
		VkeAnimationNode *getNode(VkeAnimationNode::Name &inName);

		void update();

	private:
		VkeAnimationNode::Map m_data;



	};

private:

	VkeSceneAnimation *m_parent;

	VkeAnimationChannel m_position;
	VkeAnimationChannel m_rotation;
	VkeAnimationChannel m_scale;

	Name m_name;

	VkeNodeData *m_scene_node;

};


#endif