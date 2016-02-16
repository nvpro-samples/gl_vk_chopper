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

#define DEBUG_FILTER     1

#include <GL/glew.h>
#include<nv_helpers/anttweakbar.hpp>
#include <nv_helpers_gl/WindowProfiler.hpp>
#include <nv_math/nv_math_glsltypes.h>

#include <nv_helpers_gl/error.hpp>
#include <nv_helpers_gl/programmanager.hpp>
#include <nv_helpers/geometry.hpp>
#include <nv_helpers/misc.hpp>
#include <nv_helpers_gl/glresources.hpp>
#include <nv_helpers/cameracontrol.hpp>

#include <noise/MersenneTwister.h>
#include <stdint.h>
#include<iostream>


#include"VulkanDeviceContext.h"
#include"VulkanAppContext.h"

using namespace nv_helpers;
using namespace nv_helpers_gl;
using namespace nv_math;


extern bool vulkanInitLibrary();

namespace pathclipping
{
  int const SAMPLE_SIZE_WIDTH(1024);
  int const SAMPLE_SIZE_HEIGHT(768);
  int const SAMPLE_MAJOR_VERSION(4);
  int const SAMPLE_MINOR_VERSION(5);

  int const PERLIN_GRID_U_SIZE(128);
  int const PERLIN_GRID_V_SIZE(128);



  class Sample;

 
  class Sample : public nv_helpers_gl::WindowProfiler
  {
    ProgramManager progManager;

  CameraControl m_control;


  void end(){
    TwTerminate();
  }
  
  bool mouse_pos(int x,int y){
    return !!TwEventMousePosGLFW(x,y);
  }

  bool mouse_button(int button, int action){
    return !!TwEventMouseButtonGLFW(button,action);
  }

  bool mouse_wheel(int wheel){
    return !!TwEventMouseWheelGLFW(wheel);
  }

  bool key_button(int button, int action, int mods){
    return handleTwKeyPressed(button,action,mods);
  }

  struct Tweak{
    Tweak():
      samples(1),
      curFile(0),
	  cmdBufferMode(0),
	  drawReflections(true),
	  drawShadows(false),
	  playAnimation(true),
      drawClips(false){}

    int samples;
	int cmdBufferMode;
    int curFile;
    bool drawClips;
	bool drawReflections;
	bool drawShadows;
	bool playAnimation;

  };

  Tweak tweak;
  Tweak tweakLast;

	struct {
		ResourceGLuint
			scene;
	} fbos;

	struct {
		ResourceGLuint
			scene_color,
			scene_depthstencil;
	} textures;

	struct {
		ProgramManager::ProgramID
			draw_quad;

	} programs;

    bool begin();
    void think(double time);
    void resize(int width, int height);

    bool initProgram();
    bool initVulkan();
    bool initScene();
    bool initMisc();
	
	bool initFramebuffers(int width, int height, int samples);
	public:
  };


  bool Sample::initProgram()
  {
   //No programs used for this demo.

	  bool validated(true);

	  /*
		Programs for Gazelle are managed by the App Context.
		Just return true here.
	  */
    return true;
  }

  bool Sample::initVulkan(){

	  bool vulkanReady = true;// vulkanInitLibrary();

	 /*
		Get and initialise the VulkanAppContext.
	 */
     VulkanAppContext *ctxt = VulkanAppContext::GetInstance();
     ctxt->initAppContext();
	 /*
		Load the programs
	 */
	 bool progsValid = ctxt->initPrograms(progManager);
	 /*
		Initialise the renderer.
	 */
	 ctxt->initRenderer(progManager);

     return vulkanReady;
  }

  bool Sample::initMisc()
  {
    return true;
  }

 
  bool Sample::initScene()
  {
   
    return true;
  }

