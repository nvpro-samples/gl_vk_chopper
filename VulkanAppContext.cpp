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


#include "VulkanAppContext.h"
#include "vkaUtils.h"
#include "VKSFile.h"

#include<string>


#include"VkeCreateUtils.h"
#include"VkeRenderer.h"


#include"VkeGameRendererDynamic.h"

#include"VulkanDeviceContext.h"
#include<iostream>
#include<main.h>
#ifndef INIT_COMMAND_ID
#define INIT_COMMAND_ID 1
#endif

#define DEFAULT_SCENE_ID 1

#define RENDERER vkeGameRendererDynamic



VulkanAppContext *VulkanAppContext::GetInstance(){
	static VulkanAppContext *instance = NULL;

	if (!instance){
		instance = new VulkanAppContext();
	}

	return instance;
}

VulkanAppContext::VulkanAppContext():
m_width(1024),
m_height(768),
m_ready(false),
m_rotor_node(NULL)
{
}

void VulkanAppContext::loadVKSScene(std::string &inFileName){
	VKSFile vkFile;
	vkFile.outputFile = inFileName;


	readVKSFile(&vkFile);

	/*
		Unpack meshes
	*/

	uint32_t meshCnt = vkFile.header.meshCount;

#if USE_SINGLE_VBO

	uint32_t vtxStoreSize = vkFile.vertexCount * 8 * sizeof(float);
	uint32_t idxStoreSize = vkFile.indexCount * sizeof(uint32_t);

	m_global_vbo.initBackingStore(vtxStoreSize);
	m_global_ibo.initBackingStore(idxStoreSize);

	float *vData = m_global_vbo.getBackingStore();
	memcpy(vData, vkFile.vertices.data(), vtxStoreSize);
	uint32_t *iData = m_global_ibo.getBackingStore();
	memcpy(iData, vkFile.indices.data(), idxStoreSize);
	
	
	m_global_vbo.initVKBufferData();
	m_global_ibo.initVKBufferData();

#endif


	uint32_t animCount = vkFile.header.animationCount;


	if (animCount >= 1){
		//importing the first animation only at the moment.
		VKSAnimationRecord animation = vkFile.animations[0];
		uint32_t nCnt = animation.nodecount;

		for (uint32_t n = 0; n < nCnt; ++n){
			VKSAnimationNodeRecord nodeAnim = vkFile.animationNodes[n + animation.firstNode];

			std::string newNodeName(nodeAnim.name);

			VkeAnimationNode *vAnimNode = m_animation.newNode(newNodeName);

			uint32_t keyCount = nodeAnim.positionCount;

			for (uint32_t k = 0; k < keyCount; ++k){
				VKSAnimationKeyRecord key = vkFile.animationKeys[k + nodeAnim.firstPosition];
				vAnimNode->Position().newKey(key.time, key.key);
			}

			keyCount = nodeAnim.rotationCount;

			for (uint32_t k = 0; k < keyCount; ++k){
				VKSAnimationKeyRecord key = vkFile.animationKeys[k + nodeAnim.firstRotation];
				vAnimNode->Rotation().newKey(key.time, key.key);
			}

			keyCount = nodeAnim.scaleCount;

			for (uint32_t k = 0; k < keyCount; ++k){
				VKSAnimationKeyRecord key = vkFile.animationKeys[k + nodeAnim.firstScale];
				vAnimNode->Scale().newKey(key.time, key.key);
			}
		}
	}



	for (uint32_t i = 0; i < meshCnt; ++i){
		VkeMesh *theMesh = m_mesh_data.newMesh(i, &vkFile, &vkFile.meshes[i]);

#if USE_SINGLE_VBO
#else
		theMesh->initVKBuffers();
#endif


		theMesh->setFirstIndex(vkFile.meshes[i].firstIndex);
		theMesh->setFirstVertex(vkFile.meshes[i].firstVertex);

	}

	uint32_t matCnt = vkFile.header.materialCount;


	for (uint32_t i = 0; i < matCnt; ++i){
		m_materials.newMaterial(i)->initFromData(&vkFile,&vkFile.materials[i]);
	}
	
	uint32_t nodeCnt = vkFile.header.nodeCount;
	uint32_t nodesProcessed = 0;


	while (nodesProcessed < nodeCnt){
		addVKSNode(&vkFile, nodesProcessed);
	}



	m_node_data.sortByOpacity();

}

