//c style
#include <cassert>
#include <cstddef>
#include <cstring>
// neccessry header file
#include <realtime_urdf_filter/framebufferObject.hpp>
// cpp
#include <iostream>
#include <string>
#include <vector>

FrameBufferObject::~FrameBufferObject()
{
    if(initialized_)
    {
        if(colorAttachment_)
        {
            for(int i = 0 ; i < numClorAttachment_;i++)
            {
                if(colorAttachmentRenderTexture_)
                   glDeleteTextures(1,&colorAttachmentId_[i]);
                else
                   glDeleteRenderbuffers(1,&colorAttachmentId_[i]);
            }
        }

        if(depthAttachment_)
        {
            if(depthAttachRenderTexture_)
               glDeleteTextures(1,&depthAttachmentID_);
            else
               glDeleteRenderbuffers(1,&depthAttachmentID_);
        }

        glDeleteFramebuffers(1,&frameBufferID_);
    }
}

bool FrameBufferObject::initialize(unsigned int width,unsigned int height)
{
    if(!initialized_)
        return reinitialize(width,height);

    //set up width and height
    width_ = width;
    height_ = height;

    //create framebuffer object
    glGenFramebuffers(1,&frameBufferID_);
    glBindFramebuffer(GL_FRAMEBUFFER,frameBufferID_);

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
        if(!depthAttachment_)
           std::cout<<"ERROR: framebuffer object stencil attachment is currently only"<<std::endl;
        if(stencilAttachmentRenderTexture_)
        {
            glBindTexture(textureTarget_,depthAttachmentID_);

            glTexParameterf(textureTarget_,GL_TEXTURE_WRAP_S,wrapS_);
            glTexParameterf(textureTarget_,GL_TEXTURE_WRAP_S,wrapS_);
            glTexParameterf(textureTarget_,GL_TEXTURE_WRAP_S,wrapS_);
            glTexParameterf(textureTarget_,GL_TEXTURE_WRAP_S,wrapS_);
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
            
            // attach texture to framebuffer color buffer
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                                   GL_DEPTH_ATTACHMENT,
                                   textureTarget_,
                                   depthAttachmentID_,
                                   0);
        }

    }else
    {
        // initialize depth renderbuffer
        glBindRenderbuffer(GL_RENDERBUFFER,depthAttachmentID_);
        glRenderbufferStorage(GL_RENDERBUFFER,
                              GL_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER,
                              depthAttachmentID_);
    }

    // disable framebuffer again
    glBindFramebuffer(GL_FRAMEBUFFER,0);

    // set up status of framebuffer
    initialized_ = true;
    return true;
}

