/*
 * 4coder_profile.cpp - Built in self profiling report.
 */

// TOP

function void
profile_init(Profile_Global_List *list){
    list->mutex = system_mutex_make();
    list->node_arena = make_arena_system(KB(4));
    list->disable_bits = ProfileEnable_UserBit;
}

function Profile_Thread*
prof__get_thread(Profile_Global_List *list, i32 thread_id){
    Profile_Thread *result = 0;
    for (Profile_Thread *node = list->first_thread;
         node != 0;
         node = node->next){
        if (thread_id == node->thread_id){
            result = node;
            break;
        }
    }
    if (result == 0){
        result = push_array_zero(&list->node_arena, Profile_Thread, 1);
        sll_queue_push(list->first_thread, list->last_thread, result);
        list->thread_count += 1;
        result->thread_id = thread_id;
    }
    return(result);
}

function void
profile_clear(Profile_Global_List *list){
    Mutex_Lock lock(list->mutex);
    for (Arena_Node *node = list->first_arena;
         node != 0;
         node = node->next){
        linalloc_clear(&node->arena);
    }
    list->first_arena = 0;
    list->last_arena = 0;
    
    linalloc_clear(&list->node_arena);
    list->first_thread = 0;
    list->last_thread = 0;
    list->thread_count = 0;
}

function void
profile_thread_flush(Thread_Context *tctx, Profile_Global_List *list){
    if (tctx->prof_record_count > 0){
        Mutex_Lock lock(list->mutex);
        if (list->disable_bits == 0){
            Profile_Thread* thread = prof__get_thread(list, system_thread_get_id());
            
            Arena_Node* node = push_array(&list->node_arena, Arena_Node, 1);
            sll_queue_push(list->first_arena, list->last_arena, node);
            node->arena = tctx->prof_arena;
            tctx->prof_arena = make_arena_system(KB(16));
            
            if (tctx->prof_first != 0){
            if (thread->first_record == 0){
                thread->first_record = tctx->prof_first;
                thread->last_record = tctx->prof_last;
            }
            else{
                thread->last_record->next = tctx->prof_first;
                thread->last_record = tctx->prof_last;
            }
            thread->record_count += tctx->prof_record_count;
            }
        }
        else{
            linalloc_clear(&tctx->prof_arena);
        }
        tctx->prof_record_count = 0;
        tctx->prof_first = 0;
        tctx->prof_last = 0;
    }
}

function void
profile_thread_set_name(Thread_Context *tctx, Profile_Global_List *list, String_Const_u8 name){
    Mutex_Lock lock(list->mutex);
    Profile_Thread* thread = prof__get_thread(list, system_thread_get_id());
    thread->name = name;
}

#define ProfileThreadName(tctx,list,name) profile_thread_set_name((tctx), (list), (name))

function void
profile_set_enabled(Profile_Global_List *list, b32 value, Profile_Enable_Flag flag){
    Mutex_Lock lock(list->mutex);
    if (value){
        RemFlag(list->disable_bits, flag);
    }
    else{
        AddFlag(list->disable_bits, flag);
    }
}

function void
thread_profile_record__inner(Thread_Context *tctx, Profile_ID id, u64 time,
                             String_Const_u8 name, String_Const_u8 location){
    Profile_Record *record = push_array_zero(&tctx->prof_arena, Profile_Record, 1);
    sll_queue_push(tctx->prof_first, tctx->prof_last, record);
    tctx->prof_record_count += 1;
    record->id = id;
    record->time = time;
    record->location = location;
    record->name = name;
}

function Profile_ID
thread_profile_record_push(Thread_Context *tctx, u64 time,
                           String_Const_u8 name, String_Const_u8 location){
    Profile_ID id = tctx->prof_id_counter;
    tctx->prof_id_counter += 1;
    thread_profile_record__inner(tctx, id, time, name, location);
    return(id);
}
function void
thread_profile_record_pop(Thread_Context *tctx, u64 time, Profile_ID id){
    Assert(tctx->prof_id_counter > 1);
    tctx->prof_id_counter = id;
    thread_profile_record__inner(tctx, id, time, SCu8(""), SCu8(""));
}

