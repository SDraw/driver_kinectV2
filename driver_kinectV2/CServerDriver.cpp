#include "stdafx.h"

//#define DKV2_NO_TRACKING

#include "CServerDriver.h"
#include "CEmulatedDevice.h"
#include "CTrackerVive.h"
#include "CKinectStation.h"
#include "CKinectHandler.h"

#include "CDriverConfig.h"
#include "Utils.h"

enum TrackerBodyPart : size_t
{
    TBP_Hips = 0U,
    TBP_LeftAnkle,
    TBP_RightAnkle,

    TBP_Count
};

const glm::mat4 g_Identity(1.f);
const glm::vec4 g_IdentityPoint(0.f, 0.f, 0.f, 1.f);

const std::vector<std::string> g_MessageNames
{
    "calibrate"
};
enum MessageIndex : size_t
{
    MI_Calibrate = 0U
};

const char* const CServerDriver::ms_interfaces[] = {
    vr::ITrackedDeviceServerDriver_Version,
    vr::IServerTrackedDeviceProvider_Version,
    nullptr
};

CServerDriver::CServerDriver()
{
    m_driverHost = nullptr;
    m_kinectHandler = nullptr;
    m_kinectStation = nullptr;
    m_kinectThread = nullptr;
    m_kinectActive = false;

    m_baseMatrix = g_Identity;
}
CServerDriver::~CServerDriver()
{
}

vr::EVRInitError CServerDriver::Init(vr::IVRDriverContext *pDriverContext)
{
    vr::EVRInitError l_error = vr::VRInitError_Driver_NotLoaded;

    CDriverConfig::Load();
    if(CDriverConfig::IsEnabled())
    {
        VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
        m_driverHost = vr::VRServerDriverHost();

        m_kinectHandler = new CKinectHandler();
        m_kinectActive = true;
        m_kinectThread = new std::thread(&CServerDriver::KinectProcess, this);

        // Add trackers
        CEmulatedDevice::SetInterfaces(m_driverHost, vr::VRProperties());
        for(size_t i = 0U; i < TBP_Count; i++)
        {
            CTrackerVive *l_tracker = new CTrackerVive(i);
            m_trackers.push_back(l_tracker);
            m_driverHost->TrackedDeviceAdded(l_tracker->GetSerial().c_str(), vr::TrackedDeviceClass_GenericTracker, l_tracker);
        }

        // Add fake station as command relay with Kinect model
        m_kinectStation = new CKinectStation(this);
        m_driverHost->TrackedDeviceAdded(m_kinectStation->GetSerial().c_str(), vr::TrackedDeviceClass_TrackingReference, m_kinectStation);

        // Coordinates offset
        const glm::vec3 &l_position = CDriverConfig::GetBasePosition();
        const glm::quat &l_rotation = CDriverConfig::GetBaseRotation();
        m_baseMatrix = glm::translate(g_Identity, l_position) * glm::mat4_cast(l_rotation);
        m_kinectStation->SetPosition(l_position.x, l_position.y, l_position.z);
        m_kinectStation->SetRotation(l_rotation.x, l_rotation.y, l_rotation.z,l_rotation.w);

        l_error = vr::VRInitError_None;
    }

    return l_error;
}

void CServerDriver::Cleanup()
{
    for(auto l_tracker : m_trackers) delete l_tracker;
    m_trackers.clear();
    CEmulatedDevice::SetInterfaces(nullptr, nullptr);

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

    m_driverHost = nullptr;
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
    if(m_kinectHandler)
    {
        if(m_kinectLock.try_lock())
        {
            for(size_t i = 0U; i < TBP_Count; i++)
            {
                const JointData &l_jointData = m_kinectHandler->GetJointData(i);
                glm::vec3 l_position(-l_jointData.x, l_jointData.y, -l_jointData.z);
                glm::quat l_rotation(l_jointData.rw, -l_jointData.rx, l_jointData.ry, -l_jointData.rz);

                glm::mat4 l_localTransform = glm::translate(g_Identity, l_position)*glm::mat4_cast(l_rotation);
                glm::mat4 l_globalTransform = m_baseMatrix*l_localTransform;
                l_position = l_globalTransform*g_IdentityPoint;
                l_rotation = glm::quat_cast(l_globalTransform);

                m_trackers[i]->SetPosition(l_position.x, l_position.y, l_position.z);
                m_trackers[i]->SetRotation(l_rotation.x, l_rotation.y, l_rotation.z, l_rotation.w);
            }
            m_kinectLock.unlock();
        }
    }

    // Update devices
    m_kinectStation->Update();
    for(auto *l_tracker : m_trackers) l_tracker->Update();
}

void CServerDriver::KinectProcess()
{
    std::chrono::milliseconds l_threadDelay(33U);

    bool l_initialized = m_kinectHandler->Initialize();
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
    // Messages: calibrate x y z rx ry rz rw
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
                glm::quat l_rotation(1.f,0.f,0.f,0.f);
                l_stream >> l_position.x >> l_position.y >> l_position.z >> l_rotation.x >> l_rotation.y >> l_rotation.z >> l_rotation.w;

                m_baseMatrix = glm::translate(g_Identity, l_position)*glm::mat4_cast(l_rotation);
                m_kinectStation->SetPosition(l_position.x, l_position.y, l_position.z);
                m_kinectStation->SetRotation(l_rotation.x, l_rotation.y, l_rotation.z,l_rotation.w);
            } break;
        }
    }
}
