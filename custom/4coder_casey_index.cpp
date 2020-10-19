static Content_Index *global_content_index = 0;

internal String get_name_of_index_type(Indexed_Content_Element_Type content_type)
{
    String result = {};
    
    switch(content_type)
    {
        case ContentType_Unspecified: {result = make_lit_string("Unspecified");} break;
        
        case ContentType_Function: {result = make_lit_string("Function");} break;
        case ContentType_Type: {result = make_lit_string("Type");} break;
        case ContentType_Macro: {result = make_lit_string("Macro");} break;
        case ContentType_EnumValue: {result = make_lit_string("Enum Value");} break;
        case ContentType_ForwardDeclaration: {result = make_lit_string("Forward Declaration");} break;
        
        case ContentType_TODO: {result = make_lit_string("TODO");} break;
        case ContentType_NOTE: {result = make_lit_string("Note");} break;
        case ContentType_IMPORTANT: {result = make_lit_string("Important");} break;
        case ContentType_STUDY: {result = make_lit_string("Study");} break;
    }
    
    return(result);
}

static Content_Index *
create_content_index(Application_Links *app)
{
    // TODO(casey): I really just want this to be its own allocating arena, but I can't find anything like that in 4coder?
    // All arenas seem to be fixed size and they can't get more memory if they run out :(
    Content_Index *index = (Content_Index *)malloc(sizeof(Content_Index));
    
    memset(index, 0, sizeof(*index));
    
    return(index);
}

static Indexed_Content_Element **
get_element_slot_for_name(Content_Index *index, String name)
{
    Indexed_Content_Element **slot = &index->name_hash[table_hash_u8((uint8_t *)name.str, name.size) % ArrayCount(index->name_hash)];
    return(slot);
}

static Indexed_Buffer *
get_or_create_indexed_buffer(Content_Index *index, Buffer_ID buffer_id)
{
    Indexed_Buffer *result = 0;
    
    Indexed_Buffer **slot = &index->buffer_hash[buffer_id % ArrayCount(index->buffer_hash)];
    for(Indexed_Buffer *search = *slot;
        search;
        search = search->next_in_hash)
    {
        if(search->buffer_id == buffer_id)
        {
            result = search;
            break;
        }
    }
    
    if(!result)
    {
        result = (Indexed_Buffer *)malloc(sizeof(Indexed_Buffer));
        result->buffer_id = buffer_id;
        result->first_tag = 0;
        result->next_in_hash = *slot;
        *slot = result;
    }
    
    return(result);
}

static color_pair
get_color_pair_for(Indexed_Content_Element_Type type)
{
    color_pair result = {};
    
    switch(type)
    {
        case ContentType_Function:
        {
            result.fore = CC_Function;
        } break;
        
        case ContentType_Type:
        {
            result.fore = CC_Type;
        } break;
        
        case ContentType_Macro:
        {
            result.fore = CC_Macro;
        } break;
        
        case ContentType_EnumValue:
        {
            result.fore = CC_EnumValue;
        } break;
        
        case ContentType_ForwardDeclaration:
        {
            result.fore = CC_ForwardDeclaration;
        } break;
        
        case ContentType_TODO:
        {
            result.fore = CC_TODO;
        } break;
        
        case ContentType_NOTE:
        {
            result.fore = CC_NOTE;
        } break;
        
        case ContentType_IMPORTANT:
        {
            result.fore = CC_IMPORTANT;
        } break;
        
        case ContentType_STUDY:
        {
            result.fore = CC_STUDY;
        } break;
    }
    
    return(result);
}

static Indexed_Content_Element *
begin_element_in_index(Content_Index *index, Indexed_Content_Element_Type type, Buffer_ID buffer_id, int32_t location,
                       int32_t name_size, int32_t content_size)
{
    Indexed_Content_Element *elem = 0;
    
    // TODO(casey): I really just want this to be its own allocating arena, but I can't find anything like that in 4coder?
    // All arenas seem to be fixed size and they can't get more memory if they run out :(
    uint8_t *mem = (uint8_t *)malloc(sizeof(Indexed_Content_Element) + name_size + content_size);
    if(mem)
    {
        elem = (Indexed_Content_Element *)mem;
        
        elem->next_in_hash = 0;
        elem->type = type;
        elem->name.memory_size = elem->name.size = name_size;
        elem->name.str = (char *)(elem + 1);
        elem->content.memory_size = elem->content.size = content_size;
        elem->content.str = elem->name.str + name_size;
        
        elem->color = get_color_pair_for(type);
        
        elem->buffer_id = buffer_id;
        elem->last_known_location = location;
        
        index->total_string_space += name_size + content_size;
        ++index->element_count;
    }
    
    return(elem);
}

