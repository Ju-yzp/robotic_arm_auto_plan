#ifndef REALTIME_URDF_FILTER_RENDERABLE_HPP_
#define REALTIME_URDF_FILTER_RENDERABLE_HPP_

#include <glm/ext/vector_float3.hpp>
#include <glad/glad.h> // holds all OpenGL type declarations
#include <glm/glm.hpp>
//cpp
#include<string>
// #include<vector>
//assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/mesh.h>
//tf
#include <tf2/LinearMath/Transform.h>
//urdf
#include <urdf_model/color.h>
#include <realtime_urdf_filter/shader_wrapper.hpp>
namespace realtime_urdf_filter {
    struct Vertex
    {     
        glm::vec3 position;
        glm::vec3 normal;
        //glm::vec2 texCoord;
    };

    class Renderable
    {
        public:
        void applyTransfrom(Program &program);
        std::string name;
        tf2::Transform link_to_fixed;
        tf2::Transform link_offest;
        urdf::Color color;
        virtual void render() = 0;
        protected:
        unsigned indices_num;
        unsigned int vbo;
        unsigned int vao;
        unsigned int ebo;
    };

    class RenderableCylinder:public Renderable
    {
        public:
        RenderableCylinder(float h,float r);
        ~RenderableCylinder();
        virtual void render();
        float height,radius;
        protected:
        void createCylinder();
    };

    class RenderableSphere:public Renderable
    {
        public:
        RenderableSphere(float r);
        ~RenderableSphere();
        virtual void render();
        float radius;
        protected:
        void createSphere();
    };

    class RenderableBox:public Renderable
    {
        public:
        RenderableBox(float w,float h,float len);
        ~RenderableBox();
        virtual void render();
        float width;
        float height;
        float lenght;
        protected:
        void createBox();
    };
}

#endif

