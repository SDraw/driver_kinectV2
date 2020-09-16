#pragma once

enum JointIndex : size_t
{
    JI_Hips = 0U,
    JI_LeftAnkle,
    JI_RightAnkle,

    JI_Count
};
struct JointData
{
    float x, y, z;
    float rx, ry, rz, rw;
};
struct SensorData
{
    JointData m_joints[JI_Count];
    TIMESPAN m_frameTime;
    ULONGLONG m_tick;
};

class CJointFilter;

class CKinectHandler final
{
    IKinectSensor *m_kinectSensor;
    IBodyFrameReader *m_bodyFrameReader;

    std::vector<CJointFilter*> m_jointFilters;
    SensorData m_sensorData;

    std::atomic<bool> m_paused;

    CKinectHandler(const CKinectHandler &that) = delete;
    CKinectHandler& operator=(const CKinectHandler &that) = delete;

    void Cleanup();

public:
    CKinectHandler();
    ~CKinectHandler();

    bool Initialize();
    void Terminate();

    const SensorData& GetSensorData() const;

    bool IsPaused() const;
    void SetPaused(bool f_state);

    void Update();
};
