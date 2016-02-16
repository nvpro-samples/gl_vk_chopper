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

#include "VkeSceneAnimation.h"
#include<float.h>

VkeSceneAnimation::VkeSceneAnimation():
m_current_time(0.0),
m_duration(0.0),
m_start_time(DBL_MAX),
m_end_time(DBL_MIN)
{
}


VkeSceneAnimation::~VkeSceneAnimation()
{
}


double &VkeSceneAnimation::getDuration(){
	return m_duration;
}

double &VkeSceneAnimation::getStartTime(){
	return m_start_time;
}

double &VkeSceneAnimation::getEndTime(){
	return m_end_time;
}

VkeAnimationNode::List &VkeSceneAnimation::Nodes(){
	return m_nodes;
}

void VkeSceneAnimation::setCurrentTime(double &inTime){
	m_current_time = inTime;
}

double &VkeSceneAnimation::getCurrentTime(){
	return m_current_time;
}

void VkeSceneAnimation::update(){
	m_nodes.update();
}

VkeAnimationNode *VkeSceneAnimation::newNode(VkeAnimationNode::Name &inName){
	return m_nodes.newNode(inName, this);

}

void VkeSceneAnimation::updateDuration(double &inTime){
    m_start_time = std::min(m_start_time, inTime);
    m_end_time = std::max(m_end_time, inTime);
	m_duration = m_end_time - m_start_time;
}
