#if !defined(FCODER_LEX_GEN_HAND_WRITTEN)
#define FCODER_LEX_GEN_HAND_WRITTEN

internal u64
lexeme_hash(u64 seed, u8 *ptr, umem size){
    u64 result = 0;
    for (umem i = 0; i < size; i += 1, ptr += 1){
        result ^= ((*ptr) ^ result*59) + seed;
    }
    return(result);
}

internal Lexeme_Table_Lookup
lexeme_table_lookup(u64 *hash_array, String_Const_u8 *key_array, 
                    Lexeme_Table_Value *value_array, i32 slot_count, u64 seed,
                    u8 *ptr, umem size){
    Lexeme_Table_Lookup result = {};
    u64 hash = lexeme_hash(seed, ptr, size);
    u64 comparison_hash = hash | 1;
    i32 first_index = (hash % slot_count);
    i32 index = first_index;
    for (;;){
        if (hash_array[index] == comparison_hash){
            if (string_match(SCu8(ptr, size), key_array[index])){
                result.found_match = true;
                result.base_kind = value_array[index].base_kind;
                result.sub_kind = value_array[index].sub_kind;
                break;
            }
        }
        else if (hash_array[index] == 0){
            break;
        }
        index += 1;
        if (index == slot_count){
            index = 0;
        }
        if (index == first_index){
            break;
        }
    }
    return(result);
}

