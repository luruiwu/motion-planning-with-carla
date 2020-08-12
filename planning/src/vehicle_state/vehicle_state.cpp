#include "vehicle_state/vehicle_state.hpp"
#include <tf/transform_datatypes.h>
#include <Eigen/Core>
namespace planning {
// the vehicle's coordinate, x--->forward, y-->left, z--->up

planning::VehicleState &VehicleState::Instance() {
  static planning::VehicleState instance = VehicleState();
  return instance;
}

const geometry_msgs::Pose &VehicleState::pose() const { return pose_; }
double VehicleState::linear_vel() const { return this->linear_vel_; }
double VehicleState::linear_acc() const { return this->linear_acc_; }
double VehicleState::angular_vel() const { return this->angular_vel_; }
const ros::Time &VehicleState::time_stamp() const { return this->time_stamp_; }
void VehicleState::set_pose(const geometry_msgs::Pose &pose) { this->pose_ = pose; }
void VehicleState::set_linear_vel(double vel) { this->linear_vel_ = vel; }
void VehicleState::set_linear_acc(double acc) { this->linear_acc_ = acc; }
void VehicleState::set_angular_vel(double omega) { this->angular_vel_ = omega; }

bool VehicleState::Update(const carla_msgs::CarlaEgoVehicleStatus &ego_vehicle_status,
                          const nav_msgs::Odometry &odometry, const carla_msgs::CarlaEgoVehicleInfo &vehicle_info) {
  this->pose_ = odometry.pose.pose;
  this->time_stamp_ = ego_vehicle_status.header.stamp;
  this->heading_ = tf::getYaw(odometry.pose.pose.orientation);
  this->linear_vel_ = ego_vehicle_status.velocity;
  this->angular_vel_ = odometry.twist.twist.angular.z;
  this->linear_acc_ = std::sqrt(ego_vehicle_status.acceleration.linear.x * ego_vehicle_status.acceleration.linear.x +
      ego_vehicle_status.acceleration.linear.y * ego_vehicle_status.acceleration.linear.y +
      ego_vehicle_status.acceleration.linear.z * ego_vehicle_status.acceleration.linear.z);
  tf::Quaternion quat;
  tf::quaternionMsgToTF(odometry.pose.pose.orientation, quat);
  tf::Matrix3x3 R = tf::Matrix3x3(quat);
  tf::Vector3 global_acc;
  global_acc.setX(ego_vehicle_status.acceleration.linear.x);
  global_acc.setY(ego_vehicle_status.acceleration.linear.y);
  global_acc.setZ(ego_vehicle_status.acceleration.linear.z);
  tf::Vector3 body_acc = R * global_acc;
  this->linear_acc_ = body_acc.x();
  this->centripental_acc_ = body_acc.y();
  this->time_stamp_ = ego_vehicle_status.header.stamp;
  this->steer_percentage_ = ego_vehicle_status.control.steer;
  this->reverse_ = ego_vehicle_status.control.reverse;
  this->center_of_mass_ = vehicle_info.center_of_mass;
}

geometry_msgs::Pose VehicleState::PredictNextPose(double t) {
  double v = linear_vel_;
  tf::Vector3 vec_distance(0.0, 0.0, 0.0);
  if (std::fabs(angular_vel_) < 0.0001) {
    vec_distance[0] = v * t;
    vec_distance[1] = 0.0;
  } else {
    vec_distance[0] = std::sin(angular_vel_ * t) * v / angular_vel_;
    vec_distance[1] = v * (1- std::cos(angular_vel_ * t)) / angular_vel_;
  }
  tf::Vector3 pos_vec(pose_.position.x, pose_.position.y, pose_.position.z);
  tf::Quaternion quaternion;
  tf::quaternionMsgToTF(pose_.orientation, quaternion);
  tf::Vector3 future_pos_vec = tf::Matrix3x3(quaternion) * vec_distance + pos_vec;
  geometry_msgs::Pose future_pose;
  future_pose.position.x = future_pos_vec.x();
  future_pose.position.y = future_pos_vec.y();
  future_pose.position.z = future_pos_vec.z();
  double future_yaw = tf::getYaw(pose_.orientation) + angular_vel_ * t;
  future_pose.orientation = tf::createQuaternionMsgFromYaw(future_yaw);
  return future_pose;
}
}
