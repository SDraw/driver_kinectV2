#pragma once

class CEmulatedDevice;
class CKinectHandler;
struct SensorData;

class CServerDriver final : public vr::IServerTrackedDeviceProvider
{
    enum TrackerIndex : size_t
    {
        TI_Hips = 0U,
        TI_LeftAnkle,
        TI_RightAnkle,

        TI_Count
    };

    static const char* const ms_interfaces[];

    CKinectHandler *m_kinectHandler;
    std::thread *m_kinectThread;
    std::mutex m_kinectLock;
    std::atomic<bool> m_kinectActive;
    std::vector<SensorData*> m_sensorHistory;

    CEmulatedDevice *m_kinectStation;
    CEmulatedDevice *m_trackers[TI_Count];

    bool m_hotkeyState;

    CServerDriver(const CServerDriver &that) = delete;
    CServerDriver& operator=(const CServerDriver &that) = delete;

    void KinectProcess();

    // vr::IServerTrackedDeviceProvider
    vr::EVRInitError Init(vr::IVRDriverContext *pDriverContext);
    void Cleanup();
    void EnterStandby() {};
    const char* const* GetInterfaceVersions();
    void LeaveStandby() {};
    bool ShouldBlockStandbyMode();
    void RunFrame();
public:
    CServerDriver();
    ~CServerDriver();

    void ProcessExternalMessage(const char *f_message);
};
