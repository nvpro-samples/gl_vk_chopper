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

#ifndef __H_VKE_TEXTURE_
#define __H_VKE_TEXTURE_


#include<vulkan/vulkan.h>
#include<map>
#include<vector>


#pragma once
class VkeTexture
{
public:
	typedef uint32_t ID;
	typedef std::map<VkeTexture::ID, VkeTexture *> Map;
	typedef uint32_t Count;

	VkeTexture();
	VkeTexture(const ID &inID);
	~VkeTexture();

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

		VkeTexture *newTexture();
		VkeTexture *newTexture(const VkeTexture::ID &inID);
		void addTexture(VkeTexture * const inData);
		VkeTexture *getTexture(const ID &inID);

		void update();

		ID nextID();
		Count count();

		void getData(VkeTexture::Data *outDescriptor, uint32_t offset = 0);


	private:
		VkeTexture::Map m_data;
		std::vector<VkeTexture::ID> m_deleted_keys;
	};


	void initTexture();

	void loadDDSTextureFile(const char *inFile);

#ifdef USE_LIB_PNG
	void loadTextureFile(const char *inFile);
#endif

	void loadTextureFloatData(float *inData, uint32_t inWidth, uint32_t inHeight, uint32_t inCompCount = 4);



	inline bool &isReady(){ return m_ready; }

	ID getID(){ return m_id; }
	void setID(const ID &inID){ m_id = inID; }

	inline void setFormat(VkFormat inFormat){ m_format = inFormat; }
	inline VkFormat &getFormat() { return m_format; }

	Data &getData(){
		return m_data;
	}

	inline int32_t getWidth(){ return m_width; }
	inline int32_t getHeight(){ return m_height; }


	
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
