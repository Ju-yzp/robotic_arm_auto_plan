#ifndef  REALTIME_URDF_FILTER_FRAMEBUFFER_OBJECT_HPP_
#define  REALTIME_URDF_FILTER_FRAMEBUFFER_OBJECT_HPP_

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <realtime_urdf_filter/shader_wrapper.hpp>
#include <utility>

#define MAX_COLOR_COMPONENTS 16

class FrameBufferObject
{
    public:
        // construct function
        FrameBufferObject();

        ~FrameBufferObject();

        // initialize the framebuffers and textures
        bool           initialize(unsigned int width,unsigned int height);

        bool           reinitialize(unsigned int width,unsigned int height);

        void           beginCapture(bool enablePassThroughShder = true);

        void           endCapture(bool disablePassThroughShder = true);

        // get width of the framebuffer
        unsigned int   getWidth(){return width_; }

        // get height of the framebuffer
        unsigned int   getHeight(){return height_;}

        // get the texture id of the depth attachment
        GLuint         getDepthAttachment(){ return depthAttachmentID_; }

        // get the texture id of the color attachment
        GLuint         getColorAttachment(const int index){ return colorAttachmentId_[index]; }

        // get the texture id of the stencil attachment
        GLuint         getStencilAttachment(){ return stencilAttachID_; }      

        // set the wrap s
        void           setWrapS(GLint wrapS){} 

        // set the wrap t
        void           setWrapT(GLint wrapT){} 

        void		   printFramebufferStatus();

    protected:
        // texture target, default: GL_TEXTURE_RECTANGLE_ARB
        GLenum          textureTarget_;

        // format of the color texture, default:GL_RGBA
        GLint           colorFormat_;

        // internal format of the depth texture, default: GL_RGBA
        GLint           internalColorFormat_;

        // type of the color attachment, defualt: GL_UNSIGNED_BYTE
        GLenum          colorType_;

        // format of the depth textures,default: GL_DEPTH_COMPONENT
        GLenum          depthFormat_;

        // internal format of the depth texture, default: GL_DEPTH_COMPONENT24
        // other options:GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT32
        GLenum          internalDepthFormat_;

        // type of the depth attachment, default: GL_UNSIGNED_BYTE
        GLenum          depthType_;

        // wrap s parameter for color attachments, default: GL_CLAMP_TO_EDGE
        GLint           wrapS_;

        // wrap T parameter for color attachments, default: GL_CLAMP_TO_EDGE
        GLint           wrapT_;

        // min filterfor texture interpolation, default: GL_LINEAR
        GLint           minFilter_;

        // max filterfor texture interpolation, default: GL_LINEAR
        GLint           magFilter_;

        // width and height offramebuffer 
        unsigned int    width_;
        unsigned int    height_;

        // framebuffer initialized ?
        bool            initialized_;

        // use color attachment?
        bool            colorAttachment_;

        // use color as renderTexture ?
        bool            colorAttachmentRenderTexture_;

        // use depth attachment ?
        bool            depthAttachment_;

        // use depth as renderTexture ?
        bool            depthAttachRenderTexture_;

        // use stencil attachment ?
        bool            stencilAttachment_;

        // use stencil as renderTexture ?
        bool            stencilAttachmentRenderTexture_;

        // internal stencil buffer format
        int             internalStencilFormat_;

        // pass through shader program initialized ?
        bool            passThroughProgramInitialized_;

        // id of framebuffer
        GLuint          frameBufferID_;

        // texture ids of the color attachment textures
        GLuint          colorAttachmentId_[MAX_COLOR_COMPONENTS];

        // number of color attachments
        GLint           numClorAttachment_;

        // texture id of the depth attachment
        GLuint          depthAttachmentID_;

        // texture id of the stencil attachment
        GLuint          stencilAttachID_;

        // save viewport before setting new one to restore it later
        GLint           viewport_[4];

        Program         passThroughProgram_;

        // indicates if color buffer is a float texture
        bool            floatColorBuffer_;

        private:

        /// parse the mode string and set configuration
        void			parseModeString(const char *modeString);

        typedef std::pair<std::string, std::string> KeyVal;
        /// get the key=value pair of a single token from the mode string
        KeyVal			getKeyValuePair(std::string token);
};
#endif