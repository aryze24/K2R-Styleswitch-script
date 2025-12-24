#pragma once

#include <string>
#include <vector>

// Extern global variables (declarations only)
extern int STYLE1, STYLE1EX;
extern int STYLE2, STYLE2EX;
extern int STYLE3, STYLE3EX;
extern int STYLE4, STYLE4EX;
extern int StyleProperties;

extern std::wstring g_gameDir;

// Functions
bool Initializeini();
bool GetGameDirectory(std::wstring& outDir);
std::string Trim(const std::string& s);
std::string StripInlineComment(const std::string& line);
std::string GetIniValue(const std::wstring& iniPath, const char* section, const char* key, const char* def = "");
std::vector<int> ParseMovesetList(const std::string& s);
bool LoadIni(
    const std::wstring& iniPath,
    int& STYLE1, int& STYLE1EX,
    int& STYLE2, int& STYLE2EX,
    int& STYLE3, int& STYLE3EX,
    int& STYLE4, int& STYLE4EX,
    int& StyleProperties
);
