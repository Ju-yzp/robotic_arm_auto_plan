// c style
#include <cassert>
#include <cstdlib>
#include <cstring>
// cpp system
#include <functional>
#include <glm/fwd.hpp>
#include <memory>
// image
#include <image_transport/camera_publisher.hpp>
#include <image_transport/camera_subscriber.hpp>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/hal/interface.h>
// ros2
#include <rclcpp/logging.hpp>
#include <rclcpp/parameter_value.hpp>
#include <rclcpp/qos.hpp>
// 
#include <realtime_urdf_filter/framebufferObject.hpp>
#include <realtime_urdf_filter/urdf_filter.hpp>
// tf2
#include <stdexcept>
#include <string>
#include <tf2/LinearMath/Quaternion.hpp>
#include <tf2/LinearMath/Transform.hpp>
#include <tf2/LinearMath/Vector3.hpp>
#include <tf2/exceptions.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>
#include <tf2/convert.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

namespace realtime_urdf_filter 
{
    RealTimeUrdfFilter::RealTimeUrdfFilter(rclcpp::Node::SharedPtr nh)
    : nh_(nh)
    {
        tf_buffer_ = std::make_shared<tf2_ros::Buffer>(nh_->get_clock());
        tf_listener_ = new tf2_ros::TransformListener(*tf_buffer_);

        cam_info_sub_ = nh_->create_subscription<sensor_msgs::msg::CameraInfo>("",rclcpp::SensorDataQoS(),
        [&](sensor_msgs::msg::CameraInfo::ConstSharedPtr cam_info)
        { });

        getParams();

        // set up image subcriber and publisher
        mask_pub_  = image_transport::CameraPublisher(nh_.get(),"/mask_img");
        depth_sub_ = image_transport::CameraSubscriber(nh_.get(),"/depth_img",
        std::bind(&RealTimeUrdfFilter::depthImageFilter,this,std::placeholders::_1),"");
        color_pub_ = image_transport::CameraPublisher(nh_.get(),"");
    }

    void RealTimeUrdfFilter::loadModel()
    {
        // 
        std::string model_description;
        try
        {
            model_description = nh_->get_parameter("model_description").as_string();
        }catch(rclcpp::ParameterTypeException &ex)
        {
            RCLCPP_ERROR(nh_->get_logger(),"%s",ex.what());
        }

        // get links that not used to render
        std::unordered_set<std::string> ignore_links;
        try
        {
            std::vector<std::string> ignore;
            ignore = nh_->get_parameter("ignore").as_string_array();
            if(ignore.empty())
               RCLCPP_INFO(nh_->get_logger(),"");
            for(auto &i:ignore)
                ignore_links.insert(i);
        }catch(rclcpp::ParameterTypeException &ex)
        {
            RCLCPP_ERROR(nh_->get_logger(),"%s",ex.what());
        }
        
        // 
        double scale {1.0f};
        if(nh_->has_parameter("scale"))
        {
            scale = nh_->get_parameter("scale").as_double();
        }

        std::string geometry_type = "visual";
        if(nh_->has_parameter("geometry_type"))
        {
            geometry_type = nh_->get_parameter("geometry_type").as_string();
        }
        renderables_.push_back(new UrdfRenderable(model_description,cam_frame_,fixed_frame_,geometry_type,
        scale,ignore_links,tf_buffer_,nh_.get()));
    }