void VulkanAppContext::addVKSNode(VKSFile *inFile, uint32_t &inNodesProcessed, Node *parentNode){

	uint32_t nodeID = inNodesProcessed;
	VKSNodeRecord *fileNode = &inFile->nodes[inNodesProcessed++];
	VkeNodeData *vNode;
	Node *node;
	
	uint32_t mshCount = fileNode->meshCount;

	for (uint32_t i = 0; i < mshCount; ++i){

		if (parentNode){
			node = parentNode->newChild(nodeID);
		}
		else{
			node = m_scene_graph->Nodes().newNode(nodeID);
		}

		node->setPosition(
			fileNode->position.x,
			fileNode->position.y,
			fileNode->position.z
			);

		node->setRotation(fileNode->rotation);
		node->setScale(fileNode->scale.x, fileNode->scale.y, fileNode->scale.z);


		m_node_data.newData(node->getID())->updateFromNode(node);
		m_node_data.getData(node->getID())->setMesh(m_mesh_data.getMesh(fileNode->meshIndices[i]));
	}

	std::string nameStr = std::string(fileNode->name);

	VkeAnimationNode *animNode = m_animation.Nodes().getNode(nameStr);
	if (animNode){
		animNode->setNode(m_node_data.getData(node->getID()));
	}

	if (nameStr == "main_rotor_parts02"){
		m_rotor_node = m_node_data.getData(node->getID());
	}

	uint32_t childCount = fileNode->childCount;
	for (uint32_t i = 0; i < childCount; ++i){
		addVKSNode(inFile, inNodesProcessed, node);
	}
}


