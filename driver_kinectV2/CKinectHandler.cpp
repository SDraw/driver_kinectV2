#include "stdafx.h"

#include "CKinectHandler.h"
#include "CJointFilter.h"

CKinectHandler::CKinectHandler()
{
    m_kinectSensor = nullptr;
    m_bodyFrameReader = nullptr;

    for(size_t i = 0U; i < _JointType::JointType_Count; i++) m_jointFilters[i] = new CJointFilter();

    m_frameData = new FrameData();
    m_frameData->m_frameTime = 0;
    m_frameData->m_tick = 0U;

    m_paused = false;

    detectionFOV_2 = DegreeToRadian(90.0f);
    circleMinRadins= DegreeToRadian(-180.0f);
    circleMaxRadins = DegreeToRadian(180.0f);
    radinsDeg180 = DegreeToRadian(180.f);
}

CKinectHandler::~CKinectHandler()
{
    Cleanup();

    delete m_frameData;
    m_frameData = nullptr;
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
    for(size_t i = 0U; i < _JointType::JointType_Count; i++)
    {
        delete m_jointFilters[i];
        m_jointFilters[i] = nullptr;
    }

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
    m_paused = f_state;
}

float CKinectHandler::DegreeToRadian(float degrees)
{
    return degrees * (float)M_PI / 180.0f;
}

vr::HmdQuaternionf_t CKinectHandler::GetRotation(vr::HmdMatrix34_t matrix)
{
    vr::HmdQuaternionf_t q;
    q.w = sqrtf(fmaxf(0, 1 + matrix.m[0][0] + matrix.m[1][1] + matrix.m[2][2])) / 2.0f;
    q.x = sqrtf(fmaxf(0, 1 + matrix.m[0][0] - matrix.m[1][1] - matrix.m[2][2])) / 2.0f;
    q.y = sqrtf(fmaxf(0, 1 - matrix.m[0][0] + matrix.m[1][1] - matrix.m[2][2])) / 2.0f;
    q.z = sqrtf(fmaxf(0, 1 - matrix.m[0][0] - matrix.m[1][1] + matrix.m[2][2])) / 2.0f;
    q.x = copysignf(q.x, matrix.m[2][1] - matrix.m[1][2]);
    q.y = copysignf(q.y, matrix.m[0][2] - matrix.m[2][0]);
    q.z = copysignf(q.z, matrix.m[1][0] - matrix.m[0][1]);
    return q;
}

//I didnt't use the glm::yaw function because it doesn't account for the pole it is facing.
//I am not convering all the values because I dont need them, save a little bit of CPU time.
//https://www.reddit.com/r/Vive/comments/6toiem/how_to_get_each_axis_rotation_from_vive/
float CKinectHandler::GetYawFromQuaternion(glm::quat quaternion)
{
    //If normalised is one, otherwise is correction factor
    float unit = (quaternion.x * quaternion.x) +
        (quaternion.y * quaternion.y) +
        (quaternion.z * quaternion.z) +
        (quaternion.w * quaternion.w);
    float test = quaternion.x * quaternion.w - quaternion.y * quaternion.z;

    if (test > 0.4995f * unit)
    {
        //Singularity at north pole
        return 2.0f * atan2f(quaternion.y, quaternion.x);
    }
    else if (test < -0.4995f * unit)
    {
        //Singularity at south pole
        return -2.0f * atan2f(quaternion.y, quaternion.x);
    }
    else
    {
        return atan2f(2.0f * quaternion.x * quaternion.w + 2.0f * quaternion.y * quaternion.z, 1.0f - 2.0f * (quaternion.z * quaternion.z + quaternion.w * quaternion.w));
    }
}

glm::vec3 CKinectHandler::GetEulerFromQuaternion(glm::quat quaternion)
{
    float sqw = quaternion.w * quaternion.w;
    float sqx = quaternion.x * quaternion.x;
    float sqy = quaternion.y * quaternion.y;
    float sqz = quaternion.z * quaternion.z;
    float unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
    float test = quaternion.x * quaternion.w - quaternion.y * quaternion.z;
    glm::vec3 v;

    if (test > 0.4995f * unit)
    {
        //Singularity at north pole
        v.y = 2.0f * atan2f(quaternion.y, quaternion.x);
        v.x = (float)M_PI / 2.0f;
        v.z = 0.0f;
        return v;
    }
    else if (test < -0.4995f * unit)
    {
        //Singularity at south pole
        v.y = -2.0f * atan2f(quaternion.y, quaternion.x);
        v.x = -(float)M_PI / 2.0f;
        v.z = 0.0f;
        return v;
    }
    else
    {
        v.y = atan2f(2.0f * quaternion.x * quaternion.w + 2.0 * quaternion.y * quaternion.z, 1.0f - 2.0f * (quaternion.z * quaternion.z + quaternion.w * quaternion.w));
        v.x = asinf(2.0f * (quaternion.x * quaternion.z - quaternion.w * quaternion.y));
        v.z = atan2f(2.0f * quaternion.x * quaternion.y + 2.0 * quaternion.z * quaternion.w, 1.0f - 2.0f * (quaternion.y * quaternion.y + quaternion.z * quaternion.z));
        return v;
    }
}

float CKinectHandler::WrapAround(float origin, float change, float min, float max)
{
    if (origin + change > max)
    {
        return WrapAround(fmodf(min + (origin + change), max), 0, min, max);
    }
    else
    {
        return origin + change;
    }
}