  bool Sample::initFramebuffers(int width, int height, int samples)
  {

    if (samples > 1){
      newTexture(textures.scene_color);
      glBindTexture (GL_TEXTURE_2D_MULTISAMPLE, textures.scene_color);
      glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA8, width, height, GL_FALSE);
      glBindTexture (GL_TEXTURE_2D_MULTISAMPLE, 0);

      newTexture(textures.scene_depthstencil);
      glBindTexture (GL_TEXTURE_2D_MULTISAMPLE, textures.scene_depthstencil);
      glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_DEPTH24_STENCIL8, width, height, GL_FALSE);
      glBindTexture (GL_TEXTURE_2D_MULTISAMPLE, 0);
    }
    else
    {
      newTexture(textures.scene_color);
      glBindTexture (GL_TEXTURE_2D, textures.scene_color);
      glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
      glBindTexture (GL_TEXTURE_2D, 0);

      newTexture(textures.scene_depthstencil);
      glBindTexture (GL_TEXTURE_2D, textures.scene_depthstencil);
      glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, width, height);
      glBindTexture (GL_TEXTURE_2D, 0);
    }

    newFramebuffer(fbos.scene);
    glBindFramebuffer(GL_FRAMEBUFFER,     fbos.scene);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,        textures.scene_color, 0);
   // glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, textures.scene_depthstencil, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
  }


  bool Sample::begin()
  {


    TwInit(TW_OPENGL_CORE,NULL);
    TwWindowSize(m_window.m_viewsize[0], m_window.m_viewsize[1]);
    
  //  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    bool validated(true);

    validated = validated && initMisc();
    validated = validated && initScene();
    validated = validated && initFramebuffers(m_window.m_viewsize[0],m_window.m_viewsize[1],8);

	initVulkan();


    TwBar *bar = TwNewBar("mainbar");
    TwDefine(" GLOBAL contained=true help='OpenGL Samples. \nCopyright NVIDIA Corporation 2013-2015' ");
    TwDefine(" mainbar position='0 0' size='300 150' color='0 0 0' alpha=128 valueswidth=120 ");
    TwDefine((std::string(" mainbar label='") + PROJECT_NAME + "'").c_str());


    TwEnumVal enumSampleVals[] = {
      {1,"none"},
      {2,"2x"},
      {4,"4x"},
      {8,"8x"}
    };

	TwType samplesType = TwDefineEnum("samples", enumSampleVals, sizeof(enumSampleVals) / sizeof(enumSampleVals[0]));

	TwEnumVal cmdBufferModeVals[] = {
		{1,"Recreate Command Buffers" },
		{2, "Reuse Command Buffers"}
	};

	TwType cmdBufferModeType = TwDefineEnum("cmdBufferMode", cmdBufferModeVals, sizeof(cmdBufferModeVals) / sizeof(cmdBufferModeVals[0]));

	TwAddVarRW(bar, "samples", samplesType, &tweak.samples, " label='Samples' ");
	TwAddVarRW(bar, "cmdbuffermode", cmdBufferModeType, &tweak.cmdBufferMode, " label='Command Buffer Mode' ");
	TwAddVarRW(bar, "drawreflections", TW_TYPE_BOOL32, &tweak.drawReflections, " label='Draw Reflections' ");
	TwAddVarRW(bar, "drawshadows", TW_TYPE_BOOL32, &tweak.drawShadows, " label='Draw Shadows' ");
	TwAddVarRW(bar, "playanimation", TW_TYPE_BOOL32, &tweak.playAnimation, " label='Play Animation' ");

    m_control.m_sceneOrbit = vec3(0.0f);
    m_control.m_sceneDimension = float(1.0);
	nv_math::vec3f scenePos = nv_math::vec3f(20.0, -1.0, 8.0);

	m_control.m_viewMatrix = nv_math::look_at(m_control.m_sceneOrbit - (scenePos* m_control.m_sceneDimension*0.5f), m_control.m_sceneOrbit, vec3(0, 1, 0));
  
	
	return validated;
  }


  void Sample::think(double time)
  {
    
    m_control.processActions(m_window.m_viewsize, nv_math::vec2f(m_window.m_mouseCurrent[0], m_window.m_mouseCurrent[1]), m_window.m_mouseButtonFlags, m_window.m_wheel);
    
    if (m_window.onPress(KEY_R)){
      progManager.reloadPrograms();
    }
    if (!progManager.areProgramsValid()){
      waitEvents();
      return;
    }

    int width   = m_window.m_viewsize[0];
    int height  = m_window.m_viewsize[1];

	VulkanAppContext *ctxt = VulkanAppContext::GetInstance();
	ctxt->setCameraMatrix(m_control.m_viewMatrix);

    glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    {
      NV_PROFILE_SECTION("Scene");
     // glViewport(0, 0, width, height);

	  ctxt->render();
		
    }
  

   {
    //NV_PROFILE_SECTION("TwDraw");
   // TwDraw();
    }
  }


  void Sample::resize(int width, int height)
  {
    TwWindowSize(width,height);
    VulkanDC *dc = VulkanDC::Get();
    VulkanDC::Device *device = dc->getDefaultDevice();
    VulkanAppContext *ctxt = VulkanAppContext::GetInstance();
    device->waitIdle();
    initFramebuffers(width,height,1);
    ctxt->render();
    device->waitIdle();
    ctxt->resize(width, height);
    device->waitIdle();
    ctxt->render();
  }
}

using namespace pathclipping;

int sample_main(int argc, const char** argv)
{
  Sample sample;

  return sample.run(
    PROJECT_NAME,
    argc, argv,
    SAMPLE_SIZE_WIDTH, SAMPLE_SIZE_HEIGHT,
    SAMPLE_MAJOR_VERSION, SAMPLE_MINOR_VERSION);
}
void sample_print(int level, const char * fmt)
{

}
