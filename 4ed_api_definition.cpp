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
api_call_with_location(Arena *arena, API_Definition *api, String_Const_u8 name, String_Const_u8 type, String_Const_u8 location){
    API_Call *call = push_array_zero(arena, API_Call, 1);
    sll_queue_push(api->first_call, api->last_call, call);
    api->call_count += 1;
    call->name = name;
    call->return_type = type;
    call->location_string = location;
    return(call);
}

function API_Call*
api_call_with_location(Arena *arena, API_Definition *api, char *name, char *type, char *location){
    return(api_call_with_location(arena, api, SCu8(name), SCu8(type), SCu8(location)));
}

function API_Type*
api_type_structure_with_location(Arena *arena, API_Definition *api, API_Type_Structure_Kind kind, String_Const_u8 name, List_String_Const_u8 member_list, String_Const_u8 definition, String_Const_u8 location){
    API_Type *type = push_array_zero(arena, API_Type, 1);
    sll_queue_push(api->first_type, api->last_type, type);
    api->type_count += 1;
    type->kind = APITypeKind_Structure;
    type->name = name;
    type->location_string = location;
    type->struct_type.kind = kind;
    type->struct_type.member_names = member_list;
    type->struct_type.definition_string = definition;
    return(type);
}

function API_Type*
api_type_structure_with_location(Arena *arena, API_Definition *api, API_Type_Structure_Kind kind, char *name, List_String_Const_u8 member_list, char *definition, char *location){
    return(api_type_structure_with_location(arena, api, kind, name, member_list, definition, location));
}

#define api_call(arena, api, name, type) \
api_call_with_location((arena), (api), (name), (type), file_name_line_number)

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
api_get_api(API_Definition_List *list, String_Const_u8 name){
    API_Definition *result = 0;
    for (API_Definition *node = list->first;
         node != 0;
         node = node->next){
        if (string_match(name, node->name)){
            result = node;
            break;
        }
    }
    return(result);
}

function API_Definition*
api_get_api(Arena *arena, API_Definition_List *list, String_Const_u8 name){
    API_Definition *result = api_get_api(list, name);
    if (result == 0){
        result = push_array_zero(arena, API_Definition, 1);
        sll_queue_push(list->first, list->last, result);
        list->count += 1;
        result->name = name;
    }
    return(result);
}

function API_Call*
api_get_call(API_Definition *api, String_Const_u8 name){
    API_Call *result = 0;
    for (API_Call *call = api->first_call;
         call != 0;
         call = call->next){
        if (string_match(name, call->name)){
            result = call;
            break;
        }
    }
    return(result);
}

function b32
api_call_match_sigs(API_Call *a, API_Call *b){
    b32 result = false;
    if (a->params.count == b->params.count &&
        string_match(a->return_type, b->return_type)){
        result = true;
        for (API_Param *a_param = a->params.first, *b_param = b->params.first;
             a_param != 0 && b_param != 0;
             a_param = a_param->next, b_param = b_param->next){
            if (!string_match(a_param->name, b_param->name) ||
                !string_match(a_param->type_name, b_param->type_name)){
                result = false;
                break;
            }
        }
    }
    return(result);
}

function API_Type*
api_get_type(API_Definition *api, String_Const_u8 name){
    API_Type *result = 0;
    for (API_Type *type = api->first_type;
         type != 0;
         type = type->next){
        if (string_match(type->name, name)){
            result = type;
            break;
        }
    }
    return(result);
}

