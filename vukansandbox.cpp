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

#define DEBUG_FILTER 1

#include <include_gl.h>

#include <nvmath/nvmath_glsltypes.h>

#include <nvh/cameracontrol.hpp>
#include <nvh/geometry.hpp>
#include <nvh/misc.hpp>
#include <nvh/tnulled.hpp>

#include <nvgl/appwindowprofiler_gl.hpp>
#include <nvgl/base_gl.hpp>
#include <nvgl/error_gl.hpp>



#include <iostream>
#include <noise/mersennetwister1.h>
#include <stdint.h>


#include "VulkanAppContext.h"
#include "VulkanDeviceContext.h"

using namespace nvh;
using namespace nvgl;
using namespace nvmath;


extern bool vulkanInitLibrary();

namespace pathclipping {
int const SAMPLE_SIZE_WIDTH(1024);
int const SAMPLE_SIZE_HEIGHT(768);
int const SAMPLE_MAJOR_VERSION(4);
int const SAMPLE_MINOR_VERSION(5);

int const PERLIN_GRID_U_SIZE(128);
int const PERLIN_GRID_V_SIZE(128);


class Sample;


class Sample : public nvgl::AppWindowProfilerGL
{
  CameraControl m_control;


  void end()
  {
    // TwTerminate();
  }

  bool mouse_pos(int x, int y)
  {
    return false;  // !!TwEventMousePosGLFW(x, y);
  }

  bool mouse_button(int button, int action)
  {
    return false;  // !!TwEventMouseButtonGLFW(button, action);
  }

  bool mouse_wheel(int wheel)
  {
    return false;  // !!TwEventMouseWheelGLFW(wheel);
  }

  bool key_button(int button, int action, int mods)
  {
    return false;  // handleTwKeyPressed(button, action, mods);
  }

  struct Tweak
  {
    Tweak()
        : samples(1)
        , curFile(0)
        , cmdBufferMode(0)
        , drawReflections(true)
        , drawShadows(false)
        , playAnimation(true)
        , drawClips(false)
    {
    }

    int  samples;
    int  cmdBufferMode;
    int  curFile;
    bool drawClips;
    bool drawReflections;
    bool drawShadows;
    bool playAnimation;
  };

  Tweak tweak;
  Tweak tweakLast;

  struct
  {
    GLuint scene;
  } fbos;

  struct
  {
    GLuint scene_color, scene_depthstencil;
  } textures;


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

bool Sample::initVulkan()
{

  bool vulkanReady = true;  // vulkanInitLibrary();

  /*
  	Get and initialise the VulkanAppContext.
	 */
  VulkanAppContext* ctxt = VulkanAppContext::GetInstance();
  ctxt->initAppContext();
  /*
		Load the programs
	 */
  bool progsValid = ctxt->initPrograms();
  /*
		Initialise the renderer.
	 */
  ctxt->initRenderer();

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

  if(samples > 1)
  {
    newTexture(textures.scene_color, GL_TEXTURE_2D_MULTISAMPLE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textures.scene_color);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA8, width, height, GL_FALSE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    newTexture(textures.scene_depthstencil, GL_TEXTURE_2D_MULTISAMPLE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textures.scene_depthstencil);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_DEPTH24_STENCIL8, width, height, GL_FALSE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
  }
  else
  {
    newTexture(textures.scene_color, GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures.scene_color);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
    glBindTexture(GL_TEXTURE_2D, 0);

    newTexture(textures.scene_depthstencil, GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures.scene_depthstencil);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, width, height);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  newFramebuffer(fbos.scene);
  glBindFramebuffer(GL_FRAMEBUFFER, fbos.scene);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textures.scene_color, 0);
  // glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, textures.scene_depthstencil, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return true;
}


bool Sample::begin()
{


  // TwInit(TW_OPENGL_CORE,NULL);
  // TwWindowSize(m_windowState.m_viewsize[0], m_windowState.m_viewsize[1]);

  //  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glDisable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  bool validated(true);

  validated = validated && initMisc();
  validated = validated && initScene();
  validated = validated && initFramebuffers(m_windowState.m_viewSize[0], m_windowState.m_viewSize[1], 8);

  initVulkan();

  m_control.m_sceneOrbit     = vec3(0.0f);
  m_control.m_sceneDimension = float(1.0);
  nvmath::vec3f scenePos     = nvmath::vec3f(20.0, -1.0, 8.0);

  m_control.m_viewMatrix = nvmath::look_at(m_control.m_sceneOrbit - (scenePos * m_control.m_sceneDimension * 0.5f),
                                           m_control.m_sceneOrbit, vec3(0, 1, 0));


  return validated;
}


void Sample::think(double time)
{

  m_control.processActions(m_windowState.m_viewSize,
                           nvmath::vec2f(m_windowState.m_mouseCurrent[0], m_windowState.m_mouseCurrent[1]),
                           m_windowState.m_mouseButtonFlags, m_windowState.m_mouseWheel);


  int width  = m_windowState.m_viewSize[0];
  int height = m_windowState.m_viewSize[1];

  VulkanAppContext* ctxt = VulkanAppContext::GetInstance();
  ctxt->setCameraMatrix(m_control.m_viewMatrix);

  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


  {
    ctxt->render();
  }
}


void Sample::resize(int width, int height)
{
  VulkanDC*         dc     = VulkanDC::Get();
  VulkanDC::Device* device = dc->getDefaultDevice();
  VulkanAppContext* ctxt   = VulkanAppContext::GetInstance();
  device->waitIdle();
  initFramebuffers(width, height, 1);
  ctxt->render();
  device->waitIdle();
  ctxt->resize(width, height);
  device->waitIdle();
  ctxt->render();
}
}  // namespace pathclipping

using namespace pathclipping;

int main(int argc, const char** argv)
{
  NVPWindow::System system(argv[0], PROJECT_NAME);
  Sample            sample;

  return sample.run(PROJECT_NAME, argc, argv, SAMPLE_SIZE_WIDTH, SAMPLE_SIZE_HEIGHT);
}
