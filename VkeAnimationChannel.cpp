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

#include "VkeAnimationChannel.h"
#include "VkeSceneAnimation.h"


double &VkeAnimationChannel::getDuration(){
	return m_parent->getDuration();
}

VkeAnimationKey::List &VkeAnimationChannel::Keys(){
	return m_keys;
}

void VkeAnimationChannel::setParent(VkeSceneAnimation *inParent){
	m_parent = inParent;
}

VkeAnimationKey *VkeAnimationChannel::newKey(double &inTime, nvmath::vec4f &inData){
	if (m_parent){
		m_parent->updateDuration(inTime);
	}
	return	m_keys.newKey(inTime, inData);
}

nvmath::vec4f cubicLerp(nvmath::vec4f inA, nvmath::vec4f inB, float inT){
	float c = (3.0 - 2.0 * inT) * inT * inT;

	inA *= (float)(1.0 - c);
	inB *= (float)c;
	nvmath::vec4f outValue = inA;
	outValue += inB;
	return outValue;
}

nvmath::quatf cubicLerp(nvmath::quatf inA, nvmath::quatf inB, float inT){
	float c = (3.0 - 2.0 * inT) * inT * inT;
	return nvmath::slerp_quats(c, inA, inB);
}

nvmath::quatf VkeAnimationChannel::currentQuatValue(){


	VkeAnimationKeyPair pair;
	double curTime = m_parent->getCurrentTime();
	m_keys.getKeys(curTime, &pair);

	//no keys found at all for this time.
	//Therefore there should not have been a channel
	//for this node in the first place.
    if (!pair.low && !pair.high) return nvmath::quatf();
	if (!pair.high){
		nvmath::vec4f vValue = pair.low->getValue();
		nvmath::quatf qValue(vValue.x, vValue.y, vValue.z, vValue.w);
		return qValue;

	}
	if (!pair.low){
		nvmath::vec4f vValue = pair.high->getValue();
		nvmath::quatf qValue(vValue.x, vValue.y, vValue.z, vValue.w);
		return qValue;

	}

	double timeDelta = pair.high->getTime() - pair.low->getTime();
    if (timeDelta == 0.0) return (nvmath::quatf)pair.low->getValue();

	double durationDelta = curTime - pair.low->getTime();

	double timeScale = durationDelta / timeDelta;

	nvmath::vec4f lowVal = pair.low->getValue();
	nvmath::vec4f highVal = pair.high->getValue();

	nvmath::quatf quatA(lowVal.x, lowVal.y, lowVal.z, lowVal.w);
	nvmath::quatf quatB(highVal.x, highVal.y, highVal.z, highVal.w);

	nvmath::quatf outQuat = cubicLerp(quatA, quatB, timeScale);

	return outQuat;
}

nvmath::vec4f VkeAnimationChannel::currentValue(){


	VkeAnimationKeyPair pair;
	double curTime = m_parent->getCurrentTime();
	m_keys.getKeys(curTime, &pair);

	//no keys found at all for this time.
	//Therefore there should not have been a channel
	//for this node in the first place.
	if (!pair.low && !pair.high) return nvmath::vec4f(0.0, 0.0, 0.0);
	if (!pair.high) return pair.low->getValue();
	if (!pair.low) return pair.high->getValue();

	double timeDelta = pair.high->getTime() - pair.low->getTime();
	if (timeDelta == 0.0) return pair.low->getValue();

	double durationDelta = curTime - pair.low->getTime();

	double timeScale = durationDelta / timeDelta;

	nvmath::vec4f lowVal = pair.low->getValue();
	nvmath::vec4f highVal = pair.high->getValue();

	nvmath::vec4f outVal = cubicLerp(lowVal, highVal, timeScale);

//	lowVal *= (float)(1.0 - timeScale);
//	highVal *= (float)timeScale;
//	highVal += lowVal;
	return outVal;
}

void VkeAnimationChannel::update(){

}