bool CKinectHandler::FacingKinect(float kinectYaw, float hmdYaw)
{
    float kinectMin = kinectYaw - detectionFOV_2;
    float kinectMax = kinectYaw + detectionFOV_2;

    if (kinectMax > circleMaxRadins)
    {
        float maxRangeOver = detectionFOV_2 - (circleMaxRadins - kinectYaw);
        return !(hmdYaw > circleMinRadins && hmdYaw < circleMaxRadins || hmdYaw > circleMinRadins && hmdYaw < maxRangeOver);
    }
    else if (kinectMin < circleMinRadins)
    {
        float minRangeOver = circleMaxRadins - (detectionFOV_2 - kinectYaw);
        return !(hmdYaw > circleMinRadins && hmdYaw < circleMaxRadins || hmdYaw > minRangeOver && hmdYaw < circleMaxRadins);
    }
    else
    {
        return !(hmdYaw > kinectMin && hmdYaw < kinectMax);
    }
}

//All joints are being used so it is ok to just take all of the left and right joints and flip their values.
//Make sure to reverse the rotation of all the joints.1
void CKinectHandler::FlipJoints(Joint *joints, JointOrientation* jointOrientations)
{
    FlipJoint(joints[_JointType::JointType_ShoulderLeft], joints[_JointType::JointType_ShoulderRight]);
    FlipJoint(joints[_JointType::JointType_ElbowLeft], joints[_JointType::JointType_ElbowRight]);
    FlipJoint(joints[_JointType::JointType_WristLeft], joints[_JointType::JointType_WristRight]);
    FlipJoint(joints[_JointType::JointType_HipLeft], joints[_JointType::JointType_HipRight]);
    FlipJoint(joints[_JointType::JointType_KneeLeft], joints[_JointType::JointType_KneeRight]);
    FlipJoint(joints[_JointType::JointType_AnkleLeft], joints[_JointType::JointType_AnkleRight]);
    FlipJoint(joints[_JointType::JointType_FootLeft], joints[_JointType::JointType_FootRight]);
    FlipJoint(joints[_JointType::JointType_HandTipLeft], joints[_JointType::JointType_HandTipRight]);
    FlipJoint(joints[_JointType::JointType_ThumbLeft], joints[_JointType::JointType_ThumbRight]);

    for (size_t i = 0U; i < _JointType::JointType_Count; i++)
    {
        RotateJointOrientation(jointOrientations[i]);
    }
}

void CKinectHandler::FlipJoint(Joint &j1, Joint &j2)
{
    Joint tmpJoint = j1;
    tmpJoint.JointType = j2.JointType;
    j2.JointType = j1.JointType;
    j1 = j2;
    j2 = tmpJoint;
}

void CKinectHandler::RotateJointOrientation(JointOrientation& j)
{
    //I don't really know/understand much about quaternions so I am converting these to Euler values and then back to quaternions again.
    //I don't want to do it this way but I don't understand how to do quaternion multiplication to rotate the device 180* around the z axis.
    //It also seems that this isnt quite reliable as the emulated pucks seem to have rotation drift though that could be because the kinect has a harder time tracking someone who is turned around.
    glm::vec3 xyz = GetEulerFromQuaternion(glm::quat(
        j.Orientation.w,
        j.Orientation.x,
        j.Orientation.y,
        j.Orientation.z
    ));
    glm::quat adjustedQuat = glm::quat(glm::vec3(xyz.x, WrapAround(xyz.y, radinsDeg180, circleMinRadins, circleMaxRadins), xyz.z));
    j.Orientation.w = adjustedQuat.w;
    j.Orientation.x = adjustedQuat.x;
    j.Orientation.y = adjustedQuat.y;
    j.Orientation.z = adjustedQuat.z;
}

void CKinectHandler::Update(glm::quat m_baseRotation)
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
                vr::TrackedDevicePose_t hmd_pose[1]; //HMD index is always 0.
                vr::VRServerDriverHost()->GetRawTrackedDevicePoses(0, hmd_pose, 1);
                vr::HmdQuaternionf_t hmdQuaternion = GetRotation(hmd_pose[0].mDeviceToAbsoluteTracking);
                float hmdYaw = GetYawFromQuaternion(glm::quat(hmdQuaternion.w, hmdQuaternion.x, hmdQuaternion.y, hmdQuaternion.z));
                float kinectYaw = GetYawFromQuaternion(m_baseRotation);

                for(size_t i = 0U, j = static_cast<size_t>(BODY_COUNT); i < j; i++)
                {
                    if(l_bodies[i])
                    {
                        unsigned char l_isTracked = 0U;
                        if(l_bodies[i]->get_IsTracked(&l_isTracked) >= S_OK)
                        {
                            if(l_isTracked > 0U)
                            {
                                Joint l_joints[_JointType::JointType_Count];
                                JointOrientation l_jointOrientations[_JointType::JointType_Count];
                                if((l_bodies[i]->GetJoints(_JointType::JointType_Count, l_joints) >= S_OK) && (l_bodies[i]->GetJointOrientations(JointType_Count, l_jointOrientations) >= S_OK))
                                {
                                    //Check if headset is facing away from the camera.
                                    //Take the joints and switch the left DATA with the right.
                                    //May need to do this for the JointOrientation too?
                                    if (!FacingKinect(kinectYaw, hmdYaw))
                                    {
                                        FlipJoints(l_joints, l_jointOrientations);
                                    }

                                    for(size_t k = 0U; k < _JointType::JointType_Count; k++)
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
