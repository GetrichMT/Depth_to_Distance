#include <memory>
#include <chrono>
#include <functional>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "sensor_msgs/msg/image.hpp"
#include <image_transport/image_transport.hpp>

#include "cv_bridge/cv_bridge.h"
#include <opencv2/opencv.hpp>
// #include <sensor_msgs/msg/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>


#include "rclcpp/rclcpp.hpp"

using std::placeholders::_1;
cv_bridge::CvImagePtr cv_ptr;


class MinimalSubscriber : public rclcpp::Node
{
  public:
    MinimalSubscriber()
    : Node("minimal_subscriber")
    {
      subscription_ = this->create_subscription<sensor_msgs::msg::Image>(
      "/camera/aligned_depth_to_color/image_raw", rclcpp::SensorDataQoS(), std::bind(&MinimalSubscriber::topic_callback, this, _1));
      cmd_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 5);
    }
  private:

    void topic_callback(const sensor_msgs::msg::Image::SharedPtr msg)
    { 
      if(state == 0)
      {
        RCLCPP_INFO(this->get_logger(), "Camera Running...");
      }    
      float smalldistance = 100;
      float dist;
      cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::TYPE_16UC1);
      //float distance9= 0.001*cv_ptr->image.at<uint16_t>(cv_ptr->image.rows/2,cv_ptr->image.cols/2);
            
      for (int i = 50; i <= 530; i++) //0-640
      {
        for(int j = 50; j <= 480; j++)//0-480
        { 
          dist = 0.001*cv_ptr->image.at<uint16_t>(i,j);
          if (dist < 0.10)
          {
            dist = 1000;
          }
          if (dist < smalldistance)
          {
            smalldistance = dist;
          }
        }
      }  

      if (smalldistance < 0.4)
      {
        vel_msg_.linear.x = 0.0;
        vel_msg_.angular.z = 0.0;
        cmd_pub_->publish(vel_msg_);
      } 

      //RCLCPP_INFO(this->get_logger(), "distance(m):%lf", smalldistance);
    }
    int state = 0;
    geometry_msgs::msg::Twist vel_msg_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_pub_;
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr subscription_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MinimalSubscriber>());
  rclcpp::shutdown();
  return 0;
}
