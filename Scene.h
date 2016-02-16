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

#ifndef __H_SCENE_
#define __H_SCENE_

#include<map>
#include<stdint.h>
#include"Node.h"
#include"Camera.h"
#include"Types.h"


#pragma once
class Scene
{
public:
	typedef uint32_t ID;
	typedef std::map<ID, Scene*> Map;


	Scene();
	Scene(const ID &inID);
	~Scene();

	void update();

	Node::NodeList &Nodes();
	Node::NodeList &Cameras();
	Node::NodeList &Lights();

	void CurrentCamera(Camera *inCamera);
	void CurrentCamera(const Camera::ID &inID);
	Camera *CurrentCamera();

	Camera *newCamera(const float &inX  = 0.0f, const float &inY = 0.0f, const float &inZ = -5.0f);
	Camera *getCamera(const Camera::ID &inID);

	void getTriangles(render::TriangleList &outTriangles);

private:

	ID m_id;

	Node::NodeList m_root_nodes;
	Node::NodeList m_camera_list;
	Node::NodeList m_light_list;

	Camera *m_current_camera;

};





#endif