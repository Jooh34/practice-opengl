/** 
 * Copyright (C) 2018 Tomasz Gałaj
 **/

#include <stb_image.h>

#include "Texture.h"
#include <iostream>
#include <helpers/RootDir.h>

Texture::Texture()
    : use_linear(true), to_id(0)
{
}

Texture::~Texture()
{
    if(to_id != 0)
    {
        glDeleteTextures(1, &to_id);
        to_id = 0;
    }
}

bool Texture::load(const std::string & file_name, bool gamma_correction)
{
    if(file_name.empty())
    {
        return false;
    }

    bool is_loaded = false;
    int width, height, components;
    
    unsigned char* pixels = stbi_load((ROOT_DIR + file_name).c_str(), &width, &height, &components, 4);

    GLenum internalformat = GL_RGBA;
    GLenum dataFormat;
    if (components == 1)
        internalformat = GL_RED;
    else if (components == 3)
    {
        internalformat = gamma_correction ? GL_SRGB : GL_RGB;
        dataFormat = GL_RGB;
    }
    else if (components == 4)
    {
        internalformat = gamma_correction ? GL_SRGB_ALPHA : GL_RGBA;
        dataFormat = GL_RGBA;
    }

    if(pixels != nullptr)
    {
        glGenTextures(1, &to_id);
        glBindTexture(GL_TEXTURE_2D, to_id);
        glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, pixels);
        
        glTexStorage2D(GL_TEXTURE_2D, 2 /* mip map levels */, internalformat, width, height);
        glTexSubImage2D(GL_TEXTURE_2D, 0 /* mip map level */, 0 /* xoffset */, 0 /* yoffset */, width, height, internalformat, GL_UNSIGNED_BYTE, pixels);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, dataFormat == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, dataFormat == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
    else
    {
        std::cout << "Could not load file " << file_name << std::endl;
    }

    stbi_image_free(pixels);

    return is_loaded;
}

bool Texture::loadDepthMap(int width, int height)
{
	// configure depth map FBO
	// -----------------------
    FBOWidth = width;
    FBOHeight = height;
	glGenFramebuffers(1, &FBO);

	// create depth texture
	glGenTextures(1, &to_id);
	glBindTexture(GL_TEXTURE_2D, to_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, FBOWidth, FBOHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, to_id, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind

    return true;
}

bool Texture::loadColorFrameBuffer(int width, int height)
{
    // configure color FBO
        // -----------------------
    FBOWidth = width;
    FBOHeight = height;
    glGenFramebuffers(1, &FBO);

    // create color texture
    glGenTextures(1, &to_id);
    glBindTexture(GL_TEXTURE_2D, to_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, FBOWidth, FBOHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // depth attachment
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, FBOWidth, FBOHeight);

    // attach color buffer
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, to_id, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}