static void
end_element_in_index(Content_Index *index, Indexed_Content_Element *elem, u32 flags)
{
    if(elem)
    {
        // NOTE(casey): By convention, we make content into a single line, and remove leading/trailing whitespace
        condense_whitespace(&elem->name);
        condense_whitespace(&elem->content);
        
        Indexed_Content_Element **slot = &index->unhashed;
        if(!(flags & IndexFlag_NoNameLookup))
        {
            slot = get_element_slot_for_name(index, elem->name);
        }
        elem->next_in_hash = *slot;
        *slot = elem;
        
        Indexed_Buffer *buffer = get_or_create_indexed_buffer(index, elem->buffer_id);
        ++buffer->element_type_counts[elem->type];
        ++index->element_type_counts[elem->type];
    }
}

static Indexed_Content_Element *
add_element_to_index(Application_Links *app, Content_Index *index, Indexed_Content_Element_Type type, Buffer_Summary *buffer, int32_t location,
                     int32_t name_start, int32_t name_end,
                     int32_t content_start, int32_t content_end,
                     u32 flags = 0)
{
    Indexed_Content_Element *elem = 0;
    if((name_start <= name_end) &&
       (content_start <= content_end))
    {
        elem = begin_element_in_index(index, type, buffer->buffer_id, location, name_end - name_start, content_end - content_start);
        if(elem)
        {
            buffer_read_range(app, buffer, name_start, name_end, elem->name.str);
            buffer_read_range(app, buffer, content_start, content_end, elem->content.str);
            end_element_in_index(index, elem, flags);
        }
    }
    
    return(elem);
}

static Indexed_Content_Element *
add_element_to_index(Content_Index *index, Indexed_Content_Element_Type type, Buffer_ID buffer_id, int32_t location, String name, String content,
                     u32 flags = 0)
{
    Indexed_Content_Element *elem = begin_element_in_index(index, type, buffer_id, location, name.size, content.size);
    if(elem)
    {
        copy_ss(&elem->name, name);
        copy_ss(&elem->content, content);
        end_element_in_index(index, elem, flags);
    }
    
    return(elem);
}

static Indexed_Content_Tag *
add_tag_to_index(Content_Index *index, Buffer_Summary *buffer, uint32_t type, int32_t start, int32_t end, String content)
{
    Indexed_Content_Tag *tag = 0;
    
    if(start <= end)
    {
        Indexed_Buffer *index_buffer = get_or_create_indexed_buffer(index, buffer->buffer_id);
        // TODO(casey): I really just want this to be its own allocating arena, but I can't find anything like that in 4coder?
        // All arenas seem to be fixed size and they can't get more memory if they run out :(
        uint8_t *mem = (uint8_t *)malloc(sizeof(Indexed_Content_Tag) + content.size);
        if(mem)
        {
            tag = (Indexed_Content_Tag *)mem;
            tag->content.size = tag->content.memory_size = content.size;
            tag->content.str = (char *)(tag + 1);
            copy_ss(&tag->content, content);
            tag->type = type;
            tag->start = start;
            tag->end = end;
            tag->next_in_buffer = index_buffer->first_tag;
            index_buffer->first_tag = tag;
        }
    }
    
    return(tag);
}

static Indexed_Content_Element *
get_element_by_name(Content_Index *index, String name)
{
    Indexed_Content_Element *result = 0;
    
    if(index)
    {
        for(Indexed_Content_Element *elem = *get_element_slot_for_name(index, name);
            elem;
            elem = elem->next_in_hash)
        {
            if(compare_ss(elem->name, name) == 0)
            {
                result = elem;
                break;
            }
        }
    }
    
    return(result);
}

