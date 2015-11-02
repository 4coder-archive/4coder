/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.12.2014
 *
 * Application Layer for project codename "4ed"
 *
 */

#ifndef FRED_H
#define FRED_H

struct Partition{
    u8 *base;
    i32 pos, max;
};

inline Partition
partition_open(void *memory, i32 size){
    Partition partition;
    partition.base = (u8*)memory;;
    partition.pos = 0;
    partition.max = size;
    return partition;
}

inline void*
partition_allocate(Partition *data, i32 size){
    void *ret = 0;
    if (size > 0 && data->pos + size <= data->max){
        ret = data->base + data->pos;
        data->pos += size;
    }
    return ret;
}

inline void
partition_align(Partition *data, u32 boundary){
    data->pos = (data->pos + (boundary - 1)) & (~boundary);
}

inline void*
partition_current(Partition *data){
    return data->base + data->pos;
}

inline i32
partition_remaining(Partition *data){
    return data->max - data->pos;
}

inline Partition
partition_sub_part(Partition *data, i32 size){
    Partition result = {};
    void *d = partition_allocate(data, size);
    if (d) result = partition_open(d, size);
    return result;
}

#define push_struct(part, T) (T*)partition_allocate(part, sizeof(T))
#define push_array(part, T, size) (T*)partition_allocate(part, sizeof(T)*(size))
#define push_block(part, size) partition_allocate(part, size)

enum Memory_Bubble_Flag{
    MEM_BUBBLE_USED = 0x1,
    MEM_BUBBLE_DEBUG = 0xD3000000,
    MEM_BUBBLE_SYS_DEBUG = 0x5D000000,
    MEM_BUBBLE_DEBUG_MASK = 0xFF000000
};
struct Bubble{
    Bubble *prev;
    Bubble *next;
    u32 flags;
    i32 size;
    u32 type;
    u32 _unused_;
};
struct General_Memory{
    Bubble sentinel;
};

inline void
insert_bubble(Bubble *prev, Bubble *bubble){
    bubble->prev = prev;
    bubble->next = prev->next;
    bubble->prev->next = bubble;
    bubble->next->prev = bubble;
}

inline void
remove_bubble(Bubble *bubble){
    bubble->prev->next = bubble->next;
    bubble->next->prev = bubble->prev;
}

#if FRED_INTERNAL
#define MEM_BUBBLE_FLAG_INIT MEM_BUBBLE_DEBUG
#else
#define MEM_BUBBLE_FLAG_INIT 0
#endif

internal void
general_memory_open(General_Memory *general, void *memory, i32 size){
    general->sentinel.prev = &general->sentinel;
    general->sentinel.next = &general->sentinel;
    general->sentinel.flags = MEM_BUBBLE_USED;
    general->sentinel.size = 0;
    
    Bubble *first = (Bubble*)memory;
    first->flags = (u32)MEM_BUBBLE_FLAG_INIT;
    first->size = size - sizeof(Bubble);
    insert_bubble(&general->sentinel, first);
}

internal void
general_memory_check(General_Memory *general){
    Bubble *sentinel = &general->sentinel;
    for (Bubble *bubble = sentinel->next;
         bubble != sentinel;
         bubble = bubble->next){
        Assert(bubble);
        
        Bubble *next = bubble->next;
        Assert(bubble == next->prev);
        if (next != sentinel){
            Assert(bubble->next > bubble);
            Assert(bubble > bubble->prev);
            
            char *end_ptr = (char*)(bubble + 1) + bubble->size;
            char *next_ptr = (char*)next;
            Assert(end_ptr == next_ptr);
        }
    }
}

#define BUBBLE_MIN_SIZE 1024

internal void
general_memory_attempt_split(Bubble *bubble, i32 wanted_size){
    i32 remaining_size = bubble->size - wanted_size;
    if (remaining_size >= BUBBLE_MIN_SIZE){
        bubble->size = wanted_size;
        Bubble *new_bubble = (Bubble*)((u8*)(bubble + 1) + wanted_size);
        new_bubble->flags = (u32)MEM_BUBBLE_FLAG_INIT;
        new_bubble->size = remaining_size - sizeof(Bubble);
        insert_bubble(bubble, new_bubble);
    }
}

internal void*
general_memory_allocate(General_Memory *general, i32 size, u32 type = 0){
    void *result = 0;
    for (Bubble *bubble = general->sentinel.next;
         bubble != &general->sentinel;
         bubble = bubble->next){
        if (!(bubble->flags & MEM_BUBBLE_USED)){
            if (bubble->size >= size){
                result = bubble + 1;
                bubble->flags |= MEM_BUBBLE_USED;
                bubble->type = type;
                general_memory_attempt_split(bubble, size);
                break;
            }
        }
    }
    return result;
}

inline void
general_memory_do_merge(Bubble *left, Bubble *right){
    Assert(left->next == right);
    Assert(right->prev == left);
    left->size += sizeof(Bubble) + right->size;
    remove_bubble(right);
}

inline void
general_memory_attempt_merge(Bubble *left, Bubble *right){
    if (!(left->flags & MEM_BUBBLE_USED) &&
        !(right->flags & MEM_BUBBLE_USED)){
        general_memory_do_merge(left, right);
    }
}

internal void
general_memory_free(General_Memory *general, void *memory){
    Bubble *bubble = ((Bubble*)memory) - 1;
    Assert((!FRED_INTERNAL) || (bubble->flags & MEM_BUBBLE_DEBUG_MASK) == MEM_BUBBLE_DEBUG);
    bubble->flags &= ~MEM_BUBBLE_USED;
    bubble->type = 0;
    Bubble *prev, *next;
    prev = bubble->prev;
    next = bubble->next;
    general_memory_attempt_merge(bubble, next);
    general_memory_attempt_merge(prev, bubble);
}

