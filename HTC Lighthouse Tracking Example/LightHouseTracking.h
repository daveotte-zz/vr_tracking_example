// LIGHTHOUSETRACKING.h
#ifndef _LIGHTHOUSETRACKING_H_
#define _LIGHTHOUSETRACKING_H_

// OpenVR
#include <openvr.h>
#include "samples\shared\Matrices.h"
#include <chrono>

class LighthouseTracking {
private:

	// basic stuff
	vr::IVRSystem *m_pHMD = NULL;
	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	Matrix4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];

	vr::HmdQuaternion_t LighthouseTracking::GetRotation(vr::HmdMatrix34_t matrix);
	vr::HmdVector3_t LighthouseTracking::GetPosition(vr::HmdMatrix34_t matrix);



public:
	vr::TrackedDeviceIndex_t HmdId;
	vr::TrackedDeviceIndex_t ControllerLeftId;
	vr::TrackedDeviceIndex_t ControllerRightId;

	std::chrono::high_resolution_clock::time_point curTime;
	std::chrono::high_resolution_clock::time_point	lastTime;
	std::chrono::milliseconds elapsed;
	std::chrono::milliseconds timeForOneFrame;

	~LighthouseTracking();
	LighthouseTracking();

	// Main loop that listens for openvr events and calls process and parse routines, if false the service has quit
	bool RunProcedure();

	bool RunMainLoop();

	// Process a VR event and print some general info of what happens
	bool ProcessVREvent(const vr::VREvent_t & event);

	// Parse a tracking frame and print its position / rotation
	void ParseTrackingFrame();

	void PrintTransforms();



};

#endif _LIGHTHOUSETRACKING_H_
