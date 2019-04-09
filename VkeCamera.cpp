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

#include "VkeCamera.h"


VkeCamera::VkeCamera():
VkeBuffer(),
m_id(0),
m_near(VKE_DEFAULT_CAMERA_NEAR_PLANE),
m_far(VKE_DEFAULT_CAMERA_FAR_PLANE),
m_fov(VKE_DEFAULT_CAMERA_FOV),
m_viewport(VKE_DEFAULT_CAMERA_VIEWPORT),
m_position(0.0,0.0,0.0),
m_rotation(0.0,0.0,0.0),
m_projection_needs_update(true),
m_transform_needs_update(true),
m_view_projection_needs_update(true),
m_use_look_at(false),
m_time(0.0)
{
	initCameraData();
}

VkeCamera::VkeCamera(const VkeCamera::ID &inID) :
VkeBuffer(),
m_id(inID),
m_near(VKE_DEFAULT_CAMERA_NEAR_PLANE),
m_far(VKE_DEFAULT_CAMERA_FAR_PLANE),
m_fov(VKE_DEFAULT_CAMERA_FOV),
m_viewport(VKE_DEFAULT_CAMERA_VIEWPORT),
m_position(0.0, 0.0, 0.0),
m_rotation(0.0, 0.0, 0.0),
m_projection_needs_update(true),
m_transform_needs_update(true),
m_view_projection_needs_update(true),
m_use_look_at(false),
m_time(0.0)
{
	initCameraData();
}

VkeCamera::VkeCamera(const VkeCamera::ID &inID, const float inX, const float inY, const float inZ) :
VkeBuffer(),
m_id(inID),
m_near(VKE_DEFAULT_CAMERA_NEAR_PLANE),
m_far(VKE_DEFAULT_CAMERA_FAR_PLANE),
m_fov(VKE_DEFAULT_CAMERA_FOV),
m_viewport(VKE_DEFAULT_CAMERA_VIEWPORT),
m_position(inX,inY,inZ),
m_rotation(0.0, 0.0, 0.0),
m_projection_needs_update(true),
m_transform_needs_update(true),
m_view_projection_needs_update(true),
m_use_look_at(false),
m_time(0.0)
{
	initCameraData();
}

VkeCamera::~VkeCamera()
{
}

void VkeCamera::initCameraData(){
	m_projection_needs_update = true;
	m_transform_needs_update = true;
	m_view_projection_needs_update = true;
	m_usage_flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	m_memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	initBackingStore(sizeof(VkeCameraUniform));
	initVKBufferData(false);
}

void VkeCamera::updateProjection(){
	if (!m_projection_needs_update) return;
	m_aspect = m_viewport.z / m_viewport.w;

	m_projection = nvmath::perspective(m_fov,m_aspect,m_near,m_far);

	m_projection_needs_update = false;
	m_view_projection_needs_update = true;
}

void VkeCamera::updateTransform(){
	if (!m_transform_needs_update && !m_use_look_at) return;

	m_transform.reset();

	if (!m_use_look_at){

        nvmath::vec4f tra = nvmath::vec4f(m_position,1.0);
        nvmath::vec3f fwd(0.0,0.0,1.0);
        nvmath::vec3f up(0.0,0.0,0.0);
        nvmath::vec3f lft(0.0,0.0,0.0);
        m_transform.translate(tra);
        m_transform.rotate(m_rotation.z, fwd);
        m_transform.rotate(m_rotation.y, up);
        m_transform.rotate(m_rotation.x, lft);
	}
	else{
		m_transform.setMatrix(m_look_at_matrix);
	}

	m_transform.update();

	m_transform_needs_update = false;
	m_view_projection_needs_update = true;
}

void VkeCamera::updateViewProjection(){
	if (!m_view_projection_needs_update && !m_use_look_at) return;

	m_backing_store->view_proj_matrix = m_projection;
	m_backing_store->view_proj_matrix *= m_transform.getTransform();
	m_backing_store->view_matrix = nvmath::invert(m_transform.getTransform());

	m_backing_store->view_matrix.a03 = 0.0;
	m_backing_store->view_matrix.a13 = 0.0;
	m_backing_store->view_matrix.a23 = 0.0;

	m_time += 0.01;

	m_backing_store->camera_position = nvmath::vec4f(m_position,m_time);

	m_view_projection_needs_update = false;
}

void VkeCamera::updateCameraCmd(VkCommandBuffer inCommand){

	vkCmdUpdateBuffer(inCommand, m_data.buffer, 0, sizeof(VkeCameraUniform), (const uint32_t *)&(m_backing_store[0]));

}

void VkeCamera::update(){
	updateTransform();
	updateProjection();
	updateViewProjection();
}

void VkeCamera::bind(VkCommandBuffer *inBuffer){}

void VkeCamera::setViewport(float inX, float inY, float inW, float inH){

	if ((m_viewport.z == inW) && (m_viewport.w == inH)) return;

	m_viewport.x = inX;
	m_viewport.y = inY;
	m_viewport.z = inW;
	m_viewport.w = inH;
	m_projection_needs_update = true;
}

void VkeCamera::setPosition(float inX, float inY, float inZ){
	m_position.x = inX;
	m_position.y = inY;
	m_position.z = inZ;

	m_transform_needs_update = true;
}
void VkeCamera::setRotation(float inX, float inY, float inZ){
	m_rotation.x = inX;
	m_rotation.y = inY;
	m_rotation.z = inZ;

	m_transform_needs_update = true;
}

void VkeCamera::setRotation(nvmath::quatf &inQuat){
	nvmath::vec3f angles;
	inQuat.to_euler_xyz(angles);
	setRotation(angles.x, angles.y, angles.z);

	m_transform_needs_update = true;
}

void VkeCamera::setNear(float inNear){
	m_near = inNear;
	m_projection_needs_update = true;
}

void VkeCamera::setFar(float inFar){
	m_far = inFar;
	m_projection_needs_update = true;
}

void VkeCamera::setFOV(float inFOV){
	m_fov = inFOV;
	m_projection_needs_update = true;
}

float VkeCamera::getNear(){
	return m_near;
}

float VkeCamera::getFar(){
	return m_far;
}

float VkeCamera::getFOV(){
	return m_fov;
}

nvmath::vec4f VkeCamera::worldPosition(){
    nvmath::vec4f zero(0.0,0.0,0.0,1.0);
    return worldPosition(zero);
}

nvmath::vec4f VkeCamera::worldPosition(nvmath::vec4f &inPosition){
	return m_transform(inPosition);
}

void VkeCamera::lookAt(nvmath::vec4f &inPos){
	m_use_look_at = true;
	nvmath::mat4f camView;
	m_look_at_matrix.identity();

	

	m_look_at_matrix = nvmath::look_at(nvmath::vec3f(inPos) - m_position, nvmath::vec3f(inPos), nvmath::vec3f(0, 1, 0));
    nvmath::vec3f zro(0.0f);
    m_position = m_look_at_matrix.get_translation(zro);


}

void VkeCamera::setLookAtMatrix(nvmath::mat4f &inMat){
	m_look_at_matrix = inMat;
	nvmath::mat4f inv = nvmath::invert(inMat);
	m_position.x = inv.a03;
	m_position.y = inv.a13;
	m_position.z = inv.a23;

	m_use_look_at = true;
}
