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

class CJointFilter;

class CKinectHandler final
{
    IKinectSensor *m_kinectSensor;
    IBodyFrameReader *m_bodyFrameReader;

    std::string m_uniqueId;

    std::vector<CJointFilter*> m_jointFilters;
    JointData m_jointData[JI_Count];

    CKinectHandler(const CKinectHandler &that) = delete;
    CKinectHandler& operator=(const CKinectHandler &that) = delete;

    void Cleanup();

protected:
    CKinectHandler();
    ~CKinectHandler();

    bool Initialize();
    void Terminate();

    inline const std::string& GetUniqueId() const { return m_uniqueId; }
    inline const JointData& GetJointData(size_t f_index) const { return m_jointData[f_index]; }

    void Update();

    friend class CServerDriver;
};