#endif
u64 main_keys_hash_array[122] = {
0x0000000000000000,0x07c5f278d1101ced,0x0000000000000000,0xa62bbbf7713fa62b,
0x0000000000000000,0x0000000000000000,0x189f5b933bf3b933,0xf6c02fd104e622d3,
0xa62bbbf7441048b5,0x31a9b2ff15ce8563,0x68df7003b1bc4821,0x0000000000000000,
0x0000000000000000,0xa62bbbf77f52a37b,0x189f5af5a15e570b,0x68df7003b1b8eceb,
0x0000000000000000,0x0000000000000000,0x189f5b0ad4cbeaa3,0x3af1c28e1ddb0783,
0x2d14d64d82dd3be3,0xa62bbbf5e2f33afd,0x0000000000000000,0xf6c02fd108f91d9b,
0xaf010cb51e08bbc1,0x0000000000000000,0x3192f97fcdee24d3,0x07c5f278d1101f69,
0x189f5af6a7a7ffbd,0x0000000000000000,0x189f5af5bf5767f3,0x3afed33e86adea2d,
0x0000000000000000,0xa62bbbf5e1ec09ad,0x0000000000000000,0x0000000000000000,
0x522ba3fd0f3be8c3,0x0000000000000000,0xf6c02fd104e53add,0xf6c02fd10bdeaa6f,
0x07c5df2eff098d77,0x0000000000000000,0x0000000000000000,0x3afecff9e1ae52e5,
0xf6c02fd10bb24563,0xf6c02fd104eca76b,0x0000000000000000,0x3afed5063319a1eb,
0x68df7003b1bde949,0x189f5af5d4b84fbf,0x07c5d15a314f7723,0x189f5b08d4370559,
0x0000000000000000,0x0000000000000000,0xf6c02fd10baf4dcb,0x0000000000000000,
0x0000000000000000,0x189f5b56738b1265,0x0000000000000000,0x0000000000000000,
0xa62bbbf5e0247cbf,0x3afa82656d98a37b,0xa62bbbf75fa88419,0x0000000000000000,
0x0000000000000000,0x0000000000000000,0x189f5b933833cf69,0x0000000000000000,
0xa62bbbf767d15da9,0x0000000000000000,0x19183219f06dcb63,0x68df7003b1bc2429,
0x3afec2d0663f612d,0x0000000000000000,0x0000000000000000,0x3afecf3b77007d35,
0x189f5b0ad46a3da1,0x0000000000000000,0x53cf9ef61495b3e3,0x3af6389d0658983d,
0x0000000000000000,0x0000000000000000,0x07c5df6155313bd1,0x0000000000000000,
0x0000000000000000,0xaf010cb51e08a4a3,0xf6c02fd108fab82b,0x0000000000000000,
0x0000000000000000,0x0000000000000000,0x258bb962715d5363,0x189f5af5db0cf073,
0x07c5df26bfde9b65,0xa62bbbf5d67aee5b,0x189f5b566a0beef9,0x0000000000000000,
0x3afedd01e3000695,0x0000000000000000,0x0000000000000000,0xf6c02fd10bb000ad,
0x0000000000000000,0x0000000000000000,0x68df7003b1bde535,0x0000000000000000,
0x0000000000000000,0x189f5af5d7a3840d,0xa62bbbf7710434fb,0x0000000000000000,
0x3af23ba28357b2a3,0x07c5c0b2c6388e35,0x189f5b08dfc6e6b9,0x0000000000000000,
0x0000000000000000,0x0000000000000000,0x0000000000000000,0x0000000000000000,
0x0000000000000000,0x0000000000000000,0x0000000000000000,0xf6c02fd10b9d4cbb,
0x60e2b96351c30769,0x0000000000000000,
};
u8 main_keys_key_array_1[] = {0x61,0x6c,0x69,0x67,0x6e,0x61,0x73,};
u8 main_keys_key_array_3[] = {0x66,0x61,0x6c,0x73,0x65,};
u8 main_keys_key_array_6[] = {0x65,0x78,0x74,0x65,0x72,0x6e,};
u8 main_keys_key_array_7[] = {0x63,0x61,0x73,0x65,};
u8 main_keys_key_array_8[] = {0x77,0x68,0x69,0x6c,0x65,};
u8 main_keys_key_array_9[] = {0x6e,0x61,0x6d,0x65,0x73,0x70,0x61,0x63,0x65,};
u8 main_keys_key_array_10[] = {0x6e,0x65,0x77,};
u8 main_keys_key_array_13[] = {0x75,0x73,0x69,0x6e,0x67,};
u8 main_keys_key_array_14[] = {0x74,0x79,0x70,0x65,0x69,0x64,};
u8 main_keys_key_array_15[] = {0x61,0x73,0x6d,};
u8 main_keys_key_array_18[] = {0x73,0x69,0x67,0x6e,0x65,0x64,};
u8 main_keys_key_array_19[] = {0x63,0x6f,0x6e,0x74,0x69,0x6e,0x75,0x65,};
u8 main_keys_key_array_20[] = {0x63,0x6f,0x6e,0x73,0x74,0x5f,0x63,0x61,0x73,0x74,};
u8 main_keys_key_array_21[] = {0x62,0x72,0x65,0x61,0x6b,};
u8 main_keys_key_array_23[] = {0x65,0x6e,0x75,0x6d,};
u8 main_keys_key_array_24[] = {0x69,0x66,};
u8 main_keys_key_array_26[] = {0x70,0x72,0x6f,0x74,0x65,0x63,0x74,0x65,0x64,};
u8 main_keys_key_array_27[] = {0x61,0x6c,0x69,0x67,0x6e,0x6f,0x66,};
u8 main_keys_key_array_28[] = {0x70,0x75,0x62,0x6c,0x69,0x63,};
u8 main_keys_key_array_30[] = {0x72,0x65,0x74,0x75,0x72,0x6e,};
u8 main_keys_key_array_31[] = {0x72,0x65,0x67,0x69,0x73,0x74,0x65,0x72,};
u8 main_keys_key_array_33[] = {0x63,0x6c,0x61,0x73,0x73,};
u8 main_keys_key_array_36[] = {0x74,0x68,0x72,0x65,0x61,0x64,0x5f,0x6c,0x6f,0x63,0x61,0x6c,};
u8 main_keys_key_array_38[] = {0x63,0x68,0x61,0x72,};
u8 main_keys_key_array_39[] = {0x67,0x6f,0x74,0x6f,};
u8 main_keys_key_array_40[] = {0x6e,0x75,0x6c,0x6c,0x70,0x74,0x72,};
u8 main_keys_key_array_43[] = {0x74,0x65,0x6d,0x70,0x6c,0x61,0x74,0x65,};
u8 main_keys_key_array_44[] = {0x74,0x72,0x75,0x65,};
u8 main_keys_key_array_45[] = {0x62,0x6f,0x6f,0x6c,};
u8 main_keys_key_array_47[] = {0x74,0x79,0x70,0x65,0x6e,0x61,0x6d,0x65,};
u8 main_keys_key_array_48[] = {0x69,0x6e,0x74,};
u8 main_keys_key_array_49[] = {0x73,0x77,0x69,0x74,0x63,0x68,};
u8 main_keys_key_array_50[] = {0x76,0x69,0x72,0x74,0x75,0x61,0x6c,};
u8 main_keys_key_array_51[] = {0x69,0x6e,0x6c,0x69,0x6e,0x65,};
u8 main_keys_key_array_54[] = {0x76,0x6f,0x69,0x64,};
u8 main_keys_key_array_57[] = {0x64,0x65,0x6c,0x65,0x74,0x65,};
u8 main_keys_key_array_60[] = {0x63,0x61,0x74,0x63,0x68,};
u8 main_keys_key_array_61[] = {0x65,0x78,0x70,0x6c,0x69,0x63,0x69,0x74,};
u8 main_keys_key_array_62[] = {0x75,0x6e,0x69,0x6f,0x6e,};
u8 main_keys_key_array_66[] = {0x65,0x78,0x70,0x6f,0x72,0x74,};
u8 main_keys_key_array_68[] = {0x73,0x68,0x6f,0x72,0x74,};
u8 main_keys_key_array_70[] = {0x64,0x79,0x6e,0x61,0x6d,0x69,0x63,0x5f,0x63,0x61,0x73,0x74,};
u8 main_keys_key_array_71[] = {0x74,0x72,0x79,};
u8 main_keys_key_array_72[] = {0x6e,0x6f,0x65,0x78,0x63,0x65,0x70,0x74,};
u8 main_keys_key_array_75[] = {0x76,0x6f,0x6c,0x61,0x74,0x69,0x6c,0x65,};
u8 main_keys_key_array_76[] = {0x73,0x69,0x7a,0x65,0x6f,0x66,};
u8 main_keys_key_array_78[] = {0x72,0x65,0x69,0x6e,0x74,0x65,0x72,0x70,0x72,0x65,0x74,0x5f,0x63,0x61,0x73,0x74,};
u8 main_keys_key_array_79[] = {0x64,0x65,0x63,0x6c,0x74,0x79,0x70,0x65,};
u8 main_keys_key_array_82[] = {0x74,0x79,0x70,0x65,0x64,0x65,0x66,};
u8 main_keys_key_array_85[] = {0x64,0x6f,};
u8 main_keys_key_array_86[] = {0x65,0x6c,0x73,0x65,};
u8 main_keys_key_array_90[] = {0x73,0x74,0x61,0x74,0x69,0x63,0x5f,0x63,0x61,0x73,0x74,};
u8 main_keys_key_array_91[] = {0x73,0x74,0x72,0x75,0x63,0x74,};
u8 main_keys_key_array_92[] = {0x70,0x72,0x69,0x76,0x61,0x74,0x65,};
u8 main_keys_key_array_93[] = {0x63,0x6f,0x6e,0x73,0x74,};
u8 main_keys_key_array_94[] = {0x64,0x6f,0x75,0x62,0x6c,0x65,};
u8 main_keys_key_array_96[] = {0x6f,0x70,0x65,0x72,0x61,0x74,0x6f,0x72,};
u8 main_keys_key_array_99[] = {0x74,0x68,0x69,0x73,};
u8 main_keys_key_array_102[] = {0x66,0x6f,0x72,};
u8 main_keys_key_array_105[] = {0x73,0x74,0x61,0x74,0x69,0x63,};
u8 main_keys_key_array_106[] = {0x66,0x6c,0x6f,0x61,0x74,};
u8 main_keys_key_array_108[] = {0x75,0x6e,0x73,0x69,0x67,0x6e,0x65,0x64,};
u8 main_keys_key_array_109[] = {0x64,0x65,0x66,0x61,0x75,0x6c,0x74,};
u8 main_keys_key_array_110[] = {0x66,0x72,0x69,0x65,0x6e,0x64,};
u8 main_keys_key_array_119[] = {0x6c,0x6f,0x6e,0x67,};
u8 main_keys_key_array_120[] = {0x73,0x74,0x61,0x74,0x69,0x63,0x5f,0x61,0x73,0x73,0x65,0x72,0x74,};
String_Const_u8 main_keys_key_array[122] = {
{0, 0},
{main_keys_key_array_1, 7},
{0, 0},
{main_keys_key_array_3, 5},
{0, 0},
{0, 0},
{main_keys_key_array_6, 6},
{main_keys_key_array_7, 4},
{main_keys_key_array_8, 5},
{main_keys_key_array_9, 9},
{main_keys_key_array_10, 3},
{0, 0},
{0, 0},
{main_keys_key_array_13, 5},
{main_keys_key_array_14, 6},
{main_keys_key_array_15, 3},
{0, 0},
{0, 0},
{main_keys_key_array_18, 6},
{main_keys_key_array_19, 8},
{main_keys_key_array_20, 10},
{main_keys_key_array_21, 5},
{0, 0},
{main_keys_key_array_23, 4},
{main_keys_key_array_24, 2},
{0, 0},
{main_keys_key_array_26, 9},
{main_keys_key_array_27, 7},
{main_keys_key_array_28, 6},
{0, 0},
{main_keys_key_array_30, 6},
{main_keys_key_array_31, 8},
{0, 0},
{main_keys_key_array_33, 5},
{0, 0},
{0, 0},
{main_keys_key_array_36, 12},
{0, 0},
{main_keys_key_array_38, 4},
{main_keys_key_array_39, 4},
{main_keys_key_array_40, 7},
{0, 0},
{0, 0},
{main_keys_key_array_43, 8},
{main_keys_key_array_44, 4},
{main_keys_key_array_45, 4},
{0, 0},
{main_keys_key_array_47, 8},
{main_keys_key_array_48, 3},
{main_keys_key_array_49, 6},
{main_keys_key_array_50, 7},
{main_keys_key_array_51, 6},
{0, 0},
{0, 0},
{main_keys_key_array_54, 4},
{0, 0},
{0, 0},
{main_keys_key_array_57, 6},
{0, 0},
{0, 0},
{main_keys_key_array_60, 5},
{main_keys_key_array_61, 8},
{main_keys_key_array_62, 5},
{0, 0},
{0, 0},
{0, 0},
{main_keys_key_array_66, 6},
{0, 0},
{main_keys_key_array_68, 5},
{0, 0},
{main_keys_key_array_70, 12},
{main_keys_key_array_71, 3},
{main_keys_key_array_72, 8},
{0, 0},
{0, 0},
{main_keys_key_array_75, 8},
{main_keys_key_array_76, 6},
{0, 0},
{main_keys_key_array_78, 16},
{main_keys_key_array_79, 8},
{0, 0},
{0, 0},
{main_keys_key_array_82, 7},
{0, 0},
{0, 0},
{main_keys_key_array_85, 2},
{main_keys_key_array_86, 4},
{0, 0},
{0, 0},
{0, 0},
{main_keys_key_array_90, 11},
{main_keys_key_array_91, 6},
{main_keys_key_array_92, 7},
{main_keys_key_array_93, 5},
{main_keys_key_array_94, 6},
{0, 0},
{main_keys_key_array_96, 8},
{0, 0},
{0, 0},
{main_keys_key_array_99, 4},
{0, 0},
{0, 0},
{main_keys_key_array_102, 3},
{0, 0},
{0, 0},
{main_keys_key_array_105, 6},
{main_keys_key_array_106, 5},
{0, 0},
{main_keys_key_array_108, 8},
{main_keys_key_array_109, 7},
{main_keys_key_array_110, 6},
{0, 0},
{0, 0},
{0, 0},
{0, 0},
{0, 0},
{0, 0},
{0, 0},
{0, 0},
{main_keys_key_array_119, 4},
{main_keys_key_array_120, 13},
{0, 0},
};
Lexeme_Table_Value main_keys_value_array[122] = {
{0, 0},
{4, TokenCppKind_AlignAs},
{0, 0},
{8, TokenCppKind_LiteralFalse},
{0, 0},
{0, 0},
{4, TokenCppKind_Extern},
{4, TokenCppKind_Case},
{4, TokenCppKind_While},
{4, TokenCppKind_Namespace},
{4, TokenCppKind_New},
{0, 0},
{0, 0},
{4, TokenCppKind_Using},
{4, TokenCppKind_TypeID},
{4, TokenCppKind_Asm},
{0, 0},
{0, 0},
{4, TokenCppKind_Signed},
{4, TokenCppKind_Continue},
{4, TokenCppKind_ConstCast},
{4, TokenCppKind_Break},
{0, 0},
{4, TokenCppKind_Enum},
{4, TokenCppKind_If},
{0, 0},
{4, TokenCppKind_Protected},
{4, TokenCppKind_AlignOf},
{4, TokenCppKind_Public},
{0, 0},
{4, TokenCppKind_Return},
{4, TokenCppKind_Register},
{0, 0},
{4, TokenCppKind_Class},
{0, 0},
{0, 0},
{4, TokenCppKind_ThreadLocal},
{0, 0},
{4, TokenCppKind_Char},
{4, TokenCppKind_Goto},
{4, TokenCppKind_NullPtr},
{0, 0},
{0, 0},
{4, TokenCppKind_Template},
{8, TokenCppKind_LiteralTrue},
{4, TokenCppKind_Bool},
{0, 0},
{4, TokenCppKind_Typename},
{4, TokenCppKind_Int},
{4, TokenCppKind_Switch},
{4, TokenCppKind_Virtual},
{4, TokenCppKind_Inline},
{0, 0},
{0, 0},
{4, TokenCppKind_Void},
{0, 0},
{0, 0},
{4, TokenCppKind_Delete},
{0, 0},
{0, 0},
{4, TokenCppKind_Catch},
{4, TokenCppKind_Explicit},
{4, TokenCppKind_Union},
{0, 0},
{0, 0},
{0, 0},
{4, TokenCppKind_Export},
{0, 0},
{4, TokenCppKind_Short},
{0, 0},
{4, TokenCppKind_DynamicCast},
{4, TokenCppKind_Try},
{4, TokenCppKind_NoExcept},
{0, 0},
{0, 0},
{4, TokenCppKind_Volatile},
{4, TokenCppKind_SizeOf},
{0, 0},
{4, TokenCppKind_ReinterpretCast},
{4, TokenCppKind_DeclType},
{0, 0},
{0, 0},
{4, TokenCppKind_Typedef},
{0, 0},
{0, 0},
{4, TokenCppKind_Do},
{4, TokenCppKind_Else},
{0, 0},
{0, 0},
{0, 0},
{4, TokenCppKind_StaticCast},
{4, TokenCppKind_Struct},
{4, TokenCppKind_Private},
{4, TokenCppKind_Const},
{4, TokenCppKind_Double},
{0, 0},
{4, TokenCppKind_Operator},
{0, 0},
{0, 0},
{4, TokenCppKind_This},
{0, 0},
{0, 0},
{4, TokenCppKind_For},
{0, 0},
{0, 0},
{4, TokenCppKind_Static},
{4, TokenCppKind_Float},
{0, 0},
{4, TokenCppKind_Unsigned},
{4, TokenCppKind_Default},
{4, TokenCppKind_Friend},
{0, 0},
{0, 0},
{0, 0},
{0, 0},
{0, 0},
{0, 0},
{0, 0},
{0, 0},
{4, TokenCppKind_Long},
{4, TokenCppKind_StaticAssert},
{0, 0},
};
i32 main_keys_slot_count = 122;
u64 main_keys_seed = 0x72a08ef8c3b20b14;
u64 pp_directives_hash_array[25] = {
0xad4540de30c71edd,0x0000000000000000,0x0000000000000000,0x0e18faf5e569c2b1,
0xa64e19bb79cbac21,0x0000000000000000,0xad4540de1b0f0bc9,0x0000000000000000,
0x0000000000000000,0x0000000000000000,0xad4540de18b15083,0xa64e188dd72574bd,
0xffd4966f134594b1,0xffd4966e47d3f229,0x0000000000000000,0x3b12b187afde4529,
0xffd4966e2c27f051,0x0000000000000000,0xad4540d9ef8ab729,0xffd49673f22de19f,
0x0000000000000000,0x0e18faf5e5976a43,0xad4540de12fc7049,0x0e18faf5e5976149,
0x0000000000000000,
};
u8 pp_directives_key_array_0[] = {0x65,0x72,0x72,0x6f,0x72,};
u8 pp_directives_key_array_3[] = {0x6c,0x69,0x6e,0x65,};
u8 pp_directives_key_array_4[] = {0x76,0x65,0x72,0x73,0x69,0x6f,0x6e,};
u8 pp_directives_key_array_6[] = {0x69,0x66,0x64,0x65,0x66,};
u8 pp_directives_key_array_10[] = {0x75,0x73,0x69,0x6e,0x67,};
u8 pp_directives_key_array_11[] = {0x69,0x6e,0x63,0x6c,0x75,0x64,0x65,};
u8 pp_directives_key_array_12[] = {0x64,0x65,0x66,0x69,0x6e,0x65,};
u8 pp_directives_key_array_13[] = {0x69,0x66,0x6e,0x64,0x65,0x66,};
u8 pp_directives_key_array_15[] = {0x69,0x66,};
u8 pp_directives_key_array_16[] = {0x69,0x6d,0x70,0x6f,0x72,0x74,};
u8 pp_directives_key_array_18[] = {0x65,0x6e,0x64,0x69,0x66,};
u8 pp_directives_key_array_19[] = {0x70,0x72,0x61,0x67,0x6d,0x61,};
u8 pp_directives_key_array_21[] = {0x65,0x6c,0x73,0x65,};
u8 pp_directives_key_array_22[] = {0x75,0x6e,0x64,0x65,0x66,};
u8 pp_directives_key_array_23[] = {0x65,0x6c,0x69,0x66,};
String_Const_u8 pp_directives_key_array[25] = {
{pp_directives_key_array_0, 5},
{0, 0},
{0, 0},
{pp_directives_key_array_3, 4},
{pp_directives_key_array_4, 7},
{0, 0},
{pp_directives_key_array_6, 5},
{0, 0},
{0, 0},
{0, 0},
{pp_directives_key_array_10, 5},
{pp_directives_key_array_11, 7},
{pp_directives_key_array_12, 6},
{pp_directives_key_array_13, 6},
{0, 0},
{pp_directives_key_array_15, 2},
{pp_directives_key_array_16, 6},
{0, 0},
{pp_directives_key_array_18, 5},
{pp_directives_key_array_19, 6},
{0, 0},
{pp_directives_key_array_21, 4},
{pp_directives_key_array_22, 5},
{pp_directives_key_array_23, 4},
{0, 0},
};
Lexeme_Table_Value pp_directives_value_array[25] = {
{5, TokenCppKind_PPError},
{0, 0},
{0, 0},
{5, TokenCppKind_PPLine},
{5, TokenCppKind_PPVersion},
{0, 0},
{5, TokenCppKind_PPIfDef},
{0, 0},
{0, 0},
{0, 0},
{5, TokenCppKind_PPUsing},
{5, TokenCppKind_PPInclude},
{5, TokenCppKind_PPDefine},
{5, TokenCppKind_PPIfNDef},
{0, 0},
{5, TokenCppKind_PPIf},
{5, TokenCppKind_PPImport},
{0, 0},
{5, TokenCppKind_PPEndIf},
{5, TokenCppKind_PPPragma},
{0, 0},
{5, TokenCppKind_PPElse},
{5, TokenCppKind_PPUndef},
{5, TokenCppKind_PPElIf},
{0, 0},
};
i32 pp_directives_slot_count = 25;
u64 pp_directives_seed = 0x639be38020429ccc;
u64 pp_keys_hash_array[2] = {
0x03502b4f7f689da7,0x0000000000000000,
};
u8 pp_keys_key_array_0[] = {0x64,0x65,0x66,0x69,0x6e,0x65,0x64,};
String_Const_u8 pp_keys_key_array[2] = {
{pp_keys_key_array_0, 7},
{0, 0},
};
Lexeme_Table_Value pp_keys_value_array[2] = {
{4, TokenCppKind_PPDefined},
{0, 0},
};
i32 pp_keys_slot_count = 2;
u64 pp_keys_seed = 0xbf1efe4a490d5f30;
internal Token_List
lex_full_input_cpp(Arena *arena, String_Const_u8 input){
Token_List list = {};
u32 flags_ZF0 = 0;
u32 flags_KF0 = 0;
u16 flags_KB0 = 0;
u8 *delim_first = input.str;
u8 *delim_one_past_last = input.str;
u8 *emit_ptr = input.str;
u8 *ptr = input.str;
u8 *opl_ptr = ptr + input.size;
{
state_label_1: // root
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_EOF;
token.kind = 0;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto end;
}
}
switch (*ptr){
case 0x00:case 0x01:case 0x02:case 0x03:case 0x04:case 0x05:case 0x06:
case 0x07:case 0x08:case 0x0e:case 0x0f:case 0x10:case 0x11:case 0x12:
case 0x13:case 0x14:case 0x15:case 0x16:case 0x17:case 0x18:case 0x19:
case 0x1a:case 0x1b:case 0x1c:case 0x1d:case 0x1e:case 0x1f:case 0x40:
case 0x60:case 0x7f:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LexError;
token.kind = 2;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x09:case 0x0b:case 0x0c:case 0x0d:case 0x20:
{
if ((HasFlag(flags_KF0, 0x2))){
ptr += 1;
goto state_label_4; // error_body
}
ptr += 1;
goto state_label_3; // whitespace
}break;
case 0x0a:
{
ptr += 1;
flags_KB0 &= ~(0x1);
flags_KF0 &= ~(0x1);
flags_KF0 &= ~(0x2);
goto state_label_3; // whitespace
}break;
case 0x21:
{
ptr += 1;
goto state_label_60; // op stage
}break;
case 0x22:
{
if ((HasFlag(flags_KF0, 0x1))){
ptr += 1;
goto state_label_26; // include_quotes
}
ptr += 1;
goto state_label_32; // string
}break;
case 0x23:
{
if ((!HasFlag(flags_KB0, 0x1))){
ptr += 1;
goto state_label_23; // pp_directive_whitespace
}
ptr += 1;
goto state_label_67; // op stage
}break;
default:
{
ptr += 1;
goto state_label_2; // identifier
}break;
case 0x25:
{
ptr += 1;
goto state_label_64; // op stage
}break;
case 0x26:
{
ptr += 1;
goto state_label_61; // op stage
}break;
case 0x27:
{
ptr += 1;
flags_ZF0 |= 0x40;
goto state_label_32; // string
}break;
case 0x28:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_ParenOp;
token.kind = 13;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x29:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_ParenCl;
token.kind = 14;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x2a:
{
ptr += 1;
goto state_label_63; // op stage
}break;
case 0x2b:
{
ptr += 1;
goto state_label_53; // op stage
}break;
case 0x2c:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Comma;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x2d:
{
ptr += 1;
goto state_label_54; // op stage
}break;
case 0x2e:
{
ptr += 1;
goto state_label_6; // operator_or_fnumber_dot
}break;
case 0x2f:
{
ptr += 1;
goto state_label_7; // operator_or_comment_slash
}break;
case 0x30:
{
ptr += 1;
goto state_label_9; // znumber
}break;
case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:case 0x37:
case 0x38:case 0x39:
{
ptr += 1;
goto state_label_8; // number
}break;
case 0x3a:
{
ptr += 1;
goto state_label_52; // op stage
}break;
case 0x3b:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Semicolon;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x3c:
{
if ((!HasFlag(flags_KF0, 0x1))){
ptr += 1;
goto state_label_56; // op stage
}
ptr += 1;
goto state_label_25; // include_pointy
}break;
case 0x3d:
{
ptr += 1;
goto state_label_59; // op stage
}break;
case 0x3e:
{
ptr += 1;
goto state_label_57; // op stage
}break;
case 0x3f:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Ternary;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x4c:
{
ptr += 1;
flags_ZF0 |= 0x4;
goto state_label_27; // pre_L
}break;
case 0x52:
{
ptr += 1;
goto state_label_31; // pre_R
}break;
case 0x55:
{
ptr += 1;
flags_ZF0 |= 0x20;
goto state_label_29; // pre_U
}break;
case 0x5b:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_BrackOp;
token.kind = 13;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x5c:
{
ptr += 1;
goto state_label_5; // backslash
}break;
case 0x5d:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_BrackCl;
token.kind = 14;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x5e:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Xor;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x75:
{
ptr += 1;
flags_ZF0 |= 0x10;
goto state_label_28; // pre_u
}break;
case 0x7b:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_BraceOp;
token.kind = 11;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x7c:
{
ptr += 1;
goto state_label_62; // op stage
}break;
case 0x7d:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_BraceCl;
token.kind = 12;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x7e:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Tilde;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_2: // identifier
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
if (HasFlag(flags_KB0, 0x1)){
Lexeme_Table_Lookup lookup = lexeme_table_lookup(pp_keys_hash_array, pp_keys_key_array, pp_keys_value_array, pp_keys_slot_count, pp_keys_seed, emit_ptr, token.size);
if (lookup.found_match){
token.kind = lookup.base_kind;
token.sub_kind = lookup.sub_kind;
break;
}
}
Lexeme_Table_Lookup lookup = lexeme_table_lookup(main_keys_hash_array, main_keys_key_array, main_keys_value_array, main_keys_slot_count, main_keys_seed, emit_ptr, token.size);
if (lookup.found_match){
token.kind = lookup.base_kind;
token.sub_kind = lookup.sub_kind;
break;
}
token.sub_kind = TokenCppKind_Identifier;
token.kind = 6;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
case 0x00:case 0x01:case 0x02:case 0x03:case 0x04:case 0x05:case 0x06:
case 0x07:case 0x08:case 0x09:case 0x0a:case 0x0b:case 0x0c:case 0x0d:
case 0x0e:case 0x0f:case 0x10:case 0x11:case 0x12:case 0x13:case 0x14:
case 0x15:case 0x16:case 0x17:case 0x18:case 0x19:case 0x1a:case 0x1b:
case 0x1c:case 0x1d:case 0x1e:case 0x1f:case 0x20:case 0x21:case 0x22:
case 0x23:case 0x25:case 0x26:case 0x27:case 0x28:case 0x29:case 0x2a:
case 0x2b:case 0x2c:case 0x2d:case 0x2e:case 0x2f:case 0x3a:case 0x3b:
case 0x3c:case 0x3d:case 0x3e:case 0x3f:case 0x40:case 0x5b:case 0x5c:
case 0x5d:case 0x5e:case 0x60:case 0x7b:case 0x7c:case 0x7d:case 0x7e:
case 0x7f:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
if (HasFlag(flags_KB0, 0x1)){
Lexeme_Table_Lookup lookup = lexeme_table_lookup(pp_keys_hash_array, pp_keys_key_array, pp_keys_value_array, pp_keys_slot_count, pp_keys_seed, emit_ptr, token.size);
if (lookup.found_match){
token.kind = lookup.base_kind;
token.sub_kind = lookup.sub_kind;
break;
}
}
Lexeme_Table_Lookup lookup = lexeme_table_lookup(main_keys_hash_array, main_keys_key_array, main_keys_value_array, main_keys_slot_count, main_keys_seed, emit_ptr, token.size);
if (lookup.found_match){
token.kind = lookup.base_kind;
token.sub_kind = lookup.sub_kind;
break;
}
token.sub_kind = TokenCppKind_Identifier;
token.kind = 6;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
default:
{
ptr += 1;
goto state_label_2; // identifier
}break;
}
}
{
state_label_3: // whitespace
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Whitespace;
token.kind = 1;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Whitespace;
token.kind = 1;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x09:case 0x0b:case 0x0c:case 0x0d:case 0x20:
{
ptr += 1;
goto state_label_3; // whitespace
}break;
case 0x0a:
{
ptr += 1;
flags_KB0 &= ~(0x1);
flags_KF0 &= ~(0x1);
flags_KF0 &= ~(0x2);
goto state_label_3; // whitespace
}break;
}
}
{
state_label_4: // error_body
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_PPErrorMessage;
token.kind = 10;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
ptr += 1;
goto state_label_4; // error_body
}break;
case 0x0a:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_PPErrorMessage;
token.kind = 10;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_5: // backslash
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Backslash;
token.kind = 1;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Backslash;
token.kind = 1;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x0a:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Backslash;
token.kind = 1;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_6: // operator_or_fnumber_dot
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Dot;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Dot;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x2a:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_DotStar;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x2e:
{
ptr += 1;
goto state_label_68; // op stage
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:
{
ptr += 1;
goto state_label_10; // fnumber_decimal
}break;
}
}
{
state_label_7: // operator_or_comment_slash
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Div;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Div;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x2a:
{
ptr += 1;
goto state_label_49; // comment_block
}break;
case 0x2f:
{
ptr += 1;
goto state_label_51; // comment_line
}break;
case 0x3d:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_DivEq;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_8: // number
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LiteralInteger;
token.kind = 8;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LiteralInteger;
token.kind = 8;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x2e:
{
ptr += 1;
goto state_label_10; // fnumber_decimal
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:
{
ptr += 1;
goto state_label_8; // number
}break;
case 0x45:case 0x65:
{
ptr += 1;
goto state_label_11; // fnumber_exponent
}break;
case 0x4c:
{
ptr += 1;
goto state_label_18; // L_number
}break;
case 0x55:case 0x75:
{
ptr += 1;
goto state_label_17; // U_number
}break;
case 0x6c:
{
ptr += 1;
goto state_label_20; // l_number
}break;
}
}
{
state_label_9: // znumber
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LiteralInteger;
token.kind = 8;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LiteralInteger;
token.kind = 8;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x2e:
{
ptr += 1;
goto state_label_10; // fnumber_decimal
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:
{
ptr += 1;
flags_ZF0 |= 0x2;
goto state_label_16; // number_oct
}break;
case 0x45:case 0x65:
{
ptr += 1;
goto state_label_11; // fnumber_exponent
}break;
case 0x4c:
{
ptr += 1;
goto state_label_18; // L_number
}break;
case 0x55:case 0x75:
{
ptr += 1;
goto state_label_17; // U_number
}break;
case 0x58:case 0x78:
{
ptr += 1;
flags_ZF0 |= 0x1;
goto state_label_14; // number_hex_first
}break;
case 0x6c:
{
ptr += 1;
goto state_label_20; // l_number
}break;
}
}
{
state_label_10: // fnumber_decimal
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LiteralFloat64;
token.kind = 9;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LiteralFloat64;
token.kind = 9;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:
{
ptr += 1;
goto state_label_10; // fnumber_decimal
}break;
case 0x45:case 0x65:
{
ptr += 1;
goto state_label_11; // fnumber_exponent
}break;
case 0x46:case 0x66:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LiteralFloat32;
token.kind = 9;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x4c:case 0x6c:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LiteralFloat64;
token.kind = 9;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_11: // fnumber_exponent
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LiteralFloat64;
token.kind = 9;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LiteralFloat64;
token.kind = 9;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x2b:case 0x2d:
{
ptr += 1;
goto state_label_12; // fnumber_exponent_sign
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:
{
ptr += 1;
goto state_label_13; // fnumber_exponent_digits
}break;
case 0x46:case 0x66:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LiteralFloat32;
token.kind = 9;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x4c:case 0x6c:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LiteralFloat64;
token.kind = 9;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_12: // fnumber_exponent_sign
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LiteralFloat64;
token.kind = 9;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LiteralFloat64;
token.kind = 9;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:
{
ptr += 1;
goto state_label_13; // fnumber_exponent_digits
}break;
case 0x46:case 0x66:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LiteralFloat32;
token.kind = 9;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x4c:case 0x6c:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LiteralFloat64;
token.kind = 9;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_13: // fnumber_exponent_digits
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LiteralFloat64;
token.kind = 9;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LiteralFloat64;
token.kind = 9;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:
{
ptr += 1;
goto state_label_13; // fnumber_exponent_digits
}break;
case 0x46:case 0x66:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LiteralFloat32;
token.kind = 9;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x4c:case 0x6c:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LiteralFloat64;
token.kind = 9;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_14: // number_hex_first
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LexError;
token.kind = 2;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LexError;
token.kind = 2;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
case 0x66:
{
ptr += 1;
goto state_label_15; // number_hex
}break;
}
}
{
state_label_15: // number_hex
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LiteralIntegerHex;
token.kind = 8;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LiteralIntegerHex;
token.kind = 8;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
case 0x66:
{
ptr += 1;
goto state_label_15; // number_hex
}break;
case 0x4c:
{
ptr += 1;
goto state_label_18; // L_number
}break;
case 0x55:case 0x75:
{
ptr += 1;
goto state_label_17; // U_number
}break;
case 0x6c:
{
ptr += 1;
goto state_label_20; // l_number
}break;
}
}
{
state_label_16: // number_oct
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LiteralIntegerOct;
token.kind = 8;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LiteralIntegerOct;
token.kind = 8;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:
{
ptr += 1;
flags_ZF0 |= 0x2;
goto state_label_16; // number_oct
}break;
case 0x4c:
{
ptr += 1;
goto state_label_18; // L_number
}break;
case 0x55:case 0x75:
{
ptr += 1;
goto state_label_17; // U_number
}break;
case 0x6c:
{
ptr += 1;
goto state_label_20; // l_number
}break;
}
}
{
state_label_17: // U_number
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
if (HasFlag(flags_ZF0, 0x1)){
token.sub_kind = TokenCppKind_LiteralIntegerHexU;
token.kind = 8;
break;
}
if (HasFlag(flags_ZF0, 0x2)){
token.sub_kind = TokenCppKind_LiteralIntegerOctU;
token.kind = 8;
break;
}
token.sub_kind = TokenCppKind_LiteralIntegerU;
token.kind = 8;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
if (HasFlag(flags_ZF0, 0x1)){
token.sub_kind = TokenCppKind_LiteralIntegerHexU;
token.kind = 8;
break;
}
if (HasFlag(flags_ZF0, 0x2)){
token.sub_kind = TokenCppKind_LiteralIntegerOctU;
token.kind = 8;
break;
}
token.sub_kind = TokenCppKind_LiteralIntegerU;
token.kind = 8;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x4c:
{
ptr += 1;
goto state_label_19; // UL_number
}break;
case 0x6c:
{
ptr += 1;
goto state_label_21; // Ul_number
}break;
}
}
{
state_label_18: // L_number
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
if (HasFlag(flags_ZF0, 0x1)){
token.sub_kind = TokenCppKind_LiteralIntegerHexL;
token.kind = 8;
break;
}
if (HasFlag(flags_ZF0, 0x2)){
token.sub_kind = TokenCppKind_LiteralIntegerOctL;
token.kind = 8;
break;
}
token.sub_kind = TokenCppKind_LiteralIntegerL;
token.kind = 8;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
if (HasFlag(flags_ZF0, 0x1)){
token.sub_kind = TokenCppKind_LiteralIntegerHexL;
token.kind = 8;
break;
}
if (HasFlag(flags_ZF0, 0x2)){
token.sub_kind = TokenCppKind_LiteralIntegerOctL;
token.kind = 8;
break;
}
token.sub_kind = TokenCppKind_LiteralIntegerL;
token.kind = 8;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x4c:
{
ptr += 1;
goto state_label_22; // LL_number
}break;
case 0x55:case 0x75:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
if (HasFlag(flags_ZF0, 0x1)){
token.sub_kind = TokenCppKind_LiteralIntegerHexUL;
token.kind = 8;
break;
}
if (HasFlag(flags_ZF0, 0x2)){
token.sub_kind = TokenCppKind_LiteralIntegerOctUL;
token.kind = 8;
break;
}
token.sub_kind = TokenCppKind_LiteralIntegerUL;
token.kind = 8;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_19: // UL_number
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
if (HasFlag(flags_ZF0, 0x1)){
token.sub_kind = TokenCppKind_LiteralIntegerHexUL;
token.kind = 8;
break;
}
if (HasFlag(flags_ZF0, 0x2)){
token.sub_kind = TokenCppKind_LiteralIntegerOctUL;
token.kind = 8;
break;
}
token.sub_kind = TokenCppKind_LiteralIntegerUL;
token.kind = 8;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
if (HasFlag(flags_ZF0, 0x1)){
token.sub_kind = TokenCppKind_LiteralIntegerHexUL;
token.kind = 8;
break;
}
if (HasFlag(flags_ZF0, 0x2)){
token.sub_kind = TokenCppKind_LiteralIntegerOctUL;
token.kind = 8;
break;
}
token.sub_kind = TokenCppKind_LiteralIntegerUL;
token.kind = 8;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x4c:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
if (HasFlag(flags_ZF0, 0x1)){
token.sub_kind = TokenCppKind_LiteralIntegerHexULL;
token.kind = 8;
break;
}
if (HasFlag(flags_ZF0, 0x2)){
token.sub_kind = TokenCppKind_LiteralIntegerOctULL;
token.kind = 8;
break;
}
token.sub_kind = TokenCppKind_LiteralIntegerULL;
token.kind = 8;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_20: // l_number
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
if (HasFlag(flags_ZF0, 0x1)){
token.sub_kind = TokenCppKind_LiteralIntegerHexL;
token.kind = 8;
break;
}
if (HasFlag(flags_ZF0, 0x2)){
token.sub_kind = TokenCppKind_LiteralIntegerOctL;
token.kind = 8;
break;
}
token.sub_kind = TokenCppKind_LiteralIntegerL;
token.kind = 8;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
if (HasFlag(flags_ZF0, 0x1)){
token.sub_kind = TokenCppKind_LiteralIntegerHexL;
token.kind = 8;
break;
}
if (HasFlag(flags_ZF0, 0x2)){
token.sub_kind = TokenCppKind_LiteralIntegerOctL;
token.kind = 8;
break;
}
token.sub_kind = TokenCppKind_LiteralIntegerL;
token.kind = 8;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x55:case 0x75:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
if (HasFlag(flags_ZF0, 0x1)){
token.sub_kind = TokenCppKind_LiteralIntegerHexUL;
token.kind = 8;
break;
}
if (HasFlag(flags_ZF0, 0x2)){
token.sub_kind = TokenCppKind_LiteralIntegerOctUL;
token.kind = 8;
break;
}
token.sub_kind = TokenCppKind_LiteralIntegerUL;
token.kind = 8;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x6c:
{
ptr += 1;
goto state_label_22; // LL_number
}break;
}
}
{
state_label_21: // Ul_number
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
if (HasFlag(flags_ZF0, 0x1)){
token.sub_kind = TokenCppKind_LiteralIntegerHexUL;
token.kind = 8;
break;
}
if (HasFlag(flags_ZF0, 0x2)){
token.sub_kind = TokenCppKind_LiteralIntegerOctUL;
token.kind = 8;
break;
}
token.sub_kind = TokenCppKind_LiteralIntegerUL;
token.kind = 8;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
if (HasFlag(flags_ZF0, 0x1)){
token.sub_kind = TokenCppKind_LiteralIntegerHexUL;
token.kind = 8;
break;
}
if (HasFlag(flags_ZF0, 0x2)){
token.sub_kind = TokenCppKind_LiteralIntegerOctUL;
token.kind = 8;
break;
}
token.sub_kind = TokenCppKind_LiteralIntegerUL;
token.kind = 8;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x6c:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
if (HasFlag(flags_ZF0, 0x1)){
token.sub_kind = TokenCppKind_LiteralIntegerHexULL;
token.kind = 8;
break;
}
if (HasFlag(flags_ZF0, 0x2)){
token.sub_kind = TokenCppKind_LiteralIntegerOctULL;
token.kind = 8;
break;
}
token.sub_kind = TokenCppKind_LiteralIntegerULL;
token.kind = 8;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_22: // LL_number
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
if (HasFlag(flags_ZF0, 0x1)){
token.sub_kind = TokenCppKind_LiteralIntegerHexLL;
token.kind = 8;
break;
}
if (HasFlag(flags_ZF0, 0x2)){
token.sub_kind = TokenCppKind_LiteralIntegerOctLL;
token.kind = 8;
break;
}
token.sub_kind = TokenCppKind_LiteralIntegerLL;
token.kind = 8;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
if (HasFlag(flags_ZF0, 0x1)){
token.sub_kind = TokenCppKind_LiteralIntegerHexLL;
token.kind = 8;
break;
}
if (HasFlag(flags_ZF0, 0x2)){
token.sub_kind = TokenCppKind_LiteralIntegerOctLL;
token.kind = 8;
break;
}
token.sub_kind = TokenCppKind_LiteralIntegerLL;
token.kind = 8;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x55:case 0x75:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
if (HasFlag(flags_ZF0, 0x1)){
token.sub_kind = TokenCppKind_LiteralIntegerHexULL;
token.kind = 8;
break;
}
if (HasFlag(flags_ZF0, 0x2)){
token.sub_kind = TokenCppKind_LiteralIntegerOctULL;
token.kind = 8;
break;
}
token.sub_kind = TokenCppKind_LiteralIntegerULL;
token.kind = 8;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_23: // pp_directive_whitespace
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LexError;
token.kind = 2;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LexError;
token.kind = 2;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x09:case 0x0b:case 0x0c:case 0x20:
{
ptr += 1;
goto state_label_23; // pp_directive_whitespace
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
case 0x45:case 0x46:case 0x47:case 0x48:case 0x49:case 0x4a:case 0x4b:
case 0x4c:case 0x4d:case 0x4e:case 0x4f:case 0x50:case 0x51:case 0x52:
case 0x53:case 0x54:case 0x55:case 0x56:case 0x57:case 0x58:case 0x59:
case 0x5a:case 0x5f:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
case 0x66:case 0x67:case 0x68:case 0x69:case 0x6a:case 0x6b:case 0x6c:
case 0x6d:case 0x6e:case 0x6f:case 0x70:case 0x71:case 0x72:case 0x73:
case 0x74:case 0x75:case 0x76:case 0x77:case 0x78:case 0x79:case 0x7a:
{
delim_first = ptr;
flags_KB0 |= 0x1;
ptr += 1;
goto state_label_24; // pp_directive
}break;
}
}
{
state_label_24: // pp_directive
if (ptr == opl_ptr){
if ((true)){
delim_one_past_last = ptr;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
Lexeme_Table_Lookup lookup = lexeme_table_lookup(pp_directives_hash_array, pp_directives_key_array, pp_directives_value_array, pp_directives_slot_count, pp_directives_seed, delim_first, (delim_one_past_last - delim_first));
if (lookup.found_match){
token.kind = lookup.base_kind;
token.sub_kind = lookup.sub_kind;
break;
}
token.sub_kind = TokenCppKind_PPUnknown;
token.kind = 2;
}while(0);
switch (token.sub_kind){
case TokenCppKind_PPInclude:
{
flags_KF0 |= 0x1;
}break;
case TokenCppKind_PPError:
{
flags_KF0 |= 0x2;
}break;
}
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
delim_one_past_last = ptr;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
Lexeme_Table_Lookup lookup = lexeme_table_lookup(pp_directives_hash_array, pp_directives_key_array, pp_directives_value_array, pp_directives_slot_count, pp_directives_seed, delim_first, (delim_one_past_last - delim_first));
if (lookup.found_match){
token.kind = lookup.base_kind;
token.sub_kind = lookup.sub_kind;
break;
}
token.sub_kind = TokenCppKind_PPUnknown;
token.kind = 2;
}while(0);
switch (token.sub_kind){
case TokenCppKind_PPInclude:
{
flags_KF0 |= 0x1;
}break;
case TokenCppKind_PPError:
{
flags_KF0 |= 0x2;
}break;
}
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
case 0x45:case 0x46:case 0x47:case 0x48:case 0x49:case 0x4a:case 0x4b:
case 0x4c:case 0x4d:case 0x4e:case 0x4f:case 0x50:case 0x51:case 0x52:
case 0x53:case 0x54:case 0x55:case 0x56:case 0x57:case 0x58:case 0x59:
case 0x5a:case 0x5f:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
case 0x66:case 0x67:case 0x68:case 0x69:case 0x6a:case 0x6b:case 0x6c:
case 0x6d:case 0x6e:case 0x6f:case 0x70:case 0x71:case 0x72:case 0x73:
case 0x74:case 0x75:case 0x76:case 0x77:case 0x78:case 0x79:case 0x7a:
{
ptr += 1;
goto state_label_24; // pp_directive
}break;
}
}
{
state_label_25: // include_pointy
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LexError;
token.kind = 2;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LexError;
token.kind = 2;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x20:case 0x2e:case 0x2f:case 0x30:case 0x31:case 0x32:case 0x33:
case 0x34:case 0x35:case 0x36:case 0x37:case 0x38:case 0x39:case 0x41:
case 0x42:case 0x43:case 0x44:case 0x45:case 0x46:case 0x47:case 0x48:
case 0x49:case 0x4a:case 0x4b:case 0x4c:case 0x4d:case 0x4e:case 0x4f:
case 0x50:case 0x51:case 0x52:case 0x53:case 0x54:case 0x55:case 0x56:
case 0x57:case 0x58:case 0x59:case 0x5a:case 0x5c:case 0x5f:case 0x61:
case 0x62:case 0x63:case 0x64:case 0x65:case 0x66:case 0x67:case 0x68:
case 0x69:case 0x6a:case 0x6b:case 0x6c:case 0x6d:case 0x6e:case 0x6f:
case 0x70:case 0x71:case 0x72:case 0x73:case 0x74:case 0x75:case 0x76:
case 0x77:case 0x78:case 0x79:case 0x7a:
{
ptr += 1;
goto state_label_25; // include_pointy
}break;
case 0x3e:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_PPIncludeFile;
token.kind = 10;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_26: // include_quotes
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LexError;
token.kind = 2;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LexError;
token.kind = 2;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x20:case 0x2e:case 0x2f:case 0x30:case 0x31:case 0x32:case 0x33:
case 0x34:case 0x35:case 0x36:case 0x37:case 0x38:case 0x39:case 0x41:
case 0x42:case 0x43:case 0x44:case 0x45:case 0x46:case 0x47:case 0x48:
case 0x49:case 0x4a:case 0x4b:case 0x4c:case 0x4d:case 0x4e:case 0x4f:
case 0x50:case 0x51:case 0x52:case 0x53:case 0x54:case 0x55:case 0x56:
case 0x57:case 0x58:case 0x59:case 0x5a:case 0x5c:case 0x5f:case 0x61:
case 0x62:case 0x63:case 0x64:case 0x65:case 0x66:case 0x67:case 0x68:
case 0x69:case 0x6a:case 0x6b:case 0x6c:case 0x6d:case 0x6e:case 0x6f:
case 0x70:case 0x71:case 0x72:case 0x73:case 0x74:case 0x75:case 0x76:
case 0x77:case 0x78:case 0x79:case 0x7a:
{
ptr += 1;
goto state_label_26; // include_quotes
}break;
case 0x22:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_PPIncludeFile;
token.kind = 10;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_27: // pre_L
if (ptr == opl_ptr){
if ((true)){
goto state_label_2; // identifier
}
}
switch (*ptr){
default:
{
goto state_label_2; // identifier
}break;
case 0x22:
{
ptr += 1;
goto state_label_32; // string
}break;
case 0x52:
{
ptr += 1;
goto state_label_31; // pre_R
}break;
}
}
{
state_label_28: // pre_u
if (ptr == opl_ptr){
if ((true)){
goto state_label_2; // identifier
}
}
switch (*ptr){
default:
{
goto state_label_2; // identifier
}break;
case 0x22:
{
ptr += 1;
goto state_label_32; // string
}break;
case 0x38:
{
ptr += 1;
flags_ZF0 |= 0x8;
goto state_label_30; // pre_u8
}break;
case 0x52:
{
ptr += 1;
goto state_label_31; // pre_R
}break;
}
}
{
state_label_29: // pre_U
if (ptr == opl_ptr){
if ((true)){
goto state_label_2; // identifier
}
}
switch (*ptr){
default:
{
goto state_label_2; // identifier
}break;
case 0x22:
{
ptr += 1;
goto state_label_32; // string
}break;
case 0x52:
{
ptr += 1;
goto state_label_31; // pre_R
}break;
}
}
{
state_label_30: // pre_u8
if (ptr == opl_ptr){
if ((true)){
goto state_label_2; // identifier
}
}
switch (*ptr){
default:
{
goto state_label_2; // identifier
}break;
case 0x22:
{
ptr += 1;
goto state_label_32; // string
}break;
case 0x52:
{
ptr += 1;
goto state_label_31; // pre_R
}break;
}
}
{
state_label_31: // pre_R
if (ptr == opl_ptr){
if ((true)){
goto state_label_2; // identifier
}
}
switch (*ptr){
default:
{
goto state_label_2; // identifier
}break;
case 0x22:
{
ptr += 1;
delim_first = ptr;
goto state_label_45; // raw_string_get_delim
}break;
}
}
{
state_label_32: // string
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LexError;
token.kind = 2;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
ptr += 1;
goto state_label_32; // string
}break;
case 0x0a:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LexError;
token.kind = 2;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x22:
{
if ((!HasFlag(flags_ZF0, 0x40))){
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
if (HasFlag(flags_ZF0, 0x4)){
token.sub_kind = TokenCppKind_LiteralStringWide;
token.kind = 10;
break;
}
if (HasFlag(flags_ZF0, 0x8)){
token.sub_kind = TokenCppKind_LiteralStringUTF8;
token.kind = 10;
break;
}
if (HasFlag(flags_ZF0, 0x10)){
token.sub_kind = TokenCppKind_LiteralStringUTF16;
token.kind = 10;
break;
}
if (HasFlag(flags_ZF0, 0x20)){
token.sub_kind = TokenCppKind_LiteralStringUTF32;
token.kind = 10;
break;
}
token.sub_kind = TokenCppKind_LiteralString;
token.kind = 10;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
ptr += 1;
goto state_label_32; // string
}break;
case 0x27:
{
if ((HasFlag(flags_ZF0, 0x40))){
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
if (HasFlag(flags_ZF0, 0x4)){
token.sub_kind = TokenCppKind_LiteralCharacterWide;
token.kind = 10;
break;
}
if (HasFlag(flags_ZF0, 0x8)){
token.sub_kind = TokenCppKind_LiteralCharacterUTF8;
token.kind = 10;
break;
}
if (HasFlag(flags_ZF0, 0x10)){
token.sub_kind = TokenCppKind_LiteralCharacterUTF16;
token.kind = 10;
break;
}
if (HasFlag(flags_ZF0, 0x20)){
token.sub_kind = TokenCppKind_LiteralCharacterUTF32;
token.kind = 10;
break;
}
token.sub_kind = TokenCppKind_LiteralCharacter;
token.kind = 10;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
ptr += 1;
goto state_label_32; // string
}break;
case 0x5c:
{
ptr += 1;
goto state_label_33; // string_esc
}break;
}
}
{
state_label_33: // string_esc
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LexError;
token.kind = 2;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_EOF;
token.kind = 0;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto end;
}
}
switch (*ptr){
default:
{
ptr += 1;
goto state_label_32; // string
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:
{
ptr += 1;
goto state_label_34; // string_esc_oct2
}break;
case 0x55:
{
ptr += 1;
goto state_label_37; // string_esc_universal_8
}break;
case 0x75:
{
ptr += 1;
goto state_label_41; // string_esc_universal_4
}break;
case 0x78:
{
ptr += 1;
goto state_label_36; // string_esc_hex
}break;
}
}
{
state_label_34: // string_esc_oct2
if (ptr == opl_ptr){
if ((true)){
goto state_label_32; // string
}
}
switch (*ptr){
default:
{
goto state_label_32; // string
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:
{
ptr += 1;
goto state_label_35; // string_esc_oct1
}break;
}
}
{
state_label_35: // string_esc_oct1
if (ptr == opl_ptr){
if ((true)){
goto state_label_32; // string
}
}
switch (*ptr){
default:
{
goto state_label_32; // string
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:
{
ptr += 1;
goto state_label_32; // string
}break;
}
}
{
state_label_36: // string_esc_hex
if (ptr == opl_ptr){
if ((true)){
goto state_label_32; // string
}
}
switch (*ptr){
default:
{
goto state_label_32; // string
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
case 0x66:
{
ptr += 1;
goto state_label_36; // string_esc_hex
}break;
}
}
{
state_label_37: // string_esc_universal_8
if (ptr == opl_ptr){
if ((true)){
goto state_label_32; // string
}
}
switch (*ptr){
default:
{
goto state_label_32; // string
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
case 0x66:
{
ptr += 1;
goto state_label_38; // string_esc_universal_7
}break;
}
}
{
state_label_38: // string_esc_universal_7
if (ptr == opl_ptr){
if ((true)){
goto state_label_32; // string
}
}
switch (*ptr){
default:
{
goto state_label_32; // string
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
case 0x66:
{
ptr += 1;
goto state_label_39; // string_esc_universal_6
}break;
}
}
{
state_label_39: // string_esc_universal_6
if (ptr == opl_ptr){
if ((true)){
goto state_label_32; // string
}
}
switch (*ptr){
default:
{
goto state_label_32; // string
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
case 0x66:
{
ptr += 1;
goto state_label_40; // string_esc_universal_5
}break;
}
}
{
state_label_40: // string_esc_universal_5
if (ptr == opl_ptr){
if ((true)){
goto state_label_32; // string
}
}
switch (*ptr){
default:
{
goto state_label_32; // string
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
case 0x66:
{
ptr += 1;
goto state_label_41; // string_esc_universal_4
}break;
}
}
{
state_label_41: // string_esc_universal_4
if (ptr == opl_ptr){
if ((true)){
goto state_label_32; // string
}
}
switch (*ptr){
default:
{
goto state_label_32; // string
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
case 0x66:
{
ptr += 1;
goto state_label_42; // string_esc_universal_3
}break;
}
}
{
state_label_42: // string_esc_universal_3
if (ptr == opl_ptr){
if ((true)){
goto state_label_32; // string
}
}
switch (*ptr){
default:
{
goto state_label_32; // string
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
case 0x66:
{
ptr += 1;
goto state_label_43; // string_esc_universal_2
}break;
}
}
{
state_label_43: // string_esc_universal_2
if (ptr == opl_ptr){
if ((true)){
goto state_label_32; // string
}
}
switch (*ptr){
default:
{
goto state_label_32; // string
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
case 0x66:
{
ptr += 1;
goto state_label_44; // string_esc_universal_1
}break;
}
}
{
state_label_44: // string_esc_universal_1
if (ptr == opl_ptr){
if ((true)){
goto state_label_32; // string
}
}
switch (*ptr){
default:
{
goto state_label_32; // string
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
case 0x66:
{
ptr += 1;
goto state_label_32; // string
}break;
}
}
{
state_label_45: // raw_string_get_delim
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LexError;
token.kind = 2;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_EOF;
token.kind = 0;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto end;
}
}
switch (*ptr){
default:
{
ptr += 1;
goto state_label_45; // raw_string_get_delim
}break;
case 0x20:case 0x29:case 0x5c:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LexError;
token.kind = 2;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x28:
{
delim_one_past_last = ptr;
ptr += 1;
goto state_label_46; // raw_string_find_close
}break;
}
}
{
state_label_46: // raw_string_find_close
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LexError;
token.kind = 2;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_EOF;
token.kind = 0;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto end;
}
}
switch (*ptr){
default:
{
ptr += 1;
goto state_label_46; // raw_string_find_close
}break;
case 0x29:
{
ptr += 1;
goto state_label_47; // raw_string_try_delim
}break;
}
}
{
state_label_47: // raw_string_try_delim
umem delim_length = delim_one_past_last - delim_first;
umem parse_length = 0;
for (;;){
if (parse_length == delim_length){
goto state_label_48; // raw_string_try_quote
}
if (ptr == opl_ptr){
goto state_label_48; // raw_string_try_quote
}
if (*ptr == delim_first[parse_length]){
ptr += 1;
parse_length += 1;
}
else{
goto state_label_46; // raw_string_find_close
}
}
}
{
state_label_48: // raw_string_try_quote
if (ptr == opl_ptr){
if ((true)){
goto state_label_46; // raw_string_find_close
}
}
switch (*ptr){
default:
{
goto state_label_46; // raw_string_find_close
}break;
case 0x22:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
if (HasFlag(flags_ZF0, 0x4)){
token.sub_kind = TokenCppKind_LiteralStringWideRaw;
token.kind = 10;
break;
}
if (HasFlag(flags_ZF0, 0x8)){
token.sub_kind = TokenCppKind_LiteralStringUTF8Raw;
token.kind = 10;
break;
}
if (HasFlag(flags_ZF0, 0x10)){
token.sub_kind = TokenCppKind_LiteralStringUTF16Raw;
token.kind = 10;
break;
}
if (HasFlag(flags_ZF0, 0x20)){
token.sub_kind = TokenCppKind_LiteralStringUTF32Raw;
token.kind = 10;
break;
}
token.sub_kind = TokenCppKind_LiteralStringRaw;
token.kind = 10;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_49: // comment_block
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_BlockComment;
token.kind = 3;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_EOF;
token.kind = 0;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto end;
}
}
switch (*ptr){
default:
{
ptr += 1;
goto state_label_49; // comment_block
}break;
case 0x0a:
{
ptr += 1;
flags_KB0 &= ~(0x1);
flags_KF0 &= ~(0x1);
goto state_label_49; // comment_block
}break;
case 0x2a:
{
ptr += 1;
goto state_label_50; // comment_block_try_close
}break;
}
}
{
state_label_50: // comment_block_try_close
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_BlockComment;
token.kind = 3;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_EOF;
token.kind = 0;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto end;
}
}
switch (*ptr){
default:
{
ptr += 1;
goto state_label_49; // comment_block
}break;
case 0x2a:
{
ptr += 1;
goto state_label_50; // comment_block_try_close
}break;
case 0x2f:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_BlockComment;
token.kind = 3;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_51: // comment_line
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LineComment;
token.kind = 3;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
ptr += 1;
goto state_label_51; // comment_line
}break;
case 0x0a:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LineComment;
token.kind = 3;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_52: // op stage
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Colon;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Colon;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x3a:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_ColonColon;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_53: // op stage
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Plus;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Plus;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x2b:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_PlusPlus;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x3d:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_PlusEq;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_54: // op stage
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Minus;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Minus;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x2d:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_MinusMinus;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x3d:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_MinusEq;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x3e:
{
ptr += 1;
goto state_label_55; // op stage
}break;
}
}
{
state_label_55: // op stage
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Arrow;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Arrow;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x2a:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_ArrowStar;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_56: // op stage
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Less;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Less;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x3c:
{
ptr += 1;
goto state_label_65; // op stage
}break;
case 0x3d:
{
ptr += 1;
goto state_label_58; // op stage
}break;
}
}
{
state_label_57: // op stage
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Grtr;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Grtr;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x3d:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_GrtrEq;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x3e:
{
ptr += 1;
goto state_label_66; // op stage
}break;
}
}
{
state_label_58: // op stage
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LessEq;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LessEq;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x3e:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Compare;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_59: // op stage
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Eq;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Eq;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x3d:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_EqEq;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_60: // op stage
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Not;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Not;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x3d:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_NotEq;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_61: // op stage
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_And;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_And;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x26:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_AndAnd;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_62: // op stage
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Or;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Or;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x7c:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_OrOr;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_63: // op stage
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Star;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Star;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x3d:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_StarEq;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_64: // op stage
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Mod;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_Mod;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x3d:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_ModEq;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_65: // op stage
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LeftLeft;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LeftLeft;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x3d:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LeftLeftEq;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_66: // op stage
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_RightRight;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_RightRight;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x3d:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_RightRightEq;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_67: // op stage
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_PPStringify;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_PPStringify;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x23:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_PPConcat;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
{
state_label_68: // op stage
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LexError;
token.kind = 2;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}
}
switch (*ptr){
default:
{
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_LexError;
token.kind = 2;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
case 0x2e:
{
ptr += 1;
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
token.sub_kind = TokenCppKind_DotDotDot;
token.kind = 7;
}while(0);
token_list_push(arena, &list, &token);
emit_ptr = ptr;
}
flags_ZF0 = 0;
goto state_label_1; // root
}break;
}
}
end:;
return(list);
}
