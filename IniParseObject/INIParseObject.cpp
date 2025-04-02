#include "../IniParseObject/INIParseObject.h"
// ¸¨Öúº¯Êý£ºÈ¥³ý×Ö·û´®Á½¶ËµÄ¿Õ°××Ö·û
std::string my_sdk::INIParseObject::Trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string::npos)
    {
        return ""; // È«ÊÇ¿Õ°××Ö·û
    }
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, (last - first + 1));
}

bool my_sdk::INIParseObject::IsComment(const std::string& line)
{
    return line.empty() || line[0] == ';' || line[0] == '#';
}

bool my_sdk::INIParseObject::Load(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return false;
    }

    std::string currentSection;
    std::string line;
    while (std::getline(file, line))
    {
        line = Trim(line);
        if (IsComment(line))
        {
            continue; // Ìø¹ý×¢ÊÍÐÐ
        }

        if (line[0] == '[' && line.back() == ']') // Çø¶Î
        {
            currentSection = Trim(line.substr(1, line.size() - 2));
            continue;
        }

        size_t equalPos = line.find('=');
        if (equalPos != std::string::npos) // ¼üÖµ¶Ô
        {
            std::string key = Trim(line.substr(0, equalPos));
            std::string value = Trim(line.substr(equalPos + 1));
            if (!currentSection.empty())
            {
                sections[currentSection][key] = value;
            }
        }
    }

    file.close();
    return true;
}

std::unordered_map<std::string, std::string> my_sdk::INIParseObject::GetSection(const std::string& sectionName) const
{
    auto it = sections.find(sectionName);
    if (it != sections.end())
    {
        return it->second;
    }
    return {};
}

std::string my_sdk::INIParseObject::GetValue(const std::string& sectionName, const std::string& keyName) const
{
    auto sectionIt = sections.find(sectionName);
    if (sectionIt != sections.end())
    {
        auto keyIt = sectionIt->second.find(keyName);
        if (keyIt != sectionIt->second.end())
        {
            return keyIt->second;
        }
    }
    return ""; // Î´ÕÒµ½Ê±·µ»Ø¿Õ×Ö·û´®
}

void my_sdk::INIParseObject::PrintAll() const
{
    for (const auto& section : sections)
    {
        std::cout << "[" << section.first << "]" << std::endl;
        for (const auto& keyValue : section.second)
        {
            std::cout << keyValue.first << " = " << keyValue.second << std::endl;
        }
        std::cout << std::endl;
    }
}