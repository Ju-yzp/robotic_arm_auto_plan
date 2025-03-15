#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/vector_float3.hpp>
//
#include <realtime_urdf_filter/renderable.hpp>
//
#include <tf2/LinearMath/Quaternion.hpp>
#include <tf2/LinearMath/Scalar.h>
#include <tf2/LinearMath/Transform.h>
#include <tf2/LinearMath/Transform.hpp>
#include <tf2/LinearMath/Vector3.hpp>
#include <vector>
#include <cassert>
#include <cmath>
#include <iostream>
//assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// #include<iostream>
// #include <iomanip>
namespace realtime_urdf_filter {
    void Renderable::applyTransfrom(Program &program)
    {        
        tf2::Transform tf(link_to_fixed);
        tf = tf * link_offest;
        // model coordiate define: z up,x right,y inside
        // opengl cooridate define: x right,y up,z forward
        // so we need to tarnfrom from the rviz2 coordiate to opengl coordinate
        double glTf[16];
        tf.getOpenGLMatrix(glTf);
        glm::mat4 model = glm::make_mat4(glTf);
        program.setMat4("model", model);
    }

    RenderableCylinder::RenderableCylinder(float h,float r)
    :height(h),
    radius(r)
    {
        createCylinder();
    }

    RenderableCylinder::~RenderableCylinder()
    {
        glDeleteVertexArrays(1,&vao);
        glDeleteBuffers(1,&vbo);
        glDeleteBuffers(1,&ebo);
    }

    void RenderableCylinder::createCylinder()
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        constexpr unsigned int  num = 36;
        const float step = 2.0f * M_PIf /num;

        glm::vec3 position;
        glm::vec3 normal;
        float angle;

        vertices.reserve(num * 2 + 4);
        indices.reserve(num * 3);
        //get all points that consist of each of triangle
        const float half_height = height / 2.0f;
        for(unsigned int count = 0;count < num;count++)
        {
          angle = step * count;
          position.x = radius * cos(angle);
          position.z = half_height ;
          position.y = radius * sin(angle);

          normal.x = cos(angle);
          normal.y = 0.0f;
          normal.z = sin(angle);

          vertices.push_back(Vertex{position,normal});
        }

        for(unsigned int count = 0;count < num;count++)
        {
          angle = step * count;
          position.x = radius * cos(angle);
          position.z = -half_height;
          position.y = radius * sin(angle);

          normal.x = cos(angle);
          normal.y = 0.0f;
          normal.z = sin(angle);

          vertices.push_back(Vertex{position,normal});
        }
        //fill in 
        glm::vec3 top_center{0.0f,height,0.f},buttom_center{0.0f,0.0f,0.0f};
        vertices.push_back(Vertex{top_center,normal});
        vertices.push_back(Vertex{buttom_center,normal});

        //cylinder top
        for(unsigned int i = 0 ; i < num - 1; i ++)
        {
            indices.push_back(i);
            indices.push_back(i+1);
            indices.push_back(num * 2 );
        }
        //cylinder buttom
        for(unsigned int i = 0 ; i < num - 1; i ++)
        {
            indices.push_back(num+i);
            indices.push_back(num+i+1);
            indices.push_back(num * 2 + 1 );
        }
        //cylinder side
        for(unsigned int i = 0 ; i < num - 1; i ++)
        {

            indices.push_back(i);
            indices.push_back(num+i);
            indices.push_back(num+i+1);

            indices.push_back(num+i+1);
            indices.push_back(i);
            indices.push_back(i+1);
        }

