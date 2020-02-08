#include "stdafx.h"

#include "CEmulatedDevice.h"

vr::IVRServerDriverHost *CEmulatedDevice::ms_driverHost = nullptr;
vr::CVRPropertyHelpers *CEmulatedDevice::ms_vrProperties = nullptr;

CEmulatedDevice::CEmulatedDevice()
{
    m_pose = { 0 };
    m_pose.poseTimeOffset = -0.016;
    m_pose.qWorldFromDriverRotation.w = 1.0;
    m_pose.qWorldFromDriverRotation.x = .0;
    m_pose.qWorldFromDriverRotation.y = .0;
    m_pose.qWorldFromDriverRotation.z = .0;
    m_pose.qDriverFromHeadRotation.w = 1.0;
    m_pose.qDriverFromHeadRotation.x = .0;
    m_pose.qDriverFromHeadRotation.y = .0;
    m_pose.qDriverFromHeadRotation.z = .0;
    m_pose.vecDriverFromHeadTranslation[0U] = .0;
    m_pose.vecDriverFromHeadTranslation[1U] = .0;
    m_pose.vecDriverFromHeadTranslation[2U] = .0;
    m_pose.poseIsValid = false;
    m_pose.willDriftInYaw = false;
    m_pose.shouldApplyHeadModel = false;
    m_pose.result = vr::TrackingResult_Uninitialized;
    m_pose.deviceIsConnected = false;

    m_propertyContainer = vr::k_ulInvalidPropertyContainer;
    m_trackedDevice = vr::k_unTrackedDeviceIndexInvalid;
}
CEmulatedDevice::~CEmulatedDevice()
{
}

vr::EVRInitError CEmulatedDevice::Activate(uint32_t unObjectId)
{
    vr::EVRInitError l_error = vr::VRInitError_Driver_Failed;

    if(m_trackedDevice == vr::k_unTrackedDeviceIndexInvalid)
    {
        m_trackedDevice = unObjectId;
        m_propertyContainer = ms_vrProperties->TrackedDeviceToPropertyContainer(m_trackedDevice);

        SetupProperties();

        m_pose.deviceIsConnected = true;
        m_pose.poseIsValid = true;
        m_pose.result = vr::TrackingResult_Running_OK;

        l_error = vr::VRInitError_None;
    }

    return l_error;
}

void CEmulatedDevice::Deactivate()
{
    m_trackedDevice = vr::k_unTrackedDeviceIndexInvalid;
}

void* CEmulatedDevice::GetComponent(const char* pchComponentNameAndVersion)
{
    void *l_result = nullptr;
    if(!strcmp(pchComponentNameAndVersion, vr::ITrackedDeviceServerDriver_Version)) l_result = this;
    return l_result;
}

vr::DriverPose_t CEmulatedDevice::GetPose()
{
    return m_pose;
}

bool CEmulatedDevice::IsConnected()
{
    return m_pose.deviceIsConnected;
}

void CEmulatedDevice::SetConnected(bool f_state)
{
    m_pose.deviceIsConnected = f_state;
}

void CEmulatedDevice::SetPosition(float f_x, float f_y, float f_z)
{
    m_pose.vecPosition[0U] = f_x;
    m_pose.vecPosition[1U] = f_y;
    m_pose.vecPosition[2U] = f_z;
}

void CEmulatedDevice::SetRotation(float f_x, float f_y, float f_z, float f_w)
{
    m_pose.qRotation.x = f_x;
    m_pose.qRotation.y = f_y;
    m_pose.qRotation.z = f_z;
    m_pose.qRotation.w = f_w;
}

void CEmulatedDevice::SetOffsetPosition(float f_x, float f_y, float f_z)
{
    m_pose.vecWorldFromDriverTranslation[0U] = f_x;
    m_pose.vecWorldFromDriverTranslation[1U] = f_y;
    m_pose.vecWorldFromDriverTranslation[2U] = f_z;
}

void CEmulatedDevice::SetOffsetRotation(float f_x, float f_y, float f_z, float f_w)
{
    m_pose.qWorldFromDriverRotation.x = f_x;
    m_pose.qWorldFromDriverRotation.y = f_y;
    m_pose.qWorldFromDriverRotation.z = f_z;
    m_pose.qWorldFromDriverRotation.w = f_w;
}

void CEmulatedDevice::Update()
{
    if(m_trackedDevice != vr::k_unTrackedDeviceIndexInvalid)
    {
        ms_driverHost->TrackedDevicePoseUpdated(m_trackedDevice, m_pose, sizeof(vr::DriverPose_t));
    }
}

void CEmulatedDevice::SetInterfaces(vr::IVRServerDriverHost *f_host, vr::CVRPropertyHelpers *f_properties)
{
    ms_driverHost = f_host;
    ms_vrProperties = f_properties;
}
