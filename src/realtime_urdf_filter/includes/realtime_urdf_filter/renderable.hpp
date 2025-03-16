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
#include <tf2/LinearMath/Quaternion.hpp>
#include <tf2/LinearMath/Transform.h>
//urdf
#include <tf2/LinearMath/Transform.hpp>
#include <tf2/LinearMath/Vector3.hpp>
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
        tf2::Transform link_to_fixed = 
        tf2::Transform(tf2::Quaternion(0.0,0.0,0.0,1.0f),tf2::Vector3(0.0f,0.0f,0.0f));
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
        void render();
        float height,radius;
        protected:
        void createCylinder();
    };

    class RenderableSphere:public Renderable
    {
        public:
        RenderableSphere(float r);
        ~RenderableSphere();
        void render();
        float radius;
        protected:
        void createSphere();
    };

    class RenderableBox:public Renderable
    {
        public:
        RenderableBox(float w,float h,float len);
        ~RenderableBox();
        void render();
        float width;
        float height;
        float lenght;
        protected:
        void createBox();
    };

    class RenderableMesh:public Renderable
    {
        public:
        RenderableMesh(const std::string &path,double scale);
        ~RenderableMesh();
        void render();
        protected:
        struct Mesh
        {
            void initMesh(std::vector<Vertex> &vertices,std::vector<unsigned int> &indices);
            std::vector<Vertex> vertices;
            std::vector<unsigned int> indices;
            unsigned int indices_num;
            unsigned int vbo;
            unsigned int vao;
            unsigned int ebo;
        };
        std::vector<Mesh> meshes;
        double scale_;
        void processNode(aiNode *node,const aiScene *scene);
        void processMesh(aiMesh *mesh, const aiScene *scene);

    };
}

#endif

