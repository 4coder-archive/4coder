/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 02.10.2019
 *
 * System API definition program.
 *
 */

// TOP

#include "4coder_base_types.h"

#include "4coder_base_types.cpp"
#include "4coder_stringf.cpp"

#include "4coder_malloc_allocator.cpp"

////////////////////////////////

struct API_Param{
    API_Param *next;
    String_Const_u8 type_name;
    String_Const_u8 name;
};

struct API_Param_List{
    API_Param *first;
    API_Param *last;
    i32 count;
};

struct API_Call{
    API_Call *next;
    String_Const_u8 name;
    String_Const_u8 return_type;
    API_Param_List params;
};

struct API_Definition{
    API_Call *first;
    API_Call *last;
    i32 count;
    
    String_Const_u8 name;
};

////////////////////////////////

function API_Definition*
begin_api(Arena *arena, char *name){
    API_Definition *api = push_array_zero(arena, API_Definition, 1);
    api->name = SCu8(name);
    return(api);
}

function API_Call*
api_call(Arena *arena, API_Definition *api, char *name, char *return_type){
    API_Call *call = push_array_zero(arena, API_Call, 1);
    sll_queue_push(api->first, api->last, call);
    api->count += 1;
    call->name = SCu8(name);
    call->return_type = SCu8(return_type);
    return(call);
}

function API_Param*
api_param(Arena *arena, API_Call *call, char *type_name, char *name){
    API_Param *param = push_array_zero(arena, API_Param, 1);
    sll_queue_push(call->params.first, call->params.last, param);
    call->params.count += 1;
    param->type_name = SCu8(type_name);
    param->name = SCu8(name);
    return(param);
}

////////////////////////////////

function void
generate_header(Arena *scratch, API_Definition *api, FILE *out){
    for (API_Call *call = api->first;
         call != 0;
         call = call->next){
        fprintf(out, "#define %.*s_%.*s_sig() %.*s %.*s_%.*s(",
                string_expand(api->name),
                string_expand(call->name),
                string_expand(call->return_type),
                string_expand(api->name),
                string_expand(call->name));
        if (call->params.count == 0){
            fprintf(out, "void");
        }
        else{
            for (API_Param *param = call->params.first;
                 param != 0;
                 param = param->next){
                fprintf(out, "%.*s %.*s",
                        string_expand(param->type_name),
                        string_expand(param->name));
                if (param->next != 0){
                    fprintf(out, ", ");
                }
            }
        }
        fprintf(out, ")\n");
    }
    
    for (API_Call *call = api->first;
         call != 0;
         call = call->next){
        fprintf(out, "typedef %.*s %.*s_%.*s_type(",
                string_expand(call->return_type),
                string_expand(api->name),
                string_expand(call->name));
        if (call->params.count == 0){
            fprintf(out, "void");
        }
        else{
            for (API_Param *param = call->params.first;
                 param != 0;
                 param = param->next){
                fprintf(out, "%.*s %.*s",
                        string_expand(param->type_name),
                        string_expand(param->name));
                if (param->next != 0){
                    fprintf(out, ", ");
                }
            }
        }
        fprintf(out, ");\n");
    }
    
    fprintf(out, "struct API_VTable_%.*s{\n", string_expand(api->name));
    for (API_Call *call = api->first;
         call != 0;
         call = call->next){
        fprintf(out, "%.*s_%.*s_type *",
                string_expand(api->name),
                string_expand(call->name));
        fprintf(out, "%.*s",
                string_expand(call->name));
        fprintf(out, ";\n");
    }
    fprintf(out, "};\n");
    
    fprintf(out, "#if defined(STATIC_LINK_API)\n");
    for (API_Call *call = api->first;
         call != 0;
         call = call->next){
        fprintf(out, "internal %.*s %.*s_%.*s(",
                string_expand(call->return_type),
                string_expand(api->name),
                string_expand(call->name));
        if (call->params.count == 0){
            fprintf(out, "void");
        }
        else{
            for (API_Param *param = call->params.first;
                 param != 0;
                 param = param->next){
                fprintf(out, "%.*s %.*s",
                        string_expand(param->type_name),
                        string_expand(param->name));
                if (param->next != 0){
                    fprintf(out, ", ");
                }
            }
        }
        fprintf(out, ");\n");
    }
    fprintf(out, "#undef STATIC_LINK_API\n");
    fprintf(out, "#elif defined(DYNAMIC_LINK_API)\n");
    for (API_Call *call = api->first;
         call != 0;
         call = call->next){
        fprintf(out, "global %.*s_%.*s_type *%.*s_%.*s = 0;\n",
                string_expand(api->name),
                string_expand(call->name),
                string_expand(api->name),
                string_expand(call->name));
    }
    fprintf(out, "#undef DYNAMIC_LINK_API\n");
    fprintf(out, "#endif\n");
}