bool FrameBufferObject::reinitialize(unsigned int width,unsigned int height)
{

    // clear old configuration
    glDeleteFramebuffers(1,&frameBufferID_);
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
       glBindFramebuffer(GL_FRAMEBUFFER,frameBufferID_);

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

void FrameBufferObject::printFramebufferStatus()
{
    GLenum status_code = glCheckFramebufferStatus(frameBufferID_);

    switch(status_code)
    {
        case GL_FRAMEBUFFER_COMPLETE:
        std::cout << " framebuffer object error : GL_FRAMEBUFFER_COMPLETE"<<std::endl;
        break;

        case GL_FRAMEBUFFER_UNDEFINED:
        std::cout << " framebuffer object error : GL_FRAMEBUFFER_UNDEFINED"<<std::endl;
        break;

        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        std::cout << " framebuffer object error : GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"<<std::endl;
        break;

        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        std::cout << " framebuffer object error : GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"<<std::endl;
        break;

        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        std::cout << " framebuffer object error : GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"<<std::endl;
        break;

        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        std::cout << " framebuffer object error : GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"<<std::endl;
        break;

        case GL_FRAMEBUFFER_UNSUPPORTED:
        std::cout << " framebuffer object error : GL_FRAMEBUFFER_UNSUPPORTED"<<std::endl;
        break;

        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
        std::cout << " framebuffer object error : GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE"<<std::endl;
        break;
    }
}

void FrameBufferObject::parseModeString(const char *modeString)
{
    if(!modeString || strcmp(modeString, "") == 0)
       return ;

    colorAttachment_   = false;
    depthAttachment_   = false;
    stencilAttachment_ = false;

    floatColorBuffer_  = false;

    char *mode = strdupa(modeString);

    std::vector<std::string> tokens;
    char *token = strtok(mode," ");
    while(token)
    {
        tokens.push_back(token);
        token = strtok(NULL," ");
    }

    for(auto &token:tokens)
    {
        KeyVal kv = getKeyValuePair(token);
        //<--------------------------------------------------->//
        //<----------handle RGBA color attachment------------->//
        //<--------------------------------------------------->//
        if(strcmp(kv.first.c_str(), "rgba") == 0)
        {
            colorAttachment_ = true;
            colorFormat_     = GL_RGBA;
            colorType_       = GL_UNSIGNED_BYTE;
            
            minFilter_       = GL_LINEAR;
            magFilter_       = GL_LINEAR;

            // check if float texture is requested TODO:
            if(strchr(kv.second.c_str(), 't') != NULL)
               colorAttachmentRenderTexture_ = true;
            else
               colorAttachmentRenderTexture_ = false;

            if(kv.second.find("16") != kv.second.npos)
            {
                internalColorFormat_ = GL_RGBA16;
                colorType_           = GL_HALF_FLOAT;
                floatColorBuffer_    = true;
            }

            if(kv.second.find("32") != kv.second.npos)
            {

                // linear filter is not supported for 32 framebuffer objects
                floatColorBuffer_   = true;
            }

            /*<--------------------------------------------------->*/
            /*<----------check for mutiple render target---------->*/
            /*<--------------------------------------------------->*/
            for(int i = 0; i < 16; i++)
            {
                char *param ;
                if(kv.second.find(param) != kv.second.npos )
                   numClorAttachment_ =  i;
            }
        }
        else if(strcmp(kv.first.c_str(), "rgb") == 0)
        {
            colorAttachment_ = true;
            colorFormat_     = GL_RGB;
            colorType_       = GL_UNSIGNED_BYTE;

            minFilter_       = GL_LINEAR;
            magFilter_       = GL_LINEAR;

            // check if float texture is requested TODO:
            if(strchr(kv.second.c_str(), 't') != NULL)
               colorAttachmentRenderTexture_ = true;
            else
               colorAttachmentRenderTexture_ = false;

            if(kv.second.find("16") != kv.second.npos)
            {
                internalColorFormat_ = GL_RGBA16;
                colorType_           = GL_HALF_FLOAT;
                floatColorBuffer_    = true;
            }

            if(kv.second.find("32") != kv.second.npos)
            {

                // linear filter is not supported for 32 framebuffer objects
                floatColorBuffer_   = true;
            }

            /*<--------------------------------------------------->*/
            /*<----------check for mutiple render target---------->*/
            /*<--------------------------------------------------->*/
            for(int i = 0; i < 16; i++)
            {
                char *param ;
                if(kv.second.find(param) != kv.second.npos )
                   numClorAttachment_ =  i;
            }
            
        }
        //<--------------------------------------------------->//
        //<--------------handle depth attachment-------------->//
        //<--------------------------------------------------->//
        else if (strcmp(kv.first.c_str(), "depth") == 0) 
        {
            depthAttachment_     = true;
            depthFormat_         = GL_DEPTH_COMPONENT;
            depthType_           = GL_UNSIGNED_BYTE;

            internalDepthFormat_ = GL_DEPTH_COMPONENT24;

            if(kv.second.find("t") != kv.second.npos )
               depthAttachRenderTexture_ = true;
            else
               depthAttachRenderTexture_ = false;

            if(kv.second.find("16") != kv.second.npos)
              internalDepthFormat_ = GL_DEPTH_COMPONENT16;
            if(kv.second.find("24") != kv.second.npos)
              internalDepthFormat_ = GL_DEPTH_COMPONENT24;
            if(kv.second.find("32") != kv.second.npos)
              internalDepthFormat_ = GL_DEPTH_COMPONENT32;
        }
        //<--------------------------------------------------->//
        //<------------handle stencil attachment-------------->//
        //<--------------------------------------------------->//
        else if (strcmp(kv.first.c_str(), "stencil") == 0) 
        {
            stencilAttachment_ = true;
            if(kv.second.find("t") != kv.second.npos )
               stencilAttachmentRenderTexture_ = true;
            else
               stencilAttachmentRenderTexture_ = false;
        }
        //<--------------------------------------------------->//
        //<-----------------invaild parameter----------------->//
        //<--------------------------------------------------->//       
        else {
            std::cout<< "ERROR: invaild parameter"<<std::endl;
        }
    }
}

FrameBufferObject::KeyVal FrameBufferObject::getKeyValuePair(std::string token)
{
    std::string::size_type pos = 0;
    if((pos = token.find("=")) != token.npos)
    {
        return std::make_pair(token.substr(0,pos),token.substr(pos+1));
    }
    else
        return std::make_pair(token,"");
}