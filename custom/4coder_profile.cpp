/*
 * 4coder_profile.cpp - Built in self profiling report.
 */

// TOP

global Profile_Global_List global_prof_list = {};
global System_Mutex global_prof_mutex = {};

function void
global_prof_init(void){
    global_prof_mutex = system_mutex_make();
    global_prof_list.node_arena = make_arena(get_base_allocator_system(),
                                             KB(4));
}

function Profile_Thread*
global_prof_get_thread(i32 thread_id){
    Profile_Thread *result = 0;
    for (Profile_Thread *node = global_prof_list.first_thread;
         node != 0;
         node = node->next){
        if (thread_id == node->thread_id){
            result = node;
            break;
        }
    }
    if (result == 0){
        result = push_array_zero(&global_prof_list.node_arena, Profile_Thread, 1);
        sll_queue_push(global_prof_list.first_thread, global_prof_list.last_thread, result);
        global_prof_list.thread_count += 1;
        result->thread_id = thread_id;
    }
    return(result);
}

function void
global_prof_clear(void){
    Mutex_Lock lock(global_prof_mutex);
    for (Arena_Node *node = global_prof_list.first_arena;
         node != 0;
         node = node->next){
        linalloc_clear(&node->arena);
    }
    global_prof_list.first_arena = 0;
    global_prof_list.last_arena = 0;
    
    linalloc_clear(&global_prof_list.node_arena);
    global_prof_list.first_thread = 0;
    global_prof_list.last_thread = 0;
    global_prof_list.thread_count = 0;
}

function void
thread_profile_flush(Thread_Context *tctx){
    if (tctx->prof_record_count > 0){
        Mutex_Lock lock(global_prof_mutex);
        if (global_prof_list.disable_bits == 0){
            Profile_Thread* thread = global_prof_get_thread(system_thread_get_id());
            
            Arena_Node* node = push_array(&global_prof_list.node_arena, Arena_Node, 1);
            sll_queue_push(global_prof_list.first_arena, global_prof_list.last_arena,
                           node);
            node->arena = tctx->prof_arena;
            tctx->prof_arena = make_arena(get_base_allocator_system(), KB(4));
            
            if (thread->first_record == 0){
                thread->first_record = tctx->prof_first;
                thread->last_record = tctx->prof_last;
            }
            else{
                thread->last_record->next = tctx->prof_first;
                thread->last_record = tctx->prof_last;
            }
            thread->record_count += tctx->prof_record_count;
            
            tctx->prof_record_count = 0;
            tctx->prof_first = 0;
            tctx->prof_last = 0;
        }
    }
}

function void
global_prof_set_enabled(b32 value, Profile_Enable_Flag flag){
    Mutex_Lock lock(global_prof_mutex);
    if (value){
        RemFlag(global_prof_list.disable_bits, flag);
    }
    else{
        AddFlag(global_prof_list.disable_bits, flag);
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
profile_block__init(Thread_Context *tctx, String_Const_u8 name,
                    String_Const_u8 location, Profile_Block *block){
    block->tctx = tctx;
    block->is_closed = false;
    block->id = thread_profile_record_push(tctx, system_now_time(), name, location);
}
function void
profile_block__init(Thread_Context *tctx, String_Const_u8 name,
                    String_Const_u8 location, Profile_Scope_Block *block){
    block->tctx = tctx;
    block->is_closed = false;
    block->id = thread_profile_record_push(tctx, system_now_time(), name, location);
}

////////

Profile_Block::Profile_Block(Thread_Context *tctx, String_Const_u8 name,
                             String_Const_u8 location){
    profile_block__init(tctx, name, location, this);
}
Profile_Block::Profile_Block(Application_Links *app, String_Const_u8 name,
                             String_Const_u8 location){
    profile_block__init(get_thread_context(app), name, location, this);
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

Profile_Scope_Block::Profile_Scope_Block(Thread_Context *tctx, String_Const_u8 name,
                                         String_Const_u8 location){
    profile_block__init(tctx, name, location, this);
}
Profile_Scope_Block::Profile_Scope_Block(Application_Links *app, String_Const_u8 name,
                                         String_Const_u8 location){
    profile_block__init(get_thread_context(app), name, location, this);
}
Profile_Scope_Block::~Profile_Scope_Block(){
    this->close_now();
    thread_profile_flush(this->tctx);
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
    global_prof_set_enabled(true, ProfileEnable_UserBit);
}

CUSTOM_COMMAND_SIG(profile_disable)
CUSTOM_DOC("Prevent 4coder's self profiler from gathering new profiling information.")
{
    global_prof_set_enabled(false, ProfileEnable_UserBit);
}

CUSTOM_COMMAND_SIG(profile_clear)
CUSTOM_DOC("Clear all profiling information from 4coder's self profiler.")
{
    global_prof_clear();
}

// BOTTOM
