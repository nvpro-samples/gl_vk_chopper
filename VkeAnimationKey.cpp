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

#include "VkeAnimationKey.h"

double &VkeAnimationKey::getTime(){
	return m_time;
}

nv_math::vec4f &VkeAnimationKey::getValue(){
	return m_value;
}

VkeAnimationKey *VkeAnimationKey::List::newKey(double &inTime,nv_math::vec4f &inData){
	VkeAnimationKey *outKey = new VkeAnimationKey(inTime, inData);
	m_data.push_back(outKey);
	return outKey;
}

VkeAnimationKey *VkeAnimationKey::List::getKey(const ID &inID){
	return m_data[inID];
}

void VkeAnimationKey::List::getKeys(double &inTime, VkeAnimationKeyPair *outPair){


	//search the slow way first, then we'll do a binary search.
	uint32_t sz = m_data.size();

	for (uint32_t i = 0; i < sz; ++i){
		VkeAnimationKey *key = m_data[i];
		double keyTime = key->getTime();

		if (keyTime <= inTime) outPair->low = key;
		if (keyTime > inTime){
			outPair->high = key;
			return;
		}
	}
}
