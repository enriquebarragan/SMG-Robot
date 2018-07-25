#!/bin/bash
sleep 30
source /opt/ros/kinetic/setup.bash
source /home/smgrobot/catkin_ws/devel/setup.bash
export ROS_MASTER_URI=http://192.168.1.100:11311
export ROS_HOSTNAME=192.168.1.100
roslaunch ugv start.launch
