/*
4coder_font_helper.cpp - Procedures for font setting operations
*/

// TOP

// TODO(allen): transition wrappers
static Face_Description
get_buffer_face_description(Application_Links *app, Buffer_ID buffer){
    Face_ID current_id = get_face_id(app, buffer);
    Face_Description description = {};
    if (current_id != 0){
        description = get_face_description(app, current_id);
    }
    return(description);
}

static Face_Description
get_global_face_description(Application_Links *app){
    Face_ID current_id = get_face_id(app, 0);
    Face_Description description = get_face_description(app, current_id);
    return(description);
}

static b32
descriptions_match(Face_Description *a, Face_Description *b){
    b32 result = false;
    if (string_match(SCchar(a->font.name), SCchar(b->font.name)) && a->font.in_local_font_folder == b->font.in_local_font_folder){
        if (memcmp((&a->pt_size), (&b->pt_size), sizeof(*a) - sizeof(a->font)) == 0){
            result = true;
        }
    }
    return(result);
}

static Face_ID
get_existing_face_id_matching_name(Application_Links *app, String_Const_u8 name){
    Face_ID largest_id = get_largest_face_id(app);
    Face_ID result = 0;
    for (Face_ID id = 1; id <= largest_id; ++id){
        Face_Description compare = get_face_description(app, id);
        if (string_match(SCu8(compare.font.name), name)){
            result = id;
            break;
        }
    }
    return(result);
}

static Face_ID
get_existing_face_id_matching_description(Application_Links *app, Face_Description *description){
    Face_ID largest_id = get_largest_face_id(app);
    Face_ID result = 0;
    for (Face_ID id = 1; id <= largest_id; ++id){
        Face_Description compare = get_face_description(app, id);
        if (descriptions_match(&compare, description)){
            result = id;
            break;
        }
    }
    return(result);
}

static Face_ID
get_face_id_by_name(Application_Links *app, String_Const_u8 name, Face_Description *base_description){
    Face_ID new_id = 0;
    name.size = clamp_top(name.size, sizeof(base_description->font.name) - 1);
    if (!string_match(name, SCu8(base_description->font.name))){
        new_id = get_existing_face_id_matching_name(app, name);
        if (new_id == 0){
            Face_Description description = *base_description;
            block_copy(description.font.name, name.str, name.size);
            description.font.name[name.size] = 0;
            description.font.in_local_font_folder = false;
            new_id = try_create_new_face(app, &description);
            if (new_id == 0){
                description.font.in_local_font_folder = true;
                new_id = try_create_new_face(app, &description);
            }
        }
    }
    return(new_id);
}

static Face_ID
get_face_id_by_description(Application_Links *app, Face_Description *description, Face_Description *base_description){
    Face_ID new_id = 0;
    if (!descriptions_match(description, base_description)){
        new_id = get_existing_face_id_matching_description(app, description);
        if (new_id == 0){
            new_id = try_create_new_face(app, description);
        }
    }
    return(new_id);
}

static void
set_global_face_by_name(Application_Links *app, String_Const_u8 name, b32 apply_to_all_buffers){
    Face_ID global_face_id = get_face_id(app, 0);
    Face_Description description = get_face_description(app, global_face_id);
    Face_ID new_id = get_face_id_by_name(app, name, &description);
    if (new_id != 0){
        set_global_face(app, new_id, apply_to_all_buffers);
    }
}

static void
change_global_face_by_description(Application_Links *app, Face_Description description, b32 apply_to_all_buffers){
    Face_ID face_id = get_face_id(app, 0);
    if (!try_modify_face(app, face_id, &description)){
        description.font.in_local_font_folder = !description.font.in_local_font_folder;
        try_modify_face(app, face_id, &description);
    }
}

static void
set_buffer_face_by_name(Application_Links *app, Buffer_ID buffer, String_Const_u8 name){
    Face_ID current_id = get_face_id(app, buffer);
    if (current_id != 0){
        Face_Description description = get_face_description(app, current_id);
        Face_ID new_id = get_face_id_by_name(app, name, &description);
        if (new_id != 0){
            buffer_set_face(app, buffer, new_id);
        }
    }
}

// BOTTOM

