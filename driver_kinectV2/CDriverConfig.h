#pragma once

class CDriverConfig final
{
    static glm::vec3 ms_basePosition;
    static glm::quat ms_baseRotation;
public:
    static void Load();

    static inline const glm::vec3& GetBasePosition() { return ms_basePosition; }
    static inline const glm::quat& GetBaseRotation() { return ms_baseRotation; }
};
