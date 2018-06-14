/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 30.05.2018
 *
 * Generate config parser procedures.
 *
 */

// TOP

#include <stdio.h>

#include "../4ed_defines.h"
#include "4ed_meta_generate_parser.h"

#define ConfigOpClassList(M) \
M(ConfigVar              , 1, Operations) \
M(CompoundMember         , 1, Operations) \
M(TypedArrayStep         , 0, Operations) \
M(TypedArrayCount        , 0, Types     ) \
M(TypedArrayRefernceList , 0, Types     ) \

#define ConfigRValueTypeList(M) \
M(LValue   , lvalue   ) \
M(Boolean  , bool     ) \
M(Integer  , int      ) \
M(Float    , float    ) \
M(String   , string   ) \
M(Character, character) \
M(Compound , compound ) \
M(NoType   , no_type  ) \

enum{
#define ENUM(N,C,I) OpClass_##N,
    ConfigOpClassList(ENUM)
#undef ENUM
        OpClass_COUNT,
};

Op_Class op_class_array[] = {
#define MEMBER(N,C,I) {OpClassIterate_##I},
    ConfigOpClassList(MEMBER)
#undef MEMBER
};

enum{
#define ENUM(N,P) Type_##N,
    ConfigRValueTypeList(ENUM)
#undef ENUM
        Type_COUNT,
};

char *type_names[Type_COUNT] = {
#define MEMBER(N,P) #N,
    ConfigRValueTypeList(MEMBER)
#undef MEMBER
};

char *type_proc_names[Type_COUNT] = {
#define MEMBER(N,P) #P,
    ConfigRValueTypeList(MEMBER)
#undef MEMBER
};

Operation operations[] = {
    {Type_NoType , "has"   , 0         , 0         , 0, 0, 0, 0},
    {Type_Boolean, "bool"  , "boolean" , "bool32"  , 0, 0, 0, 0},
    {Type_Integer, "int"   , "integer" , "int32_t" , 0, 0, 0, 0},
    {Type_Integer, "uint"  , "uinteger", "uint32_t", 0, 0, 0, 0},
    {Type_String , "string", "string"  , "String"  , 0, 0, 0, 0},
    {Type_String, "placed_string", "string", "String",
        ", char *space, int32_t space_size",
        ", space, space_size",
        0,
        ""
            "if (success){\n"
            "String str = *var_out;\n"
            "*var_out = make_string_cap(space, 0, space_size);\n"
            "copy(var_out, str);\n"
            "}\n",
    },
    {Type_Character, "char"    , "character", "char"            , 0, 0, 0, 0},
    {Type_Compound , "compound", "compound" , "Config_Compound*", 0, 0, 0, 0},
};

void
print_config_var(FILE *out, Operation *op){
    fprintf(out, "static bool32\n");
    fprintf(out, "config_%s_var(", op->proc_name);
    fprintf(out, "Config *config, String var_name, int32_t subscript");
    if (op->output_type != 0){
        fprintf(out, ", %s* var_out", op->output_type);
    }
    if (op->extra_params != 0){
        fprintf(out, "%s", op->extra_params);
    }
    fprintf(out, "){\n");
    if (op->code_before != 0){
        fprintf(out, "%s", op->code_before);
    }
    fprintf(out, "Config_Get_Result result = config_var(config, var_name, subscript);\n");
    fprintf(out, "bool32 success = result.success && result.type == ConfigRValueType_%s;\n", type_names[op->r_type]);
    if (op->output_type != 0){
        fprintf(out, "if (success){\n");
        fprintf(out, "*var_out = result.%s;\n", op->result_type);
        fprintf(out, "}\n");
    }
    if (op->code_after != 0){
        fprintf(out, "%s", op->code_after);
    }
    fprintf(out, "return(success);\n");
    fprintf(out, "}\n\n");
    
    fprintf(out, "static bool32\n");
    fprintf(out, "config_%s_var(", op->proc_name);
    fprintf(out, "Config *config, char *var_name, int32_t subscript");
    if (op->output_type != 0){
        fprintf(out, ", %s* var_out", op->output_type);
    }
    if (op->extra_params != 0){
        fprintf(out, "%s", op->extra_params);
    }
    fprintf(out, "){\n");
    if (op->code_before != 0){
        fprintf(out, "%s", op->code_before);
    }
    fprintf(out, "String var_name_str = make_string_slowly(var_name);\n");
    fprintf(out, "Config_Get_Result result = config_var(config, var_name_str, subscript);\n");
    fprintf(out, "bool32 success = result.success && result.type == ConfigRValueType_%s;\n", type_names[op->r_type]);
    if (op->output_type != 0){
        fprintf(out, "if (success){\n");
        fprintf(out, "*var_out = result.%s;\n", op->result_type);
        fprintf(out, "}\n");
    }
    if (op->code_after != 0){
        fprintf(out, "%s", op->code_after);
    }
    fprintf(out, "return(success);\n");
    fprintf(out, "}\n\n");
}