function void
generate_cpp(Arena *scratch, API_Definition *api, FILE *out){
    fprintf(out, "function void\n");
    fprintf(out, "%.*s_api_fill_vtable(API_VTable_%.*s *vtable){\n",
            string_expand(api->name),
            string_expand(api->name));
    for (API_Call *call = api->first;
         call != 0;
         call = call->next){
        fprintf(out, "vtable->%.*s = %.*s_%.*s;\n",
                string_expand(call->name),
                string_expand(api->name),
                string_expand(call->name));
    }
    fprintf(out, "}\n");
    
    fprintf(out, "#if defined(DYNAMIC_LINK_API)\n");
    fprintf(out, "function void\n");
    fprintf(out, "%.*s_api_read_vtable(API_VTable_%.*s *vtable){\n",
            string_expand(api->name),
            string_expand(api->name));
    for (API_Call *call = api->first;
         call != 0;
         call = call->next){
        fprintf(out, "%.*s_%.*s = vtable->%.*s;\n",
                string_expand(api->name),
                string_expand(call->name),
                string_expand(call->name));
    }
    fprintf(out, "}\n");
    fprintf(out, "#undef DYNAMIC_LINK_API\n");
    fprintf(out, "#endif\n");
}

////////////////////////////////

function API_Definition*
define_api(Arena *arena);

int
main(void){
    Arena arena = make_arena_malloc();
    API_Definition *api = define_api(&arena);
    
    ////////////////////////////////
    
    // NOTE(allen): Arrange output files
    
    String_Const_u8 path_to_self = string_u8_litexpr(__FILE__);
    path_to_self = string_remove_last_folder(path_to_self);
    
    String_Const_u8 fname_h = push_u8_stringf(&arena, "%.*sgenerated/%.*s_api.h",
                                              string_expand(path_to_self),
                                              string_expand(api->name));
    
    String_Const_u8 fname_cpp = push_u8_stringf(&arena, "%.*sgenerated/%.*s_api.cpp",
                                                string_expand(path_to_self),
                                                string_expand(api->name));
    
    FILE *out_file_h = fopen((char*)fname_h.str, "wb");
    if (out_file_h == 0){
        printf("could not open output file: '%s'\n", fname_h.str);
        exit(1);
    }
    
    FILE *out_file_cpp = fopen((char*)fname_cpp.str, "wb");
    if (out_file_cpp == 0){
        printf("could not open output file: '%s'\n", fname_cpp.str);
        exit(1);
    }
    
    printf("%s:1:\n", fname_h.str);
    printf("%s:1:\n", fname_cpp.str);
    
    ////////////////////////////////
    
    // NOTE(allen): Generate output
    
    generate_header(&arena, api, out_file_h);
    generate_cpp(&arena, api, out_file_cpp);
    
    ////////////////////////////////
    
    fclose(out_file_h);
    fclose(out_file_cpp);
    
    return(0);
}

// BOTTOM