void VulkanAppContext::initRenderer(nv_helpers_gl::ProgramManager &inProgramManager){

    RenderContext *rctxt = RenderContext::Get();
    if (!rctxt) return;

	
	/*
		Used to manage the rotation of the 
		Gazelle's rotors.
	*/

	m_rot_y = 0.0f;
	m_current_time = 0.0;

	/*
		Create the renderer.
	*/

    m_renderer = new RENDERER();

    m_scene_graph = rctxt->newScene(DEFAULT_SCENE_ID);

	std::string sceneFile = "chopper_pack32.vks";

	loadVKSScene(sceneFile);

	

	((RENDERER*)m_renderer)->setNodeData(&m_node_data);
	((RENDERER*)m_renderer)->setMaterialData(&m_materials);
	((RENDERER*)m_renderer)->initIndirectCommands();
	m_renderer->initShaders(inProgramManager);

	m_renderer->initLayouts();

	resize(m_width, m_height);
	
	VkCommandBuffer cmd = VK_NULL_HANDLE;
	VulkanDC::Device::Queue::CommandBufferID cmdID = INIT_COMMAND_ID + 300;
	VulkanDC *dc = VulkanDC::Get();
	VulkanDC::Device *device = dc->getDefaultDevice();

	dc->getDefaultQueue()->beginCommandBuffer(cmdID, &cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	dc->getDefaultQueue()->flushCommandBuffer(cmdID, NULL);
    m_ready = true;
}

void VulkanAppContext::resize(uint32_t inWidth, uint32_t inHeight){
	VulkanDC *dc = VulkanDC::Get();
	VulkanDC::Device::Queue::CommandBufferID  cmdID = INIT_COMMAND_ID;
	VulkanDC::Device *device = dc->getDefaultDevice();
	device->waitIdle();
	m_renderer->resize(inWidth, inHeight);

	dc->getDefaultQueue()->flushCommandBuffer(cmdID);

}

void VulkanAppContext::render(){
	if (!m_ready) return;

	VulkanDC *dc = VulkanDC::Get();
	VulkanDC::Device *device = dc->getDefaultDevice();

	VkFence nullFence = { VK_NULL_HANDLE };

	m_rot_y += 0.0125;

	if (m_rotor_node){
		m_rotor_node->getNode()->setRotation(0.0, 0.0, -m_rot_y*32.0);
	}

	m_current_time += 0.016;

	if (m_current_time > m_animation.getEndTime()){
		m_current_time = 0.0;
	}

	m_renderer->update();

}

void VulkanAppContext::initAppContext(){

	/*
		Set up the application create info.
	*/
	VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
	appInfo.pApplicationName = "gl_vk_chopper";
	appInfo.applicationVersion = 1;
	appInfo.pEngineName = "gl_vk_chopper";
	appInfo.engineVersion = 1;
        appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
	/*
		Set up the instance create info.
	*/
	VkInstanceCreateInfo createInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
	createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = 0;
	createInfo.ppEnabledLayerNames = NULL;
    createInfo.enabledExtensionCount = 0;
    createInfo.ppEnabledExtensionNames = NULL;
	
	/*
		Create the Vulkan Instance
	*/

    VKA_CHECK_ERROR(vulkanCreateInstance(&createInfo, &m_vk_instance), "Could not create Vulkan Instance");

	/*
		Set up and initialise the Vulkan Device Context.
	*/
	VulkanDC *dc = VulkanDC::Get();
	dc->initDC(m_vk_instance);

	/*
		Get the default graphics queue.
	*/
    VulkanDC::Device::Queue::Name queueName = "DEFAULT_GRAPHICS_QUEUE";
    VulkanDC::Device::Queue *queue = dc->getQueueForGraphics(queueName,m_surface_format);
    dc->setDefaultDevice(queue->getDevice());
	dc->setDefaultQueue(queue);

}

VulkanAppContext::~VulkanAppContext()
{
}


bool VulkanAppContext::initPrograms(nv_helpers_gl::ProgramManager &inProgramManager){

	inProgramManager.m_preprocessOnly = true;

	/*
		Initialise the shaders used for the demo.
	*/
	inProgramManager.addDirectory(std::string(PROJECT_NAME));
	inProgramManager.addDirectory(NVPWindow::sysExePath() + std::string(PROJECT_RELDIRECTORY));
	inProgramManager.addDirectory(std::string(PROJECT_ABSDIRECTORY));

	m_program_ids.scene = inProgramManager.createProgram(
		nv_helpers_gl::ProgramManager::Definition(GL_VERTEX_SHADER, "shaders/std_vertex.glsl"),
		nv_helpers_gl::ProgramManager::Definition(GL_FRAGMENT_SHADER, "shaders/std_fragment.glsl"));

	m_program_ids.scene_quad = inProgramManager.createProgram(
		nv_helpers_gl::ProgramManager::Definition(GL_VERTEX_SHADER, "shaders/vertexQuad.glsl"),
		nv_helpers_gl::ProgramManager::Definition(GL_FRAGMENT_SHADER, "shaders/fragmentQuad.glsl"));

	m_program_ids.scene_terrain = inProgramManager.createProgram(
		nv_helpers_gl::ProgramManager::Definition(GL_VERTEX_SHADER, "shaders/vertexTerrain.glsl"),
		nv_helpers_gl::ProgramManager::Definition(GL_FRAGMENT_SHADER, "shaders/fragmentTerrain.glsl"),
		nv_helpers_gl::ProgramManager::Definition(GL_TESS_CONTROL_SHADER, "shaders/tcsTerrain.glsl"),
		nv_helpers_gl::ProgramManager::Definition(GL_TESS_EVALUATION_SHADER, "shaders/tesTerrain.glsl")
		);

	/*
		Check that the programs are valid.
	*/
	return inProgramManager.areProgramsValid();
}

VkeMaterial *VulkanAppContext::getMaterial(VkeMaterial::ID inID){
	return m_materials.getMaterial(inID);

}

void VulkanAppContext::setCameraMatrix(nv_math::mat4f &inMat){
	((RENDERER*)m_renderer)->setCameraLookAt(inMat);

}
