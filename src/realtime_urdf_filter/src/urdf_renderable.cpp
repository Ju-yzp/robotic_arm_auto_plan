//cpp

#include <cstddef>
#include <memory>
//ros2
#include <rclcpp/duration.hpp>
#include <rclcpp/logging.hpp>
//
#include <rclcpp/parameter_value.hpp>
#include <realtime_urdf_filter/renderable.hpp>
#include <realtime_urdf_filter/urdf_renderable.hpp>
//urdf
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Transform.h>
#include <tf2/LinearMath/Vector3.h>
#include <tf2/exceptions.h>
#include <tf2/convert.h>
#include <urdf/model.h>
#include <urdf_model/link.h>
#include <urdf_model/pose.h>
#include <urdf_model/types.h>
//tf
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>  
#include <geometry_msgs/msg/transform_stamped.hpp>

namespace  realtime_urdf_filter
{
UrdfRenderable::UrdfRenderable(const std::shared_ptr<rclcpp::Node> &node):
node_(node)
{
    tf_buffer_ = std::make_shared<tf2_ros::Buffer>(node_->get_clock());
    tf_ = std::make_unique<tf2_ros::TransformListener>(* tf_buffer_);
    getParams();
    loadModel();
}

void UrdfRenderable::render(Program &program)
{
     updateTransfrom();// defualt not use in test 
    for(const auto &render:renders)
    {
        auto &color = render->color;
        render->applyTransfrom(program);
        program.setVec4("Color",glm::vec4(color.r,color.g,color.b,color.a));
        render->render();
    }
}

void UrdfRenderable::getParams()
{
    std::string model_file_path = "/home/zy_jp/robotic_arm_auto_plan/src/realtime_urdf_filter/urdf/example.urdf";
    try {
        model_desc_ = node_->declare_parameter("model_description",model_file_path);
    }catch(rclcpp::ParameterTypeException &e)
    {
        RCLCPP_INFO(node_->get_logger(),"model description %s",e.what());
    }
    try{
        geometry_type_ = node_->declare_parameter("geometry_type","visual");
    }catch(rclcpp::ParameterTypeException &e)
    {
        RCLCPP_INFO(node_->get_logger(),"geometry_type %s",e.what());
    }

    try{
        fixed_frame_ = node_->declare_parameter("fixed_frame","base_link");
    }catch(rclcpp::ParameterTypeException &e)
    {
        RCLCPP_INFO(node_->get_logger(),"fixed frame %s",e.what());
    }
    auto links = node_->declare_parameter("ignore",std::vector<std::string>());

    for(const auto &ignore:links)
    {
        ignore_links_.insert(ignore);
    }
}

void UrdfRenderable::updateTransfrom()
{
    
    for(auto & render:renders)
    {
        try
        {
            geometry_msgs::msg::TransformStamped tf_stamped;
            if(render->name == fixed_frame_)
               continue;
            tf_stamped = tf_buffer_->lookupTransform(fixed_frame_,render->name,node_->now(),rclcpp::Duration::from_seconds(0.005));
            tf2::fromMsg(tf_stamped.transform,render->link_to_fixed);
            // RCLCPP_INFO(node_->get_logger(),"!");
        }catch(tf2::TransformException &ex)
        {
            RCLCPP_DEBUG(node_->get_logger(),"%s",ex.what());
        }
    }
}

void UrdfRenderable::loadModel()
{
    urdf::Model model;
    if(model.initFile(model_desc_))
    {
        std::vector<std::shared_ptr<urdf::Link>> links;
        model.getLinks(links);
        RCLCPP_INFO(node_->get_logger(),"start parse xml file struct");
        for(const  auto &link :links)
        {
            processLink(link);
        }
    }
    else
    {
        RCLCPP_WARN(node_->get_logger(),"Occur mistakes when parsing URDF XML file !");
    }
}

void UrdfRenderable::processLink(const urdf::LinkSharedPtr &link)
{
    std::vector<urdf::Pose> origins;;
    std::vector<urdf::GeometryConstSharedPtr> geometries;
    std::vector<urdf::MaterialConstSharedPtr> materials;

    //skip links that not be used for mask

    if(ignore_links_.count(link->name))
       return ;

    if(geometry_type_.empty() || geometry_type_ == "visual")
    {
        for(const auto & visual:link->visual_array)
        {
            origins.emplace_back(visual->origin);
            geometries.emplace_back(visual->geometry);
            materials.emplace_back(visual->material);
        }
    }
    else if(geometry_type_ == "collision")
    {
        for(const auto & collision:link->collision_array)
        {
            origins.emplace_back(collision->origin);
            geometries.emplace_back(collision->geometry);
            materials.emplace_back(nullptr);
        }
    }
    else 
    {
        RCLCPP_WARN(node_->get_logger(),"Invaild geometry type");
    }

    for(size_t index = 0; index < geometries.size();index++)
    {
        const urdf::Pose &origin = origins[index];
        const urdf::GeometryConstSharedPtr &geometry = geometries[index];
        const urdf::MaterialConstSharedPtr &material = materials[index];
        //set up render type
        std::shared_ptr<Renderable> render;
        if(geometry->type == urdf::Geometry::BOX)
        {
            const urdf::BoxConstSharedPtr box = std::dynamic_pointer_cast<const urdf::Box>(geometry);
            render =  std::make_shared<RenderableBox>(box->dim.x,box->dim.y,box->dim.z);
        }
        else if(geometry->type == urdf::Geometry::CYLINDER)
        {
            RCLCPP_INFO(node_->get_logger(),"%s is cylinder",link->name.c_str());
            const urdf::CylinderConstSharedPtr cylinder = std::dynamic_pointer_cast<const urdf::Cylinder>(geometry);
            render = std::make_shared<RenderableCylinder>(cylinder->length,cylinder->radius);
        }
        else if(geometry->type == urdf::Geometry::SPHERE)
        {
            const urdf::SphereConstSharedPtr sphere = std::dynamic_pointer_cast<const urdf::Sphere>(geometry);
            render = std::make_shared<RenderableSphere>(sphere->radius);
        }
        else if(geometry->type == urdf::Geometry::MESH)
        {

        }
        else {
        return ;
        }
        render->name = link->name;

        RCLCPP_INFO(node_->get_logger(),"frame %s",render->name.c_str());
        //set up transform imformation
        const urdf::Vector3 position = origin.position;
        const urdf::Rotation rotation = origin.rotation;

        render->link_offest = tf2::Transform(
            tf2::Quaternion(rotation.x,rotation.y,rotation.z,rotation.w).normalize(),
            tf2::Vector3(position.x,position.y,position.z));
            
        //set up color 
        if(material)
        {
            render->color = material->color;
        }
        renders.emplace_back(render);
    }
}
}