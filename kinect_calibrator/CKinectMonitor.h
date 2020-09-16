#pragma once

class CKinectConfig;

class CKinectMonitor final
{
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
