#include "pch.h"
#include <iostream>
#include <vector>
#include <windows.h>
#include <shlwapi.h>
#include <cstdint>
#include <set>
#include "bytestuff.h"
#include "Hook/MinHook.h"
#include "constants.h"
#include "inistuff.h"
#include "PatternScan.h"
#include <string>
#include <sstream>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "resources/libMinHook.x64.lib")


struct BattleProperty {
    char pad_0000[80]; //0x0000
    float N00000059; //0x0050
    char pad_0054[36]; //0x0054
    float N0000005E; //0x0078
    char pad_007C[44]; //0x007C
    uint32_t in_feel_the_heat; //0x00A8
    char pad_00AC[28]; //0x00AC
    uint32_t in_feel_the_heat_again; //0x00C8
    char pad_00CC[156]; //0x00CC
    int32_t style; //0x0168
    char pad_016C[28]; //0x016C
    uint32_t buff; //0x0188
    char pad_018C[28]; //0x018C
    uint32_t N00000687; //0x01A8
    char pad_01AC[28]; //0x01AC
    uint32_t player_id; //0x01C8
    char pad_01CC[28]; //0x01CC
    uint32_t default_moveset; //0x01E8
    char pad_01EC[28]; //0x01EC
    int32_t default_dragon_boost_moveset; //0x0208
    char pad_020C[188]; //0x020C
    uint32_t in_dragonboost_ui; //0x02C8
    char pad_02CC[140]; //0x02CC
    uint32_t in_hact; //0x0358
    char pad_035C[76]; //0x035C
    uint32_t is_battle_mode; //0x03A8
    char pad_03AC[156]; //0x03AC
    uint32_t N000006DB; //0x0448
    char pad_044C[220]; //0x044C
    int32_t N000006F7; //0x0528
    char pad_052C[164]; //0x052C
    uint32_t next_equip_slot; //0x05D0
};

typedef __int64(__fastcall* battle)(__int64* battleproperty, DWORD* a2);
battle BattlePropertyModel_func = nullptr;
BattleProperty* g_pBattleProperty = nullptr;
battle BattlePropertyModel_trampoline;
__int64 __fastcall BattlePropertyModel(__int64* a1, DWORD* a2)
{
    g_pBattleProperty = reinterpret_cast<BattleProperty*>(a1);
    return BattlePropertyModel_trampoline(a1, a2);
}

struct player_moveset{
    char pad_0000[56]; //0x0000
    int32_t returnmoveset; //0x0038
    char pad_003C[28]; //0x003C
    int32_t movesetvalue2; //0x0058
    char pad_005C[60]; //0x005C
    int32_t movesetvalue; //0x0098
    char pad_009C[4]; //0x009C
}; //Size: 0x00A0

static __int64 player_moveset_ptr = -1;
player_moveset* g_playermoveset = nullptr;
static int style = 0;
 void __fastcall get_player_moveset(__int64 a1)
{
    __int64 movesetref = *(int*)(a1 + 0x38);
    if (player_moveset_ptr != a1)
    {
        if (movesetref == STYLE1 || movesetref == STYLE2 || movesetref == STYLE3 || movesetref == STYLE4) {
            g_playermoveset = reinterpret_cast<player_moveset*>(a1);
            player_moveset_ptr = a1;
        }
    }
    return get_player_moveset_trampoline(a1);
}
void MainThread() {

    if (!Initializeini()) {
        return;
    }

    while (true) {
        if ( player_moveset_ptr != -1 )
        {
            int movesetValue = g_playermoveset->movesetvalue;
            int Newstyle = g_pBattleProperty->style;
            int New_default_moveset = g_pBattleProperty->default_moveset;
            int New_default_dboost_moveset = g_pBattleProperty->default_dragon_boost_moveset;

                if (movesetValue == STYLE1 || movesetValue == STYLE1EX)
                {
                    Newstyle = 1,
                        New_default_moveset = STYLE1;
                    New_default_dboost_moveset = STYLE1EX;
                    g_playermoveset->returnmoveset = STYLE1;
                }
                else if (movesetValue == STYLE2 || movesetValue == STYLE2EX)
                {
                    Newstyle = 2,
                        New_default_moveset = STYLE2;
                    New_default_dboost_moveset = STYLE2EX;
                    g_playermoveset->returnmoveset = STYLE2;
                }
                else if (movesetValue == STYLE3 || movesetValue == STYLE3EX)
                {
                    Newstyle = 3,
                        New_default_moveset = STYLE3;
                    New_default_dboost_moveset = STYLE3EX;
                    g_playermoveset->returnmoveset = STYLE3;
                }
                else if (movesetValue == STYLE4 || movesetValue == STYLE4EX)
                {
                    Newstyle = 4,
                        New_default_moveset = STYLE4;
                    New_default_dboost_moveset = STYLE4EX;
                    g_playermoveset->returnmoveset = STYLE4;
                }
                if (Newstyle != g_pBattleProperty->style)
                {
                    style = Newstyle;
                    g_pBattleProperty->style = Newstyle;
                    g_pBattleProperty->default_moveset = New_default_moveset;
                    g_pBattleProperty->default_dragon_boost_moveset = New_default_dboost_moveset;

                }
            Sleep(100);
        }
    }
}

