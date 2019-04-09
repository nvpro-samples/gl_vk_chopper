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

#ifndef __H_VKE_MATERIAL_
#define __H_VKE_MATERIAL_

#pragma once

#include<nvmath/nvmath.h>
#include"VkeBuffer.h"
#include<map>
#include<vector>
#include"MeshUtils.h"
#include"VkeTexture.h"

typedef struct VkeMaterialUniform{
	//nvmath::vec4f diffuseColor;
	//nvmath::vec4f ambientColor;
	//nvmath::vec4f specularColor;
	float reflectivity;
	float shininess;
	float opacity;

	float padding;
	float morepadding[48];

} VkeMaterialUniform;

class VKSFile;
class VKSMaterialRecord;

class VkeMaterial : public VkeBuffer<VkeMaterialUniform>
{
public:

	typedef uint32_t ID;
	typedef std::map<VkeMaterial::ID, VkeMaterial *> Map;
	typedef uint32_t Count;

	class List{
	public:
		List();
		~List();

		VkeMaterial *newMaterial();
		VkeMaterial *newMaterial(const VkeMaterial::ID &inID);
		void addMaterial(VkeMaterial * const inMaterial);
		VkeMaterial *getMaterial(const ID &inID);

		ID nextID();
		Count count();

		uint32_t getTextureCount();
		void getTextureData(VkeTexture::Data *outData);

		VkeMaterial *getDefault(){ 
			if (!m_default){

				m_default = new VkeMaterial(99999);
			}
			return m_default; 
		}



	private:


		VkeMaterial::Map m_data;
		std::vector<VkeMaterial::ID> m_deleted_keys;
		VkeMaterial *m_default;
	};


	VkeMaterial();
	VkeMaterial(const ID &inID);
	~VkeMaterial();

	void initMaterialData();
	void initMaterialDataSubAlloc();

	void bind(VkCommandBuffer *inBuffer);

	void initFromData(meshimport::MaterialDataf *inData);
	void initFromData(VKSFile *inFile, VKSMaterialRecord *inMaterial);
	void initWithDefaults();

	void updateVKBufferData(VkeMaterialUniform *inData);

	VkeTexture::List &getTextures(){ return m_textures; }

	ID getID(){ return m_id; }
	void setID(const ID &inID){ m_id = inID; }

private:
	ID m_id;

	VkeTexture::List m_textures;

};


#endif
