#pragma once

class CJointFilter;
struct JointData
{
    float x, y, z;
    float rx, ry, rz, rw;
};
struct FrameData
{
    JointData m_joints[JointType_Count];
    TIMESPAN m_frameTime;
    ULONGLONG m_tick;
};

class CKinectHandler final
{
    IKinectSensor *m_kinectSensor;
    IBodyFrameReader *m_bodyFrameReader;

    CJointFilter *m_jointFilters[JointType_Count];
    FrameData *m_frameData;

    std::atomic<bool> m_paused;

    float detectionFOV_2;
    float circleMinRadins;
    float circleMaxRadins;
    float radinsDeg180;

    CKinectHandler(const CKinectHandler &that) = delete;
    CKinectHandler& operator=(const CKinectHandler &that) = delete;

    float DegreeToRadian(float degrees);
    vr::HmdQuaternionf_t GetRotation(vr::HmdMatrix34_t matrix);
    float GetYawFromQuaternion(glm::quat quaternion);
    glm::vec3 GetEulerFromQuaternion(glm::quat quaternion);
    float WrapAround(float origin, float change, float min, float max);
    bool FacingKinect(float kinectYaw, float hmdYaw);
    void FlipJoints(Joint *joints, JointOrientation *jointOrientations);
    void FlipJoint(Joint &j1, Joint &j2);
    void RotateJointOrientation(JointOrientation& j);

    void Cleanup();
public:
    explicit CKinectHandler();
    ~CKinectHandler();

    bool Initialize();
    void Terminate();

    const FrameData* GetFrameData() const;

    bool IsPaused() const;
    void SetPaused(bool f_state);

    void Update(glm::quat m_baseRotation);
};