    void RealTimeUrdfFilter::depthImageFilter(sensor_msgs::msg::Image::ConstSharedPtr msg)
    {
        // convert ros2 to OpenCV cv::Mat 
        cv::Mat img;
        if(msg->encoding == sensor_msgs::image_encodings::TYPE_32FC1)
        {
            img = cv_bridge::toCvShare(msg,sensor_msgs::image_encodings::TYPE_32FC1)->image;
        }
        else if(msg->encoding == sensor_msgs::image_encodings::TYPE_16UC1)
        {
            cv_bridge::CvImageConstPtr depth_img;
            depth_img = cv_bridge::toCvShare(msg,sensor_msgs::image_encodings::TYPE_16UC1);
            depth_img->image.convertTo(img,CV_32FC1,0.001);
        }
        
        uchar * depthData = bufferFromImage(img);

        glm::mat4 projection_matrix;
        getProjectionMatrix(projection_matrix);

        filter(depthData,projection_matrix,img.cols,img.rows);

        if(color_pub_.getNumSubscribers() > 0 )
        {
            cv::Mat masked_depth_image(cam_info_->height,cam_info_->width,CV_32FC1,mask_depth_);
            if(msg->encoding == sensor_msgs::image_encodings::TYPE_16UC1)
            {
                masked_depth_image.convertTo(masked_depth_image, CV_16UC1,1000.0);
            }
            
            cv_bridge::CvImage out_masked_depth;
            out_masked_depth.header = msg->header;
            out_masked_depth.encoding = msg->encoding;
            out_masked_depth.image = masked_depth_image;
            color_pub_.publish (out_masked_depth.toImageMsg (), cam_info_);
        }

        if(mask_pub_.getNumSubscribers())
        {
            cv::Mat mask_image(cam_info_->height,cam_info_->width,CV_8UC1,mask_);
            cv_bridge::CvImage out_mask;
            out_mask.header = msg->header;
            out_mask.encoding = sensor_msgs::image_encodings::MONO8;
            out_mask.image = mask_image;
            mask_pub_.publish (out_mask.toImageMsg (), cam_info_);       
        }
    }

    void RealTimeUrdfFilter::filter(uchar *buffer,glm::mat4 projection_matrix,int width,int height)
    {
        if( width != width_ || height != height_)
        {
            if(width != 0 || height != 0)
              RCLCPP_ERROR(nh_->get_logger(),"image size is changed (%ix%i -> %ix%i)",width_,height_,width,height);
            width_ = width;
            height_ = height;
            initGL();
        }

        if(renderables_.empty())
           return;

        if(mask_pub_.getNumSubscribers() > 0)
           need_mask_ = true;
        else 
           need_mask_ = false;

        textureFromBuffer(buffer,width * height * sizeof(uchar));

        render(projection_matrix);
    }
    
    uchar * RealTimeUrdfFilter::bufferFromImage(cv::Mat &depth_image)
    {
        unsigned int width = static_cast<unsigned int>(depth_image.cols);
        unsigned int height = static_cast<unsigned int>(depth_image.rows);

        static uchar * buffer = nullptr;

        if(depth_image.isContinuous())
        {
            buffer  = depth_image.data;
        }
        else
        {
            if(buffer == nullptr)
               buffer = static_cast<uchar *>(malloc(width * height * sizeof(uchar)));
            for(unsigned int i = 0 ; i < height;i++)
            {
               memcpy(buffer+i*width,&depth_image.data[i],width*sizeof(uchar));
            }
        }
        return buffer;
    }

    void RealTimeUrdfFilter::textureFromBuffer(uchar * data ,size_t data_size)
    {
        if(depth_fbo_ == GL_INVALID_VALUE)
        {
            RCLCPP_DEBUG(nh_->get_logger(),"");
            glGenBuffers(1,&depth_fbo_);
        }

        // send data to GPU
        glBindBuffer(GL_ARRAY_BUFFER,depth_fbo_);
        glBufferData(GL_ARRAY_BUFFER,data_size,data,GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER,0);

        if(depth_texture_ == GL_INVALID_VALUE)
        {
            glGenTextures(1,&depth_texture_);
        }

        glBindTexture(GL_TEXTURE_BUFFER,depth_texture_);
        glTexBuffer(GL_TEXTURE_BUFFER,GL_R32F,depth_fbo_);
    }


    void RealTimeUrdfFilter::initGL()
    {
        static bool gl_initialized = false;

        if(!gl_initialized)
        {
            glfwInit();
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

            window = glfwCreateWindow(960, 480, "view", NULL, NULL);

            glfwMakeContextCurrent(window);

            if(!show_gui_)
               glfwHideWindow(window);

            gl_initialized = true;
        }

        initFrameBufferObject();

        loadModel();

        if(renderables_.empty())
           throw std::runtime_error("Could not load any models for filtering!");
        else
           RCLCPP_INFO(nh_->get_logger(),"Loaded %i models for filtering",static_cast<int>(renderables_.size()));

        mask_depth_ = reinterpret_cast<float *>(malloc(width_ * height_ * sizeof(float)));

        mask_ = reinterpret_cast<GLubyte *>(malloc(width_ * height_ * sizeof(GLubyte)));
    }

