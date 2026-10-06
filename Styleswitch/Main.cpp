#include "pch.h"
#include <iostream>
#include <vector>
#include <windows.h>
#include <cstdint>
#include <set>
#include "bytestuff.h"
#include "Hook/MinHook.h"
#include "PatternScan.h"
#include <string>
#include <sstream>
#include "db.h"
#pragma comment(lib, "resources/libMinHook.x64.lib")

void Patch(BYTE* dst, BYTE* src, unsigned int size);
uintptr_t instruction_targetaddr(const char* signature)
{
    std::string sig = signature;
    uint8_t* sig_addr = PatternScan(GetModuleHandle(NULL), signature);

    if (sig.starts_with("48 8D") || sig.starts_with("4C 8D") || sig.starts_with("48 8B") || sig.starts_with("4C 8B") || sig.starts_with("80 3D"))
    {

        int32_t offset = *(int32_t*)(sig_addr + 0x3);
        uintptr_t result = ((uintptr_t)sig_addr + offset + 0x7);

        return result;
    }
    else if (sig.starts_with("E9") || sig.starts_with("E8"))
    {

        int32_t offset = *(int32_t*)(sig_addr + 0x1); // skip to amount
        uintptr_t result = (uintptr_t)sig_addr + offset + 0x5; // jump the size of the instruction and offset

        return result;
    }
    return 0;
}
uintptr_t find_address_by_handle(unsigned int handle)
{
    if (handle)
    {
        static uintptr_t ptr;

        if (!ptr) ptr = instruction_targetaddr("4C 8D 05 ? ? ? ? C1 E9 14 66 42 39 4C 00 08 75 0D 66 42 83 7C  00 0C 03 75 04 4A 8B 3C");

        __int64* some_entity_reference_array = (__int64*)ptr;

        __int64 id = handle & 0xFFFFF;
        if (id < 0x80000)
        {
            __int64 offset = 4 * id;
            return some_entity_reference_array[offset];
        }
    }
    return 0;
}
uintptr_t get_scene_entity(unsigned int scn)
{
    unsigned int _handle = 0;

    typedef unsigned int* (__fastcall* _get_handle)(unsigned int* a1, __int64 a2);
    static _get_handle get_handle = (_get_handle)(PatternScan(GetModuleHandle(NULL), "8B C2 48 8D ? ? ? ? ? 8B 04 82 89 01 48 8B C1 C3"));

    typedef unsigned int* (__fastcall* _get_scene_entity)(void*, unsigned int* result, unsigned int);
    static _get_scene_entity get_scene_entity = (_get_scene_entity)(PatternScan(GetModuleHandle(NULL), "8B 41 28 45 33 C9 85 C0  74 33 8B C8 81 E1 FF FF"));


    get_handle(&_handle, 1);

    uintptr_t game_var = 0;
    if (_handle)
    {
        game_var = find_address_by_handle(_handle);
    }

    if (game_var)
    {
        unsigned int result_handle = 0;
        unsigned int scene_handle = *(unsigned int*)get_scene_entity((void*)game_var, &result_handle, scn);
        if (scene_handle)
        {
            return find_address_by_handle(scene_handle);
        }
    }
}

#pragma optimize("", off)
uintptr_t __fastcall get_player_get_commandset()
{
    typedef uint8_t**(__fastcall* _func)();
    static _func func = (_func)instruction_targetaddr("E8 ? ? ? ? 48 8B D0 48 8D 4C 24 24 E8");

    uint8_t** fighter = func();
    if(fighter)
    {
        uint8_t* chara = *fighter;
        uint8_t* human_mode = *(uint8_t**)(chara + 0x5880);
        uintptr_t commandset = *(uintptr_t*)(human_mode + 0x228);
        return (uintptr_t)(human_mode + 0x228);
    }
    return 0;
}

struct style
{
    unsigned int moveset;
    unsigned int dboost_moveset;
    unsigned int dboost_start_gmt_id;
    bool enable_speedstyle_animationcancel;
    unsigned int bgm_cuesheet;
    unsigned int bgm_cue;
};

