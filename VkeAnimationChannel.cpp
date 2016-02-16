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

VkeAnimationKey *VkeAnimationChannel::newKey(double &inTime, nv_math::vec4f &inData){
	if (m_parent){
		m_parent->updateDuration(inTime);
	}
	return	m_keys.newKey(inTime, inData);
}

nv_math::vec4f cubicLerp(nv_math::vec4f inA, nv_math::vec4f inB, float inT){
	float c = (3.0 - 2.0 * inT) * inT * inT;

	inA *= (float)(1.0 - c);
	inB *= (float)c;
	nv_math::vec4f outValue = inA;
	outValue += inB;
	return outValue;
}

nv_math::quatf cubicLerp(nv_math::quatf inA, nv_math::quatf inB, float inT){
	float c = (3.0 - 2.0 * inT) * inT * inT;
	return nv_math::slerp_quats(c, inA, inB);
}

nv_math::quatf VkeAnimationChannel::currentQuatValue(){


	VkeAnimationKeyPair pair;
	double curTime = m_parent->getCurrentTime();
	m_keys.getKeys(curTime, &pair);

	//no keys found at all for this time.
	//Therefore there should not have been a channel
	//for this node in the first place.
    if (!pair.low && !pair.high) return nv_math::quatf();
	if (!pair.high){
		nv_math::vec4f vValue = pair.low->getValue();
		nv_math::quatf qValue(vValue.x, vValue.y, vValue.z, vValue.w);
		return qValue;

	}
	if (!pair.low){
		nv_math::vec4f vValue = pair.high->getValue();
		nv_math::quatf qValue(vValue.x, vValue.y, vValue.z, vValue.w);
		return qValue;

	}

	double timeDelta = pair.high->getTime() - pair.low->getTime();
    if (timeDelta == 0.0) return (nv_math::quatf)pair.low->getValue();

	double durationDelta = curTime - pair.low->getTime();

	double timeScale = durationDelta / timeDelta;

	nv_math::vec4f lowVal = pair.low->getValue();
	nv_math::vec4f highVal = pair.high->getValue();

	nv_math::quatf quatA(lowVal.x, lowVal.y, lowVal.z, lowVal.w);
	nv_math::quatf quatB(highVal.x, highVal.y, highVal.z, highVal.w);

	nv_math::quatf outQuat = cubicLerp(quatA, quatB, timeScale);

	return outQuat;
}

nv_math::vec4f VkeAnimationChannel::currentValue(){


	VkeAnimationKeyPair pair;
	double curTime = m_parent->getCurrentTime();
	m_keys.getKeys(curTime, &pair);

	//no keys found at all for this time.
	//Therefore there should not have been a channel
	//for this node in the first place.
	if (!pair.low && !pair.high) return nv_math::vec4f(0.0, 0.0, 0.0);
	if (!pair.high) return pair.low->getValue();
	if (!pair.low) return pair.high->getValue();

	double timeDelta = pair.high->getTime() - pair.low->getTime();
	if (timeDelta == 0.0) return pair.low->getValue();

	double durationDelta = curTime - pair.low->getTime();

	double timeScale = durationDelta / timeDelta;

	nv_math::vec4f lowVal = pair.low->getValue();
	nv_math::vec4f highVal = pair.high->getValue();

	nv_math::vec4f outVal = cubicLerp(lowVal, highVal, timeScale);

//	lowVal *= (float)(1.0 - timeScale);
//	highVal *= (float)timeScale;
//	highVal += lowVal;
	return outVal;
}

void VkeAnimationChannel::update(){

}