struct next_motion {
        char pad_0000[88]; //0x0000
        int32_t next_motion; //0x0058
};

typedef __int64(__fastcall* _next_motion)(__int64* a1, __int64 a2);
_next_motion Battle_next_motion_func = nullptr;
next_motion* g_playernextmove = nullptr;
_next_motion Battle_next_motion_trampoline;
__int64 __fastcall battle_set_next_motion(__int64* a1, __int64 a2)
{
    g_playernextmove = reinterpret_cast<next_motion*>(a1);
    if (style == 4 && g_playernextmove->next_motion == 3236)
    {
        g_playernextmove->next_motion = 3238;
    }
    return Battle_next_motion_trampoline(a1, a2);
}

DWORD WINAPI BytesThread(LPVOID) {
    DisableRestriction();
    return 0;
}

DWORD WINAPI MovesetThread(LPVOID lpParam)
{

    get_player_moveset_func = (_get_player_moveset)(PatternScan(GetModuleHandle(NULL), "8B 91 98 00 00 00 48 8B ? ? ? ? ? E9 ? ? ? ? CC"));

    MH_CreateHook((void*)get_player_moveset_func, &get_player_moveset, (LPVOID*)&get_player_moveset_trampoline);
    MH_EnableHook((void*)get_player_moveset_func);


    while (true)
    {
        Sleep(9999);
    }
    return 0;
}

DWORD WINAPI StyleThread(LPVOID lpParam)
{
    MH_Initialize();
    BattlePropertyModel_func = (battle)(PatternScan(GetModuleHandle(NULL), "48 89 4C 24 08 55 53 56 57 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC 48 48 8B f2"));

    MH_CreateHook((void*)BattlePropertyModel_func, &BattlePropertyModel, (LPVOID*)&BattlePropertyModel_trampoline);
    MH_EnableHook((void*)BattlePropertyModel_func);


    while (true)
    {
        Sleep(9999);
    }
    return 0;
}

DWORD WINAPI MotionThread(LPVOID lpParam)
{

    Battle_next_motion_func = (_next_motion)(PatternScan(GetModuleHandle(NULL), "40 53 48 83 EC 20 48 8B D9 48 83 C1 18 E8 ? ? ? ? 48 85 C0 74 10 48 8B C8 E8"));

    MH_CreateHook((void*)Battle_next_motion_func, &battle_set_next_motion, (LPVOID*)&Battle_next_motion_trampoline);
    MH_EnableHook((void*)Battle_next_motion_func);

    while (true)
    {
        Sleep(9999);
    }
    return 0;
}

DWORD WINAPI AppThread(LPVOID lpParam) {
    MainThread();
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, AppThread, hModule, 0, nullptr);
        CreateThread(nullptr, 0, BytesThread, hModule, 0, nullptr);
        CreateThread(nullptr, 0, MovesetThread, hModule, 0, nullptr);
        CreateThread(nullptr, 0, StyleThread, hModule, 0, nullptr);
        CreateThread(nullptr, 0, MotionThread, hModule, 0, nullptr);
    }
    return TRUE;
}