internal void*
general_memory_reallocate(General_Memory *general, void *old, i32 old_size, i32 size, u32 type = 0){
    void *result = old;
    Bubble *bubble = ((Bubble*)old) - 1;
    bubble->type = type;
    Assert((!FRED_INTERNAL) || (bubble->flags & MEM_BUBBLE_DEBUG_MASK) == MEM_BUBBLE_DEBUG);
    i32 additional_space = size - bubble->size;
    if (additional_space > 0){
        Bubble *next = bubble->next;
        if (!(next->flags & MEM_BUBBLE_USED) &&
            next->size + sizeof(Bubble) >= additional_space){
            general_memory_do_merge(bubble, next);
            general_memory_attempt_split(bubble, size);
        }
        else{
            result = general_memory_allocate(general, size, type);
            if (old_size) memcpy(result, old, old_size);
            general_memory_free(general, old);
        }
    }
    return result;
}

inline void*
general_memory_reallocate_nocopy(General_Memory *general, void *old, i32 size, u32 type = 0){
    return general_memory_reallocate(general, old, 0, size, type);
}

struct Temp_Memory{
    Partition *part;
    i32 pos;
};

internal Temp_Memory
begin_temp_memory(Partition *data){
    Temp_Memory result;
    result.part = data;
    result.pos = data->pos;
    return result;
}

internal void
end_temp_memory(Temp_Memory temp){
    temp.part->pos = temp.pos;
}

struct Mem_Options{
    Partition part;
    General_Memory general;
};

#if SOFTWARE_RENDER
struct Render_Target{
	void *pixel_data;
	i32 width, height, pitch;
};
#else
struct Render_Target{
    void *handle;
    void *context;
    i32_Rect clip_boxes[5];
    i32 clip_top;
	i32 width, height;
    i32 bound_texture;
    u32 color;
};
#endif

struct Application_Memory{
    void *vars_memory;
    i32 vars_memory_size;
    void *target_memory;
    i32 target_memory_size;
};

#define KEY_INPUT_BUFFER_SIZE 4
#define KEY_INPUT_BUFFER_DSIZE (KEY_INPUT_BUFFER_SIZE << 1)

enum Key_Control{
	CONTROL_KEY_SHIFT,
	CONTROL_KEY_CONTROL,
	CONTROL_KEY_ALT,
	// always last
	CONTROL_KEY_COUNT
};

struct Key_Event_Data{
	u16 keycode;
	u16 loose_keycode;
	u16 character;
	u16 character_no_caps_lock;
};

struct Key_Input_Data{
	// NOTE(allen): keycodes here
	Key_Event_Data press[KEY_INPUT_BUFFER_SIZE];
	Key_Event_Data hold[KEY_INPUT_BUFFER_SIZE];
	i32 press_count;
    i32 hold_count;
	
	// NOTE(allen):
	//  true when the key is held down
	//  false when the key is not held down
	bool8 control_keys[CONTROL_KEY_COUNT];
	bool8 caps_lock;
};

struct Key_Summary{
    i32 count;
    Key_Event_Data keys[KEY_INPUT_BUFFER_DSIZE];
	bool8 modifiers[CONTROL_KEY_COUNT];
};

struct Key_Single{
    Key_Event_Data key;
    bool8 *modifiers;
};

inline Key_Single
get_single_key(Key_Summary *summary, i32 index){
    Assert(index >= 0 && index < summary->count);
    Key_Single key;
    key.key = summary->keys[index];
    key.modifiers = summary->modifiers;
    return key;
}

struct Mouse_State{
	bool32 out_of_window;
	bool32 left_button, right_button;
	bool32 left_button_prev, right_button_prev;
	i32 x, y;
	i16 wheel;
};

struct Mouse_Summary{
    i32 mx, my;
    bool32 l, r;
    bool32 press_l, press_r;
    bool32 release_l, release_r;
    bool32 out_of_window;
    bool32 wheel_used;
    i16 wheel_amount;
};

struct Input_Summary{
    Mouse_Summary mouse;
    Key_Summary keys;
    Key_Codes *codes;
};

// TODO(allen): This can go, and we can just use a String for it.
struct Clipboard_Contents{
	u8 *str;
	i32 size;
};

struct Thread_Context;

internal bool32
app_init(Thread_Context *thread,
		 Application_Memory *memory,
		 Key_Codes *lose_codes,
         Clipboard_Contents clipboard);

enum Application_Mouse_Cursor{
	APP_MOUSE_CURSOR_DEFAULT,
	APP_MOUSE_CURSOR_ARROW,
	APP_MOUSE_CURSOR_IBEAM,
	APP_MOUSE_CURSOR_LEFTRIGHT,
	APP_MOUSE_CURSOR_UPDOWN,
	// never below this
	APP_MOUSE_CURSOR_COUNT
};

struct Application_Step_Result{
	Application_Mouse_Cursor mouse_cursor_type;
	bool32 redraw;
};

internal Application_Step_Result
app_step(Thread_Context *thread,
		 Key_Codes *codes,
		 Key_Input_Data *input, Mouse_State *state,
		 bool32 time_step, Render_Target *target,
		 Application_Memory *memory,
		 Clipboard_Contents clipboard,
		 bool32 first_step, bool32 force_redraw);

#endif
