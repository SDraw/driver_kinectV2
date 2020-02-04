#pragma once


class CEmulatedDevice : public vr::ITrackedDeviceServerDriver
{
    static vr::IVRServerDriverHost *ms_driverHost;
    vr::DriverPose_t m_pose;

    CEmulatedDevice(const CEmulatedDevice &that) = delete;
    CEmulatedDevice& operator=(const CEmulatedDevice &that) = delete;

    virtual void SetupProperties() {};

    // vr::ITrackedDeviceServerDriver
    vr::EVRInitError Activate(uint32_t unObjectId);
    void Deactivate();
    virtual void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) {}
    void EnterStandby() {}
    void* GetComponent(const char* pchComponentNameAndVersion);
    vr::DriverPose_t GetPose();
protected:
    static vr::CVRPropertyHelpers *ms_vrProperties;
    vr::PropertyContainerHandle_t m_propertyContainer;
    uint32_t m_trackedDevice;

    std::string m_serial;

    CEmulatedDevice();
    virtual ~CEmulatedDevice();

    inline const std::string& GetSerial() const { return m_serial; }

    void SetPosition(float f_x, float f_y, float f_z);
    void SetRotation(float f_x, float f_y, float f_z, float f_w);

    void Update();

    static void SetInterfaces(vr::IVRServerDriverHost *f_host, vr::CVRPropertyHelpers *f_properties);
    
    friend class CServerDriver;
};
