/*
 * Mr. 4th Dimention - Allen Webster
 * 
 * 16.05.2015
 * 
 * Fascilities available for development but not intended for shipping.
 * 
 */

/*
 * Profiling
 */

#if FRED_INTERNAL == 1
enum Debug_Event_Type{
    DBGEV_START,
    DBGEV_END,
    DBGEV_MOMENT,
    // never below this
    DBGEV_COUNT
};

struct Debug_Event{
    i64 time;
    char *name;
    Debug_Event_Type type;
    i32 which_hit;
    i32 event_index;
    i32 thread_index;
};

struct Debug_Event_Array{
    volatile u32 count;
    Debug_Event e[512];
};

struct Profile_Frame{
    Debug_Event_Array events;
    i32 dbg_procing_start;
    i32 dbg_procing_end;
    i32 index;
    i32 first_key;
};

Profile_Frame profile_frame;

#define PAST_PROFILE_COUNT 30
Profile_Frame past_frames[PAST_PROFILE_COUNT];

extern const i32 INTERNAL_event_index_count;
extern u32 INTERNAL_event_hits[];
i64 INTERNAL_frame_start_time;

bool32 INTERNAL_collecting_events;

inline u32
post_debug_event(char *name, Debug_Event_Type type, i32 event_index, i32 thread_index, u32 which_hit){
    u32 result = 0;
    if (INTERNAL_collecting_events){
        u32 index =
            InterlockedIncrement(&profile_frame.events.count);
        --index;
    
        Assert(index < ArrayCount(profile_frame.events.e));
    
        Debug_Event ev;
        ev.time = system_time() - INTERNAL_frame_start_time;
        ev.name = name;
        ev.type = type;
        ev.event_index = event_index;
        ev.thread_index = thread_index;
    
        if (type == DBGEV_END){
            ev.which_hit = which_hit;
        }
        else{
            ev.which_hit = InterlockedIncrement(INTERNAL_event_hits + event_index) - 1;
        }
    
        profile_frame.events.e[index] = ev;
        result = ev.which_hit;
    }

    return result;
}

internal u32
quick_partition(Debug_Event *es, u32 start, u32 pivot){
    Debug_Event *p = es + pivot;
    
    i32 m = (start + pivot) >> 1;
    Swap(*p, es[m]);
    
    i32 pn = p->thread_index;
    i32 pe = p->event_index;
    i32 ph = p->which_hit;
    i32 pt = p->type;
    
    for (u32 i = start; i < pivot; ++i){
        Debug_Event *e = es + i;
        
        bool32 smaller = 0;
        
        if (e->thread_index < pn) smaller = 1;
        else if (e->thread_index == pn){
            if (e->type != DBGEV_MOMENT && pt == DBGEV_MOMENT) smaller = 1;
            else if (e->type != DBGEV_MOMENT){
                if (e->event_index < pe) smaller = 1;
                else if (e->event_index == pe){
                    if (e->which_hit < ph) smaller = 1;
                    else if (e->which_hit == ph){
                        if (e->type < pt) smaller = 1;
                    }
                }
            }
            else if (pt == DBGEV_MOMENT){
                if (e->time < p->time) smaller = 1;
            }
        }
        
        if (smaller){
            Swap(*e, es[start]);
            ++start;
        }
    }
    Swap(*p, es[start]);
    
    return start;
}

internal void
quick_sort(Debug_Event *e, u32 start, u32 pivot){
    u32 mid = quick_partition(e, start, pivot);
    if (start + 1 < mid) quick_sort(e, start, mid - 1);
    if (mid + 1 < pivot) quick_sort(e, mid + 1, pivot);
}

inline void
sort(Debug_Event_Array *events){
    quick_sort(events->e, 0, events->count - 1);
}

globalvar i32 INTERNAL_frame_index;
globalvar bool32 INTERNAL_updating_profile;

#define ProfileStart_(name, start, counter, hit, thread, n, c)\
    name = n; counter = c; start = system_time(); hit = post_debug_event(n, DBGEV_START, counter, thread, 0)
#define ProfileEnd_(name, start, counter, hit, thread) post_debug_event(name, DBGEV_END, counter, thread, hit)
#define ProfileMoment_(name, counter, thread) post_debug_event(name, DBGEV_MOMENT, counter, thread, 0)

struct INTERNAL_Profile_Block{
    char *name;
    i64 start;
    i32 counter;
    i32 thread;
    i32 hit;
    
    INTERNAL_Profile_Block(char *n, i32 c, i32 t){
        ProfileStart_(name, start, counter, hit, t, n, c);
        thread = t;
    }
    
    ~INTERNAL_Profile_Block(){
        ProfileEnd_(name, start, counter, hit, thread);
    }
};

#define ProfileBlock(name, thread) INTERNAL_Profile_Block name(#name, __COUNTER__, thread)
#define ProfileBlockFunction() INTERNAL_Profile_Block name(__FUNCTION__, __COUNTER__, 0)

#define ProfileStart(name) char *_pname_##name; i64 _pstart_##name; i32 _pcounter_##name; u32 _phit_##name; \
    ProfileStart_(_pname_##name, _pstart_##name, _pcounter_##name, _phit_##name, system_thread_get_id(thread), #name, __COUNTER__)

#define ProfileEnd(name) ProfileEnd_(_pname_##name, _pstart_##name, _pcounter_##name, _phit_##name, system_thread_get_id(thread))

#define ProfileMoment(name, thread) ProfileMoment_(#name, __COUNTER__, thread)
#define ProfileMomentFunction() ProfileMoment_(__FUNCTION__, __COUNTER__, 0)

#else

#define ProfileBlock(name)
#define ProfileStart(name)
#define ProfileEnd(name)
#define ProfileMoment(name)
#define ProfileMomentFunction()

#endif

