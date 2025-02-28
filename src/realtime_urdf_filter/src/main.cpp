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
#include <realtime_urdf_filter/urdf_renderable.hpp>
// #include <iostream>

const unsigned int width = 800;
const unsigned int height = 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

int main(int argc,char **argv)
{
    rclcpp::init(argc, argv);
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

    glEnable(GL_DEPTH_TEST);
    auto node = std::make_shared<rclcpp::Node>("render_node");
    realtime_urdf_filter::UrdfRenderable urdfRenderable(node);
    // rclcpp::spin(node);
    // rclcpp::shutdown();
//     std::string vertex_path = "/home/zy_jp/robotic_arm_auto_plan/src/realtime_urdf_filter/shader/vertex.glsl";
//     std::string fragment_path = "/home/zy_jp/robotic_arm_auto_plan/src/realtime_urdf_filter/shader/fragment.glsl";
//     Shader vertex_shader(vertex_path.c_str(),Shader_type::VERTEX_SHADER);
//     Shader fragment_shader(fragment_path.c_str(),Shader_type::FRAGMENT_SHADER);
//     std::vector<Shader> shaders;
//     shaders.emplace_back(vertex_shader);
//     shaders.emplace_back(fragment_shader);
//     Program program(shaders);

//     // realtime_urdf_filter::RenderableCylinder cylinder(0.5f,0.5f);
//     // realtime_urdf_filter::RenderableBox box(0.7f,1.2f,1.0f);
//     realtime_urdf_filter::RenderableSphere sphere(1.0f);
//     if(!program.linkSatus())
//     {
//         std::cout<<"program failed to link "<<std::endl;
//     }

//     // glViewport(0, 0, width, height);

    while(!glfwWindowShouldClose(window))
    {
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
        
        // program.use();
        // glm::mat4 view          = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        // glm::mat4 projection    = glm::mat4(1.0f);
        // projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
        // view       = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
        // // pass transformation matrices to the shader
        // program.setMat4("projection", projection); // note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
        // program.setMat4("view", view);
        // glm::mat4 model = glm::mat4(1.0f);
        // model = glm::translate(model,glm::vec3(0.0f,0.0f,0.0f));
        // float angle = 20.0f ;
        // model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        // program.setMat4("model", model);
        // //cylinder.render();
        // sphere.render();
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