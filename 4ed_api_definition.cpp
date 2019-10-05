/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 03.10.2019
 *
 * System API definition program.
 *
 */

// TOP

function API_Definition*
begin_api(Arena *arena, char *name){
    API_Definition *api = push_array_zero(arena, API_Definition, 1);
    api->name = SCu8(name);
    return(api);
}

function API_Call*
api_call(Arena *arena, API_Definition *api, String_Const_u8 name, String_Const_u8 return_type){
    API_Call *call = push_array_zero(arena, API_Call, 1);
    sll_queue_push(api->first, api->last, call);
    api->count += 1;
    call->name = name;
    call->return_type = return_type;
    return(call);
}

function API_Call*
api_call(Arena *arena, API_Definition *api, char *name, char *return_type){
    return(api_call(arena, api, SCu8(name), SCu8(return_type)));
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

function void
api_set_param_list(API_Call *call, API_Param_List list){
    call->params = list;
}

function API_Definition*
api_get_api(Arena *arena, API_Definition_List *list, String_Const_u8 name){
    API_Definition *result = 0;
    for (API_Definition *node = list->first;
         node != 0;
         node = node->next){
        if (string_match(name, node->name)){
            result = node;
            break;
        }
    }
    if (result == 0){
        result = push_array_zero(arena, API_Definition, 1);
        sll_queue_push(list->first, list->last, result);
        list->count += 1;
        result->name = name;
    }
    return(result);
}

////////////////////////////////

#if !defined(SKIP_STDIO)
#include <stdio.h>
#include "4coder_stringf.cpp"

function String_Const_u8
api_get_callable_name(Arena *arena, String_Const_u8 api_name, String_Const_u8 name, API_Generation_Flag flags){
    String_Const_u8 result = {};
    if (HasFlag(flags, APIGeneration_NoAPINameOnCallables)){
        result = push_u8_stringf(arena, "%.*s", string_expand(name));
    }
    else{
        result = push_u8_stringf(arena, "%.*s_%.*s",
                                 string_expand(api_name),
                                 string_expand(name));
    }
    return(result);
}

////////////////////////////////

function void
generate_api_master_list(Arena *scratch, API_Definition *api, API_Generation_Flag flags, FILE *out){
    for (API_Call *call = api->first;
         call != 0;
         call = call->next){
        fprintf(out, "api(%.*s) function %.*s %.*s(",
                string_expand(api->name),
                string_expand(call->return_type),
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
}

function void
generate_header(Arena *scratch, API_Definition *api, API_Generation_Flag flags, FILE *out){
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
        String_Const_u8 callable_name = api_get_callable_name(scratch, api->name, call->name, flags);
        fprintf(out, "internal %.*s %.*s(",
                string_expand(call->return_type),
                string_expand(callable_name));
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
        String_Const_u8 callable_name = api_get_callable_name(scratch, api->name, call->name, flags);
        fprintf(out, "global %.*s_%.*s_type *%.*s = 0;\n",
                string_expand(api->name),
                string_expand(call->name),
                string_expand(callable_name));
    }
    fprintf(out, "#undef DYNAMIC_LINK_API\n");
    fprintf(out, "#endif\n");
}

function void
generate_cpp(Arena *scratch, API_Definition *api, API_Generation_Flag flags, FILE *out){
    fprintf(out, "function void\n");
    fprintf(out, "%.*s_api_fill_vtable(API_VTable_%.*s *vtable){\n",
            string_expand(api->name),
            string_expand(api->name));
    for (API_Call *call = api->first;
         call != 0;
         call = call->next){
        String_Const_u8 callable_name = api_get_callable_name(scratch, api->name, call->name, flags);
        fprintf(out, "vtable->%.*s = %.*s;\n",
                string_expand(call->name),
                string_expand(callable_name));
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
        String_Const_u8 callable_name = api_get_callable_name(scratch, api->name, call->name, flags);
        fprintf(out, "%.*s = vtable->%.*s;\n",
                string_expand(callable_name),
                string_expand(call->name));
    }
    fprintf(out, "}\n");
    fprintf(out, "#undef DYNAMIC_LINK_API\n");
    fprintf(out, "#endif\n");
}

////////////////////////////////

function b32
api_definition_generate_api_includes(Arena *arena, API_Definition *api, Generated_Group group, API_Generation_Flag flags){
    // NOTE(allen): Arrange output files
    
    String_Const_u8 path_to_self = string_u8_litexpr(__FILE__);
    path_to_self = string_remove_last_folder(path_to_self);
    
    String_Const_u8 fname_ml = {};
    String_Const_u8 fname_h = {};
    String_Const_u8 fname_cpp = {};
    
    String_Const_u8 root = {};
    switch (group){
        case GeneratedGroup_Core:
        {
            root = string_u8_litexpr("generated/");
        }break;
        case GeneratedGroup_Custom:
        {
            root = string_u8_litexpr("custom/generated/");
        }break;
    }
    
    fname_ml = push_u8_stringf(arena, "%.*s%.*s%.*s_api_master_list.h",
                               string_expand(path_to_self),
                               string_expand(root),
                               string_expand(api->name));
                               
    fname_h = push_u8_stringf(arena, "%.*s%.*s%.*s_api.h",
                              string_expand(path_to_self),
                              string_expand(root),
                              string_expand(api->name));
                              
    fname_cpp = push_u8_stringf(arena, "%.*s%.*s%.*s_api.cpp",
                                string_expand(path_to_self),
                                string_expand(root),
                                string_expand(api->name));
                                
    FILE *out_file_ml = fopen((char*)fname_ml.str, "wb");
    if (out_file_ml == 0){
        printf("could not open output file: '%s'\n", fname_ml.str);
        return(false);
    }
    
    FILE *out_file_h = fopen((char*)fname_h.str, "wb");
    if (out_file_h == 0){
        printf("could not open output file: '%s'\n", fname_h.str);
        return(false);
    }
    
    FILE *out_file_cpp = fopen((char*)fname_cpp.str, "wb");
    if (out_file_cpp == 0){
        printf("could not open output file: '%s'\n", fname_cpp.str);
        return(false);
    }
    
    printf("%s:1:\n", fname_ml.str);
    printf("%s:1:\n", fname_h.str);
    printf("%s:1:\n", fname_cpp.str);
    
    ////////////////////////////////
    
    // NOTE(allen): Generate output
    
    generate_api_master_list(arena, api, flags, out_file_ml);
    generate_header(arena, api, flags, out_file_h);
    generate_cpp(arena, api, flags, out_file_cpp);
    
    ////////////////////////////////
    
    fclose(out_file_ml);
    fclose(out_file_h);
    fclose(out_file_cpp);
    return(true);
}

#endif

// BOTTOM

