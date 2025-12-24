#include "IniStuff.h"
#include <windows.h>
#include <shlwapi.h>
#include <sstream>
#include <iostream>

#pragma comment(lib, "Shlwapi.lib")

int STYLE1 = -1, STYLE1EX = -1;
int STYLE2 = -1, STYLE2EX = -1;
int STYLE3 = -1, STYLE3EX = -1;
int STYLE4 = -1, STYLE4EX = -1;
int StyleProperties = 0;

std::wstring g_gameDir;

bool GetGameDirectory(std::wstring& outDir)
{
    WCHAR buffer[MAX_PATH] = {};
    if (!GetModuleFileNameW(nullptr, buffer, MAX_PATH))
        return false;

    if (!PathRemoveFileSpecW(buffer))
        return false;

    outDir = buffer;
    return true;
}

std::string Trim(const std::string& s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

std::string StripInlineComment(const std::string& line)
{
    size_t pos = line.find("//");
    if (pos != std::string::npos)
        return line.substr(0, pos);
    return line;
}

std::string GetIniValue(const std::wstring& iniPath, const char* section, const char* key, const char* def)
{
    char buffer[512]{};
    std::string iniPathStr(iniPath.begin(), iniPath.end());
    GetPrivateProfileStringA(section, key, def, buffer, sizeof(buffer), iniPathStr.c_str());
    std::string value = buffer;
    value = StripInlineComment(value);
    return Trim(value);
}

std::vector<int> ParseMovesetList(const std::string& s)
{
    std::vector<int> result;
    size_t start = s.find('[');
    size_t end = s.find(']');
    if (start == std::string::npos || end == std::string::npos || end <= start)
        return result;

    std::string inner = s.substr(start + 1, end - start - 1);
    std::stringstream ss(inner);
    std::string item;

    while (std::getline(ss, item, ',')) {
        item = Trim(item);
        try {
            result.push_back(std::stoi(item));
        }
        catch (...) {
        }
    }
    return result;
}

bool LoadIni(
    const std::wstring& iniPath,
    int& STYLE1, int& STYLE1EX,
    int& STYLE2, int& STYLE2EX,
    int& STYLE3, int& STYLE3EX,
    int& STYLE4, int& STYLE4EX,
    int& StyleProperties
)
{
    STYLE1 = STYLE1EX = -1;
    STYLE2 = STYLE2EX = -1;
    STYLE3 = STYLE3EX = -1;
    STYLE4 = STYLE4EX = -1;
    StyleProperties = 0;

    std::vector<int> s1 = ParseMovesetList(GetIniValue(iniPath, "Movesets", "Style 1 Movesets", "[]"));
    std::vector<int> s2 = ParseMovesetList(GetIniValue(iniPath, "Movesets", "Style 2 Movesets", "[]"));
    std::vector<int> s3 = ParseMovesetList(GetIniValue(iniPath, "Movesets", "Style 3 Movesets", "[]"));
    std::vector<int> s4 = ParseMovesetList(GetIniValue(iniPath, "Movesets", "Style 4 Movesets", "[]"));

    if (!s1.empty()) STYLE1 = s1[0];
    STYLE1EX = (s1.size() > 1) ? s1[1] : STYLE1;

    if (!s2.empty()) STYLE2 = s2[0];
    STYLE2EX = (s2.size() > 1) ? s2[1] : STYLE2;

    if (!s3.empty()) STYLE3 = s3[0];
    STYLE3EX = (s3.size() > 1) ? s3[1] : STYLE3;

    if (!s4.empty()) STYLE4 = s4[0];
    STYLE4EX = (s4.size() > 1) ? s4[1] : STYLE4;
    try {
        StyleProperties = std::stoi(GetIniValue(iniPath, "Options", "Disable Style properties", "1"));
    }
    catch (...) {
        StyleProperties = 1;
    }

    return true;
}

bool Initializeini()
{
    if (!GetGameDirectory(g_gameDir)) {
        return false;
    }

    std::wstring iniPath = g_gameDir + L"\\mods\\Styleswitch\\Styleswitch.ini";

    if (!LoadIni(
        iniPath,
        STYLE1, STYLE1EX,
        STYLE2, STYLE2EX,
        STYLE3, STYLE3EX,
        STYLE4, STYLE4EX,
        StyleProperties
    )) {

        return false;
    }

    return true;
}
