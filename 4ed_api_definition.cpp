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

function void
generate_api_master_list(Arena *scratch, API_Definition *api, FILE *out){
    fprintf(out, "// %.*s\n", string_expand(api->name));
    for (API_Call *call = api->first;
         call != 0;
         call = call->next){
        fprintf(out, "%.*s %.*s(",
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

#endif

// BOTTOM

