
#include "realtime_urdf_filter/shader_wrapper.hpp"
#include <cassert>
#include <cstddef>
#include <realtime_urdf_filter/framebufferObject.hpp>

bool FrameBufferObject::initialize(unsigned int width,unsigned int height)
{
    if(!initialized_)
        return reinitialize(width,height);

    //set up width and height
    width_ = width;
    height_ = height;

    //create framebuffer object
    glGenFramebuffers(1,&frameBufferId_);
    glBindFramebuffer(GL_FRAMEBUFFER,frameBufferId_);

    //<-------------------------------------------------------->//
    //<---------------------color attachment------------------->//
    //<-------------------------------------------------------->//

    if(colorAttachment_)
    {
        if(colorAttachmentRenderTexture_)
        {
            int count{0};
            bool successful{false};

            //initialize texture
            glGenTextures(1,&colorAttachmentId_[0]);

            do{

            }while( !(successful));

            //initialize multiple renders targets
            for(int i = 1; i < numClorAttachment_; i++)
            {
                glGenTextures(1,&colorAttachmentId_[i]);
                glBindTexture(textureTarget_,colorAttachmentId_[i]);

                glTextureParameteri(textureTarget_,GL_TEXTURE_WRAP_S,wrapS_);
                glTextureParameteri(textureTarget_,GL_TEXTURE_WRAP_S,wrapT_);
                glTextureParameteri(textureTarget_,GL_TEXTURE_MIN_FILTER,minFilter_);
                glTextureParameteri(textureTarget_,GL_TEXTURE_MAG_FILTER,magFilter_);
                glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);

                glTexImage2D(textureTarget_,
                             0,
                             internalColorFormat_,
                             width_,
                             height_,
                             0,
                             colorFormat_,
                             colorType_,
                             NULL
                );

                GLint colorAttachmentMacro = GL_COLOR_ATTACHMENT0 + i;

                //attach texture to framebuffer color buffer
                glFramebufferTexture2D( GL_FRAMEBUFFER,
                                        colorAttachmentMacro,

                                        textureTarget_,
                                        colorAttachmentId_[i],
                                        0);

            }
        }else
        {
            //initialize multiple render targets
            for(int i = 0;i < numClorAttachment_;i++)
            {
                glGenRenderbuffers(1,&colorAttachmentId_[i]);

                //initialize color renderbuffer
                glBindRenderbuffer(GL_RENDERBUFFER,colorAttachmentId_[i]);
                glRenderbufferStorage(GL_RENDERBUFFER,
                                      internalColorFormat_,
                                      width_,
                                      height_);

                GLint colorAttachmentMacro = GL_COLOR_ATTACHMENT0 + i;
                glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                          colorAttachmentMacro,
                                          GL_RENDERBUFFER,
                                          colorAttachmentId_[i]);
            }


        }
    }

    //<--------------------------------------------------------------------->//
    //<-------------------------depth attachment---------------------------->//
    //<--------------------------------------------------------------------->//

    if(depthAttachment_)
    {
        if(stencilAttachment_)
        {

        }

        if(depthAttachRenderTexture_)
        {
            // initialize depth texture
            glGenTextures(1,&depthAttachmentID_);
            glBindTexture(textureTarget_,depthAttachmentID_);

            glTexParameteri(textureTarget_,GL_TEXTURE_WRAP_S,wrapS_);
            glTexParameteri(textureTarget_,GL_TEXTURE_WRAP_T,wrapT_);
            glTextureParameteri(textureTarget_,GL_TEXTURE_MIN_FILTER,minFilter_);
            glTextureParameteri(textureTarget_,GL_TEXTURE_MAG_FILTER,magFilter_);
            glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);

            glTexImage2D(textureTarget_,
                          0,
                          internalDepthFormat_,
                          width_,
                          height_,
                          0,
                          depthFormat_,
                          depthType_,
                          NULL);

            glFramebufferTexture2D(GL_FRAMEBUFFER,
                         GL_DEPTH_ATTACHMENT,
                         textureTarget_,
                         depthAttachmentID_,
                         0);
        }else
        {
            glGenRenderbuffers(1,&depthAttachmentID_);

            //initialize depth renderbuffer
            glBindRenderbuffer(GL_RENDERBUFFER,depthAttachmentID_);
            glRenderbufferStorage(GL_RENDERBUFFER,
                                  internalDepthFormat_,
                                  width_,
                                  height_);
            
            // attach renderbuffer to farmebuffer depth buffer
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                      GL_DEPTH_ATTACHMENT,
                                      GL_RENDERBUFFER,
                                      depthAttachmentID_);
            
        }
    }

    //<------------------------------------------------------------------->//
    //<------------------------stencil attachment------------------------->//
    //<------------------------------------------------------------------->//

    if(stencilAttachment_)
    {

    }else
    {

    }

    initialized_ = true;
    return true;
}

bool FrameBufferObject::reinitialize(unsigned int width,unsigned int height)
{

    // clear old configuration
    glDeleteFramebuffers(1,&frameBufferId_);
    if(colorAttachment_)
    {
        if(colorAttachmentRenderTexture_)
           glDeleteTextures(1,&colorAttachmentId_[0]);
        else
           glDeleteRenderbuffers(1,&colorAttachmentId_[0]);
    }
    if(depthAttachment_)
    {
        if(depthAttachment_)
           glDeleteTextures(1,&depthAttachmentID_);
        else
           glDeleteRenderbuffers(1,&depthAttachmentID_);
    }

    // reset status and initialize again
    initialized_ = false;
    return initialize(width,height);
}


void FrameBufferObject::beginCapture(bool enablePassThroughShder )
{
    glGetIntegerv(GL_VIEWPORT,viewport_);
    glViewport(0,0,width_,height_);

    if(initialized_)
       glBindFramebuffer(GL_FRAMEBUFFER,frameBufferId_);

    if( enablePassThroughShder )
    {
        passThroughProgram_.use();
    }
}


void FrameBufferObject::endCapture(bool disablePassThroughShder )
{
    glViewport(viewport_[0],viewport_[1],viewport_[2],viewport_[3]);

    if(initialized_)
       glBindFramebuffer(GL_FRAMEBUFFER,0);
}