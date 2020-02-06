#include "stdafx.h"

#include "CKinectMonitor.h"
#include "CKinectConfig.h"

const char *g_NotificationText = "Base calibration\nLeft touchpad: Up - Down - Left - Right\tRight touchpad: Forward - Backward - Rotate left - Rotate right";
const std::chrono::milliseconds g_ThreadDelay(11U);

const float g_Pi = glm::pi<float>();
const glm::vec3 g_AxisX(1.f, 0.f, 0.f);
const glm::vec3 g_AxisXN(-1.f, 0.f, 0.f);
const glm::vec3 g_AxisY(0.f, 1.f, 0.f);
const glm::vec3 g_AxisYN(0.f, -1.f, 0.f);
const glm::vec3 g_AxisZ(0.f, 0.f, 1.f);
const glm::vec3 g_AxisZN(0.f, 0.f, -1.f);

enum TouchpadQuadrant : unsigned char
{
    TQ_Right = 0U,
    TQ_Up,
    TQ_Left,
    TQ_Down,

    TQ_Unknown = 255U
};

CKinectMonitor::CKinectMonitor()
{
    m_active = false;
    m_vrSystem = nullptr;
    m_vrOverlay = nullptr;
    m_vrNotifications = nullptr;
    m_vrDebug = nullptr;
    m_overlay = vr::k_ulOverlayHandleInvalid;
    m_notification = 0U;
    m_kinectDevice = vr::k_unTrackedDeviceIndexInvalid;
    m_rightHand = vr::k_unTrackedDeviceIndexInvalid;
    m_leftHand = vr::k_unTrackedDeviceIndexInvalid;
    m_event = { 0 };
    m_kinectConfig = nullptr;
    m_basePosition = glm::vec3(0.f);
    m_baseRotation = glm::quat(1.f, 0.f, 0.f, 0.f);
}
CKinectMonitor::~CKinectMonitor()
{
}

bool CKinectMonitor::Initialize()
{
    if(!m_vrSystem)
    {
        vr::EVRInitError l_initError = vr::VRInitError_None;
        m_vrSystem = vr::VR_Init(&l_initError, vr::VRApplication_Overlay);
        if(l_initError == vr::VRInitError_None)
        {
            m_vrOverlay = vr::VROverlay();
            m_vrNotifications = vr::VRNotifications();
            m_vrDebug = vr::VRDebug();

            m_vrOverlay->CreateOverlay("monitor.kinect.main", "Kinect Calibrator", &m_overlay);

            for(uint32_t i = 0U; i < vr::k_unMaxTrackedDeviceCount; i++)
            {
                if(m_vrSystem->IsTrackedDeviceConnected(i))
                {
                    if(m_vrSystem->GetUint64TrackedDeviceProperty(i, vr::Prop_VendorSpecific_Reserved_Start) == 0x4B696E6563745632)
                    {
                        m_kinectDevice = i;
                        break;
                    }
                }
            }
            m_leftHand = m_vrSystem->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);
            m_rightHand = m_vrSystem->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);

            m_kinectConfig = new CKinectConfig("\\..\\..\\resources\\settings.xml");
            m_kinectConfig->Load();
            m_basePosition = m_kinectConfig->GetPosition();
            m_baseRotation = m_kinectConfig->GetRotation();

            m_vrNotifications->CreateNotification(m_overlay, 0U, vr::EVRNotificationType_Persistent, g_NotificationText, vr::EVRNotificationStyle_Application, nullptr, &m_notification);

            if(m_kinectDevice == vr::k_unTrackedDeviceIndexInvalid) std::cout << "[!] Unable to find Kinect base station device. Calibration will be performed without real-time results." << std::endl;
            if(m_leftHand == vr::k_unTrackedDeviceIndexInvalid) std::cout << "[!] Right controller isn't active." << std::endl;
            if(m_rightHand == vr::k_unTrackedDeviceIndexInvalid) std::cout << "[!] Left controller isn't active." << std::endl;
            std::cout << "[i] Follow instructions in notification." << std::endl;
            std::cout << "[i] Calibrated data will be saved after application menu button press or SteamVR shutdown." << std::endl;

            m_active = true;
        }
        else
        {
            std::cout << "[!] Unable to launch. Reason: " << vr::VR_GetVRInitErrorAsEnglishDescription(l_initError) << std::endl;

            m_vrSystem = nullptr;
        }
    }
    return (m_vrSystem != nullptr);
}
void CKinectMonitor::Terminate()
{
    if(m_vrSystem)
    {
        if(m_notification != 0U)
        {
            m_vrNotifications->RemoveNotification(m_notification);
            m_notification = 0U;
        }
        if(m_overlay != vr::k_ulOverlayHandleInvalid)
        {
            m_vrOverlay->DestroyOverlay(m_overlay);
            m_overlay = vr::k_ulOverlayHandleInvalid;
        }

        m_vrSystem = nullptr;
        m_vrOverlay = nullptr;
        m_vrNotifications = nullptr;
        m_vrDebug = nullptr;

        m_kinectDevice = vr::k_unTrackedDeviceIndexInvalid;
        m_rightHand = vr::k_unTrackedDeviceIndexInvalid;
        m_leftHand = vr::k_unTrackedDeviceIndexInvalid;

        delete m_kinectConfig;
        m_kinectConfig = nullptr;
    }
}

