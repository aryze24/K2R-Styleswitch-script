#pragma once

typedef void(__fastcall* _get_player_moveset)(__int64 a1);
_get_player_moveset get_player_moveset_func = nullptr;
_get_player_moveset get_player_moveset_trampoline;

inline std::vector<uintptr_t> styleoffsets = { 0x168 };