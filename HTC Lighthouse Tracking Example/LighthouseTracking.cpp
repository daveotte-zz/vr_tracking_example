//
// HTC Lighthouse Tracking
// 
// By Peter Thor 2016
//
#include "stdafx.h"
#include "LighthouseTracking.h"
#include <sstream>
#include <fstream>
LighthouseTracking::~LighthouseTracking() {
	if (m_pHMD != NULL)
	{
		vr::VR_Shutdown();
		m_pHMD = NULL;
	}
}

LighthouseTracking::LighthouseTracking() 

	: ControllerLeftId(-1)
	, ControllerRightId(-1)
	, HmdId(-1)
	, elapsed(0)
	, timeForOneFrame(42)
	, lastTime(curTime)
{
	vr::EVRInitError eError = vr::VRInitError_None;
	m_pHMD = vr::VR_Init(&eError, vr::VRApplication_Background);
	

	if (eError != vr::VRInitError_None) 
	{
		m_pHMD = NULL;
		char buf[1024];
		sprintf_s(buf, sizeof(buf), "Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription(eError));
		printf_s(buf);
		exit(EXIT_FAILURE);
	}
}


/*
* Loop-listen for events then parses them (e.g. prints the to user)
* Returns true if success or false if openvr has quit
*/
bool LighthouseTracking::RunMainLoop(void) {

	// Process VREvent
	vr::VREvent_t event;

		// Process event
		if (!ProcessVREvent(event)) {
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "(OpenVR) service quit\n");
			printf_s(buf);
			return false;
		}
		
		//print transforms at every 'timeForOneFrame', which will be 24fps for now.
		curTime = std::chrono::high_resolution_clock::now();
		elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(curTime - lastTime);
		if (elapsed < timeForOneFrame) return true;
		lastTime = curTime;
		PrintTransforms();
	

	return true;
}

/*
* Loop-listen for events then parses them (e.g. prints the to user)
* Returns true if success or false if openvr has quit
*/
bool LighthouseTracking::RunProcedure(void) {

	// Process VREvent
	vr::VREvent_t event;
	while (m_pHMD->PollNextEvent(&event, sizeof(event)))
	{
		// Process event
		if (!ProcessVREvent(event)) {
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "(OpenVR) service quit\n");
			printf_s(buf);
			return false;
		}

		// Parse while something is happening
		ParseTrackingFrame();
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Processes a single VR event
//-----------------------------------------------------------------------------

bool LighthouseTracking::ProcessVREvent(const vr::VREvent_t & event)
{
	switch (event.eventType)
	{
		case vr::VREvent_TrackedDeviceActivated:
		{
			//SetupRenderModelForTrackedDevice(event.trackedDeviceIndex);
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "(OpenVR) Device : %d attached\n", event.trackedDeviceIndex);
			printf_s(buf);
		}
		break;

		case vr::VREvent_TrackedDeviceDeactivated:
		{
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "(OpenVR) Device : %d detached\n", event.trackedDeviceIndex);
			printf_s(buf);
		}
		break;

		case vr::VREvent_TrackedDeviceUpdated:
		{
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "(OpenVR) Device : %d updated\n", event.trackedDeviceIndex);
			printf_s(buf);
		}
		break;

		case (vr::VREvent_DashboardActivated) :
		{
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "(OpenVR) Dashboard activated\n");
			printf_s(buf);
		}
		break;

		case (vr::VREvent_DashboardDeactivated) :
		{
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "(OpenVR) Dashboard deactivated\n");
			printf_s(buf);

		}
		break;

		case (vr::VREvent_ChaperoneDataHasChanged) :
		{
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "(OpenVR) Chaperone data has changed\n");
			printf_s(buf);

		}
		break;

		case (vr::VREvent_ChaperoneSettingsHaveChanged) :
		{
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "(OpenVR) Chaperone settings have changed\n");
			printf_s(buf);
		}
		break;

		case (vr::VREvent_ChaperoneUniverseHasChanged) :
		{
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "(OpenVR) Chaperone universe has changed\n");
			printf_s(buf);

		}
		break;

		case (vr::VREvent_ApplicationTransitionStarted) :
		{
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "(OpenVR) Application Transition: Transition has started\n");
			printf_s(buf);

		}
		break;

		case (vr::VREvent_ApplicationTransitionNewAppStarted) :
		{
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "(OpenVR) Application transition: New app has started\n");
			printf_s(buf);

		}
		break;

		case (vr::VREvent_Quit) :
		{
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "(OpenVR) Received SteamVR Quit\n");
			printf_s(buf);

			return false;
		}
		break;
	}

	return true;
}



