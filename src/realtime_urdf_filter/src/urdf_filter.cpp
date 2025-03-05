#include <realtime_urdf_filter/urdf_filter.hpp>

namespace realtime_urdf_filter 
{
    RealTimeUrdfFilter::RealTimeUrdfFilter(rclcpp::Node::SharedPtr nh)
    : nh_(nh)
    {

    }

    RealTimeUrdfFilter::~RealTimeUrdfFilter()
    {
        if(fbo_initialzed_)
           delete fbo_;
        for(auto & renderable : renderables_)
        {
            if(renderable != nullptr)
               delete renderable;
        }
    }
}