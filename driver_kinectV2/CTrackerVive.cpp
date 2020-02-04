#include "stdafx.h"

#include "CTrackerVive.h"

CTrackerVive::CTrackerVive(size_t f_index)
{
    m_serial.assign("LHR-CB0CD00");
    m_serial.append(std::to_string(f_index));
}
CTrackerVive::~CTrackerVive()
{
}

void CTrackerVive::SetupProperties()
{
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_TrackingSystemName_String, "lighthouse");
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_ModelNumber_String, "Vive Tracker Pro MV");
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_SerialNumber_String, m_serial.c_str()); // Changed
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_RenderModelName_String, "{htc}vr_tracker_vive_1_0");
    ms_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_WillDriftInYaw_Bool, false);
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_ManufacturerName_String, "HTC");
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_TrackingFirmwareVersion_String, "1541800000 RUNNER-WATCHMAN$runner-watchman@runner-watchman 2018-01-01 FPGA 512(2.56/0/0) BL 0 VRC 1541800000 Radio 1518800000"); // Changed
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_HardwareRevision_String, "product 128 rev 2.5.6 lot 2000/0/0 0"); // Changed
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_ConnectedWirelessDongle_String, "D0000BE000"); // Changed
    ms_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_DeviceIsWireless_Bool, true);
    ms_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_DeviceIsCharging_Bool, false);
    ms_vrProperties->SetFloatProperty(m_propertyContainer, vr::Prop_DeviceBatteryPercentage_Float, 1.f); // Always charged

    vr::HmdMatrix34_t l_transform = { -1.f, 0.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f };
    ms_vrProperties->SetProperty(m_propertyContainer, vr::Prop_StatusDisplayTransform_Matrix34, &l_transform, sizeof(vr::HmdMatrix34_t), vr::k_unHmdMatrix34PropertyTag);

    ms_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_Firmware_UpdateAvailable_Bool, false);
    ms_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_Firmware_ManualUpdate_Bool, false);
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_Firmware_ManualUpdateURL_String, "https://developer.valvesoftware.com/wiki/SteamVR/HowTo_Update_Firmware");
    ms_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_HardwareRevision_Uint64, 2214720000); // Changed
    ms_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_FirmwareVersion_Uint64, 1541800000); // Changed
    ms_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_FPGAVersion_Uint64, 512); // Changed
    ms_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_VRCVersion_Uint64, 1514800000); // Changed
    ms_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_RadioVersion_Uint64, 1518800000); // Changed
    ms_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_DongleVersion_Uint64, 8933539758); // Changed, based on vr::Prop_ConnectedWirelessDongle_String above
    ms_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_DeviceProvidesBatteryStatus_Bool, true);
    ms_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_DeviceCanPowerOff_Bool, true);
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_Firmware_ProgrammingTarget_String, m_serial.c_str());
    ms_vrProperties->SetInt32Property(m_propertyContainer, vr::Prop_DeviceClass_Int32, vr::TrackedDeviceClass_GenericTracker);
    ms_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_Firmware_ForceUpdateRequired_Bool, false);
    //ms_vrProperties->SetUint64Property(m_propertyContainer, vr::Prop_ParentDriver_Uint64, 8589934597); // Strange value from dump
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_ResourceRoot_String, "htc");

    std::string l_registeredType("htc/vive_tracker");
    l_registeredType.append(m_serial);
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_RegisteredDeviceType_String, l_registeredType.c_str());

    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_InputProfilePath_String, "{htc}/input/vive_tracker_profile.json");
    ms_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_Identifiable_Bool, false);
    ms_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_Firmware_RemindUpdate_Bool, false);
    ms_vrProperties->SetInt32Property(m_propertyContainer, vr::Prop_ControllerRoleHint_Int32, vr::TrackedControllerRole_Invalid);
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_ControllerType_String, "vive_tracker_handed");
    ms_vrProperties->SetInt32Property(m_propertyContainer, vr::Prop_ControllerHandSelectionPriority_Int32, -1);
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceOff_String, "{htc}/icons/tracker_status_off.png");
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceSearching_String, "{htc}/icons/tracker_status_searching.gif");
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceSearchingAlert_String, "{htc}/icons/tracker_status_searching_alert.gif");
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceReady_String, "{htc}/icons/tracker_status_ready.png");
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceReadyAlert_String, "{htc}/icons/tracker_status_ready_alert.png");
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceNotReady_String, "{htc}/icons/tracker_status_error.png");
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceStandby_String, "{htc}/icons/tracker_status_standby.png");
    ms_vrProperties->SetStringProperty(m_propertyContainer, vr::Prop_NamedIconPathDeviceAlertLow_String, "{htc}/icons/tracker_status_ready_low.png");
    ms_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_HasDisplayComponent_Bool, false);
    ms_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_HasCameraComponent_Bool, false);
    ms_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_HasDriverDirectModeComponent_Bool, false);
    ms_vrProperties->SetBoolProperty(m_propertyContainer, vr::Prop_HasVirtualDisplayComponent_Bool, false);
}
