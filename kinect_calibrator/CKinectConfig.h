#pragma once

class CKinectConfig final
{
    pugi::xml_document *m_document;
    std::string m_path;

    glm::vec3 m_position;
    glm::quat m_rotation;

    CKinectConfig(const CKinectConfig &that) = delete;
    CKinectConfig& operator=(const CKinectConfig &that) = delete;
public:
    explicit CKinectConfig(const char *f_path);
    ~CKinectConfig();

    void Load();
    void Save();

    void SetPosition(const glm::vec3 &f_pos);
    inline const glm::vec3& GetPosition() const { return m_position; }

    void SetRotation(const glm::quat &f_rot);
    inline const glm::quat& GetRotation() const { return m_rotation; }
};
