#include "stdafx.h"

#include "CServerDriver.h"
#include "CEmulatedDevice.h"
#include "CTrackerVive.h"
#include "CKinectStation.h"
#include "CKinectHandler.h"

#include "CDriverConfig.h"
#include "Utils.h"

const std::vector<std::string> g_MessageNames
{
    "calibrate", "switch"
};
enum MessageIndex : size_t
{
    MI_Calibrate = 0U,
    MI_Switch
};

enum HistoryIndex : size_t
{
    HI_Previous = 0U,
    HI_Last,

    HI_Count
};

const char* const CServerDriver::ms_interfaces[]
{
    vr::ITrackedDeviceServerDriver_Version,
        vr::IServerTrackedDeviceProvider_Version,
        nullptr
};

CServerDriver::CServerDriver()
{
    m_kinectHandler = nullptr;
    m_kinectStation = nullptr;
    for(size_t i = 0U; i < TI_Count; i++) m_trackers[i] = nullptr;
    m_kinectThread = nullptr;
    m_kinectActive = false;
    m_hotkeyState = false;
}
CServerDriver::~CServerDriver()
{
}

vr::EVRInitError CServerDriver::Init(vr::IVRDriverContext *pDriverContext)
{
    CDriverConfig::Load();
    VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);

    m_kinectHandler = new CKinectHandler();
    m_kinectActive = true;
    m_kinectThread = new std::thread(&CServerDriver::KinectProcess, this);

    // Add trackers
    for(size_t i = 0U; i < TI_Count; i++)
    {
        m_trackers[i] = new CTrackerVive(i);
        vr::VRServerDriverHost()->TrackedDeviceAdded(m_trackers[i]->GetSerial().c_str(), vr::TrackedDeviceClass_GenericTracker, m_trackers[i]);
    }

    // Add fake station as command relay with Kinect model
    m_kinectStation = new CKinectStation(this);
    vr::VRServerDriverHost()->TrackedDeviceAdded(m_kinectStation->GetSerial().c_str(), vr::TrackedDeviceClass_TrackingReference, m_kinectStation);

    // Coordinates offset
    const glm::vec3 &l_position = CDriverConfig::GetBasePosition();
    const glm::quat &l_rotation = CDriverConfig::GetBaseRotation();
    m_kinectStation->SetPosition(l_position.x, l_position.y, l_position.z);
    m_kinectStation->SetRotation(l_rotation.x, l_rotation.y, l_rotation.z, l_rotation.w);
    for(size_t i = 0U; i < TI_Count; i++)
    {
        m_trackers[i]->SetOffsetPosition(l_position.x, l_position.y, l_position.z);
        m_trackers[i]->SetOffsetRotation(l_rotation.x, l_rotation.y, l_rotation.z, l_rotation.w);
    }

    return vr::VRInitError_None;
}

void CServerDriver::Cleanup()
{
    for(size_t i = 0U; i < TI_Count; i++)
    {
        delete m_trackers[i];
        m_trackers[i] = nullptr;
    }

    delete m_kinectStation;
    m_kinectStation = nullptr;

    if(m_kinectThread)
    {
        m_kinectActive = false;
        m_kinectThread->join();
        m_kinectThread = nullptr;
    }

    delete m_kinectHandler;
    m_kinectHandler = nullptr;

    for(auto l_history : m_sensorHistory) delete l_history;
    m_sensorHistory.clear();

    VR_CLEANUP_SERVER_DRIVER_CONTEXT();
}

const char* const* CServerDriver::GetInterfaceVersions()
{
    return ms_interfaces;
}

bool CServerDriver::ShouldBlockStandbyMode()
{
    return false;
}

