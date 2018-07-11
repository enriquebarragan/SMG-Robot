#include <hardware_interface/joint_command_interface.h>
#include <hardware_interface/joint_state_interface.h>
#include <hardware_interface/robot_hw.h>

#include "ugv/RoboteqDevice.h"
#include "ugv/ErrorCodes.h"
#include "ugv/Constants.h"
#include <unistd.h>

class UGV : public hardware_interface::RobotHW {
private:
	RoboteqDevice device;
	hardware_interface::JointStateInterface jnt_state;
	hardware_interface::VelocityJointInterface jnt_vel;
	double cmd[2];
	double pos[2];
	double vel[2];
	double eff[2];
	int ret, initial_encoder[2];
public:
	UGV() {
		// Initialize all values to zero
		cmd[0] = 0; cmd[1] = 0;
		pos[0] = 0; pos[1] = 0;
		vel[0] = 0; vel[1] = 0;
		eff[0] = 0; eff[1] = 0;

		// Create our joints and register them
		hardware_interface::JointStateHandle stateA("Right",&pos[0],&vel[0],&eff[0]);
		hardware_interface::JointStateHandle stateB("Left",&pos[1],&vel[1],&eff[1]);
		jnt_state.registerHandle(stateA);
		jnt_state.registerHandle(stateB);
		registerInterface(&jnt_state);
		hardware_interface::JointHandle velA(jnt_state.getHandle("Right"),&cmd[0]);
		hardware_interface::JointHandle velB(jnt_state.getHandle("Left"),&cmd[1]);
		jnt_vel.registerHandle(velA);
		jnt_vel.registerHandle(velB);
		registerInterface(&jnt_vel);

		// Prepare our motor controller
		int status = device.Connect("/dev/ttyACM0");
		device.GetValue(_ABCNTR,1,initial_encoder[0]);
		device.GetValue(_ABCNTR,2,initial_encoder[1]);

	}

	// Functions to assist with ROS time-management
	ros::Time getTime() const {return ros::Time::now();}
        ros::Duration getPeriod() const {return ros::Duration(0.05);}

	void read() {
		// Determine pos, vel, and eff and place them into variables
		// _ABSPEED returns RPM, we need Rads/Sec

		// vel[0] = device.GetValue(_ABSPEED,1,ret)/9.5493;
		// vel[1] = device.GetValue(_ABSPEED,2,ret)/9.5493;
		// pos[0] = 3.14159265359*device.GetValue(_ABCNTR,1,ret)/(6000.0);
		// pos[1] = 3.14159265359*device.GetValue(_ABCNTR,2,ret)/(6000.0);
		vel[0] = device.GetValue(_ABSPEED,1,ret);
		vel[0] = 3.14159265359*ret/30;
		vel[1] = device.GetValue(_ABSPEED,2,ret);
		vel[1] = 3.14159265359*ret/30;
		pos[0] = device.GetValue(_ABCNTR,1,ret);
		pos[0] = 3.14159265359*(ret-initial_encoder[0])/(6000.0);
		pos[0] = pos[0]/4.0;
		pos[1] = device.GetValue(_ABCNTR,2,ret);
		pos[1] = 3.14159265359*(ret-initial_encoder[1])/(6000.0);
		pos[1] = pos[1]/4.0;
		eff[0] = device.GetValue(_MOTPWR,1,ret);
		eff[0] = ret;
		eff[1] = device.GetValue(_MOTPWR,2,ret);
		eff[1] = ret;
	}

	void write() {
		// Write 'cmd' out to the motor driver
		// cmd[] is in rads per second
		// _GO takes in a value from -1000 to 1000 linearly from -60RPM to 60RPM
		//159.155
		// 34
		device.SetCommand(_GO,1,(int)(cmd[0]*159.155*(1.0/1000)*60.0));

		device.SetCommand(_GO,2,(int)(cmd[1]*159.155*(1.0/1000)*60.0));
	}
};


