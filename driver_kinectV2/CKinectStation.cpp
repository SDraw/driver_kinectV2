#include "stdafx.h"

#include "CKinectStation.h"
#include "CServerDriver.h"

CKinectStation::CKinectStation(CServerDriver *f_server)
{
    m_serverDriver = f_server;
    m_serial.assign("KS-0000");
}
CKinectStation::~CKinectStation()
{
}

void CKinectStation::SetupProperties()
{
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_TrackingSystemName_String, "kinect");
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_SerialNumber_String, m_serial.c_str());
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_ModelNumber_String, "Kinect Sensor 2.0");
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_ManufacturerName_String, "Microsoft");
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_ModeLabel_String, "S");

    ms_vrProperties->SetInt32Property(m_propertyContainer, vr::Prop_DeviceClass_Int32, vr::TrackedDeviceClass_TrackingReference);
    ms_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_IsOnDesktop_Bool, false);
    ms_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_NeverTracked_Bool, false);
    ms_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_WillDriftInYaw_Bool, false);
    ms_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_CanWirelessIdentify_Bool, false);

    ms_vrProperties->SetFloatProperty(m_propertyContainer, vr::Prop_FieldOfViewLeftDegrees_Float, 70.f);
    ms_vrProperties->SetFloatProperty(m_propertyContainer, vr::Prop_FieldOfViewRightDegrees_Float, 70.f);
    ms_vrProperties->SetFloatProperty(m_propertyContainer, vr::Prop_FieldOfViewTopDegrees_Float, 60.f);
    ms_vrProperties->SetFloatProperty(m_propertyContainer, vr::Prop_FieldOfViewBottomDegrees_Float, 60.f);
    ms_vrProperties->SetFloatProperty(m_propertyContainer, vr::Prop_TrackingRangeMinimumMeters_Float, 0.5f);
    ms_vrProperties->SetFloatProperty(m_propertyContainer, vr::Prop_TrackingRangeMaximumMeters_Float, 4.5f);

    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_ResourceRoot_String, "kinectV2");
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_RenderModelName_String, "{kinectV2}kinect_sensor_2_0");

    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceOff_String, "{kinectV2}/icons/base_status_off.png");
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceSearching_String, "{kinectV2}/icons/base_status_searching.gif");
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceSearchingAlert_String, "{kinectV2}/icons/base_status_searching_alert.gif");
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceReady_String, "{kinectV2}/icons/base_status_ready.png");
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceReadyAlert_String, "{kinectV2}/icons/base_status_ready_alert.png");
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceNotReady_String, "{kinectV2}/icons/base_status_error.png");
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceStandby_String, "{kinectV2}/icons/base_status_standby.png");

    ms_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_HasDisplayComponent_Bool, false);
    ms_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_HasCameraComponent_Bool, false);
    ms_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_HasDriverDirectModeComponent_Bool, false);
    ms_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_HasVirtualDisplayComponent_Bool, false);

    ms_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_VendorSpecific_Reserved_Start, 0x4B696E6563745632); // "KinectV2"
}

void CKinectStation::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{
    m_serverDriver->ProcessExternalMessage(pchRequest);
}
