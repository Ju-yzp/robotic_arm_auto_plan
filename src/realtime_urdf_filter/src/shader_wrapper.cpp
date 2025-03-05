#include "realtime_urdf_filter/renderable.hpp"
#include <realtime_urdf_filter/shader_wrapper.hpp>
//stream
#include <fstream>
#include <iostream>
#include <sstream>

Shader::Shader(const char *shaderFilePath,const Shader_type type)
{
    std::string shaderCode;
    std::ifstream file;
    try
    {
        file.open(shaderFilePath);
        std::stringstream shaderStream;
        shaderStream << file.rdbuf();
        file.close();
        shaderCode = shaderStream.str();
    }
    catch(std::ifstream::failure &error)
    {
        std::cout<<"error occur :"<<error.what()<<std::endl;
    }

    //create and compile shader
    const char * sourceCode = shaderCode.c_str();
    if(type == Shader_type::VERTEX_SHADER)
       shader_id = glCreateShader(GL_VERTEX_SHADER);
    else if( type == Shader_type::FRAGMENT_SHADER)
        shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(shader_id,1,&sourceCode,NULL);
    glCompileShader(shader_id);
    compileSatus();
}


bool Shader::compileSatus()
{
    int success;
    glGetShaderiv(shader_id,GL_COMPILE_STATUS,&success);
    if(success)
       return true;
    else 
    {
        char infoLog[1024];
        glGetShaderInfoLog(shader_id,1024,NULL,infoLog);
        std::cout<<"error: "<<std::string(infoLog)<<std::endl;
        return false;
    }
}

Program::Program(const std::vector<Shader> &shaders)
{
    program_id = glCreateProgram();

    for(auto &shader:shaders)
    {
        glAttachShader(program_id,shader.get_id());
    }

    glLinkProgram(program_id);
    linkSatus();
    for(auto &shader:shaders)
    {
        glDeleteShader(shader.get_id());
    }
}

Program::~Program()
{
    glDeleteProgram(program_id);
}

void Program::use()
{//when start work ,we need to use program
    glUseProgram(program_id);
}

bool Program::linkSatus()
{//check wether program is link rightly 
    int suceess;
    glGetProgramiv(program_id,GL_LINK_STATUS,&suceess);
    if(suceess)
       return true;
    else 
    {
        char infoLog[1024];
        glGetProgramInfoLog(program_id,1024,NULL,infoLog);
        std::cout<<"link error: "<<infoLog<<std::endl;
        return false;
    }
}

void Program::setMat4(const std::string name,const glm::mat4 value)
{
    glUniformMatrix4fv(glGetUniformLocation(program_id,name.c_str()),1,GL_FALSE,&value[0][0]);
}

void Program::setVec4(const std::string name,const glm::vec4 value)
{
    glUniform4fv(glGetUniformLocation(program_id,name.c_str()),1,&value[0]);
}
