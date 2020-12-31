/*
4coder_variables.cpp - Variables system
*/

// TOP

////////////////////////////////
// NOTE(allen): String hashing

global Arena vars_arena = {};
global Table_Data_u64 vars_string_to_id = {};
global Table_u64_Data vars_id_to_string = {};
global String_ID vars_string_id_counter = 0;

function void
_vars_init(void){
    local_persist b32 did_init = false;
    if (!did_init){
        did_init = true;
        Base_Allocator *base = get_base_allocator_system();
        vars_arena = make_arena(base);
        vars_string_to_id = make_table_Data_u64(base, 100);
        vars_id_to_string = make_table_u64_Data(base, 100);
    }
}

function String_ID
vars_save_string(String_Const_u8 string){
    _vars_init();
    
    String_ID result = 0;
    String_Const_u8 string_data = make_data(string.str, string.size);
    Table_Lookup location = table_lookup(&vars_string_to_id, string_data);
    if (location.found_match){
        table_read(&vars_string_to_id, location, &result);
    }
    else{
        vars_string_id_counter += 1;
        result = vars_string_id_counter;
        string_data = push_data_copy(&vars_arena, string_data);
        table_insert(&vars_string_to_id, string_data, result);
        table_insert(&vars_id_to_string, result, string_data);
    }
    return(result);
}

function String_Const_u8
vars_read_string(Arena *arena, String_ID id){
    _vars_init();
    
    String_Const_u8 result = {};
    Table_Lookup location = table_lookup(&vars_id_to_string, id);
    if (location.found_match){
        String_Const_u8 data = {};
        table_read(&vars_id_to_string, location, &data);
        result.str = push_array(arena, u8 , data.size);
        block_copy(result.str, data.str, data.size);
        result.size = data.size;
    }
    return(result);
}

////////////////////////////////
// NOTE(allen): Variable structure

global Variable vars_global_root = {};
global Variable vars_nil = {};
global Variable *vars_free_variables = 0;

function Variable_Handle
vars_get_root(void){
    Variable_Handle handle = {&vars_global_root};
    local_persist b32 first_call = true;
    if (first_call){
        first_call = false;
        Variable *nil = vars_get_nil().ptr;
        vars_global_root.parent = nil;
        vars_global_root.next = nil;
        vars_global_root.first = nil;
        vars_global_root.last = nil;
    }
    return(handle);
}

function Variable_Handle
vars_get_nil(void){
    Variable_Handle handle = {&vars_nil};
    if (vars_nil.parent == 0){
        vars_nil.parent = &vars_nil;
        vars_nil.next = &vars_nil;
        vars_nil.first = &vars_nil;
        vars_nil.last = &vars_nil;
    }
    return(handle);
}

function b32
vars_is_nil(Variable_Handle var){
    return(var.ptr == &vars_nil);
}

function b32
vars_is_nil(Variable* ptr) {
	return(ptr == &vars_nil);
}

function b32
vars_match(Variable_Handle a, Variable_Handle b){
    return(a.ptr == b.ptr);
}

function Variable_Handle
vars_first_child(Variable_Handle var){
    Variable_Handle result = {var.ptr->first};
    return(result);
}

function Variable_Handle
vars_next_sibling(Variable_Handle var){
    Variable_Handle result = {var.ptr->next};
    return(result);
}

function Variable_Handle
vars_parent(Variable_Handle var){
    Variable_Handle result = {var.ptr->parent};
    return(result);
}

function String_ID
vars_key_id_from_var(Variable_Handle var){
    return(var.ptr->key);
}

function String_Const_u8
vars_key_from_var(Arena *arena, Variable_Handle var){
    String_ID id = vars_key_id_from_var(var);
    String_Const_u8 result = vars_read_string(arena, id);
    return(result);
}

function String_ID
vars_string_id_from_var(Variable_Handle var){
    return(var.ptr->string);
}

function String_Const_u8
vars_string_from_var(Arena *arena, Variable_Handle var){
    String_ID id = vars_string_id_from_var(var);
    String_Const_u8 result = vars_read_string(arena, id);
    return(result);
}

function b32
vars_b32_from_var(Variable_Handle var){
    String_ID val = vars_string_id_from_var(var);
    b32 result = (val != 0 && val != vars_save_string_lit("false"));
    return(result);
}

function u64
vars_u64_from_var(Application_Links *app, Variable_Handle var){
    Scratch_Block scratch(app);
    String_ID val = vars_string_id_from_var(var);
    String_Const_u8 string = vars_read_string(scratch, val);
    u64 result = 0;
    if (string_match(string_prefix(string, 2), str8_lit("0x"))){
        String_Const_u8 string_hex = string_skip(string, 2);
        if (string_is_integer(string_hex, 0x10)){
            result = string_to_integer(string_hex, 0x10);
        }
    }
    else{
        if (string_is_integer(string, 10)){
            result = string_to_integer(string, 10);
        }
    }
    return(result);
}

