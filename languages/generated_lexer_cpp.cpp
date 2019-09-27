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
u64 main_keys_hash_array[123] = {
0x0000000000000000,0x6f4a79edb9a5533f,0xe5852ac396610dc5,0x0000000000000000,
0x0000000000000000,0x0000000000000000,0xa2a5dfeb017ed5bd,0xcd6340d166644e7f,
0x8408b482fc52169f,0x0000000000000000,0xcedf49ae63560b69,0x0000000000000000,
0x0000000000000000,0x0000000000000000,0xe75cf7ee7286ec9d,0x0000000000000000,
0x8408b482c15279f5,0x0000000000000000,0xe5852ab101aa40f9,0x0000000000000000,
0x0000000000000000,0x0000000000000000,0x0000000000000000,0x0000000000000000,
0x0000000000000000,0x0000000000000000,0xcede45b28f565327,0x0000000000000000,
0x0000000000000000,0x3cb9f5ca3e080c89,0x0000000000000000,0x0000000000000000,
0xe5852ac24b661f2d,0xe5852ab17aec8db7,0x0000000000000000,0xcd6340d166659c89,
0x0000000000000000,0x0000000000000000,0x8408b482facd5b93,0x12cbed639027df6f,
0xcedeb002ce9cbaf7,0xcee7dcc940536a31,0x0000000000000000,0x3cb9f5ca3e080c1d,
0x0000000000000000,0xe5852ac3c0cbc80d,0xcede5b51ee94b7c1,0x9a20703d4bd687c5,
0x8408b4828b3f4029,0x0000000000000000,0x0000000000000000,0xcedf9451bda57031,
0xe5852ac394d6e7af,0x44af72658a8ddf6f,0xe5852ab45a7e4877,0x0000000000000000,
0x3cb9f115b38586fd,0xe5852aeaff3b438d,0x666eef765675122f,0x0000000000000000,
0x8408b482cd76ef71,0xcd6340d1662d7e9f,0x0000000000000000,0x0000000000000000,
0xa2a5dfeb017e0e17,0x0000000000000000,0x0000000000000000,0x0000000000000000,
0xcd6340d166330431,0x0000000000000000,0x0000000000000000,0xcede1b147919cf71,
0x0000000000000000,0xe5852ab14f69af13,0x0000000000000000,0x0000000000000000,
0x0000000000000000,0xbe87c88e9f25c76f,0xe5852ac371f40231,0x0000000000000000,
0x6fcb06cae6e806ff,0x0000000000000000,0x8408b482fa970537,0x3cb9f05f96b9d771,
0xe5852ab17aadebb5,0x23945d1e642d9f6f,0xe75cf7ee7286f2a7,0x0000000000000000,
0x0000000000000000,0x3cb9f6891369547f,0xcd6340d16634823f,0x0000000000000000,
0xe5852ab1ac65f55f,0x8408b482caf244cd,0xcd6340d1629b9d0f,0x0000000000000000,
0x0000000000000000,0x0000000000000000,0x8408b482fa83e8a9,0x3cb9cc3692b7c61b,
0xcede1b03b4c5ebb7,0x3cb9f5df428aa011,0xa2a5dfeb017fd991,0x0000000000000000,
0xcd6340d16794567f,0xcd6340d166781707,0x0000000000000000,0xcedf6504dde4ee5f,
0x8408b482fc53d937,0xe5852ab148ad9eb1,0x0000000000000000,0x0000000000000000,
0x0000000000000000,0x0000000000000000,0x0000000000000000,0xcd6340d16639a18b,
0xcd6340d1662d6e7f,0xcee7dfce15a7a951,0xa2a5dfeb017136ed,0x8408b482ca53786f,
0x0000000000000000,0xe5852ab148e5344f,0xa2a5dfeb01710e1d,
};
u8 main_keys_key_array_1[] = {0x6e,0x61,0x6d,0x65,0x73,0x70,0x61,0x63,0x65,};
u8 main_keys_key_array_2[] = {0x65,0x78,0x70,0x6f,0x72,0x74,};
u8 main_keys_key_array_6[] = {0x74,0x72,0x79,};
u8 main_keys_key_array_7[] = {0x74,0x72,0x75,0x65,};
u8 main_keys_key_array_8[] = {0x66,0x61,0x6c,0x73,0x65,};
u8 main_keys_key_array_10[] = {0x72,0x65,0x67,0x69,0x73,0x74,0x65,0x72,};
u8 main_keys_key_array_14[] = {0x69,0x66,};
u8 main_keys_key_array_16[] = {0x73,0x68,0x6f,0x72,0x74,};
u8 main_keys_key_array_18[] = {0x70,0x75,0x62,0x6c,0x69,0x63,};
u8 main_keys_key_array_26[] = {0x65,0x78,0x70,0x6c,0x69,0x63,0x69,0x74,};
u8 main_keys_key_array_29[] = {0x61,0x6c,0x69,0x67,0x6e,0x61,0x73,};
u8 main_keys_key_array_32[] = {0x64,0x6f,0x75,0x62,0x6c,0x65,};
u8 main_keys_key_array_33[] = {0x73,0x69,0x67,0x6e,0x65,0x64,};
u8 main_keys_key_array_35[] = {0x74,0x68,0x69,0x73,};
u8 main_keys_key_array_38[] = {0x63,0x61,0x74,0x63,0x68,};
u8 main_keys_key_array_39[] = {0x73,0x74,0x61,0x74,0x69,0x63,0x5f,0x63,0x61,0x73,0x74,};
u8 main_keys_key_array_40[] = {0x63,0x6f,0x6e,0x74,0x69,0x6e,0x75,0x65,};
u8 main_keys_key_array_41[] = {0x6f,0x70,0x65,0x72,0x61,0x74,0x6f,0x72,};
u8 main_keys_key_array_43[] = {0x61,0x6c,0x69,0x67,0x6e,0x6f,0x66,};
u8 main_keys_key_array_45[] = {0x66,0x72,0x69,0x65,0x6e,0x64,};
u8 main_keys_key_array_46[] = {0x64,0x65,0x63,0x6c,0x74,0x79,0x70,0x65,};
u8 main_keys_key_array_47[] = {0x73,0x74,0x61,0x74,0x69,0x63,0x5f,0x61,0x73,0x73,0x65,0x72,0x74,};
u8 main_keys_key_array_48[] = {0x62,0x72,0x65,0x61,0x6b,};
u8 main_keys_key_array_51[] = {0x74,0x65,0x6d,0x70,0x6c,0x61,0x74,0x65,};
u8 main_keys_key_array_52[] = {0x65,0x78,0x74,0x65,0x72,0x6e,};
u8 main_keys_key_array_53[] = {0x64,0x79,0x6e,0x61,0x6d,0x69,0x63,0x5f,0x63,0x61,0x73,0x74,};
u8 main_keys_key_array_54[] = {0x74,0x79,0x70,0x65,0x69,0x64,};
u8 main_keys_key_array_56[] = {0x74,0x79,0x70,0x65,0x64,0x65,0x66,};
u8 main_keys_key_array_57[] = {0x69,0x6e,0x6c,0x69,0x6e,0x65,};
u8 main_keys_key_array_58[] = {0x74,0x68,0x72,0x65,0x61,0x64,0x5f,0x6c,0x6f,0x63,0x61,0x6c,};
u8 main_keys_key_array_60[] = {0x77,0x68,0x69,0x6c,0x65,};
u8 main_keys_key_array_61[] = {0x65,0x6c,0x73,0x65,};
u8 main_keys_key_array_64[] = {0x61,0x73,0x6d,};
u8 main_keys_key_array_68[] = {0x63,0x68,0x61,0x72,};
u8 main_keys_key_array_71[] = {0x76,0x6f,0x6c,0x61,0x74,0x69,0x6c,0x65,};
u8 main_keys_key_array_73[] = {0x73,0x77,0x69,0x74,0x63,0x68,};
u8 main_keys_key_array_77[] = {0x72,0x65,0x69,0x6e,0x74,0x65,0x72,0x70,0x72,0x65,0x74,0x5f,0x63,0x61,0x73,0x74,};
u8 main_keys_key_array_78[] = {0x64,0x65,0x6c,0x65,0x74,0x65,};
u8 main_keys_key_array_80[] = {0x70,0x72,0x6f,0x74,0x65,0x63,0x74,0x65,0x64,};
u8 main_keys_key_array_82[] = {0x63,0x6f,0x6e,0x73,0x74,};
u8 main_keys_key_array_83[] = {0x70,0x72,0x69,0x76,0x61,0x74,0x65,};
u8 main_keys_key_array_84[] = {0x73,0x69,0x7a,0x65,0x6f,0x66,};
u8 main_keys_key_array_85[] = {0x63,0x6f,0x6e,0x73,0x74,0x5f,0x63,0x61,0x73,0x74,};
u8 main_keys_key_array_86[] = {0x64,0x6f,};
u8 main_keys_key_array_89[] = {0x76,0x69,0x72,0x74,0x75,0x61,0x6c,};
u8 main_keys_key_array_90[] = {0x63,0x61,0x73,0x65,};
u8 main_keys_key_array_92[] = {0x72,0x65,0x74,0x75,0x72,0x6e,};
u8 main_keys_key_array_93[] = {0x75,0x6e,0x69,0x6f,0x6e,};
u8 main_keys_key_array_94[] = {0x6c,0x6f,0x6e,0x67,};
u8 main_keys_key_array_98[] = {0x63,0x6c,0x61,0x73,0x73,};
u8 main_keys_key_array_99[] = {0x6e,0x75,0x6c,0x6c,0x70,0x74,0x72,};
u8 main_keys_key_array_100[] = {0x75,0x6e,0x73,0x69,0x67,0x6e,0x65,0x64,};
u8 main_keys_key_array_101[] = {0x64,0x65,0x66,0x61,0x75,0x6c,0x74,};
u8 main_keys_key_array_102[] = {0x66,0x6f,0x72,};
u8 main_keys_key_array_104[] = {0x62,0x6f,0x6f,0x6c,};
u8 main_keys_key_array_105[] = {0x76,0x6f,0x69,0x64,};
u8 main_keys_key_array_107[] = {0x74,0x79,0x70,0x65,0x6e,0x61,0x6d,0x65,};
u8 main_keys_key_array_108[] = {0x66,0x6c,0x6f,0x61,0x74,};
u8 main_keys_key_array_109[] = {0x73,0x74,0x61,0x74,0x69,0x63,};
u8 main_keys_key_array_115[] = {0x67,0x6f,0x74,0x6f,};
u8 main_keys_key_array_116[] = {0x65,0x6e,0x75,0x6d,};
u8 main_keys_key_array_117[] = {0x6e,0x6f,0x65,0x78,0x63,0x65,0x70,0x74,};
u8 main_keys_key_array_118[] = {0x6e,0x65,0x77,};
u8 main_keys_key_array_119[] = {0x75,0x73,0x69,0x6e,0x67,};
u8 main_keys_key_array_121[] = {0x73,0x74,0x72,0x75,0x63,0x74,};
u8 main_keys_key_array_122[] = {0x69,0x6e,0x74,};
String_Const_u8 main_keys_key_array[123] = {
{0, 0},
{main_keys_key_array_1, 9},
{main_keys_key_array_2, 6},
{0, 0},
{0, 0},
{0, 0},
{main_keys_key_array_6, 3},
{main_keys_key_array_7, 4},
{main_keys_key_array_8, 5},
{0, 0},
{main_keys_key_array_10, 8},
{0, 0},
{0, 0},
{0, 0},
{main_keys_key_array_14, 2},
{0, 0},
{main_keys_key_array_16, 5},
{0, 0},
{main_keys_key_array_18, 6},
{0, 0},
{0, 0},
{0, 0},
{0, 0},
{0, 0},
{0, 0},
{0, 0},
{main_keys_key_array_26, 8},
{0, 0},
{0, 0},
{main_keys_key_array_29, 7},
{0, 0},
{0, 0},
{main_keys_key_array_32, 6},
{main_keys_key_array_33, 6},
{0, 0},
{main_keys_key_array_35, 4},
{0, 0},
{0, 0},
{main_keys_key_array_38, 5},
{main_keys_key_array_39, 11},
{main_keys_key_array_40, 8},
{main_keys_key_array_41, 8},
{0, 0},
{main_keys_key_array_43, 7},
{0, 0},
{main_keys_key_array_45, 6},
{main_keys_key_array_46, 8},
{main_keys_key_array_47, 13},
{main_keys_key_array_48, 5},
{0, 0},
{0, 0},
{main_keys_key_array_51, 8},
{main_keys_key_array_52, 6},
{main_keys_key_array_53, 12},
{main_keys_key_array_54, 6},
{0, 0},
{main_keys_key_array_56, 7},
{main_keys_key_array_57, 6},
{main_keys_key_array_58, 12},
{0, 0},
{main_keys_key_array_60, 5},
{main_keys_key_array_61, 4},
{0, 0},
{0, 0},
{main_keys_key_array_64, 3},
{0, 0},
{0, 0},
{0, 0},
{main_keys_key_array_68, 4},
{0, 0},
{0, 0},
{main_keys_key_array_71, 8},
{0, 0},
{main_keys_key_array_73, 6},
{0, 0},
{0, 0},
{0, 0},
{main_keys_key_array_77, 16},
{main_keys_key_array_78, 6},
{0, 0},
{main_keys_key_array_80, 9},
{0, 0},
{main_keys_key_array_82, 5},
{main_keys_key_array_83, 7},
{main_keys_key_array_84, 6},
{main_keys_key_array_85, 10},
{main_keys_key_array_86, 2},
{0, 0},
{0, 0},
{main_keys_key_array_89, 7},
{main_keys_key_array_90, 4},
{0, 0},
{main_keys_key_array_92, 6},
{main_keys_key_array_93, 5},
{main_keys_key_array_94, 4},
{0, 0},
{0, 0},
{0, 0},
{main_keys_key_array_98, 5},
{main_keys_key_array_99, 7},
{main_keys_key_array_100, 8},
{main_keys_key_array_101, 7},
{main_keys_key_array_102, 3},
{0, 0},
{main_keys_key_array_104, 4},
{main_keys_key_array_105, 4},
{0, 0},
{main_keys_key_array_107, 8},
{main_keys_key_array_108, 5},
{main_keys_key_array_109, 6},
{0, 0},
{0, 0},
{0, 0},
{0, 0},
{0, 0},
{main_keys_key_array_115, 4},
{main_keys_key_array_116, 4},
{main_keys_key_array_117, 8},
{main_keys_key_array_118, 3},
{main_keys_key_array_119, 5},
{0, 0},
{main_keys_key_array_121, 6},
{main_keys_key_array_122, 3},
};
Lexeme_Table_Value main_keys_value_array[123] = {
{0, 0},
{4, TokenCppKind_Namespace},
{4, TokenCppKind_Export},
{0, 0},
{0, 0},
{0, 0},
{4, TokenCppKind_Try},
{8, TokenCppKind_LiteralTrue},
{8, TokenCppKind_LiteralFalse},
{0, 0},
{4, TokenCppKind_Register},
{0, 0},
{0, 0},
{0, 0},
{4, TokenCppKind_If},
{0, 0},
{4, TokenCppKind_Short},
{0, 0},
{4, TokenCppKind_Public},
{0, 0},
{0, 0},
{0, 0},
{0, 0},
{0, 0},
{0, 0},
{0, 0},
{4, TokenCppKind_Explicit},
{0, 0},
{0, 0},
{4, TokenCppKind_AlignAs},
{0, 0},
{0, 0},
{4, TokenCppKind_Double},
{4, TokenCppKind_Signed},
{0, 0},
{4, TokenCppKind_This},
{0, 0},
{0, 0},
{4, TokenCppKind_Catch},
{4, TokenCppKind_StaticCast},
{4, TokenCppKind_Continue},
{4, TokenCppKind_Operator},
{0, 0},
{4, TokenCppKind_AlignOf},
{0, 0},
{4, TokenCppKind_Friend},
{4, TokenCppKind_DeclType},
{4, TokenCppKind_StaticAssert},
{4, TokenCppKind_Break},
{0, 0},
{0, 0},
{4, TokenCppKind_Template},
{4, TokenCppKind_Extern},
{4, TokenCppKind_DynamicCast},
{4, TokenCppKind_TypeID},
{0, 0},
{4, TokenCppKind_Typedef},
{4, TokenCppKind_Inline},
{4, TokenCppKind_ThreadLocal},
{0, 0},
{4, TokenCppKind_While},
{4, TokenCppKind_Else},
{0, 0},
{0, 0},
{4, TokenCppKind_Asm},
{0, 0},
{0, 0},
{0, 0},
{4, TokenCppKind_Char},
{0, 0},
{0, 0},
{4, TokenCppKind_Volatile},
{0, 0},
{4, TokenCppKind_Switch},
{0, 0},
{0, 0},
{0, 0},
{4, TokenCppKind_ReinterpretCast},
{4, TokenCppKind_Delete},
{0, 0},
{4, TokenCppKind_Protected},
{0, 0},
{4, TokenCppKind_Const},
{4, TokenCppKind_Private},
{4, TokenCppKind_SizeOf},
{4, TokenCppKind_ConstCast},
{4, TokenCppKind_Do},
{0, 0},
{0, 0},
{4, TokenCppKind_Virtual},
{4, TokenCppKind_Case},
{0, 0},
{4, TokenCppKind_Return},
{4, TokenCppKind_Union},
{4, TokenCppKind_Long},
{0, 0},
{0, 0},
{0, 0},
{4, TokenCppKind_Class},
{4, TokenCppKind_NullPtr},
{4, TokenCppKind_Unsigned},
{4, TokenCppKind_Default},
{4, TokenCppKind_For},
{0, 0},
{4, TokenCppKind_Bool},
{4, TokenCppKind_Void},
{0, 0},
{4, TokenCppKind_Typename},
{4, TokenCppKind_Float},
{4, TokenCppKind_Static},
{0, 0},
{0, 0},
{0, 0},
{0, 0},
{0, 0},
{4, TokenCppKind_Goto},
{4, TokenCppKind_Enum},
{4, TokenCppKind_NoExcept},
{4, TokenCppKind_New},
{4, TokenCppKind_Using},
{0, 0},
{4, TokenCppKind_Struct},
{4, TokenCppKind_Int},
};
i32 main_keys_slot_count = 123;
u64 main_keys_seed = 0xf38c04110edf5c20;
u64 pp_directives_hash_array[25] = {
0x6b2525b7e6f87877,0x0000000000000000,0xb6682cd11ed9fd9f,0x6b2525b40373aacb,
0xb6682cd11ea5bda7,0x0000000000000000,0x0000000000000000,0x0d56a51c5ba8244b,
0x0000000000000000,0x6b2525b7e6f864c5,0xb6682cd10cf54325,0x0dbf8cd197510b47,
0x83d780e616d1416b,0x0000000000000000,0x83d7ee6ef708249b,0x83d780e1399294e1,
0x0000000000000000,0xb6682cd10c14755f,0x6c3839b38ad2ee53,0x0000000000000000,
0x83d7ee6e1191055f,0xb6682d358a49de5f,0x0000000000000000,0x0000000000000000,
0x0000000000000000,
};
u8 pp_directives_key_array_0[] = {0x23,0x65,0x6c,0x69,0x66,};
u8 pp_directives_key_array_2[] = {0x23,0x65,0x72,0x72,0x6f,0x72,};
u8 pp_directives_key_array_3[] = {0x23,0x6c,0x69,0x6e,0x65,};
u8 pp_directives_key_array_4[] = {0x23,0x65,0x6e,0x64,0x69,0x66,};
u8 pp_directives_key_array_7[] = {0x23,0x69,0x6e,0x63,0x6c,0x75,0x64,0x65,};
u8 pp_directives_key_array_9[] = {0x23,0x65,0x6c,0x73,0x65,};
u8 pp_directives_key_array_10[] = {0x23,0x75,0x73,0x69,0x6e,0x67,};
u8 pp_directives_key_array_11[] = {0x23,0x76,0x65,0x72,0x73,0x69,0x6f,0x6e,};
u8 pp_directives_key_array_12[] = {0x23,0x64,0x65,0x66,0x69,0x6e,0x65,};
u8 pp_directives_key_array_14[] = {0x23,0x69,0x6d,0x70,0x6f,0x72,0x74,};
u8 pp_directives_key_array_15[] = {0x23,0x70,0x72,0x61,0x67,0x6d,0x61,};
u8 pp_directives_key_array_17[] = {0x23,0x75,0x6e,0x64,0x65,0x66,};
u8 pp_directives_key_array_18[] = {0x23,0x69,0x66,};
u8 pp_directives_key_array_20[] = {0x23,0x69,0x66,0x6e,0x64,0x65,0x66,};
u8 pp_directives_key_array_21[] = {0x23,0x69,0x66,0x64,0x65,0x66,};
String_Const_u8 pp_directives_key_array[25] = {
{pp_directives_key_array_0, 5},
{0, 0},
{pp_directives_key_array_2, 6},
{pp_directives_key_array_3, 5},
{pp_directives_key_array_4, 6},
{0, 0},
{0, 0},
{pp_directives_key_array_7, 8},
{0, 0},
{pp_directives_key_array_9, 5},
{pp_directives_key_array_10, 6},
{pp_directives_key_array_11, 8},
{pp_directives_key_array_12, 7},
{0, 0},
{pp_directives_key_array_14, 7},
{pp_directives_key_array_15, 7},
{0, 0},
{pp_directives_key_array_17, 6},
{pp_directives_key_array_18, 3},
{0, 0},
{pp_directives_key_array_20, 7},
{pp_directives_key_array_21, 6},
{0, 0},
{0, 0},
{0, 0},
};
Lexeme_Table_Value pp_directives_value_array[25] = {
{5, TokenCppKind_PPElIf},
{0, 0},
{5, TokenCppKind_PPError},
{5, TokenCppKind_PPLine},
{5, TokenCppKind_PPEndIf},
{0, 0},
{0, 0},
{5, TokenCppKind_PPInclude},
{0, 0},
{5, TokenCppKind_PPElse},
{5, TokenCppKind_PPUsing},
{5, TokenCppKind_PPVersion},
{5, TokenCppKind_PPDefine},
{0, 0},
{5, TokenCppKind_PPImport},
{5, TokenCppKind_PPPragma},
{0, 0},
{5, TokenCppKind_PPUndef},
{5, TokenCppKind_PPIf},
{0, 0},
{5, TokenCppKind_PPIfNDef},
{5, TokenCppKind_PPIfDef},
{0, 0},
{0, 0},
{0, 0},
};
i32 pp_directives_slot_count = 25;
u64 pp_directives_seed = 0x2bbd5804bccb85e6;
u64 pp_keys_hash_array[2] = {
0x0000000000000000,0x89659c00a323190b,
};
u8 pp_keys_key_array_1[] = {0x64,0x65,0x66,0x69,0x6e,0x65,0x64,};
String_Const_u8 pp_keys_key_array[2] = {
{0, 0},
{pp_keys_key_array_1, 7},
};
Lexeme_Table_Value pp_keys_value_array[2] = {
{0, 0},
{4, TokenCppKind_PPDefined},
};
i32 pp_keys_slot_count = 2;
u64 pp_keys_seed = 0xc87165e79dc2fa67;
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
goto state_label_59; // op stage
}break;
case 0x22:
{
if ((HasFlag(flags_KF0, 0x1))){
ptr += 1;
goto state_label_25; // include_quotes
}
ptr += 1;
goto state_label_31; // string
}break;
case 0x23:
{
if ((!HasFlag(flags_KB0, 0x1))){
ptr += 1;
flags_KB0 |= 0x1;
goto state_label_23; // pp_directive
}
ptr += 1;
goto state_label_66; // op stage
}break;
default:
{
ptr += 1;
goto state_label_2; // identifier
}break;
case 0x25:
{
ptr += 1;
goto state_label_63; // op stage
}break;
case 0x26:
{
ptr += 1;
goto state_label_60; // op stage
}break;
case 0x27:
{
ptr += 1;
flags_ZF0 |= 0x40;
goto state_label_31; // string
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
goto state_label_62; // op stage
}break;
case 0x2b:
{
ptr += 1;
goto state_label_52; // op stage
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
goto state_label_53; // op stage
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
goto state_label_51; // op stage
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
goto state_label_55; // op stage
}
ptr += 1;
goto state_label_24; // include_pointy
}break;
case 0x3d:
{
ptr += 1;
goto state_label_58; // op stage
}break;
case 0x3e:
{
ptr += 1;
goto state_label_56; // op stage
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
goto state_label_26; // pre_L
}break;
case 0x52:
{
ptr += 1;
goto state_label_30; // pre_R
}break;
case 0x55:
{
ptr += 1;
flags_ZF0 |= 0x20;
goto state_label_28; // pre_U
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
goto state_label_27; // pre_u
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
goto state_label_61; // op stage
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
goto state_label_67; // op stage
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
goto state_label_48; // comment_block
}break;
case 0x2f:
{
ptr += 1;
goto state_label_50; // comment_line
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
state_label_23: // pp_directive
if (ptr == opl_ptr){
if ((true)){
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
Lexeme_Table_Lookup lookup = lexeme_table_lookup(pp_directives_hash_array, pp_directives_key_array, pp_directives_value_array, pp_directives_slot_count, pp_directives_seed, emit_ptr, token.size);
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
{
Token token = {};
token.pos = (i64)(emit_ptr - input.str);
token.size = (i64)(ptr - emit_ptr);
token.flags = flags_KB0;
do{
Lexeme_Table_Lookup lookup = lexeme_table_lookup(pp_directives_hash_array, pp_directives_key_array, pp_directives_value_array, pp_directives_slot_count, pp_directives_seed, emit_ptr, token.size);
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
flags_KB0 |= 0x1;
goto state_label_23; // pp_directive
}break;
}
}
{
state_label_24: // include_pointy
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
goto state_label_24; // include_pointy
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
state_label_25: // include_quotes
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
goto state_label_25; // include_quotes
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
state_label_26: // pre_L
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
goto state_label_31; // string
}break;
case 0x52:
{
ptr += 1;
goto state_label_30; // pre_R
}break;
}
}
{
state_label_27: // pre_u
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
goto state_label_31; // string
}break;
case 0x38:
{
ptr += 1;
flags_ZF0 |= 0x8;
goto state_label_29; // pre_u8
}break;
case 0x52:
{
ptr += 1;
goto state_label_30; // pre_R
}break;
}
}
{
state_label_28: // pre_U
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
goto state_label_31; // string
}break;
case 0x52:
{
ptr += 1;
goto state_label_30; // pre_R
}break;
}
}
{
state_label_29: // pre_u8
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
goto state_label_31; // string
}break;
case 0x52:
{
ptr += 1;
goto state_label_30; // pre_R
}break;
}
}
{
state_label_30: // pre_R
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
goto state_label_44; // raw_string_get_delim
}break;
}
}
{
state_label_31: // string
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
goto state_label_31; // string
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
goto state_label_31; // string
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
goto state_label_31; // string
}break;
case 0x5c:
{
ptr += 1;
goto state_label_32; // string_esc
}break;
}
}
{
state_label_32: // string_esc
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
goto state_label_31; // string
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:
{
ptr += 1;
goto state_label_33; // string_esc_oct2
}break;
case 0x55:
{
ptr += 1;
goto state_label_36; // string_esc_universal_8
}break;
case 0x75:
{
ptr += 1;
goto state_label_40; // string_esc_universal_4
}break;
case 0x78:
{
ptr += 1;
goto state_label_35; // string_esc_hex
}break;
}
}
{
state_label_33: // string_esc_oct2
if (ptr == opl_ptr){
if ((true)){
goto state_label_31; // string
}
}
switch (*ptr){
default:
{
goto state_label_31; // string
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:
{
ptr += 1;
goto state_label_34; // string_esc_oct1
}break;
}
}
{
state_label_34: // string_esc_oct1
if (ptr == opl_ptr){
if ((true)){
goto state_label_31; // string
}
}
switch (*ptr){
default:
{
goto state_label_31; // string
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:
{
ptr += 1;
goto state_label_31; // string
}break;
}
}
{
state_label_35: // string_esc_hex
if (ptr == opl_ptr){
if ((true)){
goto state_label_31; // string
}
}
switch (*ptr){
default:
{
goto state_label_31; // string
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
case 0x66:
{
ptr += 1;
goto state_label_35; // string_esc_hex
}break;
}
}
{
state_label_36: // string_esc_universal_8
if (ptr == opl_ptr){
if ((true)){
goto state_label_31; // string
}
}
switch (*ptr){
default:
{
goto state_label_31; // string
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
case 0x66:
{
ptr += 1;
goto state_label_37; // string_esc_universal_7
}break;
}
}
{
state_label_37: // string_esc_universal_7
if (ptr == opl_ptr){
if ((true)){
goto state_label_31; // string
}
}
switch (*ptr){
default:
{
goto state_label_31; // string
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
case 0x66:
{
ptr += 1;
goto state_label_38; // string_esc_universal_6
}break;
}
}
{
state_label_38: // string_esc_universal_6
if (ptr == opl_ptr){
if ((true)){
goto state_label_31; // string
}
}
switch (*ptr){
default:
{
goto state_label_31; // string
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
case 0x66:
{
ptr += 1;
goto state_label_39; // string_esc_universal_5
}break;
}
}
{
state_label_39: // string_esc_universal_5
if (ptr == opl_ptr){
if ((true)){
goto state_label_31; // string
}
}
switch (*ptr){
default:
{
goto state_label_31; // string
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
case 0x66:
{
ptr += 1;
goto state_label_40; // string_esc_universal_4
}break;
}
}
{
state_label_40: // string_esc_universal_4
if (ptr == opl_ptr){
if ((true)){
goto state_label_31; // string
}
}
switch (*ptr){
default:
{
goto state_label_31; // string
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
case 0x66:
{
ptr += 1;
goto state_label_41; // string_esc_universal_3
}break;
}
}
{
state_label_41: // string_esc_universal_3
if (ptr == opl_ptr){
if ((true)){
goto state_label_31; // string
}
}
switch (*ptr){
default:
{
goto state_label_31; // string
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
case 0x66:
{
ptr += 1;
goto state_label_42; // string_esc_universal_2
}break;
}
}
{
state_label_42: // string_esc_universal_2
if (ptr == opl_ptr){
if ((true)){
goto state_label_31; // string
}
}
switch (*ptr){
default:
{
goto state_label_31; // string
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
case 0x66:
{
ptr += 1;
goto state_label_43; // string_esc_universal_1
}break;
}
}
{
state_label_43: // string_esc_universal_1
if (ptr == opl_ptr){
if ((true)){
goto state_label_31; // string
}
}
switch (*ptr){
default:
{
goto state_label_31; // string
}break;
case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
case 0x66:
{
ptr += 1;
goto state_label_31; // string
}break;
}
}
{
state_label_44: // raw_string_get_delim
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
goto state_label_44; // raw_string_get_delim
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
goto state_label_45; // raw_string_find_close
}break;
}
}
{
state_label_45: // raw_string_find_close
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
goto state_label_45; // raw_string_find_close
}break;
case 0x29:
{
ptr += 1;
goto state_label_46; // raw_string_try_delim
}break;
}
}
{
state_label_46: // raw_string_try_delim
umem delim_length = delim_one_past_last - delim_first;
umem parse_length = 0;
for (;;){
if (parse_length == delim_length){
goto state_label_47; // raw_string_try_quote
}
if (ptr == opl_ptr){
goto state_label_47; // raw_string_try_quote
}
if (*ptr == delim_first[parse_length]){
ptr += 1;
parse_length += 1;
}
else{
goto state_label_45; // raw_string_find_close
}
}
}
{
state_label_47: // raw_string_try_quote
if (ptr == opl_ptr){
if ((true)){
goto state_label_45; // raw_string_find_close
}
}
switch (*ptr){
default:
{
goto state_label_45; // raw_string_find_close
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
state_label_48: // comment_block
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
goto state_label_48; // comment_block
}break;
case 0x0a:
{
ptr += 1;
flags_KB0 &= ~(0x1);
flags_KF0 &= ~(0x1);
goto state_label_48; // comment_block
}break;
case 0x2a:
{
ptr += 1;
goto state_label_49; // comment_block_try_close
}break;
}
}
{
state_label_49: // comment_block_try_close
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
goto state_label_48; // comment_block
}break;
case 0x2a:
{
ptr += 1;
goto state_label_49; // comment_block_try_close
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
state_label_50: // comment_line
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
goto state_label_50; // comment_line
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
state_label_51: // op stage
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
state_label_52: // op stage
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
state_label_53: // op stage
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
goto state_label_54; // op stage
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
state_label_55: // op stage
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
goto state_label_64; // op stage
}break;
case 0x3d:
{
ptr += 1;
goto state_label_57; // op stage
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
goto state_label_65; // op stage
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
state_label_58: // op stage
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
state_label_59: // op stage
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
state_label_60: // op stage
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
state_label_61: // op stage
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
state_label_62: // op stage
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
state_label_63: // op stage
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
state_label_64: // op stage
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
state_label_65: // op stage
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
state_label_66: // op stage
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
state_label_67: // op stage
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
