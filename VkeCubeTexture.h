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

#ifndef __H_VKE_CUBE_TEXTURE_
#define __H_VKE_CUBE_TEXTURE_

#include<vulkan/vulkan.h>
#include<map>
#include<vector>


#pragma once


#pragma once
class VkeCubeTexture
{
public:
	typedef uint32_t ID;
	typedef std::map<VkeCubeTexture::ID, VkeCubeTexture *> Map;
	typedef uint32_t Count;

	VkeCubeTexture();
	VkeCubeTexture(const ID &inID);
	~VkeCubeTexture();

	struct Data{
		VkSampler sampler;
		VkImage image;
		VkImageLayout imageLayout;
		VkDeviceMemory memory;
		VkImageView view;
	};

	class List{
	public:
		List();
		~List();

		VkeCubeTexture *newTexture();
		VkeCubeTexture *newTexture(const VkeCubeTexture::ID &inID);
		void addTexture(VkeCubeTexture * const inData);
		VkeCubeTexture *getTexture(const ID &inID);

		void update();

		ID nextID();
		Count count();

		void getData(VkeCubeTexture::Data *outDescriptor, uint32_t offset = 0);


	private:
		VkeCubeTexture::Map m_data;
		std::vector<VkeCubeTexture::ID> m_deleted_keys;
	};


	void initTexture();

	void loadCubeDDS(const char *inPath);
#ifdef USE_LIB_PNG
	void loadTextureFiles(const char **inFile);
#endif

	inline bool &isReady(){ return m_ready; }

	ID getID(){ return m_id; }
	void setID(const ID &inID){ m_id = inID; }

	Data &getData(){
		return m_data;
	}


private:

	ID m_id;

	int32_t m_width;
	int32_t m_height;
	bool m_ready;

	VkImageTiling m_tiling;
	VkImageUsageFlags  m_usage_flags;
	VkFlags m_memory_flags;
	VkFormat m_format;

	Data m_data;
	uint32_t m_mip_level;

};

#endif

