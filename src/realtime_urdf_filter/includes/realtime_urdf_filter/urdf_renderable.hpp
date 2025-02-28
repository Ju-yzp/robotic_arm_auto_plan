#ifndef REALTIME_URDF_FILTER_UDRF_RENDERABLE_HPP_
#define REALTIME_URDF_FILTER_UDRF_RENDERABLE_HPP_
//ros2
#include <rclcpp/rclcpp.hpp>
#include <rclcpp/node.hpp>
#include <rclcpp/subscription.hpp>
#include <rclcpp/node_options.hpp>
//render
#include <realtime_urdf_filter/renderable.hpp>
//tf
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
    UrdfRenderable(const std::shared_ptr<rclcpp::Node> &node);
    void render();
    void updateTransfrom();
    private:
    void getParams();
    void loadModel();
    void processLink(const urdf::LinkSharedPtr &link);
    std::vector<std::shared_ptr<Renderable>> renders;
    //ros2 node
    std::shared_ptr<rclcpp::Node> node_;
    //tf listener
    std::unique_ptr<tf2_ros::TransformListener> tf_;
    std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
    //model stuff
    std::string model_desc_;
    std::string geometry_type_;
    std::string fixed_frame_;
    std::unordered_set<std::string> ignore_links_;
};
}
#endif