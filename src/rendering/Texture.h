/** 
 * Copyright (C) 2018 Tomasz Gałaj
 **/

#pragma once
#include <string>
#include <glad/glad.h>

class Texture
{
public:
    Texture();
    ~Texture();

    bool load(const std::string & file_name);
    bool loadDepthMap(int width, int height);
    
    void bind(int index = 0) const
    {
        if(to_id != 0)
        {
            glActiveTexture(GL_TEXTURE0 + index);
            glBindTexture(GL_TEXTURE_2D, to_id);
        }
    }

    void bindFrameBuffer() const
    {
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glViewport(0, 0, ShadowWidth, ShadowHeight);
    }

    bool use_linear;

private:
    float ShadowWidth;
	float ShadowHeight;

    GLuint to_id;
    unsigned int FBO = 0;
};
