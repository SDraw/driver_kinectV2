#pragma once

class CKinectConfig final
{
    std::string m_path;

    glm::vec3 m_basePosition;
    glm::quat m_baseRotation;

    CKinectConfig(const CKinectConfig &that) = delete;
    CKinectConfig& operator=(const CKinectConfig &that) = delete;
public:
    explicit CKinectConfig(const char *f_path);
    ~CKinectConfig();

    void Load();
    void Save();

    void SetBasePosition(const glm::vec3 &f_pos);
    const glm::vec3& GetBasePosition() const;

    void SetBaseRotation(const glm::quat &f_rot);
    const glm::quat& GetBaseRotation() const;
};
