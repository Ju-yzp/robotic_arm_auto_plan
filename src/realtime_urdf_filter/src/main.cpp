// #include "glm/ext/matrix_transform.hpp"
// #include "glm/geometric.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glm/trigonometric.hpp>
#include <memory>
#include <rclcpp/executors.hpp>
#include <rclcpp/utilities.hpp>
#include <realtime_urdf_filter/shader_wrapper.hpp>
#include <realtime_urdf_filter/renderable.hpp>
#include <realtime_urdf_filter/framebufferObject.hpp>
#include <realtime_urdf_filter/urdf_renderable.hpp>
#include <string>
#include <unordered_set>

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

    Shader vert_s = Shader("/home/zy_jp/robotic_arm_auto_plan/src/realtime_urdf_filter/shader/vertex.glsl",Shader_type::VERTEX_SHADER);
    Shader frag_s = Shader("/home/zy_jp/robotic_arm_auto_plan/src/realtime_urdf_filter/shader/test.frag",Shader_type::FRAGMENT_SHADER);
    vert_s.compileSatus();
    frag_s.compileSatus();
    std::vector<Shader> shaders = {vert_s,frag_s};
    Program program(shaders);
    program.linkSatus();

    rclcpp::init(argc,argv);
    auto node = std::make_shared<rclcpp::Node>("urdf_renderable");

    std::unordered_set<std::string> ignore_links;
    ignore_links.insert("world");
    std::string fixed_frame = "world";
    std::string geometry_type = "visual";
    std::string model_description = "/home/zy_jp/robotic_arm_auto_plan/src/realtime_urdf_filter/urdf/a0912.urdf";
    auto renders = std::make_shared<realtime_urdf_filter::UrdfRenderable>(
         model_description,"camera_link",fixed_frame,geometry_type,1.0,ignore_links,node.get());

    glEnable(GL_DEPTH_TEST);

    while(!glfwWindowShouldClose(window))
    {
        // fbo.beginCapture();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        program.use();

        glm::mat4 rostogl = glm::mat4( 0.0f,0.0f,-1.0f,0.0f,
                                       -1.0f, 0.0f,0.0f,0.0f,
                                      0.0f, 1.0f,0.0f,0.0f,
                                       0.0f, 0.0f,0.0f,1.0f);

        glm::mat4 scale = glm::mat4( 0.001f,0.0f,0.0f,0.0f,
                                        0.0f, 0.001f,0.0f,0.0f,
                                       0.0f, 0.0f,0.001f,0.0f,
                                        0.0f, 0.0f,0.0f,1.0f);
                                        
        glm::mat4 projection = glm::mat4( 1.0f,0.0f,0.0f,0.0f,
                                            0.0f, 1.0f,0.0f,0.0f,
                                           0.0f, 0.0f,1.0f,0.0f,
                                            0.0f, 0.0f,-13.0f,1.0f);
        program.setMat4("rostogl", rostogl);
        program.setMat4("projection", projection);
        program.setMat4("view", scale);
        renders->render(program,node->now());
        // model_.render();
        // glBindFramebuffer(GL_READ_FRAMEBUFFER,fbo.getFrameBufferID());
        // glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
        // glViewport(0,0,width,height);
        // glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
        // glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

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