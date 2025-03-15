#ifndef REALTIME_URDF_FILTER_UDRF_RENDERABLE_HPP_
#define REALTIME_URDF_FILTER_UDRF_RENDERABLE_HPP_
//ros2
#include <rclcpp/logger.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp/node.hpp>
#include <rclcpp/subscription.hpp>
#include <rclcpp/node_options.hpp>
#include <rclcpp/time.hpp>
//render
#include <realtime_urdf_filter/renderable.hpp>
#include <realtime_urdf_filter/shader_wrapper.hpp>
//tf
#include <tf2/buffer_core.h>
#include <tf2_ros/transform_listener.h>
#include <tf2_ros/buffer.h>
//cpp
#include <memory>
#include <string>
#include <unordered_set>
#include <urdf_model/types.h>
//urdf

namespace realtime_urdf_filter
{
class UrdfRenderable
{
    public:
    UrdfRenderable(std::string model_description,
                   std::string cam_frame,
                   std::string fixed_frame,
                   const std::string &geometry_type,
                   double scale,
                   const std::unordered_set<std::string> &ignore,
                //    const tf2_ros::Buffer::SharedPtr tf_buffer,
                   rclcpp:: Node *node);

    void render(Program &program, rclcpp::Time timestamp);

    void updateTransfrom(rclcpp::Time timestamp);

    private:

    void loadModel();

    void processLink(const urdf::LinkSharedPtr &link);

    std::vector<std::shared_ptr<Renderable>> renders;

    // ros2 node
    std::shared_ptr<rclcpp::Node> node_;

    // tf listener
    std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
    std::unique_ptr<tf2_ros::TransformListener> tf_listener_;

    // camera stuff
    std::string cam_frame_;

    // model stuff
    std::string model_desc_;
    std::string geometry_type_;
    std::string fixed_frame_;
    double scale_;
    std::unordered_set<std::string> ignore_links_;
};
}
#endif