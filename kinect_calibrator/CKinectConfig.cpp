#include "stdafx.h"

#include "CKinectConfig.h"

#include "Utils.h"

const std::vector<std::string> g_SettingNames
{
    "basePosition", "baseRotation"
};
enum SettingIndex : size_t
{
    SI_BasePosition = 0U,
    SI_BaseRotation,

    SI_Count
};

CKinectConfig::CKinectConfig(const char *f_path)
{
    char l_buffer[MAX_PATH];
    GetModuleFileNameA(NULL, l_buffer, MAX_PATH);
    m_path.assign(l_buffer);
    m_path.assign(m_path.substr(0U, m_path.find_last_of("\\/")));
    m_path.append(f_path);

    m_basePosition = glm::vec3(0.f);
    m_baseRotation = glm::quat(1.f, 0.f, 0.f, 0.f);
}

CKinectConfig::~CKinectConfig()
{
}

void CKinectConfig::Load()
{
    pugi::xml_document *l_document = new pugi::xml_document();
    if(l_document->load_file(m_path.c_str()))
    {
        const pugi::xml_node l_root = l_document->child("settings");
        if(l_root)
        {
            for(pugi::xml_node l_node = l_root.child("setting"); l_node; l_node = l_node.next_sibling("setting"))
            {
                const pugi::xml_attribute l_attribName = l_node.attribute("name");
                const pugi::xml_attribute l_attribValue = l_node.attribute("value");
                if(l_attribName && l_attribValue)
                {
                    switch(ReadEnumVector(l_attribName.as_string(), g_SettingNames))
                    {
                        case SettingIndex::SI_BasePosition:
                        {
                            std::stringstream l_stream(l_attribValue.as_string("0.0 0.0 0.0"));
                            l_stream >> m_basePosition.x >> m_basePosition.y >> m_basePosition.z;
                        } break;
                        case SettingIndex::SI_BaseRotation:
                        {
                            std::stringstream l_stream(l_attribValue.as_string("0.0 0.0 0.0 1.0"));
                            l_stream >> m_baseRotation.x >> m_baseRotation.y >> m_baseRotation.z >> m_baseRotation.w;
                        } break;
                    }
                }
            }
        }
    }
    delete l_document;
}

void CKinectConfig::Save()
{
    pugi::xml_document *l_document = new pugi::xml_document();
    pugi::xml_node l_root = l_document->append_child("settings");
    if(l_root)
    {
        for(size_t i = 0U; i < SI_Count; i++)
        {
            pugi::xml_node l_settingNode = l_root.append_child("setting");
            pugi::xml_attribute l_nameAttrib = l_settingNode.append_attribute("name");
            pugi::xml_attribute l_valueAttrib = l_settingNode.append_attribute("value");
            if(l_nameAttrib && l_valueAttrib)
            {
                l_nameAttrib.set_value(g_SettingNames[i].c_str());
                switch(i)
                {
                    case SettingIndex::SI_BasePosition:
                    {
                        std::string l_pos;
                        for(int j = 0; j < 3; j++)
                        {
                            l_pos.append(std::to_string(m_basePosition[j]));
                            if(j < 2) l_pos.push_back(' ');
                        }
                        l_valueAttrib.set_value(l_pos.c_str());
                    } break;
                    case SettingIndex::SI_BaseRotation:
                    {
                        std::string l_rot;
                        for(int j = 0; j < 4; j++)
                        {
                            l_rot.append(std::to_string(m_baseRotation[j]));
                            if(j < 3) l_rot.push_back(' ');
                        }
                        l_valueAttrib.set_value(l_rot.c_str());
                    } break;
                }
            }
        }
    }
    l_document->save_file(m_path.c_str());
    delete l_document;
}

void CKinectConfig::SetBasePosition(const glm::vec3 &f_pos)
{
    std::memcpy(&m_basePosition, &f_pos, sizeof(glm::vec3));
}

const glm::vec3& CKinectConfig::GetBasePosition() const
{
    return m_basePosition;
}

void CKinectConfig::SetBaseRotation(const glm::quat &f_rot)
{
    std::memcpy(&m_baseRotation, &f_rot, sizeof(glm::quat));
}

const glm::quat& CKinectConfig::GetBaseRotation() const
{
    return m_baseRotation;
}
