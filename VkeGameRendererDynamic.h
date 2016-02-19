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

#ifndef __H_VKE_GAME_RENDERER_DYNAMIC_
#define __H_VKE_GAME_RENDERER_DYNAMIC_

#pragma once

#include"VkeRenderer.h"
#include "VkeMaterial.h"
#include"VkeCubeTexture.h"
#include"VkeScreenQuad.h"
#include"VkeTerrainQuad.h"

#include<stdint.h>
#include<thread>
#include<mutex>
#include<condition_variable>

class VkeCamera;

#define COMMAND_BUFFER_COUNT 2

const float r2d = 180 / (3.14159265359);

struct FlightPath{

	nv_math::vec2f m_start_position;
	nv_math::vec2f m_end_position;
	nv_math::vec2f m_position;
	nv_math::vec2f m_direction;
	nv_math::vec2f m_range;
	float m_t;

	float m_velocity;
	float m_altitude;

	FlightPath():
		m_start_position(0.0,0.0),
		m_end_position(0.0,0.0),
		m_position(0.0,0.0),
		m_direction(0.0,1.0),
		m_velocity(0.01),
		m_altitude(10.0){
	
	}

	FlightPath(
		nv_math::vec2f &initialPosition,
		nv_math::vec2f &endPosition,
		float startT = 0.0, 
		float inAltitude = 10,
		float inVelocity = 0.001):
		m_start_position(initialPosition),
		m_end_position(endPosition),
		m_position(0.0,0.0),
		m_velocity(inVelocity),
		m_altitude(inAltitude)
	{

		m_t = startT;
		m_range = m_end_position - m_start_position;
	
	}

	~FlightPath(){}

	void update(nv_math::mat4f *inMat, float inT){

		m_t += (m_velocity);
		if (m_t >= 1.0) m_t = 0.0;

		m_position = (m_range * m_t) + m_start_position;
		m_direction = nv_math::normalize(m_range);

		float yRot = atan2(m_range.x, m_range.y);

		nv_math::mat4f rotMat;
		rotMat.identity();
		rotMat.translate(nv_math::vec3f(m_position.x, m_altitude, m_position.y));


		inMat->identity();
		inMat->rotate(yRot, nv_math::vec3f(0.0, 1.0, 0.0));
		inMat->rotate(-90 / r2d, nv_math::vec3f(1.0, 0.0, 0.0));
		*inMat = rotMat * (*inMat);

	}

};

class vkeGameRendererDynamic;

class VkeDrawCall{
public:
	VkeDrawCall(vkeGameRendererDynamic *inRenderer);
	~VkeDrawCall();

	VkCommandBuffer getDrawCommand(const uint32_t inFrameIndex);
	
	void initDescriptorPool();
	void initDescriptor();
	void initCommandPool();
	void initDrawCommands(const uint32_t inCount, const uint32_t inBufferIndex);

private:

	vkeGameRendererDynamic *m_renderer;
	VkDescriptorSet m_transform_descriptor_set;
	VkDescriptorPool m_descriptor_pool;
	VkCommandBuffer m_draw_command[COMMAND_BUFFER_COUNT];
	VkCommandPool m_command_pool;

	nv_math::mat4f m_draw_transform;

	bool m_buffer_ready;

};

class vkeGameRendererDynamic : public VkeRenderer
{
public:
	vkeGameRendererDynamic();
	~vkeGameRendererDynamic();


	void initRenderer();

	void initIndirectCommands();
	virtual void initDescriptorLayout();
	virtual void initDescriptorSets();
	virtual void initPipeline();
	virtual void initRenderPass();
	virtual void initFramebuffer(uint32_t inWidth, uint32_t inHeight);
	virtual void releaseFramebuffer();

	void initTerrainCommand();

	void initDrawCalls();
	void generateDrawCommands();

	void setNodeData(VkeNodeData::List *inData);
	void setMaterialData(VkeMaterial::List *inData);
	virtual uint32_t getRequiredDescriptorCount();

	void initCamera();
	virtual void update();

	virtual void present();
	virtual void initShaders(nv_helpers_gl::ProgramManager &inProgramManager);
	virtual void setCameraLookAt(nv_math::mat4f &inMat);

