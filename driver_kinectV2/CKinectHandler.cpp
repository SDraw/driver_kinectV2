#include "stdafx.h"

#include "CKinectHandler.h"
#include "CJointFilter.h"

size_t g_AssignedTrackerJoint[3U] = {
    JointType_SpineBase,
    JointType_AnkleLeft,
    JointType_AnkleRight,
};

CKinectHandler::CKinectHandler()
{
    m_kinectSensor = nullptr;
    m_bodyFrameReader = nullptr;
    for(size_t i = 0U; i < JI_Count; i++)
    {
        m_jointData[i] = { 0.f };
        m_jointFilters.push_back(new CJointFilter());
    }
}
CKinectHandler::~CKinectHandler()
{
    Cleanup();
}

bool CKinectHandler::Initialize()
{
    bool l_result = false;
    if(!m_kinectSensor)
    {
        if(GetDefaultKinectSensor(&m_kinectSensor) >= S_OK)
        {
            wchar_t l_wideString[32U];
            if(m_kinectSensor->get_UniqueKinectId(32U, l_wideString) >= S_OK)
            {
                char l_string[32U];
                size_t l_written = 0U;
                wcstombs_s(&l_written, l_string, l_wideString, 32U);
                m_uniqueId.assign(l_string);
                m_uniqueId.resize(l_written);
            }
            else m_uniqueId.assign("Kinect_V2");

            IBodyFrameSource* l_bodyFrameSource = nullptr;

            if(m_kinectSensor->Open() >= S_OK)
            {
                if(m_kinectSensor->get_BodyFrameSource(&l_bodyFrameSource) >= S_OK)
                {
                    l_result = (l_bodyFrameSource->OpenReader(&m_bodyFrameReader) >= S_OK);
                }
            }

            if(l_bodyFrameSource) l_bodyFrameSource->Release();
        }
    }
    if(!l_result) Cleanup();
    return l_result;
}
void CKinectHandler::Terminate()
{
    Cleanup();
}

void CKinectHandler::Cleanup()
{
    for(auto l_filter : m_jointFilters) delete l_filter;
    m_jointFilters.clear();

    if(m_bodyFrameReader)
    {
        m_bodyFrameReader->Release();
        m_bodyFrameReader = nullptr;
    }

    if(m_kinectSensor)
    {
        m_kinectSensor->Close();
        m_kinectSensor->Release();
        m_kinectSensor = nullptr;
    }
}

void CKinectHandler::Update()
{
    if(m_kinectSensor && m_bodyFrameReader)
    {
        IBodyFrame* l_bodyFrame = nullptr;
        if(m_bodyFrameReader->AcquireLatestFrame(&l_bodyFrame) >= S_OK)
        {
            IBody *l_bodies[BODY_COUNT] = { nullptr };
            if(l_bodyFrame->GetAndRefreshBodyData(BODY_COUNT, l_bodies) >= S_OK) // Only first visible body
            {
                for(size_t i = 0U, j = static_cast<size_t>(BODY_COUNT); i < j; i++)
                {
                    if(l_bodies[i])
                    {
                        unsigned char l_isTracked = 0U;
                        if(l_bodies[i]->get_IsTracked(&l_isTracked) >= S_OK)
                        {
                            if(l_isTracked > 0U)
                            {
                                Joint l_joints[JointType_Count];
                                JointOrientation l_jointOrientations[JointType_Count];
                                if((l_bodies[i]->GetJoints(JointType_Count, l_joints) >= S_OK) && (l_bodies[i]->GetJointOrientations(JointType_Count, l_jointOrientations) >= S_OK))
                                {
                                    for(size_t k = 0U; k < JI_Count; k++)
                                    {
                                        m_jointFilters[k]->Update(l_joints[g_AssignedTrackerJoint[k]]);
                                        const glm::vec3& l_position = m_jointFilters[k]->GetFiltered();
                                        m_jointData[k].x = l_position.x;
                                        m_jointData[k].y = l_position.y;
                                        m_jointData[k].z = l_position.z;

                                        const Vector4 &l_kinectRotation = l_jointOrientations[g_AssignedTrackerJoint[k]].Orientation;
                                        glm::quat l_newRotation(l_kinectRotation.w, l_kinectRotation.x, l_kinectRotation.y, l_kinectRotation.z);
                                        glm::quat l_oldRotation(m_jointData[k].rw, m_jointData[k].rx, m_jointData[k].ry, m_jointData[k].rz);
                                        glm::quat l_smoothedRotation = glm::slerp(l_oldRotation, l_newRotation, 0.7f);
                                        m_jointData[k].rx = l_smoothedRotation.x;
                                        m_jointData[k].ry = l_smoothedRotation.y;
                                        m_jointData[k].rz = l_smoothedRotation.z;
                                        m_jointData[k].rw = l_smoothedRotation.w;
                                    }
                                }

                                break;
                            }
                        }
                    }
                }
            }

            for(size_t i = 0U, j = static_cast<size_t>(BODY_COUNT); i < j; i++)
            {
                if(l_bodies[i]) l_bodies[i]->Release();
            }
        }

        if(l_bodyFrame) l_bodyFrame->Release();
    }
}
