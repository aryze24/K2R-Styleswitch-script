#pragma once
#include <cstdint>
uintptr_t instruction_targetaddr(const char* signature);
namespace db
{
    enum bytetype : uint8_t
    {
        invalid = -1,
        uint32 = 0,
        uint16 = 1,
        uint8 = 2,
        int32 = 3,
        int16 = 4,
        int8 = 5,
        bitmap = 6,
        float32 = 7,
        uint64 = 8,
        table = 9,
        int64 = 10,
        float64 = 11,
        string = 12,
    };
    struct table_header_t
    {
        uint32_t m_record_num; //0x0020
        uint32_t m_field_num; //0x0024
        uint32_t m_value_string_num; //0x0028
        uint32_t m_record_invalid; //0x002C
        uint32_t m_pp_record_id_string; //0x0030
        uint32_t m_p_record_existence; //0x0034
        uint32_t m_p_field_value_type; //0x0038
        uint32_t m_pp_value; //0x003C
        uint32_t flags; //0x0040
        uint32_t m_pp_value_string; //0x0044
        uint32_t m_pp_field_id_string; //0x0048
        uint32_t m_field_invalid; //0x004C
        uint32_t m_p_record_display_order; //0x0050
        uint32_t m_p_field_display_order; //0x0054
        uint32_t m_p_field_existence; //0x0058
        uint32_t m_p_indexer; //0x005C
        uint32_t m_p_game_var_field_id_list; //0x0060
        uint32_t m_pp_empty_value_list; //0x0064
        uint32_t m_p_raw_record_member_info; //0x0068
        uint32_t m_p_field_info; //0x006C
    };
    struct binary_file_header_t
    {
        char pad_0000[16]; //0x0000
        int32_t m_p_table; //0x0010
        uint32_t tag_id; //0x0014
        int8_t platform_id; //0x0018
        int8_t endian; //0x0019
        int8_t size_extend; //0x001A
        int8_t relocated; //0x001B
        uint32_t version; //0x001C
        table_header_t* header;
    }; //Size: 0x0454
    template <typename T>
    inline T get_value_from_field(binary_file_header_t* binary_ptr, unsigned int record_id, unsigned int field_id)
    {
        uintptr_t db_base = (uintptr_t)binary_ptr;

        table_header_t* m_p_table = (table_header_t*)(db_base + binary_ptr->m_p_table);

        T record_value;

        if (m_p_table->m_field_num > field_id) {

            uintptr_t valuetype = (uintptr_t)((uintptr_t)db_base + (uint64_t)m_p_table->m_p_field_value_type + (uint64_t)field_id);
            uint8_t value_type = *(uint8_t*)valuetype;

            uint32_t value_start_ofs = *(uint32_t*)(db_base + m_p_table->m_pp_value + (field_id * 4));

            switch (value_type) {
            case uint8:
                if (record_id < m_p_table->m_record_num)
                    record_value = *(uint8_t*)(db_base + value_start_ofs + record_id);
                break;
            case int8:
                if (record_id < m_p_table->m_record_num)
                    record_value = *(int8_t*)(db_base + value_start_ofs + record_id);
                break;
            case uint16:
                if (record_id < m_p_table->m_record_num)
                    record_value = *(uint16_t*)(db_base + value_start_ofs + (record_id * 2));
                break;
            case int16:
                if (record_id < m_p_table->m_record_num)
                    record_value = *(int16_t*)(db_base + value_start_ofs + (record_id * 2));
                break;
            case uint32:
                if (record_id < m_p_table->m_record_num)
                    record_value = *(uint32_t*)(db_base + value_start_ofs + (record_id * 4));
                break;
            case int32:
                if (record_id < m_p_table->m_record_num)
                    record_value = *(int32_t*)(db_base + value_start_ofs + (record_id * 4));
                break;

            case uint64:
                if (record_id < m_p_table->m_record_num)
                    record_value = *(uint64_t*)(db_base + value_start_ofs + (record_id * 8));
                break;
            case int64:
                if (record_id < m_p_table->m_record_num)
                    record_value = *(int64_t*)(db_base + value_start_ofs + (record_id * 8));
                break;

            case bitmap:
                if (record_id < m_p_table->m_record_num) {
                    if (value_start_ofs == 0xFFFFFFFF) record_value = 1;
                    else if (value_start_ofs != 0) {
                        uint32_t bitset = *(uint32_t*)(db_base + value_start_ofs + 4 * (record_id >> 5));
                        record_value = _bittest((long*)&bitset, record_id & 0x1F);
                    }
                }
                break;

            case float32:
                if (record_id < m_p_table->m_record_num) {
                    record_value = *(float*)(db_base + value_start_ofs + (record_id * 4));
                }
                break;

            case float64:
                if (record_id < m_p_table->m_record_num) {
                    record_value = *(double*)(db_base + value_start_ofs + (record_id * 8));
                }
                break;
            }

        }
        return  record_value;
    }
    inline binary_file_header_t* get_binary_ptr(unsigned int id)
    {
        typedef binary_file_header_t* (__fastcall* _func)(unsigned int id);
        _func func = (_func)(instruction_targetaddr("E8 ? ? ? ? 4C 8B  D0 48 85 C0 74 08 8B 50"));
        return func(id);
    }
    inline bool __fastcall is_ready(unsigned int id)
    {
        typedef bool(__fastcall* _func)(unsigned int id);
        static _func func = (_func)(instruction_targetaddr("E8 ? ? ? ? B9 FB 1D 00 00"));
        return func(id);
    }
}