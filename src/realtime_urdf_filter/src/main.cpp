// #include "glm/ext/matrix_transform.hpp"
// #include "glm/geometric.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <memory>
#include <rclcpp/executors.hpp>
#include <rclcpp/utilities.hpp>
#include <realtime_urdf_filter/shader_wrapper.hpp>
#include <realtime_urdf_filter/renderable.hpp>
#include <realtime_urdf_filter/framebufferObject.hpp>
#include <realtime_urdf_filter/urdf_renderable.hpp>

// #include <iostream>

const unsigned int width = 800;
const unsigned int height = 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

int main(int argc,char **argv)
{

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow *window = glfwCreateWindow(width, height, "test", NULL, NULL);



    if(window == NULL)
    {
        std::cout<<"Failed to create GLFW window"<<std::endl;
        glfwTerminate();
        return -1;
    }


    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    FrameBufferObject fbo("rgba=16 depth=16 ");
    fbo.initialize(width, height);
    // auto cylinder = std::make_unique<realtime_urdf_filter::RenderableBox>(1.0,1.0,1.0);

    Shader vert_s = Shader("/home/zy_jp/robotic_arm_auto_plan/src/realtime_urdf_filter/shader/vertex.glsl",Shader_type::VERTEX_SHADER);
    Shader frag_s = Shader("/home/zy_jp/robotic_arm_auto_plan/src/realtime_urdf_filter/shader/fragment.glsl",Shader_type::FRAGMENT_SHADER);
    vert_s.compileSatus();
    frag_s.compileSatus();
    std::vector<Shader> shaders = {vert_s,frag_s};
    Program program(shaders);
    program.linkSatus();

    // rclcpp::init(argc,argv);
    // auto node = std::make_shared<rclcpp::Node>("urdf_renderable");
    // auto renders = std::make_shared<realtime_urdf_filter::UrdfRenderable>(node);
    auto model_ = realtime_urdf_filter::RenderableMesh("/home/zy_jp/robotic_arm_auto_plan/src/realtime_urdf_filter/resources/backpack.obj");
    glEnable(GL_DEPTH_TEST);

    while(!glfwWindowShouldClose(window))
    {
        fbo.beginCapture();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        program.use();
        glm::mat4 view          = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        glm::mat4 projection    = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
        view       = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
        //pass transformation matrices to the shader
        program.setMat4("projection", projection); // note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
        program.setMat4("view", view);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(0.0f,0.0f,0.0f));
        float angle = 20.0f ;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        program.setMat4("model", model);
        model_.render();
        glBindFramebuffer(GL_READ_FRAMEBUFFER,fbo.getFrameBufferID());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
        glViewport(0,0,width,height);
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        // fbo.endCapture();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}