void CKinectMonitor::SendCalibration()
{
    if(m_kinectDevice != vr::k_unTrackedDeviceIndexInvalid)
    {
        std::string l_data("calibrate");
        for(int i = 0; i < 3; i++)
        {
            l_data.push_back(' ');
            l_data.append(std::to_string(m_basePosition[i]));
        }
        for(int i = 0; i < 4; i++)
        {
            l_data.push_back(' ');
            l_data.append(std::to_string(m_baseRotation[i]));
        }

        char l_response[32U];
        m_vrDebug->DriverDebugRequest(m_kinectDevice, l_data.c_str(), l_response, 32U);
    }
}

bool CKinectMonitor::DoPulse()
{
    while(m_vrSystem->PollNextEvent(&m_event, sizeof(vr::VREvent_t)))
    {
        switch(m_event.eventType)
        {
            case vr::VREvent_TrackedDeviceDeactivated:
            {
                if(m_leftHand == m_event.trackedDeviceIndex)
                {
                    m_leftHand = vr::k_unTrackedDeviceIndexInvalid;
                    std::cout << "[!] Left controller is deactivated." << std::endl;
                }
                else if(m_rightHand == m_event.trackedDeviceIndex)
                {
                    m_rightHand = vr::k_unTrackedDeviceIndexInvalid;
                    std::cout << "[!] Right controller is deactivated." << std::endl;
                }
            } break;

            case vr::VREvent_ButtonPress:
            {
                switch(m_event.data.controller.button)
                {
                    case vr::k_EButton_SteamVR_Touchpad:
                    {
                        vr::VRControllerState_t l_state;
                        m_vrSystem->GetControllerState(m_event.trackedDeviceIndex, &l_state, sizeof(vr::VRControllerState_t));

                        float l_theta = atan2f(l_state.rAxis[0U].y, l_state.rAxis[0U].x);
                        float l_absTheta = abs(l_theta);

                        TouchpadQuadrant l_quad = TQ_Unknown;
                        if((l_absTheta >= 0.f) && (l_absTheta <= g_Pi*0.25f)) l_quad = TQ_Right;
                        else
                        {
                            if((l_absTheta >= g_Pi*0.75) && (l_absTheta <= g_Pi)) l_quad = TQ_Left;
                            else l_quad = (l_theta > 0.f) ? TQ_Up : TQ_Down;
                        }

                        if(m_event.trackedDeviceIndex == m_leftHand)
                        {
                            switch(l_quad)
                            {
                                case TQ_Right:
                                    m_basePosition += (m_baseRotation*g_AxisX) * 0.01f;
                                    break;
                                case TQ_Left:
                                    m_basePosition += (m_baseRotation*g_AxisXN)* 0.01f;
                                    break;
                                case TQ_Up:
                                    m_basePosition += (m_baseRotation*g_AxisY) * 0.01f;
                                    break;
                                case TQ_Down:
                                    m_basePosition += (m_baseRotation*g_AxisYN) * 0.01f;
                                    break;
                            }
                        }
                        else if(m_event.trackedDeviceIndex == m_rightHand)
                        {
                            switch(l_quad)
                            {
                                case TQ_Right:
                                    m_baseRotation = glm::rotate(m_baseRotation, g_Pi / 180.f, g_AxisY);
                                    break;
                                case TQ_Left:
                                    m_baseRotation = glm::rotate(m_baseRotation, -g_Pi / 180.f, g_AxisY);
                                    break;
                                case TQ_Up:
                                    m_basePosition += (m_baseRotation*g_AxisZN) * 0.01f;
                                    break;
                                case TQ_Down:
                                    m_basePosition += (m_baseRotation*g_AxisZ) * 0.01f;
                                    break;
                            }
                        }

                        m_kinectConfig->SetPosition(m_basePosition);
                        m_kinectConfig->SetRotation(m_baseRotation);
                        SendCalibration();
                    } break;

                    case vr::k_EButton_ApplicationMenu:
                    {
                        m_kinectConfig->Save();
                        m_active = false;
                    } break;
                }
            } break;

            case vr::VREvent_Quit:
            {
                m_kinectConfig->Save();
                m_active = false;
            } break;
        }
    }

    if(m_leftHand == vr::k_unTrackedDeviceIndexInvalid)
    {
        m_leftHand = m_vrSystem->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);
        if(m_leftHand != vr::k_unTrackedDeviceIndexInvalid) std::cout << "[i] Left controller is activated." << std::endl;
    }
    if(m_rightHand == vr::k_unTrackedDeviceIndexInvalid)
    {
        m_rightHand = m_vrSystem->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);
        if(m_rightHand != vr::k_unTrackedDeviceIndexInvalid) std::cout << "[i] Right controller is activated." << std::endl;
    }

    std::this_thread::sleep_for(g_ThreadDelay);
    return m_active;
}