void CServerDriver::RunFrame()
{
    if(m_kinectLock.try_lock())
    {
        const SensorData &l_sensorData = m_kinectHandler->GetSensorData();
        if(m_sensorHistory.size() < HI_Count) m_sensorHistory.push_back(new SensorData(l_sensorData)); // Frame time is irrelevant
        else
        {
            if(m_sensorHistory[HI_Last]->m_frameTime != l_sensorData.m_frameTime)
            {
                // New frame, move data
                std::memcpy(m_sensorHistory[HI_Previous], m_sensorHistory[HI_Last], sizeof(SensorData));
                std::memcpy(m_sensorHistory[HI_Last], &l_sensorData, sizeof(SensorData));
            }
        }
        m_kinectLock.unlock();
    }

    // Smooth history data
    if(m_sensorHistory.size() == HI_Count)
    {
        const ULONGLONG l_diff = (GetTickCount64() - m_sensorHistory[HI_Last]->m_tick);
        float l_smooth = static_cast<float>(l_diff) / 33.333333f;
        l_smooth = glm::clamp(l_smooth, 0.f, 1.5f); // Extra clamp if new frame from Kinect will be slightly delayed

        for(size_t i = 0U; i < TI_Count; i++)
        {
            const JointData &l_jointA = m_sensorHistory[HI_Previous]->m_joints[i];
            const JointData &l_jointB = m_sensorHistory[HI_Last]->m_joints[i];

            const glm::vec3 l_jointPosA(-l_jointA.x, l_jointA.y, -l_jointA.z);
            const glm::vec3 l_jointPosB(-l_jointB.x, l_jointB.y, -l_jointB.z);

            const glm::quat l_jointRotA(l_jointA.rw, -l_jointA.rx, l_jointA.ry, -l_jointA.rz);
            const glm::quat l_jointRotB(l_jointB.rw, -l_jointB.rx, l_jointB.ry, -l_jointB.rz);

            const glm::vec3 l_linearPos = glm::mix(l_jointPosA, l_jointPosB, l_smooth);
            const glm::quat l_linearRot = glm::slerp(l_jointRotA, l_jointRotB, l_smooth);

            m_trackers[i]->SetPosition(l_linearPos.x, l_linearPos.y, l_linearPos.z);
            m_trackers[i]->SetRotation(l_linearRot.x, l_linearRot.y, l_linearRot.z, l_linearRot.w);
        }
    }

    const bool l_hotkeyState = ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && (GetAsyncKeyState(0x54) & 0x8000)); // Ctrl+T
    if(m_hotkeyState != l_hotkeyState)
    {
        m_hotkeyState = l_hotkeyState;
        if(m_hotkeyState)
        {
            // Switch tracking
            for(size_t i = 0U; i < TI_Count; i++)
            {
                const bool l_connected = m_trackers[i]->IsConnected();
                m_trackers[i]->SetConnected(!l_connected);
            }
            m_kinectHandler->SetPaused(!m_kinectHandler->IsPaused());
        }
    }

    // Update devices
    m_kinectStation->RunFrame();
    for(size_t i = 0U; i < TI_Count; i++) m_trackers[i]->RunFrame();
}

void CServerDriver::KinectProcess()
{
    const std::chrono::milliseconds l_threadDelay(33U);

    const bool l_initialized = m_kinectHandler->Initialize();
    while(m_kinectActive)
    {
        if(l_initialized)
        {
            m_kinectLock.lock();
            m_kinectHandler->Update();
            m_kinectLock.unlock();
        }

        std::this_thread::sleep_for(l_threadDelay);
    }

    if(l_initialized) m_kinectHandler->Terminate();
}

void CServerDriver::ProcessExternalMessage(const char *f_message)
{
    // [ Messages ]
    // "calibrate x y z rx ry rz rw" - change base offset
    // "switch" - switch devices and tracking
    std::stringstream l_stream(f_message);

    std::string l_message;
    l_stream >> l_message;
    if(!l_message.empty() && !l_stream.fail())
    {
        switch(ReadEnumVector(l_message, g_MessageNames))
        {
            case MessageIndex::MI_Calibrate:
            {
                glm::vec3 l_position(0.f);
                glm::quat l_rotation(1.f, 0.f, 0.f, 0.f);
                l_stream >> l_position.x >> l_position.y >> l_position.z >> l_rotation.x >> l_rotation.y >> l_rotation.z >> l_rotation.w;

                m_kinectStation->SetPosition(l_position.x, l_position.y, l_position.z);
                m_kinectStation->SetRotation(l_rotation.x, l_rotation.y, l_rotation.z, l_rotation.w);
                for(size_t i = 0U; i < TI_Count; i++)
                {
                    m_trackers[i]->SetOffsetPosition(l_position.x, l_position.y, l_position.z);
                    m_trackers[i]->SetOffsetRotation(l_rotation.x, l_rotation.y, l_rotation.z, l_rotation.w);
                }
            } break;
            case MessageIndex::MI_Switch:
            {
                for(size_t i = 0U; i < TI_Count; i++)
                {
                    const bool l_connected = m_trackers[i]->IsConnected();
                    m_trackers[i]->SetConnected(!l_connected);
                }
                m_kinectHandler->SetPaused(!m_kinectHandler->IsPaused());
            } break;
        }
    }
}