function Variable_Handle
vars_read_key(Variable_Handle var, String_ID key){
    Variable_Handle result = vars_get_nil();
    for (Variable *node = var.ptr->first;
         !vars_is_nil(node);
         node = node->next){
        if (node->key == key){
            result.ptr = node;
            break;
        }
    }
    return(result);
}

function Variable_Handle
vars_read_key(Variable_Handle var, String_Const_u8 key){
    String_ID id = vars_save_string(key);
    Variable_Handle result = vars_read_key(var, id);
    return(result);
}

function void
vars_set_string(Variable_Handle var, String_ID string){
    if (var.ptr != &vars_nil){
        var.ptr->string = string;
    }
}

function void
vars_set_string(Variable_Handle var, String_Const_u8 string){
    String_ID id = vars_save_string(string);
    vars_set_string(var, id);
}

function void
_vars_free_variable_children(Variable *var){
    for (Variable *node = var->first;
         !vars_is_nil(node);
         node = node->next){
        _vars_free_variable_children(node);
    }
    
    if (!vars_is_nil(var->first)){
        var->last->next = vars_free_variables;
        vars_free_variables = var->first;
    }
}

function void
vars_erase(Variable_Handle var, String_ID key){
    if (var.ptr != &vars_nil){
        Variable *prev = vars_get_nil().ptr;
        Variable *node = var.ptr->first;
        for (; vars_is_nil(node);
             node = node->next){
            if (node->key == key){
                break;
            }
            prev = node;
        }
        
        if (!vars_is_nil(node)){
            _vars_free_variable_children(node);
            if (!vars_is_nil(prev)){
                prev->next = node->next;
            }
            if (var.ptr->first == node){
                var.ptr->first = node->next;
            }
            if (var.ptr->last == node){
                var.ptr->last = prev;
            }
            sll_stack_push(vars_free_variables, node);
        }
    }
}

function Variable_Handle
vars_new_variable(Variable_Handle var, String_ID key){
    Variable_Handle handle = vars_get_nil();
    if (var.ptr != &vars_nil){
        Variable *prev = vars_get_nil().ptr;
        Variable *node = var.ptr->first;
        for (; !vars_is_nil(node);
             node = node->next){
            if (node->key == key){
                break;
            }
            prev = node;
        }
        
        if (!vars_is_nil(node)){
            handle.ptr = node;
            _vars_free_variable_children(node);
        }
        else{
            handle.ptr = vars_free_variables;
            if (handle.ptr != 0){
                sll_stack_pop(vars_free_variables);
            }
            else{
                handle.ptr = push_array(&vars_arena, Variable, 1);
            }
			if (vars_is_nil(var.ptr->first)){
				var.ptr->first = var.ptr->last = handle.ptr;
            }
			else{
				var.ptr->last->next = handle.ptr;
				var.ptr->last = handle.ptr;
			}
            handle.ptr->parent = var.ptr;
			handle.ptr->next = vars_get_nil().ptr;
            handle.ptr->key = key;
        }
        
        handle.ptr->string = 0;
        handle.ptr->first = handle.ptr->last = vars_get_nil().ptr;
    }
    return(handle);
}

function Variable_Handle
vars_new_variable(Variable_Handle var, String_ID key, String_ID string){
    Variable_Handle result = vars_new_variable(var, key);
    vars_set_string(result, string);
    return(result);
}

function void
vars_clear_keys(Variable_Handle var){
    if (var.ptr != &vars_nil){
        _vars_free_variable_children(var.ptr);
    }
}

function void
vars_print_indented(Application_Links *app, Variable_Handle var, i32 indent){
    Scratch_Block scratch(app);
    local_persist char spaces[] =
        "                                                                "
        "                                                                "
        "                                                                "
        "                                                                ";
    
    String_Const_u8 var_key = vars_key_from_var(scratch, var);
    String_Const_u8 var_val = vars_string_from_var(scratch, var);
    
    String_Const_u8 line = push_stringf(scratch, "%.*s%.*s: \"%.*s\"\n",
                                        clamp_top(indent, sizeof(spaces)), spaces,
                                        string_expand(var_key),
                                        string_expand(var_val));
    print_message(app, line);
    
    i32 sub_indent = indent + 1;
    for (Variable_Handle sub = vars_first_child(var);
         !vars_is_nil(sub);
         sub = vars_next_sibling(sub)){
        vars_print_indented(app, sub, sub_indent);
    }
}

function void
vars_print(Application_Links *app, Variable_Handle var){
    vars_print_indented(app, var, 0);
}

// BOTTOM