style allstyles[20];
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

typedef __int64(__fastcall* _battle)(__int64* battleproperty, DWORD* a2);
_battle BattlePropertyModel_func = nullptr;
BattleProperty* g_pBattleProperty = nullptr;
_battle BattlePropertyModel_trampoline;
__int64 __fastcall BattlePropertyModel(__int64* a1, DWORD* a2)
{
    g_pBattleProperty = reinterpret_cast<BattleProperty*>(a1);
    db::binary_file_header_t* binary_ptr = db::get_binary_ptr(1275);
    db::table_header_t* m_p_table = (db::table_header_t*)((uintptr_t)binary_ptr + binary_ptr->m_p_table);
    for (int i = 0; i < 20; ++i)
    {
        style& s = allstyles[i];
        if (m_p_table->m_record_num == i)
            break;
        s.moveset = db::get_value_from_field<unsigned int>(binary_ptr, i, 1);
        s.dboost_moveset = db::get_value_from_field<unsigned int>(binary_ptr, i, 2);
        s.dboost_start_gmt_id = db::get_value_from_field<unsigned int>(binary_ptr, i, 3);
        s.enable_speedstyle_animationcancel = db::get_value_from_field<bool>(binary_ptr, i, 4);
        s.bgm_cuesheet = db::get_value_from_field<unsigned int>(binary_ptr, i, 5);
        s.bgm_cue = db::get_value_from_field<unsigned int>(binary_ptr, i, 6);
    }
    return BattlePropertyModel_trampoline(a1, a2);
}
#pragma optimize("", on)
typedef uintptr_t(__fastcall* _play_bgm)(__int64 a1, int a2, unsigned int id, unsigned int a4, int a5, float a6, float a7);
_play_bgm play_bgm_func = (_play_bgm)PatternScan(GetModuleHandle(NULL), "48 89 5C 24 10 55 56 57  41 54 41 55 41 56 41 57 48 83 EC 50 41 8B F1 45  8B F8 4C 63 F2 48 8B D9");
_play_bgm play_bgm_trampoline;
__int64 __fastcall play_bgm(__int64 a1,
    int a2,
    unsigned int id,
    unsigned int a4,
    int a5,
    float a6,
    float a7)
{
    if (id == 0x00080006 || id == 0x00080023)
        id = ((uint32_t)allstyles[g_pBattleProperty->style].bgm_cuesheet << 16) | allstyles[g_pBattleProperty->style].bgm_cue;

    return play_bgm_trampoline(a1, a2, id, a4, a5, a6, a7);
}
uintptr_t __fastcall get_bgm_priority(uintptr_t a1, unsigned int id)
{
    typedef uintptr_t(__fastcall* _func)(uintptr_t a1, unsigned int id);
    static _func func = (_func)PatternScan(GetModuleHandle(NULL), "48 89 5C 24 10 48 89 74  24 18 48 89 7C 24 20 41 54 41 55 41 57 48 83 EC  20 44 8B FA 44 0F B7 E2");
    return func(a1, id);
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


typedef void(__fastcall* _SetCommandSet)(uintptr_t a1, int a2, int a3);
_SetCommandSet SetCommandSet_func = nullptr;
_SetCommandSet SetCommandSet_trampoline;
 void __fastcall SetCommandSet(__int64 a1, int a2, int a3)
{
     if (get_player_get_commandset() == a1)
     {
         for (int i = 0; i < sizeof(allstyles); ++i)
         {
             style& s = allstyles[i];

             if (s.dboost_moveset == a3 || s.moveset == a3)
             {
                 g_pBattleProperty->style = i;
                 g_pBattleProperty->default_moveset = s.moveset;
                 g_pBattleProperty->default_dragon_boost_moveset = s.dboost_moveset;

                 static BYTE* styleproperty = PatternScan(GetModuleHandle(NULL), "8B 91 68 01 00 00 83 EA");
                 if (styleproperty)
                 {
                     if(s.enable_speedstyle_animationcancel)
                     {
                         BYTE nopPatch[] = { 0x8B, 0x91, 0x68, 0x01, 0x00, 0x00 };
                         Patch(styleproperty, nopPatch, sizeof(nopPatch));
                     }
                     else
                     {
                         BYTE nopPatch[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
                         Patch(styleproperty, nopPatch, sizeof(nopPatch));
                     }
                 }
                 if(s.bgm_cuesheet)
                 {
                     uint32_t cuesheetcue = ((uint32_t)s.bgm_cuesheet << 16) | s.bgm_cue;
                     uintptr_t sound_manager = get_scene_entity(34);
                     unsigned int priority = get_bgm_priority(sound_manager, cuesheetcue);

                     play_bgm(sound_manager, 1, cuesheetcue, priority, 0, 0.0, 0.0);
                 }
                 break;
             }
         }
     }
     return SetCommandSet_trampoline(a1, a2, a3);
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
    if (g_playernextmove->next_motion == 3236 || g_playernextmove->next_motion == 7945 || g_playernextmove->next_motion == 7944)
    {
        g_playernextmove->next_motion = allstyles[g_pBattleProperty->style].dboost_start_gmt_id;
    }
    return Battle_next_motion_trampoline(a1, a2);
}


DWORD WINAPI AppThread(LPVOID lpParam) {
    MH_Initialize();
    DisableRestriction();

    SetCommandSet_func = (_SetCommandSet)(PatternScan(GetModuleHandle(NULL), "44 89 44 24 18 89 54 24  10 53 56 57 48 83 EC 20"));

    MH_CreateHook((void*)SetCommandSet_func, &SetCommandSet, (LPVOID*)&SetCommandSet_trampoline);
    MH_EnableHook((void*)SetCommandSet_func);

    Battle_next_motion_func = (_next_motion)(PatternScan(GetModuleHandle(NULL), "40 53 48 83 EC 20 48 8B D9 48 83 C1 18 E8 ? ? ? ? 48 85 C0 74 10 48 8B C8 E8"));

    MH_CreateHook((void*)Battle_next_motion_func, &battle_set_next_motion, (LPVOID*)&Battle_next_motion_trampoline);
    MH_EnableHook((void*)Battle_next_motion_func);

    play_bgm_func = (_play_bgm)PatternScan(GetModuleHandle(NULL), "48 89 5C 24 10 55 56 57  41 54 41 55 41 56 41 57 48 83 EC 50 41 8B F1 45  8B F8 4C 63 F2 48 8B D9");
    MH_CreateHook((void*)play_bgm_func, &play_bgm, (LPVOID*)&play_bgm_trampoline);
    MH_EnableHook((void*)play_bgm_func);

    Sleep(1000);
    BattlePropertyModel_func = (_battle)(instruction_targetaddr("E8 ? ? ? ? EB 03 49 8B C7 48 89 05 ? ? ? ? B9 40 01 00 00"));

    MH_CreateHook((void*)BattlePropertyModel_func, &BattlePropertyModel, (LPVOID*)&BattlePropertyModel_trampoline);
    MH_EnableHook((void*)BattlePropertyModel_func);


    HMODULE hModule = GetModuleHandleA("k2db.asi");
    if (hModule)
    {
        typedef void(*_db_load)(unsigned int id, const char* name);
        _db_load db_load = (_db_load)GetProcAddress(hModule, "db_load");
        db_load(1275, 0);
    }
    else
    {
        MessageBoxA(nullptr, "k2db.asi is missing, styleswitch.asi cannot work without it", "MISSING ASI!", MB_OK | MB_ICONERROR);
        TerminateProcess(GetCurrentProcess(), 0);
    }
    while (true)
    {
        Sleep(9999);
    }
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, AppThread, hModule, 0, nullptr);
    }
    return TRUE;
}

