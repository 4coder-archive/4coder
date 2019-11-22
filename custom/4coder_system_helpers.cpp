/*
 * 4coder_system_types.h - Implementation of universal (cross platform) helpers
 */

// TOP

function String_Const_u8
get_file_path_in_fonts_folder(Arena *arena, String_Const_u8 base_name){
    String_Const_u8 binary = system_get_path(arena, SystemPath_Binary);
    return(push_u8_stringf(arena, "%.*sfonts/%.*s", string_expand(binary), string_expand(base_name)));
}

////////////////////////////////

Mutex_Lock::Mutex_Lock(System_Mutex m){
    system_mutex_acquire(m);
    this->mutex = m;
}

Mutex_Lock::~Mutex_Lock(){
    system_mutex_release(this->mutex);
}

Mutex_Lock::operator System_Mutex(){
    return(this->mutex);
}

// BOTTOM