	VkDescriptorSet getSceneDescriptorSet(){
		return m_scene_descriptor_set;
	}

	VkDescriptorSet *getTextureDescriptorSets(){
		return m_texture_descriptor_sets;
	}

	VkBuffer getSceneIndirectBuffer(){
		return m_scene_indirect_buffer;
	}

	VkDescriptorSetLayout *getTransformDescriptorLayout(){
		return &m_transform_descriptor_layout;
	}

	VkDescriptorBufferInfo *getTransformsDescriptor(){
		return &m_transforms_descriptor;
	}

	bool drawCallsReady(){
		return (m_calls_generated == m_max_draw_calls);
	}

	void incrementDrawCallsGenerated(){
		++m_calls_generated;
	}

	const uint32_t getCurrentBufferIndex(){
		return m_current_buffer_index;
	}

	bool primaryCommandReady(){
		return m_primary_cmd_ready;
	}

protected:

	bool							m_primary_cmd_ready;


	VkCommandPool					m_primary_buffer_cmd_pool;
	VkCommandBuffer					m_primary_commands[2];
	VkCommandBuffer					m_update_commands[2];

	FlightPath						*m_test_fp;
	FlightPath						**m_flight_paths;

	uint32_t						m_current_buffer_index;

	uint32_t						m_max_draw_calls;
	uint32_t						m_calls_generated;

	DepthImageView					m_depth_attachment;
	ColorImageView					m_color_attachment;
	ColorImageView					m_resolve_attachment[2];

	VkeNodeData::List				*m_node_data;
	VkeMaterial::List				*m_materials;

	VkCommandBuffer					m_scene_command[2];
	VkCommandBuffer					m_terrain_command[2];

	VkSemaphore						m_present_done[2];
	VkSemaphore						m_render_done[2];
	VkFence							m_update_fence[2];


	/*
		Will become part of the VkeDrawCall
	*/
	VkDescriptorSetLayout			m_transform_descriptor_layout;
	VkDescriptorSet					m_transform_descriptor_set;
	VkDescriptorBufferInfo			m_transforms_descriptor;
	/**/



	VkeDrawCall						**m_draw_calls;

	VkDescriptorSet					m_scene_descriptor_set;
	VkDescriptorSetLayout			m_scene_descriptor_layout;

	VkDescriptorSet					*m_texture_descriptor_sets;
	VkDescriptorSetLayout			m_texture_descriptor_set_layout;


	VkPipeline						m_quad_pipeline;
	VkPipelineLayout				m_quad_pipeline_layout;
	VkDescriptorSetLayout			m_quad_descriptor_set_layout;
	VkDescriptorSet					m_quad_descriptor_set;

	VkPipeline						m_terrain_pipeline;
	VkPipelineLayout				m_terrain_pipeline_layout;
	VkDescriptorSetLayout			m_terrain_descriptor_set_layout;
	VkDescriptorSet					m_terrain_descriptor_set;

	VkBuffer						m_uniforms_buffer_staging;
	VkDeviceMemory					m_uniforms_staging;

	VkBuffer						m_uniforms_buffer;
	VkDeviceMemory					m_uniforms_memory;
	

	VkDescriptorBufferInfo			m_uniforms_descriptor;

	float							*m_uniforms_local;

	VkBuffer						m_material_buffer_staging;
	VkDeviceMemory					m_material_staging;

	VkBuffer						m_scene_indirect_buffer;
	VkDeviceMemory					m_scene_indirect_memory;

	VkBuffer						m_transforms_buffer;
	VkDeviceMemory					m_transforms_memory;


	VkeCamera						*m_camera;
	VkeCamera						*m_light;
	VkeCubeTexture::List			m_cube_textures;

	VkeScreenQuad					m_screen_quad;
	VkeTerrainQuad					m_terrain_quad;

	VkCommandBuffer					m_update_buffer;

	uint32_t						m_instance_count;

	VkeTexture::List				m_textures;

	struct {
		VkShaderModule
			scene_vertex,
			scene_fragment,
			quad_vertex,
			quad_fragment,
			terrain_vertex,
			terrain_fragment,
			terrain_tcs,
			terrain_tes;
	} m_shaders;


	virtual void initDescriptorPool();

};

#endif