function Profile_ID
thread_profile_record_push(Application_Links *app, u64 time,
                           String_Const_u8 name, String_Const_u8 location){
    Thread_Context *tctx = get_thread_context(app);
    return(thread_profile_record_push(tctx, time, name, location));
}
function void
thread_profile_record_pop(Application_Links *app, u64 time, Profile_ID id){
    Thread_Context *tctx = get_thread_context(app);
    thread_profile_record_pop(tctx, time, id);
}

////////////////////////////////

function void
profile_block__init(Thread_Context *tctx, Profile_Global_List *list,
                    String_Const_u8 name, String_Const_u8 location, Profile_Block *block){
    block->tctx = tctx;
    block->list = list;
    block->is_closed = false;
    block->id = thread_profile_record_push(tctx, system_now_time(), name, location);
}
function void
profile_block__init(Thread_Context *tctx, Profile_Global_List *list,
                    String_Const_u8 name, String_Const_u8 location,
                    Profile_Scope_Block *block){
    block->tctx = tctx;
    block->list = list;
    block->is_closed = false;
    block->id = thread_profile_record_push(tctx, system_now_time(), name, location);
}

////////

Profile_Block::Profile_Block(Thread_Context *tctx, Profile_Global_List *list,
                             String_Const_u8 name, String_Const_u8 location){
    profile_block__init(tctx, list, name, location, this);
}
Profile_Block::Profile_Block(Application_Links *app, String_Const_u8 name,
                             String_Const_u8 location){
    Thread_Context *v_tctx = get_thread_context(app);
    Profile_Global_List *v_list = get_core_profile_list(app);
    profile_block__init(v_tctx, v_list, name, location, this);
}
Profile_Block::~Profile_Block(){
    this->close_now();
}
void
Profile_Block::close_now(){
    if (!this->is_closed){
        thread_profile_record_pop(this->tctx, system_now_time(), this->id);
        this->is_closed = true;
    }
}

////////

Profile_Scope_Block::Profile_Scope_Block(Thread_Context *tctx, Profile_Global_List *list,
                                         String_Const_u8 name, String_Const_u8 location){
    profile_block__init(tctx, list, name, location, this);
}
Profile_Scope_Block::Profile_Scope_Block(Application_Links *app, String_Const_u8 name,
                                         String_Const_u8 location){
    Thread_Context *v_tctx = get_thread_context(app);
    Profile_Global_List *v_list = get_core_profile_list(app);
    profile_block__init(v_tctx, v_list, name, location, this);
}
Profile_Scope_Block::~Profile_Scope_Block(){
    this->close_now();
    profile_thread_flush(this->tctx, this->list);
}
void
Profile_Scope_Block::close_now(){
    if (!this->is_closed){
        thread_profile_record_pop(this->tctx, system_now_time(), this->id);
        this->is_closed = true;
    }
}

////////////////////////////////

CUSTOM_COMMAND_SIG(profile_enable)
CUSTOM_DOC("Allow 4coder's self profiler to gather new profiling information.")
{
    Profile_Global_List *list = get_core_profile_list(app);
    profile_set_enabled(list, true, ProfileEnable_UserBit);
}

CUSTOM_COMMAND_SIG(profile_disable)
CUSTOM_DOC("Prevent 4coder's self profiler from gathering new profiling information.")
{
    Profile_Global_List *list = get_core_profile_list(app);
    profile_set_enabled(list, false, ProfileEnable_UserBit);
}

CUSTOM_COMMAND_SIG(profile_clear)
CUSTOM_DOC("Clear all profiling information from 4coder's self profiler.")
{
    Profile_Global_List *list = get_core_profile_list(app);
    profile_clear(list);
}

// BOTTOM
