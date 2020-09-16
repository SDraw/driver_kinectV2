#include "stdafx.h"

#include "CDriverConfig.h"

#include "Utils.h"

extern char g_ModulePath[];

const std::vector<std::string> g_SettingNames
{
    "basePosition", "baseRotation"
};
enum SettingIndex : size_t
{
    SI_BasePosition = 0U,
    SI_BaseRotation
};

glm::vec3 CDriverConfig::ms_basePosition(0.f);
glm::quat CDriverConfig::ms_baseRotation(1.f, 0.f, 0.f, 0.f);

void CDriverConfig::Load()
{
    std::string l_path(g_ModulePath);
    l_path.erase(l_path.begin() + l_path.rfind('\\'), l_path.end());
    l_path.append("\\..\\..\\resources\\settings.xml");

    pugi::xml_document *l_settings = new pugi::xml_document();
    if(l_settings->load_file(l_path.c_str()))
    {
        const pugi::xml_node l_root = l_settings->child("settings");
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
                            l_stream >> ms_basePosition.x >> ms_basePosition.y >> ms_basePosition.z;
                        } break;
                        case SettingIndex::SI_BaseRotation:
                        {
                            std::stringstream l_stream(l_attribValue.as_string("0.0 0.0 0.0 1.0"));
                            l_stream >> ms_baseRotation.x >> ms_baseRotation.y >> ms_baseRotation.z >> ms_baseRotation.w;
                        } break;
                    }
                }
            }
        }
    }
    delete l_settings;
}

const glm::vec3& CDriverConfig::GetBasePosition()
{
    return ms_basePosition;
}

const glm::quat& CDriverConfig::GetBaseRotation()
{
    return ms_baseRotation;
}