function b32
api_type_match(API_Type *a, API_Type *b){
    b32 result = false;
    if (a->kind == b->kind && string_match(a->name, b->name)){
        switch (a->kind){
            case APITypeKind_Structure:
            {
                if (a->kind == b->kind &&
                    string_list_match(a->struct_type.member_names, b->struct_type.member_names) &&
                    string_match(a->struct_type.definition_string, b->struct_type.definition_string)){
                    result = true;
                }
            }break;
            
            case APITypeKind_Enum:
            {
                if (a->enum_type.val_count == b->enum_type.val_count &&
                    string_match(a->enum_type.type_name, b->enum_type.type_name)){
                    result = true;
                    for (API_Enum_Value *a_node = a->enum_type.first_val, *b_node = b->enum_type.first_val;
                         a_node != 0 && b_node != 0;
                         a_node = a_node->next, b_node = b_node->next){
                        if (!string_match(a_node->name, b_node->name) ||
                            !string_match(a_node->val, b_node->val)){
                            result = false;
                            break;
                        }
                    }
                }
            }break;
            
            case APITypeKind_Typedef:
            {
                if (string_match(a->typedef_type.name, b->typedef_type.name) &&
                    string_match(a->typedef_type.definition_text, b->typedef_type.definition_text)){
                    result = false;
                }
            }break;
        }
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
    for (API_Call *call = api->first_call;
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
    for (API_Call *call = api->first_call;
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
    
    for (API_Call *call = api->first_call;
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
    for (API_Call *call = api->first_call;
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
    for (API_Call *call = api->first_call;
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
    for (API_Call *call = api->first_call;
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
    for (API_Call *call = api->first_call;
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
    for (API_Call *call = api->first_call;
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

function void
generate_constructor(Arena *scratch, API_Definition *api, API_Generation_Flag flags, FILE *out){
    fprintf(out, "function API_Definition*\n");
    fprintf(out, "%.*s_api_construct(Arena *arena){\n",
            string_expand(api->name));
    fprintf(out, "API_Definition *result = begin_api(arena, \"%.*s\");\n",
            string_expand(api->name));
    
    for (API_Call *call = api->first_call;
         call != 0;
         call = call->next){
        fprintf(out, "{\n");
        fprintf(out, "API_Call *call = api_call_with_location(arena, result, "
                "string_u8_litexpr(\"%.*s\"), "
                "string_u8_litexpr(\"%.*s\"), "
                "string_u8_litexpr(\"\"));\n",
                string_expand(call->name),
                string_expand(call->return_type));
        
        if (call->params.count == 0){
            fprintf(out, "(void)call;\n");
        }
        else{
            for (API_Param *param = call->params.first;
                 param != 0;
                 param = param->next){
                fprintf(out, "api_param(arena, call, \"%.*s\", \"%.*s\");\n",
                        string_expand(param->type_name),
                        string_expand(param->name));
            }
        }
        
        fprintf(out, "}\n");
    }
    
    fprintf(out, "return(result);\n");
    fprintf(out, "}\n");
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
    String_Const_u8 fname_con = {};
    
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
    
    fname_con = push_u8_stringf(arena, "%.*s%.*s%.*s_api_constructor.cpp",
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
    
    FILE *out_file_con = fopen((char*)fname_con.str, "wb");
    if (out_file_cpp == 0){
        printf("could not open output file: '%s'\n", fname_con.str);
        return(false);
    }
    
    printf("%s:1:\n", fname_ml.str);
    printf("%s:1:\n", fname_h.str);
    printf("%s:1:\n", fname_cpp.str);
    printf("%s:1:\n", fname_con.str);
    
    ////////////////////////////////
    
    // NOTE(allen): Generate output
    
    generate_api_master_list(arena, api, flags, out_file_ml);
    generate_header(arena, api, flags, out_file_h);
    generate_cpp(arena, api, flags, out_file_cpp);
    generate_constructor(arena, api, flags, out_file_con);
    
    ////////////////////////////////
    
    fclose(out_file_ml);
    fclose(out_file_h);
    fclose(out_file_cpp);
    return(true);
}

////////////////////////////////

function void
api_definition_error(Arena *arena, List_String_Const_u8 *list,
                     char *e1, API_Call *c1, char *e2, API_Call *c2){
    Assert(e1 != 0);
    Assert(c1 != 0);
    string_list_pushf(arena, list,
                      "%.*s error: %s '%.*s'",
                      string_expand(c1->location_string),
                      e1, string_expand(c1->name));
    if (e2 != 0){
        string_list_pushf(arena, list, " %s", e2);
        if (c2 != 0){
            string_list_pushf(arena, list, " '%.*s'", string_expand(c2->name));
        }
    }
    string_list_push(arena, list, string_u8_litexpr("\n"));
    if (c2 != 0){
        string_list_push(arena, list, c2->location_string);
        string_list_pushf(arena, list, " note: see declaration of '%.*s'\n", string_expand(c2->name));
    }
}

function void
api_definition_error(Arena *arena, List_String_Const_u8 *list,
                     char *e1, API_Call *c1, char *e2){
    api_definition_error(arena, list, e1, c1, e2, 0);
}

function void
api_definition_error(Arena *arena, List_String_Const_u8 *list,
                     char *e1, API_Call *c1){
    api_definition_error(arena, list, e1, c1, 0, 0);
}

function void
api_definition_error(Arena *arena, List_String_Const_u8 *list,
                     char *e1, API_Definition *api1, char *e2){
    Assert(e1 != 0);
    Assert(api1 != 0);
    string_list_pushf(arena, list, "error: %s '%.*s'",
                      e1, string_expand(api1->name));
    if (e2 != 0){
        string_list_pushf(arena, list, " %s", e2);
    }
    string_list_push(arena, list, string_u8_litexpr("\n"));
}

function void
api_definition_error(Arena *arena, List_String_Const_u8 *list,
                     char *e1, API_Definition *api1){
    api_definition_error(arena, list, e1, api1, 0);
}

function void
api_definition_check(Arena *arena, API_Definition *correct, API_Definition *remote, API_Check_Flag flags, List_String_Const_u8 *error_list){
    b32 report_missing = HasFlag(flags, APICheck_ReportMissingAPI);
    b32 report_extra = HasFlag(flags, APICheck_ReportExtraAPI);
    b32 report_mismatch = HasFlag(flags, APICheck_ReportMismatchAPI);
    
    b32 iterate_correct = (report_missing || report_mismatch);
    if (iterate_correct){
        for (API_Call *call = correct->first_call;
             call != 0;
             call = call->next){
            API_Call *remote_call = api_get_call(remote, call->name);
            if (remote_call == 0 && report_missing){
                api_definition_error(arena, error_list,
                                     "no remote call for", call);
            }
            if (remote_call != 0 && !api_call_match_sigs(call, remote_call) && report_mismatch){
                api_definition_error(arena, error_list,
                                     "remote call", remote_call,
                                     "does not match signature for", call);
            }
        }
    }
    
    b32 iterate_remote = (report_extra);
    if (iterate_remote){
        for (API_Call *call = remote->first_call;
             call != 0;
             call = call->next){
            API_Call *correct_call = api_get_call(correct, call->name);
            if (correct_call == 0 && report_extra){
                api_definition_error(arena, error_list,
                                     "remote call", call, 
                                     "does not exist in api master");
            }
        }
    }
}

function void
api_list_check(Arena *arena, API_Definition_List *correct, API_Definition_List *remote, API_Check_Flag flags, List_String_Const_u8 *error_list){
    b32 report_missing = HasFlag(flags, APICheck_ReportMissingAPI);
    b32 report_extra = HasFlag(flags, APICheck_ReportExtraAPI);
    
    b32 iterate_correct = (report_missing);
    if (iterate_correct){
        for (API_Definition *api = correct->first;
             api != 0;
             api = api->next){
            API_Definition *remote_api = api_get_api(remote, api->name);
            if (remote_api == 0 && report_missing){
                api_definition_error(arena, error_list,
                                     "no remote api for", api);
            }
        }
    }
    
    b32 iterate_remote = (report_extra);
    if (iterate_remote){
        for (API_Definition *api = remote->first;
             api != 0;
             api = api->next){
            API_Definition *correct_api = api_get_api(correct, api->name);
            if (correct_api == 0 && report_extra){
                api_definition_error(arena, error_list,
                                     "remote api", api,
                                     "does not have a master");
            }
        }
    }
    
    for (API_Definition *api = correct->first;
         api != 0;
         api = api->next){
        API_Definition *remote_api = api_get_api(remote, api->name);
        if (remote_api != 0){
            api_definition_check(arena, api, remote_api, flags, error_list);
        }
    }
}

#endif

// BOTTOM