void
print_compound_member(FILE *out, Operation *op){
    fprintf(out, "static bool32\n");
    fprintf(out, "config_compound_%s_member(", op->proc_name);
    fprintf(out, "Config *config, Config_Compound *compound,\n");
    fprintf(out, "String var_name, int32_t index");
    if (op->output_type != 0){
        fprintf(out, ", %s* var_out", op->output_type);
    }
    if (op->extra_params != 0){
        fprintf(out, "%s", op->extra_params);
    }
    fprintf(out, "){\n");
    if (op->code_before != 0){
        fprintf(out, "%s", op->code_before);
    }
    fprintf(out, "Config_Get_Result result = config_compound_member(config, compound, var_name, index);\n");
    fprintf(out, "bool32 success = result.success && result.type == ConfigRValueType_%s;\n", type_names[op->r_type]);
    if (op->output_type != 0){
        fprintf(out, "if (success){\n");
        fprintf(out, "*var_out = result.%s;\n", op->result_type);
        fprintf(out, "}\n");
    }
    if (op->code_after != 0){
        fprintf(out, "%s", op->code_after);
    }
    fprintf(out, "return(success);\n");
    fprintf(out, "}\n\n");
    
    fprintf(out, "static bool32\n");
    fprintf(out, "config_compound_%s_member(", op->proc_name);
    fprintf(out, "Config *config, Config_Compound *compound,\n");
    fprintf(out, "char *var_name, int32_t index");
    if (op->output_type != 0){
        fprintf(out, ", %s* var_out", op->output_type);
    }
    if (op->extra_params != 0){
        fprintf(out, "%s", op->extra_params);
    }
    fprintf(out, "){\n");
    fprintf(out, "String var_name_str = make_string_slowly(var_name);\n");
    if (op->code_before != 0){
        fprintf(out, "%s", op->code_before);
    }
    fprintf(out, "Config_Get_Result result = config_compound_member(config, compound, var_name_str, index);\n");
    fprintf(out, "bool32 success = result.success && result.type == ConfigRValueType_%s;\n", type_names[op->r_type]);
    if (op->output_type != 0){
        fprintf(out, "if (success){\n");
        fprintf(out, "*var_out = result.%s;\n", op->result_type);
        fprintf(out, "}\n");
    }
    if (op->code_after != 0){
        fprintf(out, "%s", op->code_after);
    }
    fprintf(out, "return(success);\n");
    fprintf(out, "}\n\n");
}

void
print_typed_array(FILE *out, Operation *op){
    fprintf(out, "static Iteration_Step_Result\n");
    fprintf(out, "typed_%s_array_iteration_step(", op->proc_name);
    fprintf(out, "Config *config, Config_Compound *compound, int32_t index");
    if (op->output_type != 0){
        fprintf(out, ", %s* var_out", op->output_type);
    }
    if (op->extra_params != 0){
        fprintf(out, "\n%s", op->extra_params);
    }
    fprintf(out, "){\n");
    if (op->code_before != 0){
        fprintf(out, "%s", op->code_before);
    }
    fprintf(out, "Config_Iteration_Step_Result result = typed_array_iteration_step(config, compound, ConfigRValueType_%s, index);\n", type_names[op->r_type]);
    if (op->output_type != 0 || op->code_after != 0){
        fprintf(out, "bool32 success = (result.step == Iteration_Good);\n");
    }
    if (op->output_type != 0){
        fprintf(out, "if (success){\n");
        fprintf(out, "*var_out = result.get.%s;\n", op->result_type);
        fprintf(out, "}\n");
    }
    if (op->code_after != 0){
        fprintf(out, "%s", op->code_after);
    }
    fprintf(out, "return(result.step);\n");
    fprintf(out, "}\n\n");
}

void
print_typed_array_count(FILE *out, int32_t r_type){
    if (r_type == Type_LValue){
        return;
    }
    fprintf(out, "static int32_t\n");
    fprintf(out, "typed_%s_array_get_count(", type_proc_names[r_type]);
    fprintf(out, "Config *config, Config_Compound *compound){\n");
    fprintf(out, "int32_t count = typed_array_get_count(config, compound, ConfigRValueType_%s);\n",
            type_names[r_type]);
    fprintf(out, "return(count);\n");
    fprintf(out, "}\n\n");
}

void
print_typed_array_reference_list(FILE *out, int32_t r_type){
    if (r_type == Type_LValue){
        return;
    }
    fprintf(out, "static Config_Get_Result_List\n");
    fprintf(out, "typed_%s_array_reference_list(", type_proc_names[r_type]);
    fprintf(out, "Partition *arena, Config *config, Config_Compound *compound){\n");
    fprintf(out, "Config_Get_Result_List list = "
            "typed_array_reference_list(arena, config, compound, ConfigRValueType_%s);\n",
            type_names[r_type]);
    fprintf(out, "return(list);\n");
    fprintf(out, "}\n\n");
}

void
print_config_queries(void){
    FILE *out = stdout;
    for (int32_t i = 0; i < OpClass_COUNT; ++i){
        Op_Class *op_class = &op_class_array[i];
        switch (op_class->iteration_type){
            case OpClassIterate_Operations:
            {
                Operation *op = operations;
                for (int32_t j = 0; j < ArrayCount(operations); ++j, ++op){
                    switch (i){
                        case OpClass_ConfigVar:
                        {
                            print_config_var(out, op);
                        }break;
                        case OpClass_CompoundMember:
                        {
                            print_compound_member(out, op);
                        }break;
                        case OpClass_TypedArrayStep:
                        {
                            print_typed_array(out, op);
                        }break;
                    }
                    fflush(out);
                }
            }break;
            
            case OpClassIterate_Types:
            {
                for (int32_t j = 0; j < Type_COUNT; ++j){
                    switch (i){
                        case OpClass_TypedArrayCount:
                        {
                            print_typed_array_count(out, j);
                        }break;
                        case OpClass_TypedArrayRefernceList:
                        {
                            print_typed_array_reference_list(out, j);
                        }break;
                    }
                    fflush(out);
                }
            }break;
        }
    }
}

int main(void){
    print_config_queries();
    return(0);
}

// BOTTOM