    void RealTimeUrdfFilter::initFrameBufferObject()
    {
        if(fbo_initialzed_)
           return;

        fbo_ = new FrameBufferObject("");
        fbo_->initialize(width_,height_);
        fbo_initialzed_ = true;

        GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if(status != GL_FRAMEBUFFER_COMPLETE)
           RCLCPP_INFO(nh_->get_logger(),"Framebuffer occur error: %i",status);
    }

    void RealTimeUrdfFilter::getProjectionMatrix(glm::mat4 &projection_matrix)
    {
        #ifdef USE_OWN_CALIBRATION

        #else
        // get camera parameters from camera info 
        const auto  & k = cam_info_->k;
        double fx = k[0];
        double fy = k[4];
        double cx = k[2];
        double cy = k[5];
        #endif

        //
    }

    void RealTimeUrdfFilter::render(glm::mat4 projection_matrix)
    {
        if(!fbo_initialzed_)
           return;

        // get transfromation from camera to fixed frame(world)
        try {
        geometry_msgs::msg::TransformStamped tf;
        tf = tf_buffer_->lookupTransform(cam_frame_,fixed_frame_,nh_->now());
        RCLCPP_DEBUG_STREAM(nh_->get_logger(), "Camera to world translation"<<
        cam_frame_<<"->"<<fixed_frame_<<
        "["<<
        " "<<tf.transform.translation.x<<
        " "<<tf.transform.translation.y<<
        " "<<tf.transform.translation.z<<
        "]");
        }catch(tf2::TransformException &ex)
        {
            RCLCPP_ERROR(nh_->get_logger(),"%s",ex.what());
            return;
        }

        // 
        std::string realtive_path = "/home/zy_jp/robotic_arm_auto_plan/src/realtime_urdf_filter";
        static Shader vert_s = Shader((realtive_path+std::string("/shader/vertex.glsl")).c_str(),Shader_type::VERTEX_SHADER);
        static Shader frag_s = Shader("/shader/fragment.glsl",Shader_type::FRAGMENT_SHADER);
        static Program program(std::vector<Shader>{vert_s,frag_s});
        // clear the buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);

        GLuint depth_texture_id = 0;

        program.use();
        program.setVal1i(std::string("depth_texture"),depth_texture_id);
        program.setVal1i(std::string("width"), int(width_));
        program.setVal1f(std::string("z_far"), far_);
        program.setVal1f(std::string("z_near"), near_);
        program.setVal1f(std::string("max_diff"), far_);

        // transfrom matrix
        double glTf[16];

        for(auto &render:renderables_)
        {
            render->render(program,nh_->now());
        }

        // swap framebuffer
        if(show_gui_)
        {
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    void RealTimeUrdfFilter::getParams()
    {
        // get fixed frame name
        try
        {
            nh_->get_parameter("fixed_frame",fixed_frame_);
        }catch(rclcpp::ParameterTypeException &ex)
        {
            RCLCPP_ERROR(nh_->get_logger(),"%s",ex.what());
        }

        // get camera frame name
        try
        {
            nh_->get_parameter("camera_frame",cam_frame_);
        }catch(rclcpp::ParameterTypeException &ex)
        {
            RCLCPP_ERROR(nh_->get_logger(),"%s",ex.what());
        }

        // get camera offest(include rotation and translation)
        try
        {
            std::vector<double> cam_t,cam_r;
            cam_t = nh_->get_parameter("camera_translation").as_double_array();
            cam_r = nh_->get_parameter("camera_rotation").as_double_array();

            assert(cam_t.size()==3);
            assert(cam_r.size()==4);

            camera_offest_t_ = tf2::Quaternion(cam_r[0],cam_r[0],cam_r[0],cam_r[0]);
            camera_offest_q_ = tf2::Vector3(cam_t[0],cam_t[1],cam_t[2]);
        }catch(rclcpp::ParameterTypeException &ex)
        {
            RCLCPP_ERROR(nh_->get_logger(),"%s",ex.what());
        }

        // set up display configuration
        try
        {
            nh_->declare_parameter("show_gui",false);
            nh_->get_parameter("show_gui",show_gui_);
            RCLCPP_INFO(nh_->get_logger(),"%s",(show_gui_?"ON":"OFF"));
        }catch(rclcpp::ParameterTypeException &ex)
        {
            RCLCPP_ERROR(nh_->get_logger(),"%s",ex.what());
        }
    }
}