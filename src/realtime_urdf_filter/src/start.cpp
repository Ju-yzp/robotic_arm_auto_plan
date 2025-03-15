#include <rclcpp/node.hpp>
#include <rclcpp/utilities.hpp>

#include <realtime_urdf_filter/urdf_filter.hpp>

#include <memory>

int main(int argc,char **argv)
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<rclcpp::Node>("urdf_filter");
    realtime_urdf_filter::RealTimeUrdfFilter filter(node);

    rclcpp::shutdown();
    return 0;
}