        indices_num = static_cast<unsigned int>(indices.size());
        //send data fronm cpu to gpu
        glGenVertexArrays(1,&vao);
        glGenBuffers(1,&vbo);
        glGenBuffers(1,&ebo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(Vertex),vertices.data(),GL_STATIC_DRAW);

        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,indices.size()*sizeof(unsigned int),indices.data(),GL_STATIC_DRAW);

        
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void *)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    void RenderableCylinder::render()
    {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES,indices_num,GL_UNSIGNED_INT,0);
        glBindVertexArray(0);
    }

    RenderableSphere::RenderableSphere(float r)
    :radius(r)
    {
        // createSphere();
    }

    RenderableSphere::~RenderableSphere()
    {
        glDeleteVertexArrays(1,&vao);
        glDeleteBuffers(1,&vbo);
        glDeleteBuffers(1,&ebo);
    }

    void RenderableSphere::render()
    {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES,indices_num,GL_UNSIGNED_INT,0);
        glBindVertexArray(0); 
    };

    void RenderableSphere::createSphere()
    {
        std::vector<Vertex> sphere_vertices;
        std::vector<unsigned int> sphere_indices;

        glm::vec3 position,normal;

        constexpr unsigned int stackNum= 36;
        constexpr unsigned int sectorNum = 20;

        const float sectorStep = 2 * M_PIf / sectorNum;
        const float stackStep = M_PIf / stackNum;
        const float lenInv = 1.0f / radius;
        float sectorAngle,stackAngle,xy;

        //fill in all points that consist of sphere 
        sphere_vertices.push_back(Vertex{glm::vec3(0.0f,0.0f,radius),glm::vec3(0.0f,0.0f,1.0f)});

        for(unsigned int i = 1; i < stackNum; i++)
        {
            stackAngle = M_PIf / 2.0f - i * stackStep;
            position.z = radius * sinf(stackAngle);
            xy = radius * cosf(stackAngle);
            for(unsigned int j = 0; j <= sectorNum; j++)
            {
                sectorAngle = j * sectorStep;
                position.x = xy * cosf(sectorAngle);
                position.y = xy * sinf(sectorAngle);

                normal.x = position.x * lenInv;
                normal.y = position.y * lenInv;
                normal.z = position.z * lenInv;
                sphere_vertices.push_back(Vertex{position,normal});
            }
        }
        sphere_vertices.push_back(Vertex{glm::vec3(0.0f,0.0f,-radius),glm::vec3(0.0f,0.0f,-1.0f)});

        for(unsigned int i = 1 ;i <= sectorNum;i++)
        {
            sphere_indices.push_back(0);
            sphere_indices.push_back(i);
            sphere_indices.push_back((i+1)%sectorNum + 1);

            sphere_indices.push_back(sphere_vertices.size()-1);
            sphere_indices.push_back(sphere_vertices.size() - i -1);
            sphere_indices.push_back(sphere_vertices.size() - ((i + 1) % sectorNum )-2);
        }

        for (unsigned int i = 0; i < stackNum - 1; i++) {
            for (unsigned int j = 0; j < sectorNum; j++) {
                unsigned int first = (i * (sectorNum + 1)) + j + 1; 
                unsigned int second = first + 1; 
                unsigned int third = ((i + 1) * (sectorNum + 1)) + j + 1; 
                unsigned int fourth = third + 1;
                
                sphere_indices.push_back(first);
                sphere_indices.push_back(third);
                sphere_indices.push_back(second);

                sphere_indices.push_back(second);
                sphere_indices.push_back(third);
                sphere_indices.push_back(fourth);
            }
        }

        indices_num  = static_cast<unsigned int>(sphere_indices.size());
        glGenVertexArrays(1,&vao);
        glGenBuffers(1,&vbo);
        glGenBuffers(1,&ebo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,sphere_vertices.size() * sizeof(Vertex),sphere_vertices.data(),GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,indices_num * sizeof(unsigned int),sphere_indices.data(),GL_STATIC_DRAW);

        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void *)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    RenderableBox::RenderableBox(float len,float w,float h)
    :width(w),
    height(h),
    lenght(len)
    {
        createBox();
    }

    RenderableBox::~RenderableBox()
    {
        glDeleteVertexArrays(1,&vao);
        glDeleteBuffers(1,&vbo);
        glDeleteBuffers(1,&ebo);
    }

    void RenderableBox::createBox()
    {
        std::vector<Vertex> box_vertices;

        const float half_height = height / 2.0f;
        const float half_width = width / 2.0f;
        const float half_lenght = lenght / 2.0f;

        glm::vec3 normal(0.0f,0.0f,0.0f);
        box_vertices.push_back(Vertex{glm::vec3(-half_lenght,-half_width,half_height),normal});     //front right top
        box_vertices.push_back(Vertex{glm::vec3(-half_lenght,half_width,half_height),normal});    //front left  top
        box_vertices.push_back(Vertex{glm::vec3(-half_lenght,-half_width,-half_height),normal});    //front right buttom
        box_vertices.push_back(Vertex{glm::vec3(-half_lenght,half_width,-half_height),normal});   //front left  buttom
        box_vertices.push_back(Vertex{glm::vec3(half_lenght,-half_width,half_height),normal});      //back  right top
        box_vertices.push_back(Vertex{glm::vec3(half_lenght,half_width,half_height),normal});     //back  left  top
        box_vertices.push_back(Vertex{glm::vec3(half_lenght,-half_width,-half_height),normal});     //back  right buttom
        box_vertices.push_back(Vertex{glm::vec3(half_lenght,half_width,-half_height),normal});    //back  left  buttom
 
        unsigned int box_indices[] = {
            0,1,3,
            0,2,3,   //front
            4,5,6,
            7,5,6, //back
           0,1,4,
           5,1,4, //top
           6,7,2,
           3,7,2, //buttom
           0,4,2,
           6,4,2, //right
           5,1,7,
           2,1,7  //left

        };
        glGenVertexArrays(1,&vao);
        glGenBuffers(1,&vbo);
        glGenBuffers(1,&ebo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,box_vertices.size()*sizeof(Vertex),box_vertices.data(),GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,36 * sizeof(unsigned int),box_indices,GL_STATIC_DRAW);

        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void *)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    void RenderableBox::render()
    {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_INT,0);
        glBindVertexArray(0);       
    }

    RenderableMesh::RenderableMesh(const std::string &path)
    {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(path,aiProcess_Triangulate | aiProcess_FlipUVs);
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
            return;
        }
        processNode(scene->mRootNode, scene);
    }

    RenderableMesh::~RenderableMesh()
    {
        for(auto &mesh:meshes)
        {
            glDeleteVertexArrays(1,&mesh.vao);
            glDeleteBuffers(1,&mesh.vbo);
            glDeleteBuffers(1,&mesh.ebo);
        }
    }

    void RenderableMesh::processNode(aiNode *node,const aiScene *scene)
    {
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            processMesh(mesh, scene);
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }

    void RenderableMesh::processMesh(aiMesh *mesh, const aiScene *scene)
    {
        static const float scale = 0.6f;
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // positions
            vector.x = mesh->mVertices[i].x * scale;
            vector.y = mesh->mVertices[i].y * scale;
            vector.z = mesh->mVertices[i].z * scale;
            vertex.position = vector;
            // normals
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.normal = vector;
            }
            vertices.push_back(vertex);
        } 
        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for(unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);        
        }
        Mesh m;
        m.initMesh(vertices,indices);
        meshes.emplace_back(m);
    }

    void RenderableMesh::render()
    {
        for(auto &mesh:meshes)
        {
            glBindVertexArray(mesh.vao);
            glDrawElements(GL_TRIANGLES,mesh.indices_num,GL_UNSIGNED_INT,0);
            glBindVertexArray(0);
        }
    }

    void RenderableMesh::Mesh::initMesh(std::vector<Vertex> &vertices,std::vector<unsigned int> &indices)
    {
        indices_num = static_cast<unsigned int>(indices.size());
        glGenVertexArrays(1,&vao);
        glGenBuffers(1,&vbo);
        glGenBuffers(1,&ebo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(Vertex),vertices.data(),GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,indices.size()*sizeof(unsigned int),indices.data(),GL_STATIC_DRAW);

        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void *)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }
}