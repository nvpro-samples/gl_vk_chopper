/*
 * Copyright (c) 2014-2021, NVIDIA CORPORATION.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-FileCopyrightText: Copyright (c) 2014-2021 NVIDIA CORPORATION
 * SPDX-License-Identifier: Apache-2.0
 */

/* Contact chebert@nvidia.com (Chris Hebert) for feedback */

#include "vkaUtils.h"

#ifdef USE_LIB_PNG
#include "png.h"
#endif


void dumpGlobalLayerNames(VkLayerProperties* props, uint32_t count)
{

  LOGI("Found : %d layers.\n", count);
  for(uint32_t i = 0; i < count; ++i)
  {
    LOGI("Layer %d : %s.\n", i, props[i].layerName);
  }
}

bool loadTextFile(const char* filename, char** buffer, size_t& outSize)
{
  if(!buffer)
    return false;
  FILE* fp = NULL;

  fp = fopen(filename, "rb");
  if(!fp)
    return false;

  outSize = ftell(fp);
  fseek(fp, 0, SEEK_END);

  *buffer            = (char*)malloc(outSize + 1);
  (*buffer)[outSize] = '\0';
  rewind(fp);

  fread(*buffer, outSize, 1, fp);

  fclose(fp);
  return true;
}

#ifdef USE_LIB_PNG
bool loadTexture(const char* filename, uint8_t** rgba_data, uint32_t inRowPitch, int32_t* width, int32_t* height, bool doAlloc)
{
  //header for testing if it is a png
  png_byte    header[8];
  int         is_png, bit_depth, color_type, rowbytes;
  size_t      retval;
  png_uint_32 i, twidth, theight;
  png_structp png_ptr;
  png_infop   info_ptr, end_info;
  png_byte*   image_data;
  png_bytep*  row_pointers;

  //open file as binary
  FILE* fp;
  fp = fopen(filename, "rb");

  if(!fp)
  {
    return false;
  }

  //read the header
  retval = fread(header, 1, 8, fp);
  if(retval != 8)
  {
    fclose(fp);
    return false;
  }

  //test if png
  is_png = !png_sig_cmp(header, 0, 8);
  if(!is_png)
  {
    fclose(fp);
    return false;
  }

  //create png struct
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(!png_ptr)
  {
    fclose(fp);
    return (false);
  }

  //create png info struct
  info_ptr = png_create_info_struct(png_ptr);
  if(!info_ptr)
  {
    png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    fclose(fp);
    return (false);
  }

  //create png info struct
  end_info = png_create_info_struct(png_ptr);
  if(!end_info)
  {
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    fclose(fp);
    return (false);
  }

  //png error stuff, not sure libpng man suggests this.
  if(setjmp(png_jmpbuf(png_ptr)))
  {
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    fclose(fp);
    return (false);
  }

  //init png reading
  png_init_io(png_ptr, fp);

  //let libpng know you already read the first 8 bytes
  png_set_sig_bytes(png_ptr, 8);

  // read all the info up to the image data
  png_read_info(png_ptr, info_ptr);

  // get info about png
  png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type, NULL, NULL, NULL);

  //update width and height based on png info
  *width  = twidth;
  *height = theight;

  // Require that incoming texture be 8bits per color component
  // and 4 components (RGBA).
  if(png_get_bit_depth(png_ptr, info_ptr) != 8 || png_get_channels(png_ptr, info_ptr) != 4)
  {
    return false;
  }

  if(rgba_data == NULL)
  {

    if(doAlloc)
    {
      //(*rgba_data) = malloc();
    }

    // If data pointer is null, we just want the width & height
    // clean up memory and close stuff
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    fclose(fp);

    return true;
  }

  // Update the png info struct.
  png_read_update_info(png_ptr, info_ptr);

  // Row size in bytes.
  rowbytes = png_get_rowbytes(png_ptr, info_ptr);

  // Allocate the image_data as a big block, to be given to opengl
  image_data = (png_byte*)malloc(rowbytes * theight * sizeof(png_byte));
  if(!image_data)
  {
    //clean up memory and close stuff
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    fclose(fp);
    return false;
  }

  // row_pointers is for pointing to image_data for reading the png with libpng
  row_pointers = (png_bytep*)malloc(theight * sizeof(png_bytep));
  if(!row_pointers)
  {
    //clean up memory and close stuff
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    // delete[] image_data;
    fclose(fp);
    return false;
  }
  // set the individual row_pointers to point at the correct offsets of image_data
  for(i = 0; i < theight; ++i)
    row_pointers[theight - 1 - i] = *rgba_data + i * inRowPitch;

  // read the png into image_data through row_pointers
  png_read_image(png_ptr, row_pointers);

  // clean up memory and close stuff
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
  free(row_pointers);
  free(image_data);
  fclose(fp);

  return true;
}


#endif