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

    bool m_jointUsed[JointType_Count];
    CJointFilter *m_jointFilters[JointType_Count];
    FrameData *m_frameData;

    std::atomic<bool> m_paused;

    CKinectHandler(const CKinectHandler &that) = delete;
    CKinectHandler& operator=(const CKinectHandler &that) = delete;

    void Cleanup();
public:
    CKinectHandler(const std::vector<size_t> &f_indexes);
    ~CKinectHandler();

    bool Initialize();
    void Terminate();

    const FrameData* GetFrameData() const;

    bool IsPaused() const;
    void SetPaused(bool f_state);

    void Update();
};