vr::HmdQuaternion_t LighthouseTracking::GetRotation(vr::HmdMatrix34_t matrix) {
	vr::HmdQuaternion_t q;

	q.w = sqrt(fmax(0, 1 + matrix.m[0][0] + matrix.m[1][1] + matrix.m[2][2])) / 2;
	q.x = sqrt(fmax(0, 1 + matrix.m[0][0] - matrix.m[1][1] - matrix.m[2][2])) / 2;
	q.y = sqrt(fmax(0, 1 - matrix.m[0][0] + matrix.m[1][1] - matrix.m[2][2])) / 2;
	q.z = sqrt(fmax(0, 1 - matrix.m[0][0] - matrix.m[1][1] + matrix.m[2][2])) / 2;
	q.x = copysign(q.x, matrix.m[2][1] - matrix.m[1][2]);
	q.y = copysign(q.y, matrix.m[0][2] - matrix.m[2][0]);
	q.z = copysign(q.z, matrix.m[1][0] - matrix.m[0][1]);
	return q;
}

vr::HmdVector3_t LighthouseTracking::GetPosition(vr::HmdMatrix34_t matrix) {
	vr::HmdVector3_t vector;

	vector.v[0] = matrix.m[0][3];
	vector.v[1] = matrix.m[1][3];
	vector.v[2] = matrix.m[2][3];

	return vector;
}

/*
* Parse a Frame with data from the tracking system
*
* Handy reference:
* https://github.com/TomorrowTodayLabs/NewtonVR/blob/master/Assets/SteamVR/Scripts/SteamVR_Utils.cs
*
* Also:
* Open VR Convention (same as OpenGL)
* right-handed system
* +y is up
* +x is to the right
* -z is going away from you
* http://www.3dgep.com/understanding-the-view-matrix/
*
*/
void LighthouseTracking::ParseTrackingFrame() {

	// Process SteamVR device states
	for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
	{
		if (!m_pHMD->IsTrackedDeviceConnected(unDevice))
			continue;

		vr::VRControllerState_t state;
		if (m_pHMD->GetControllerState(unDevice, &state, sizeof(state)))
		{
			vr::TrackedDevicePose_t trackedDevicePose;
			vr::TrackedDevicePose_t *devicePose = &trackedDevicePose;

			vr::TrackedDevicePose_t trackedControllerPose;
			vr::TrackedDevicePose_t *controllerPose = &trackedControllerPose;

			vr::VRControllerState_t controllerState;
			vr::VRControllerState_t *controllerState_ptr = &controllerState;

			vr::HmdVector3_t vector;
			vr::HmdQuaternion_t quaternion;

			vr::ETrackedDeviceClass trackedDeviceClass = vr::VRSystem()->GetTrackedDeviceClass(unDevice);
			switch (trackedDeviceClass) {
			case vr::ETrackedDeviceClass::TrackedDeviceClass_HMD:
				vr::VRSystem()->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseStanding, 0, &trackedDevicePose, 1);

				vector = GetPosition(devicePose->mDeviceToAbsoluteTracking);
				quaternion = GetRotation(devicePose->mDeviceToAbsoluteTracking);

				// print stuff for the HMD here, see controller example below

				break;

			case vr::ETrackedDeviceClass::TrackedDeviceClass_Controller:
				vr::VRSystem()->GetControllerStateWithPose(vr::TrackingUniverseStanding, unDevice, &controllerState, sizeof(controllerState), &trackedControllerPose);



				vector = GetPosition(controllerPose->mDeviceToAbsoluteTracking);
				quaternion = GetRotation(controllerPose->mDeviceToAbsoluteTracking);

				switch (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(unDevice)) {
				case vr::TrackedControllerRole_Invalid:
					// invalide hand... 
					break;

				case vr::TrackedControllerRole_LeftHand:
					char buf[1024];
					sprintf_s(buf, sizeof(buf), "\nLeft Controller\nx: %.2f y: %.2f z: %.2f\n", vector.v[0], vector.v[1], vector.v[2]);
					printf_s(buf);

					sprintf_s(buf, sizeof(buf), "qw: %.2f qx: %.2f qy: %.2f qz: %.2f\n", quaternion.w, quaternion.x, quaternion.y, quaternion.z);
					printf_s(buf);


					break;

				case vr::TrackedControllerRole_RightHand:

					break;

				}

				break;
			}

		}
	}
}



