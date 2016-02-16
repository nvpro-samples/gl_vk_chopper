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

#include "Scene.h"


Scene::Scene():
m_id(0)
{
}

Scene::Scene(const Scene::ID &inID) :
m_id(inID){

}

void Scene::update(){
	uint32_t cnt = m_root_nodes.count();

	for (uint32_t i = 0; i < cnt; ++i){
		m_root_nodes.getNode(i)->update();
	}

	cnt = m_camera_list.count();

	for (uint32_t i = 0; i < cnt; ++i){
		m_camera_list.getNode(i)->update();
	}

}

Node::NodeList &Scene::Nodes(){
	return m_root_nodes;
}

Node::NodeList &Scene::Cameras(){
	return m_camera_list;
}

Node::NodeList &Scene::Lights(){
	return m_light_list;
}

void Scene::CurrentCamera(Camera *inCamera){
	m_current_camera = inCamera;
}

void Scene::CurrentCamera(const Camera::ID &inCamera){
	m_current_camera = (Camera *)m_camera_list.getNode(inCamera);
}

Camera *Scene::CurrentCamera(){
	return m_current_camera;
}

Camera *Scene::newCamera(const float &inX, const float &inY, const float &inZ){
	m_current_camera = m_camera_list.newNodeClass<Camera>(NULL);
	m_current_camera->GetTransform().translate(inX, inY, inZ);
	return m_current_camera;
}

Camera *Scene::getCamera(const Camera::ID &inID){
	return (Camera*)m_camera_list.getNode(inID);
}

void Scene::getTriangles(render::TriangleList &outTriangles){

	uint32_t cnt = m_root_nodes.count();
	for (uint32_t i = 0; i < cnt; ++i){
		m_root_nodes.getNode(i)->getTriangles(outTriangles);
	}
}

Scene::~Scene()
{
}
