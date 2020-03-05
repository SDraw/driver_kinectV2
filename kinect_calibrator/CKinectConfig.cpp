#include "stdafx.h"

#include "CKinectConfig.h"

#include "Utils.h"

const std::vector<std::string> g_SettingNames
{
    "enabled",
    "basePosition", "baseRotation"
};
enum SettingIndex : size_t
{
    SI_Enabled,
    SI_BasePosition,
    SI_BaseRotation,

    SI_Count
};

CKinectConfig::CKinectConfig(const char *f_path)
{
    m_document = new pugi::xml_document();

    char l_buffer[MAX_PATH];
    GetModuleFileNameA(NULL, l_buffer, MAX_PATH);
    m_path.assign(l_buffer);
    m_path.assign(m_path.substr(0U, m_path.find_last_of("\\/")));
    m_path.append(f_path);

    m_position = glm::vec3(0.f);
    m_rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
}
CKinectConfig::~CKinectConfig()
{
    delete m_document;
}


void CKinectConfig::Load()
{
    if(m_document->load_file(m_path.c_str()))
    {
        const pugi::xml_node l_root = m_document->child("settings");
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
                            l_stream >> m_position.x >> m_position.y >> m_position.z;
                        } break;
                        case SettingIndex::SI_BaseRotation:
                        {
                            std::stringstream l_stream(l_attribValue.as_string("0.0 0.0 0.0 1.0"));
                            l_stream >> m_rotation.x >> m_rotation.y >> m_rotation.z >> m_rotation.w;
                        } break;
                    }
                }
            }
        }
    }
    else
    {
        std::cout << "[!] Unable to load existing 'resources/settings.xml', creating new." << std::endl;

        pugi::xml_node l_root = m_document->append_child("settings");
        if(l_root)
        {
            for(size_t i = 0U; i < SI_Count; i++)
            {
                pugi::xml_node l_settingNode = l_root.append_child("setting");
                if(l_settingNode)
                {
                    pugi::xml_attribute l_nameAttrib = l_settingNode.append_attribute("name");
                    if(l_nameAttrib) l_nameAttrib.set_value(g_SettingNames[i].c_str());
                    pugi::xml_attribute l_valueAttrib = l_settingNode.append_attribute("value");
                    if(l_valueAttrib)
                    {
                        switch(i)
                        {
                            case SI_Enabled:
                                l_valueAttrib.set_value(true);
                                break;
                            case SI_BasePosition:
                                l_valueAttrib.set_value("0.0 0.0 0.0");
                                break;
                            case SI_BaseRotation:
                                l_valueAttrib.set_value("0.0 0.0 0.0 1.0");
                                break;
                        }
                    }
                }
            }
        }

        m_document->save_file(m_path.c_str());
    }
}

void CKinectConfig::Save()
{
    const pugi::xml_node l_root = m_document->child("settings");
    if(l_root)
    {
        for(pugi::xml_node l_node = l_root.child("setting"); l_node; l_node = l_node.next_sibling("setting"))
        {
            const pugi::xml_attribute l_attribName = l_node.attribute("name");
            pugi::xml_attribute l_attribValue = l_node.attribute("value");
            if(l_attribName && l_attribValue)
            {
                switch(ReadEnumVector(l_attribName.as_string(), g_SettingNames))
                {
                    case SettingIndex::SI_BasePosition:
                    {
                        std::string l_pos;
                        for(int i = 0; i < 3; i++)
                        {
                            l_pos.append(std::to_string(m_position[i]));
                            if(i < 2) l_pos.push_back(' ');
                        }
                        l_attribValue.set_value(l_pos.c_str());
                    } break;
                    case SettingIndex::SI_BaseRotation:
                    {
                        std::string l_rot;
                        for(int i = 0; i < 4; i++)
                        {
                            l_rot.append(std::to_string(m_rotation[i]));
                            if(i < 3) l_rot.push_back(' ');
                        }
                        l_attribValue.set_value(l_rot.c_str());
                    } break;
                }
            }
        }
    }

    if(!m_document->save_file(m_path.c_str())) std::cout << "[!] Unable to save file 'resources/settings.xml'." << std::endl;
}

void CKinectConfig::SetPosition(const glm::vec3 &f_pos)
{
    std::memcpy(&m_position, &f_pos, sizeof(glm::vec3));
}

void CKinectConfig::SetRotation(const glm::quat &f_rot)
{
    std::memcpy(&m_rotation, &f_rot, sizeof(glm::quat));
}
