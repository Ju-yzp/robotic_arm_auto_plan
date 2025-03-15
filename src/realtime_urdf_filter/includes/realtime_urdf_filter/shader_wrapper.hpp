#ifndef REALTIME_URDF_FILTER_SHADER_WRAPPER_HPP_
#define REALTIME_URDF_FILTER_SHADER_WRAPPER_HPP_
//gl
#include <glad/glad.h>
#include <GLFW/glfw3.h>
//glm
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
//cpp
#include <vector>
#include <string>
enum class Shader_type 
{
    VERTEX_SHADER = 0,
    FRAGMENT_SHADER = 1,
    GEOMETRY_SHADER = 2
};

class Shader
{
    public:

    Shader(const char *shaderFilePath,const Shader_type type);
    
    bool compileSatus();

    unsigned int get_id()const{return shader_id;};

    private:

    unsigned int shader_id;
};

class Program
{
    public:
    Program (const std::vector<Shader> &shaders);

    Program (Program &other) = delete;

    ~Program();

    void use();

    bool linkSatus();

    void setMat4(const std::string name,const glm::mat4 value);

    void setVec4(const std::string name,const glm::vec4 value);

    void setVal1i(const std::string name,const int value);

    void setVal1f(const std::string name,const float value);

    private:
    unsigned int program_id;
};
#endif