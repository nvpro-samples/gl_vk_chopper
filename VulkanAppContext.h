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

#ifndef __H_VULKAN_APP_CONTEXT_
#define __H_VULKAN_APP_CONTEXT_



#include<vulkan/vulkan.h>
#include"vkaUtils.h"
#include"VkeNodeData.h"
#include"VkeMaterial.h"
#include"RenderContext.h"
#include"VkeMesh.h"
#include<nv_math/nv_math.h>
#include<nv_helpers_gl/programmanager_gl.hpp>
#include"VkeSceneAnimation.h"



#define TEXTURE_COUNT 2

class VkeMesh;
class VkeDeferredRenderer;
class VkeRenderer;


#pragma once
class VulkanAppContext
{
public:
	VulkanAppContext();
	~VulkanAppContext();

	static VulkanAppContext *GetInstance();

	void initAppContext();

	void initRenderer(nv_helpers_gl::ProgramManager &inProgramManager);

	void loadVKSScene(std::string &inFileName);
	void addVKSNode(VKSFile *inFile, uint32_t &inNodesProcessed, Node *parentNode = NULL);

	void render();

	bool initPrograms(nv_helpers_gl::ProgramManager &inProgramManager);

	void resize(uint32_t inWidth, uint32_t inHeight);
	VkeMaterial *getMaterial(VkeMaterial::ID inID);

	nv_math::vec4f getRotorPos(){
        nv_math::vec4f pos(0.0f,0.0f,-1.0f,1.0f);
        return m_rotor_node->getNode()->worldPosition(pos);
	}

	VkeVBO *getVBO(){ return &m_global_vbo; }
	VkeIBO *getIBO(){ return &m_global_ibo; }

	void setCameraMatrix(nv_math::mat4f &inMat);

	float getOpacity(uint32_t inMatID){
		return m_materials.getMaterial(inMatID)->getBackingStore()->opacity;

	}

private:


	VkInstance				m_vk_instance;

    uint32_t                m_width;
    uint32_t                m_height;



	VkFormat						m_surface_format;
    VkeRenderer                     *m_renderer;
    VkeNodeData::List               m_node_data;
    VkeMesh::List                   m_mesh_data;
    Scene                           *m_scene_graph;
    VkeMaterial::List               m_materials;
	VkeNodeData						*m_rotor_node;

	float m_rot_y;
	double m_current_time;

	bool							m_ready;
//	uint32_t						m_current_frame;
	VkeSceneAnimation				m_animation;

	VkeVBO							m_global_vbo;
	VkeIBO							m_global_ibo;

	



	struct ProgIDs {
		nv_helpers_gl::ProgramManager::ProgramID
			scene,
			scene_quad,
			scene_terrain;
	} m_program_ids;

public:

	ProgIDs &getProgramIDs() { return m_program_ids; }


};


#endif
