#ifndef REALTIME_URDF_FILTER_URDF_FILTER_HPP_
#define REALTIME_URDF_FILTER_URDF_FILTER_HPP_

#include <rclcpp/node.hpp>
#include <image_transport/camera_publisher.hpp>
#include <image_transport/camera_subscriber.hpp>
#include <cv_bridge/cv_bridge.h>

#include <realtime_urdf_filter/framebufferObject.hpp>
#include <realtime_urdf_filter/renderable.hpp>
#include <realtime_urdf_filter/urdf_renderable.hpp>

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

        // 
        void render();
        
        private:
        // ROS2 objects
        rclcpp::Node::SharedPtr nh_;
        image_transport::CameraSubscriber depth_sub_;
        image_transport::CameraPublisher color_pub_;
        image_transport::CameraPublisher mask_pub_;

        // rendering objects
        FrameBufferObject *fbo_;
        bool               fbo_initialzed_;

        //vector of renderables
        std::vector<UrdfRenderable *> renderables_;

        // image size 
        GLint              width_;
        GLint              height_;

        // OpenGL virtual camera configuration
        double             near_;
        double             far_;
    };
}

#endif