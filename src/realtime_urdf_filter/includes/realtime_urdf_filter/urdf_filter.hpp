#ifndef REALTIME_URDF_FILTER_URDF_FILTER_HPP_
#define REALTIME_URDF_FILTER_URDF_FILTER_HPP_
//ros2
#include <memory>
#include <opencv2/core/hal/interface.h>
#include <rclcpp/node.hpp>
#include <image_transport/camera_publisher.hpp>
#include <image_transport/camera_subscriber.hpp>
#include <cv_bridge/cv_bridge.h>
//msg
#include <rclcpp/subscription.hpp>
#include <sensor_msgs/msg/detail/camera_info__struct.hpp>
#include <sensor_msgs/msg/detail/image__struct.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
//
#include <realtime_urdf_filter/framebufferObject.hpp>
#include <realtime_urdf_filter/renderable.hpp>
#include <realtime_urdf_filter/urdf_renderable.hpp>
//tf
#include <tf2_ros/transform_listener.h>
#include <tf2_ros/buffer.h>
#include <tf2/LinearMath/Quaternion.hpp>
#include <tf2/LinearMath/Vector3.hpp>

namespace realtime_urdf_filter
{
    class RealTimeUrdfFilter
    {
        public:
        // constructor
        RealTimeUrdfFilter(rclcpp::Node::SharedPtr nh);

        ~RealTimeUrdfFilter();

        // initialize OpenGL environment
        void initGL();

        void loadModel();
        // 
        void render(glm::mat4 projection_matrix);
        
        uchar * bufferFromImage(cv::Mat &depth_image);

        void textureFromBuffer(uchar * data ,size_t data_size);

        void filter(uchar *buffer,glm::mat4 projecttion_matrix,int width,int height);

        void depthImageFilter(sensor_msgs::msg::Image::ConstSharedPtr msg);

        void initFrameBufferObject();

        void getParams();

        void getProjectionMatrix(glm::mat4 &projection_matrix);

        private:
        // OpenGL window resource
        GLFWwindow *window;

        // camera info 
        std::shared_ptr<sensor_msgs::msg::CameraInfo> cam_info_;
        rclcpp::Subscription<sensor_msgs::msg::CameraInfo>::SharedPtr cam_info_sub_;

        // ROS2 objects
        rclcpp::Node::SharedPtr nh_;
        image_transport::CameraSubscriber depth_sub_;
        image_transport::CameraPublisher color_pub_;
        image_transport::CameraPublisher mask_pub_;
        tf2_ros::TransformListener *tf_listener_;
        std::shared_ptr<tf2_ros::Buffer> tf_buffer_;

        // rendering objects
        FrameBufferObject *fbo_;
        bool               fbo_initialzed_;
        GLuint             depth_texture_;
        GLuint             depth_fbo_;

        //vector of renderables
        std::vector<UrdfRenderable *> renderables_;


        // do we have subscribers for the mask image?
        bool need_mask_;    

        // image size 
        GLint              width_;
        GLint              height_;

        // OpenGL virtual camera configuration
        double             near_;
        double             far_;
        double             depth_distance_threshold_;

        // parameters from configration file
        tf2::Quaternion camera_offest_t_;
        tf2::Vector3 camera_offest_q_;
        std::string cam_frame_;
        std::string fixed_frame_;
        bool show_gui_;

        // output from rendering
        GLfloat *mask_depth_ = nullptr;
        GLubyte  *mask_ = nullptr; 
    };
}

#endif