static void
remove_buffer_from_index(Application_Links *app, Content_Index *index, Buffer_ID buffer_id)
{
    for(int hash_index = 0;
        hash_index <= ArrayCount(index->name_hash);
        ++hash_index)
    {
        for(Indexed_Content_Element **elem_ptr = &index->name_hash[hash_index];
            *elem_ptr;
            )
        {
            Indexed_Content_Element *elem = *elem_ptr;
            if(elem->buffer_id == buffer_id)
            {
                *elem_ptr = elem->next_in_hash;
                free(elem);
            }
            else
            {
                elem_ptr = &elem->next_in_hash;
            }
        }
    }
    
    Indexed_Buffer *index_buffer = get_or_create_indexed_buffer(index, buffer_id);
    while(index_buffer->first_tag)
    {
        Indexed_Content_Tag *tag = index_buffer->first_tag;
        index_buffer->first_tag = tag->next_in_buffer;
        free(tag);
    }
    
    for(u32 type_index = 0;
        type_index < ContentType_Count;
        ++type_index)
    {
        index->element_type_counts[type_index] -= index_buffer->element_type_counts[type_index];
        index_buffer->element_type_counts[type_index] = 0;
    }
}

static void
add_buffer_to_index(Application_Links *app, Content_Index *index, Buffer_Summary *buffer)
{
    ++index->buffer_count;
    
    Token_Iterator iter = iterate_tokens(app, buffer, 0);
    
    int32_t paren_level = 0;
    int32_t brace_level = 0;
    
    while(iter.valid)
    {
        Cpp_Token token = get_next_token(app, &iter);
        
        if(token.flags & CPP_TFLAG_PP_BODY)
        {
            // TODO(casey): Do we need to pick up macros here?
        }
        else
        {
            switch(token.type)
            {
                case CPP_TOKEN_COMMENT:
                {
                    // NOTE(casey): Comments can contain special operations that we want to perform, so we thunk to a special parser to handle those
                    parse_comment(app, index, buffer, token);
                } break;
                
                case CPP_PP_DEFINE:
                {
                    int32_t content_start = token.start;
                    token = get_next_token(app, &iter);
                    
                    // TODO(casey): Allen, how do I scan for the next "not continued newline"?
                    int32_t content_end = token.start + token.size;
                    
                    add_element_to_index(app, index, ContentType_Macro, buffer, token.start,
                                         token.start, token.start + token.size,
                                         content_start, content_end);
                } break;
                
                case CPP_TOKEN_KEY_TYPE_DECLARATION:
                {
                    bool32 forward_declaration = true;
                    int32_t content_start = token.start;
                    Cpp_Token element_name = {};
                    Cpp_Token last_identifier = {};
                    
                    String typedef_keyword = make_lit_string("typedef");
                    if(token_text_is(app, buffer, token, typedef_keyword))
                    {
                        // NOTE(casey): Typedefs can't be "forward declared", so we always record their location.
                        forward_declaration = false;
                        
                        // NOTE(casey): This is a simple "usually works" parser for typedefs.  It is well-known that C grammar
                        // doesn't actually allow you to parse typedefs properly without actually knowing the declared types
                        // at the time, which of course we cannot do since we parse per-file, and don't even know things we
                        // would need (like -D switches, etc.)
                        
                        // TODO(casey): If eventually this were upgraded to a more thoughtful parser, struct/union/enum defs
                        // would be parsed from brace to brace, so they could be handled recursively inside of a typedef for
                        // the pattern "typedef struct {} foo;", which currently we do not handle (we ignore the foo).
                        int typedef_paren_level = 0;
                        while(iter.valid)
                        {
                            token = get_next_token(app, &iter);
                            if(token.type == CPP_TOKEN_IDENTIFIER)
                            {
                                last_identifier = token;
                            }
                            else if(token.type == CPP_TOKEN_PARENTHESE_OPEN)
                            {
                                if(typedef_paren_level == 0)
                                {
                                    // NOTE(casey): If we are going back into a parenthetical, it means that whatever the last
                                    // identifier we saw is our best candidate.
                                    element_name = last_identifier;
                                }
                                ++typedef_paren_level;
                            }
                            else if(token.type == CPP_TOKEN_PARENTHESE_CLOSE)
                            {
                                --typedef_paren_level;
                            }
                            else if(token.type == CPP_TOKEN_KEY_TYPE_DECLARATION)
                            {
                                // TODO(casey): If we _really_ wanted to, we could parse the type here recursively,
                                // and then we could capture things that were named differently a la "typedef struct foo {...} bar;"
                                // but I think that should wait until the 4coder parser is more structural, because I don't
                                // know how much parsing we _really_ want to do here.
                                forward_declaration = true;
                                break;
                            }
                            else if(token.type == CPP_TOKEN_SEMICOLON)
                            {
                                break;
                            }
                        }
                        
                        if(element_name.size == 0)
                        {
                            element_name = last_identifier;
                        }
                    }
                    
                    if(!element_name.size)
                    {
                        while(iter.valid)
                        {
                            token = peek_token(app, &iter);
                            if(token.type == CPP_TOKEN_IDENTIFIER)
                            {
                                element_name = token;
                                get_next_token(app, &iter);
                            }
                            else if((token.type == CPP_TOKEN_KEY_MODIFIER) ||
                                    (token.type == CPP_TOKEN_KEY_QUALIFIER) ||
                                    (token.type == CPP_TOKEN_KEY_ACCESS) ||
                                    (token.type == CPP_TOKEN_KEY_LINKAGE))
                            {
                                // NOTE(casey): Let it go.
                                get_next_token(app, &iter);
                            }
                            else if(token.type == CPP_TOKEN_BRACE_OPEN)
                            {
                                // NOTE(casey): It's probably type definition
                                forward_declaration = false;
                                break;
                            }
                            else
                            {
                                // NOTE(casey): It's probably a forward declaration
                                break;
                            }
                        }
                    }
                    
                    int32_t content_end = token.start;
                    
                    if(element_name.size)
                    {
                        Indexed_Content_Element_Type type = ContentType_Type;
                        if(forward_declaration)
                        {
                            type = ContentType_ForwardDeclaration;
                        }
                        
                        add_element_to_index(app, index, type, buffer, element_name.start,
                                             element_name.start, element_name.start + element_name.size,
                                             content_start, content_end);
                    }
                } break;
                
                case CPP_TOKEN_PARENTHESE_OPEN:
                {
                    ++paren_level;
                } break;
                
                case CPP_TOKEN_PARENTHESE_CLOSE:
                {
                    --paren_level;
                } break;
                
                case CPP_TOKEN_BRACE_OPEN:
                {
                    if((paren_level == 0) &&
                       (brace_level == 0))
                    {
                        // NOTE(casey): This is presumably a function, see if we can find it's name.
                        int32_t content_end = token.start;
                        int32_t content_start = content_end;
                        int32_t name_start = 0;
                        int32_t name_end = 0;
                        
                        Token_Iterator back = iter;
                        get_prev_token(app, &back);
                        get_prev_token(app, &back);
                        Cpp_Token comment_pass = get_prev_token(app, &back);
                        while(comment_pass.type == CPP_TOKEN_COMMENT)
                        {
                            comment_pass = get_prev_token(app, &back);
                        }
                        if(comment_pass.type == CPP_TOKEN_PARENTHESE_CLOSE)
                        {
                            paren_level = -1;
                            while(back.valid)
                            {
                                if(token.type != CPP_TOKEN_COMMENT)
                                {
                                    content_start = token.start;
                                }
                                token = get_prev_token(app, &back);
                                if((paren_level == 0) &&
                                   (brace_level == 0) &&
                                   ((token.flags & CPP_TFLAG_PP_BODY) ||
                                    (token.type == CPP_TOKEN_BRACE_CLOSE) ||
                                    (token.type == CPP_TOKEN_SEMICOLON) ||
                                    (token.type == CPP_TOKEN_EOF)))
                                {
                                    break;
                                }
                                else
                                {
                                    switch(token.type)
                                    {
                                        case CPP_TOKEN_PARENTHESE_OPEN:
                                        {
                                            ++paren_level;
                                        } break;
                                        
                                        case CPP_TOKEN_PARENTHESE_CLOSE:
                                        {
                                            --paren_level;
                                        } break;
                                        
                                        case CPP_TOKEN_BRACE_OPEN:
                                        {
                                            ++brace_level;
                                        } break;
                                        
                                        case CPP_TOKEN_BRACE_CLOSE:
                                        {
                                            --brace_level;
                                        } break;
                                        
                                        case CPP_TOKEN_IDENTIFIER:
                                        {
                                            if((paren_level == 0) &&
                                               (brace_level == 0) &&
                                               (name_start == 0))
                                            {
                                                name_start = token.start;
                                                name_end = token.start + token.size;
                                                Token_Iterator probe = back;
                                                get_next_token(app, &probe);
                                                for(;;)
                                                {
                                                    Cpp_Token test = get_next_token(app, &probe);
                                                    if((test.type == 0) ||
                                                       (test.type == CPP_TOKEN_EOF) ||
                                                       (test.type == CPP_TOKEN_PARENTHESE_CLOSE) ||
                                                       (test.type == CPP_TOKEN_PARENTHESE_OPEN))
                                                    {
                                                        name_end = test.start;
                                                        break;
                                                    }
                                                }
                                            }
                                        } break;
                                    }
                                }
                            }
                            
                            if(name_start < name_end)
                            {
                                Indexed_Content_Element_Type type = ContentType_Function;
                                add_element_to_index(app, index, type, buffer, name_start,
                                                     name_start, name_end,
                                                     content_start, content_end);
                            }
                            
                            brace_level = 0;
                            paren_level = 0;
                        }
                    }
                    
                    ++brace_level;
                } break;
                
                case CPP_TOKEN_BRACE_CLOSE:
                {
                    --brace_level;
                } break;
            }
        }
    }
}