void LighthouseTracking::PrintTransforms() {

	

	// Process SteamVR device states
	for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
	{
		if (!m_pHMD->IsTrackedDeviceConnected(unDevice))
			continue;

		vr::VRControllerState_t state;
		if (m_pHMD->GetControllerState(unDevice, &state, sizeof(state)))
		{

			vr::ETrackedDeviceClass trackedDeviceClass = vr::VRSystem()->GetTrackedDeviceClass(unDevice);
			switch (trackedDeviceClass) {
				//std::cout << "Checking.\n";
			case vr::ETrackedDeviceClass::TrackedDeviceClass_HMD:
				//std::cout << "In HMD. \n";
				HmdId = unDevice;
				break;

			case vr::ETrackedDeviceClass::TrackedDeviceClass_Controller:
				

				switch (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(unDevice)) {
				case vr::TrackedControllerRole_Invalid:
					// invalide hand... 
					break;

				case vr::TrackedControllerRole_LeftHand:
					ControllerRightId = unDevice;
					break;

				case vr::TrackedControllerRole_RightHand:
					ControllerLeftId = unDevice;
					break;
				}

				break;
			}

		}
	}
	vr::TrackedDevicePose_t trackedDevicePose;
	vr::TrackedDevicePose_t *devicePose = &trackedDevicePose;

	vr::TrackedDevicePose_t trackedControllerPose;
	vr::TrackedDevicePose_t *controllerPose = &trackedControllerPose;

	vr::VRControllerState_t controllerState;
	vr::VRControllerState_t *controllerState_ptr = &controllerState;
	
	std::stringstream buffer;

	vr::HmdMatrix34_t matPose;
	std::string telemetry;
	
	//std::cout << "HmdId: " << HmdId  << std::endl;
	if ((HmdId != -1) && (ControllerRightId != -1) && (ControllerLeftId != -1)) {

		//Get the HMD matrix
		vr::VRSystem()->GetControllerStateWithPose(vr::TrackingUniverseStanding, HmdId, &controllerState, sizeof(controllerState), &trackedControllerPose);
		vr::HmdMatrix34_t matPose = controllerPose->mDeviceToAbsoluteTracking;

		buffer << "[" << matPose.m[0][0] << "," << matPose.m[1][0] << "," << matPose.m[2][0] << "," << 0.0 << "," <<
			matPose.m[0][1] << "," << matPose.m[1][1] << "," << matPose.m[2][1] << "," << 0.0 << "," <<
			matPose.m[0][2] << "," << matPose.m[1][2] << "," << matPose.m[2][2] << "," << 0.0 << "," <<
			matPose.m[0][3]*100 << "," << matPose.m[1][3] * 100 << "," << matPose.m[2][3] * 100 << "," << 1.0 << ",";

		//Get the right controller
		vr::VRSystem()->GetControllerStateWithPose(vr::TrackingUniverseStanding, ControllerRightId, &controllerState, sizeof(controllerState), &trackedControllerPose);
		matPose = controllerPose->mDeviceToAbsoluteTracking;

		buffer << matPose.m[0][0] << "," << matPose.m[1][0] << "," << matPose.m[2][0] << "," << 0.0 << "," <<
			matPose.m[0][1] << "," << matPose.m[1][1] << "," << matPose.m[2][1] << "," << 0.0 << "," <<
			matPose.m[0][2] << "," << matPose.m[1][2] << "," << matPose.m[2][2] << "," << 0.0 << "," <<
			matPose.m[0][3] * 100 << "," << matPose.m[1][3] * 100 << "," << matPose.m[2][3] * 100 << "," << 1.0 << ",";

		//Get the left controller
		vr::VRSystem()->GetControllerStateWithPose(vr::TrackingUniverseStanding, ControllerLeftId, &controllerState, sizeof(controllerState), &trackedControllerPose);
		matPose = controllerPose->mDeviceToAbsoluteTracking;

		buffer << matPose.m[0][0] << "," << matPose.m[1][0] << "," << matPose.m[2][0] << "," << 0.0 << "," <<
			matPose.m[0][1] << "," << matPose.m[1][1] << "," << matPose.m[2][1] << "," << 0.0 << "," <<
			matPose.m[0][2] << "," << matPose.m[1][2] << "," << matPose.m[2][2] << "," << 0.0 << "," <<
			matPose.m[0][3] * 100 << "," << matPose.m[1][3] * 100 << "," << matPose.m[2][3] * 100 << "," << 1.0 << "]";
		std::cout << buffer.str() << std::endl;
		
		std::fstream myfile;
		myfile.open("C:/Users/Dave/Desktop/0020_vive_telemetry.txt", std::fstream::app);
		myfile << buffer.str() << std::endl;
		myfile.close(); 
	}
	else
	{
		std::cout << "We lost one of the devices.\n";
	}

	//The ids get set every poll. This allows us to stop if we lose tracking with one of the devices.
	HmdId = -1;
	ControllerRightId = -1;
	ControllerLeftId = -1;


}



