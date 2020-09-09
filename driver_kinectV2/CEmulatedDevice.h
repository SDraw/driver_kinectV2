#pragma once


class CEmulatedDevice : public vr::ITrackedDeviceServerDriver
{
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
public:
    inline const std::string& GetSerial() const { return m_serial; }
    bool IsConnected();
protected:
    vr::PropertyContainerHandle_t m_propertyContainer;
    uint32_t m_trackedDevice;

    std::string m_serial;

    CEmulatedDevice();
    virtual ~CEmulatedDevice();

    void SetConnected(bool f_state);

    void SetPosition(float f_x, float f_y, float f_z);
    void SetRotation(float f_x, float f_y, float f_z, float f_w);

    void SetOffsetPosition(float f_x, float f_y, float f_z);
    void SetOffsetRotation(float f_x, float f_y, float f_z, float f_w);

    void RunFrame();

    friend class CServerDriver;
};