static void
add_all_buffers_to_index(Application_Links *app, Content_Index *index)
{
    for(Buffer_Summary buffer = get_buffer_first(app, AccessAll);
        buffer.exists;
        get_buffer_next(app, &buffer, AccessAll))
    {
        int32_t Unimportant = true;
        buffer_get_setting(app, &buffer, BufferSetting_Unimportant, &Unimportant);
        
        if(buffer.tokens_are_ready && !Unimportant)
        {
            add_buffer_to_index(app, index, &buffer);
        }
    }
}

static void
update_index_for_buffer(Application_Links *app, Content_Index *index, Buffer_Summary *buffer)
{
    remove_buffer_from_index(app, index, buffer->buffer_id);
    add_buffer_to_index(app, index, buffer);
}

static Content_Index *
get_global_content_index(Application_Links *app)
{
    Content_Index *result = global_content_index;
    if(!result)
    {
        global_content_index = create_content_index(app);
        add_all_buffers_to_index(app, global_content_index);
        result = global_content_index;
    }
    
    return(result);
}

static void
jump_to_element(Application_Links *app, View_Summary *view, Indexed_Content_Element *elem)
{
    if (view->buffer_id != elem->buffer_id)
    {
        view_set_buffer(app, view, elem->buffer_id, 0);
    }
    Buffer_Seek seek;
    seek.type = buffer_seek_pos;
    seek.pos = elem->last_known_location;;
    view_set_cursor(app, view, seek, true);
}

