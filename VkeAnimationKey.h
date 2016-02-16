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

#ifndef __H_VKE_ANIMATION_KEY_
#define __H_VKE_ANIMATION_KEY_

#pragma once

#include<nv_math/nv_math.h>
#include<vector>
#include<stdint.h>

struct VkeAnimationKeyPair;


class VkeAnimationKey
{
public:

	typedef uint32_t ID;
	typedef uint32_t Count;

	VkeAnimationKey(){}
	VkeAnimationKey(double &inTime,nv_math::vec4f &inData):
		m_time(inTime),
		m_value(inData){

	}
	~VkeAnimationKey(){}

	double &getTime();
	nv_math::vec4f &getValue();

	class List{
	public:
		List(){}
		~List(){}

		VkeAnimationKey *newKey(double &inTime, nv_math::vec4f &inData);

		VkeAnimationKey *getKey(const ID &inID);

		void getKeys(double &inTime, VkeAnimationKeyPair *outPair);

		std::vector<VkeAnimationKey*> m_data;

	};


private:
	
	nv_math::vec4f m_value;
	double m_time;
};


struct VkeAnimationKeyPair{
	VkeAnimationKey *low;
	VkeAnimationKey *high;

	VkeAnimationKeyPair() :
		low(NULL), high(NULL){}

};


#endif
