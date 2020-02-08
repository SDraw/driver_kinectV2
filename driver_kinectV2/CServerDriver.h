#pragma once

class CEmulatedDevice;
class CKinectHandler;

class CServerDriver final : public vr::IServerTrackedDeviceProvider
{
    vr::IVRServerDriverHost* m_driverHost;
    static const char* const ms_interfaces[];

    CKinectHandler *m_kinectHandler;
    std::thread *m_kinectThread;
    std::mutex m_kinectLock;
    std::atomic<bool> m_kinectActive;

    CEmulatedDevice *m_kinectStation;
    std::vector<CEmulatedDevice*> m_trackers;

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