static void
jump_to_element_activate(Application_Links *app, Partition *scratch, Heap *heap,
                         View_Summary *view, Lister_State *state,
                         String text_field, void *user_data, bool32 activated_by_mouse)
{
    lister_default(app, scratch, heap, view, state, ListerActivation_Finished);
    if(user_data)
    {
        jump_to_element(app, view, (Indexed_Content_Element *)user_data);
    }
}

typedef bool32 element_type_predicate(Application_Links *app, Content_Index *index, Indexed_Content_Element *elem, void *user_data);

static bool32
element_is_definition(Application_Links *app, Content_Index *index, Indexed_Content_Element *elem, void *user_data)
{
    bool32 result = ((elem->type == ContentType_Function) ||
                     (elem->type == ContentType_Type) ||
                     (elem->type == ContentType_Macro) ||
                     (elem->type == ContentType_EnumValue));
    return(result);
}

static bool32
element_type_equals(Application_Links *app, Content_Index *index, Indexed_Content_Element *elem, void *user_data)
{
    bool32 result = (elem->type == (uint64_t)user_data);
    return(result);
}

static void
jump_to_element_lister(Application_Links *app, char *Label, element_type_predicate *predicate, void *user_data = 0)
{
    Partition *arena = &global_part;
    
    View_Summary view = get_active_view(app, AccessAll);
    view_end_ui_mode(app, &view);
    Temp_Memory temp = begin_temp_memory(arena);
    
    Content_Index *index = get_global_content_index(app);
    
    Lister_Option *options = push_array(arena, Lister_Option, index->element_count);
    int32_t option_index = 0;
    for(int hash_index = 0;
        hash_index <= ArrayCount(index->name_hash);
        ++hash_index)
    {
        for(Indexed_Content_Element *elem = index->name_hash[hash_index];
            elem;
            elem = elem->next_in_hash)
        {
            if(predicate(app, index, elem, user_data))
            {
                options[option_index].text_color = get_color(elem->color.fore);
                options[option_index].pop_color = get_color(CC_DefaultText);
                options[option_index].back_color = get_color(elem->color.back);
                options[option_index].string = elem->name;
                options[option_index].status = elem->content;
                options[option_index].user_data = elem;
                ++option_index;
            }
        }
    }
    begin_integrated_lister__basic_list(app, Label, jump_to_element_activate, 0, 0, options, option_index, index->total_string_space, &view);
    end_temp_memory(temp);
}
