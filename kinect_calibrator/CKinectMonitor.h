#pragma once

class CKinectConfig;

class CKinectMonitor final
{
    enum OverlayIndex : size_t
    {
        OI_Base = 0U,
        OI_SwitchCalibration,
        OI_SwitchTrackers,

        OI_CalibrationUp,
        OI_CalibrationDown,
        OI_CalibrationLeft,
        OI_CalibrationRight,
        OI_CalibrationForward,
        OI_CalibrationBackward,
        OI_CalibrationRotateLeft,
        OI_CalibrationRotateRight,
        OI_CalibrationEnd, // Near user to calibrate properly

        OI_Count
    };

    bool m_active;

    vr::IVRSystem *m_vrSystem;
    vr::VROverlayHandle_t m_overlay;
    vr::VRNotificationId m_notification;
    vr::TrackedDeviceIndex_t m_kinectDevice;
    vr::TrackedDeviceIndex_t m_leftHand;
    vr::TrackedDeviceIndex_t m_rightHand;
    vr::VREvent_t m_event;

    CKinectConfig *m_kinectConfig;
    glm::vec3 m_basePosition;
    glm::quat m_baseRotation;
    bool m_triggerPressed;

    CKinectMonitor(const CKinectMonitor &that) = delete;
    CKinectMonitor& operator=(const CKinectMonitor &that) = delete;

    void SendCalibration();
public:
    CKinectMonitor();
    ~CKinectMonitor();

    bool Initialize();
    void Terminate();
    bool DoPulse();
};
