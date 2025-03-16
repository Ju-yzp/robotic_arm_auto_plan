//cpp
#include <cstddef>
#include <memory>
//ros2
#include <rclcpp/duration.hpp>
#include <rclcpp/logger.hpp>
#include <rclcpp/logging.hpp>
//
#include <rclcpp/node.hpp>
#include <rclcpp/parameter_value.hpp>
#include <rclcpp/time.hpp>
#include <realtime_urdf_filter/renderable.hpp>
#include <realtime_urdf_filter/urdf_renderable.hpp>
//urdf
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Transform.hpp>
#include <tf2/LinearMath/Vector3.h>
#include <tf2/exceptions.h>
#include <tf2/convert.h>
#include <urdf/model.h>
#include <urdf_model/link.h>
#include <urdf_model/pose.h>
#include <urdf_model/types.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>  
#include <geometry_msgs/msg/transform_stamped.hpp>

namespace  realtime_urdf_filter
{

UrdfRenderable::UrdfRenderable(std::string model_description,
                   std::string cam_frame,
                   std::string fixed_frame,
                   const std::string &geometry_type,
                   double scale,
                   const std::unordered_set<std::string> &ignore,
                //    const tf2_ros::Buffer::SharedPtr tf_buffer,
                   rclcpp::Node *node)
                   :model_desc_(model_description),
                   cam_frame_(cam_frame),
                   fixed_frame_(fixed_frame),
                   scale_(scale),
                   ignore_links_(ignore),
                //    tf_buffer_(tf_buffer),
                   node_(node)
                   {
                    tf_buffer_ = std::make_shared<tf2_ros::Buffer>(node_->get_clock());
                    tf_listener_ = std::make_unique<tf2_ros::TransformListener>(*tf_buffer_);
                     loadModel();
                   }

void UrdfRenderable::render(Program &program, rclcpp::Time timestamp)
{
     updateTransfrom(timestamp);// defualt not use in test 
    for(const auto &render:renders)
    {
        auto &color = render->color;
        render->applyTransfrom(program);
        program.setVec4("Color",glm::vec4(color.r,color.g,color.b,color.a));
        render->render();
    }
}

void UrdfRenderable::updateTransfrom(rclcpp::Time timestamp)
{
    
    for(auto & render:renders)
    {
        try
        {
            geometry_msgs::msg::TransformStamped tf_stamped;
            tf_stamped = tf_buffer_->lookupTransform(fixed_frame_,render->name,timestamp,rclcpp::Duration(0,1000));
            tf2::fromMsg(tf_stamped.transform,render->link_to_fixed);
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
            const urdf::MeshConstSharedPtr mesh = std::dynamic_pointer_cast<const urdf::Mesh>(geometry);
            if(mesh->filename.find("package://") == 0)
            {
                std::string package_path = "/home/zy_jp/robotic_arm_auto_plan/src/";
                std::string filename = package_path + mesh->filename.substr(10);
                render = std::make_shared<RenderableMesh>(filename,0.001);
                RCLCPP_INFO(node_->get_logger(),"file %s",filename.c_str());
            }
            else {
                render = std::make_shared<RenderableMesh>(mesh->filename,0.001);
                RCLCPP_INFO(node_->get_logger(),"file %s",mesh->filename.c_str());
            }
        }
        else {
            RCLCPP_WARN(node_->get_logger(),"Invaild geometry type");
            continue;
        }
        render->name = link->name;

        RCLCPP_INFO(node_->get_logger(),"link name is %s",link->name.c_str());
        //set up transform imformation
        const urdf::Vector3 position = origin.position;
        const urdf::Rotation rotation = origin.rotation;

        RCLCPP_INFO(node_->get_logger(),"position offest is x:%f y:%f z:%f",position.x,position.y,position.z);
        RCLCPP_INFO(node_->get_logger(),"rotation offest is x:%f y:%f z:%f w:%f",rotation.x,rotation.y,rotation.z,rotation.w);
        render->link_offest = tf2::Transform(
            tf2::Quaternion(rotation.x,rotation.y,rotation.z,rotation.w).normalize(),
            tf2::Vector3(position.x,position.y,position.z));
            
        //set up color 
        if(material)
        {
            render->color = material->color;
        }
        else {
            render->color.a = 1.0;
            render->color.r = 1.0;
            render->color.g = 0.0;
            render->color.b = 0.0;
        }
        renders.emplace_back(render);
    }
}
}