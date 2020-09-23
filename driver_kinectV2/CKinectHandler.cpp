#include "stdafx.h"

#include "CKinectHandler.h"
#include "CJointFilter.h"

CKinectHandler::CKinectHandler(const std::vector<size_t> &f_indexes)
{
    m_kinectSensor = nullptr;
    m_bodyFrameReader = nullptr;

    for(size_t i = 0U; i < JointType_Count; i++)
    {
        m_jointUsed[i] = false;
        m_jointFilters[i] = nullptr;
    }
    for(auto l_index : f_indexes)
    {
        if(l_index >= JointType_Count) continue;
        if(!m_jointUsed[l_index])
        {
            m_jointUsed[l_index] = true;
            m_jointFilters[l_index] = new CJointFilter();
        }
    }

    m_frameData = new FrameData();
    m_frameData->m_frameTime = 0;
    m_frameData->m_tick = 0U;

    m_paused = false;
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
            IBodyFrameSource *l_bodyFrameSource = nullptr;
            if(m_kinectSensor->Open() >= S_OK)
            {
                if(m_kinectSensor->get_BodyFrameSource(&l_bodyFrameSource) >= S_OK)
                {
                    if(l_bodyFrameSource->OpenReader(&m_bodyFrameReader) >= S_OK)
                    {
                        m_paused = false;
                        l_result = true;
                    }
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
    for(size_t i = 0U; i < JointType_Count; i++)
    {
        m_jointUsed[i] = false;
        delete m_jointFilters[i];
        m_jointFilters[i] = nullptr;
    }

    delete m_frameData;
    m_frameData = nullptr;

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

    m_paused = false;
}

const FrameData* CKinectHandler::GetFrameData() const
{
    return m_frameData;
}

bool CKinectHandler::IsPaused() const
{
    return m_paused;
}

void CKinectHandler::SetPaused(bool f_state)
{
    if(m_kinectSensor && m_bodyFrameReader) m_paused = f_state;
}

void CKinectHandler::Update()
{
    if(m_kinectSensor && m_bodyFrameReader && !m_paused)
    {
        IBodyFrame* l_bodyFrame = nullptr;
        if(m_bodyFrameReader->AcquireLatestFrame(&l_bodyFrame) >= S_OK)
        {
            TIMESPAN l_timeSpan = 0;
            l_bodyFrame->get_RelativeTime(&l_timeSpan);
            if(l_timeSpan != m_frameData->m_frameTime)
            {
                m_frameData->m_frameTime = l_timeSpan;
                m_frameData->m_tick = GetTickCount64();
            }

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
                                    for(size_t k = 0U; k < JointType_Count; k++)
                                    {
                                        if(m_jointUsed[k])
                                        {
                                            m_jointFilters[k]->Update(l_joints[k]);
                                            const glm::vec3 &l_position = m_jointFilters[k]->GetFiltered();
                                            JointData &l_jointData = m_frameData->m_joints[k];
                                            l_jointData.x = l_position.x;
                                            l_jointData.y = l_position.y;
                                            l_jointData.z = l_position.z;

                                            const Vector4 &l_kinectRotation = l_jointOrientations[k].Orientation;
                                            const glm::quat l_newRotation(l_kinectRotation.w, l_kinectRotation.x, l_kinectRotation.y, l_kinectRotation.z);
                                            const glm::quat l_oldRotation(l_jointData.rw, l_jointData.rx, l_jointData.ry, l_jointData.rz);
                                            const glm::quat l_smoothedRotation = glm::slerp(l_oldRotation, l_newRotation, 0.75f);
                                            l_jointData.rx = l_smoothedRotation.x;
                                            l_jointData.ry = l_smoothedRotation.y;
                                            l_jointData.rz = l_smoothedRotation.z;
                                            l_jointData.rw = l_smoothedRotation.w;
                                        }
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
