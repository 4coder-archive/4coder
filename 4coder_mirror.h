/*
4coder_mirror.h - Types for the mirror buffer system.
*/

#if !defined(FCODER_MIRROR_H)
#define FCODER_MIRROR_H

struct Mirror_Range{
    Buffer_ID source_buffer_id;
    int32_t mirror_first;
    int32_t source_first;
    int32_t length;
};

////////////////////////////////

typedef int32_t Mirror_Mode;
enum{
    MirrorMode_Constructing,
    MirrorMode_Reflecting,
};

typedef uint32_t Mirror_Flags;
enum{
    MirrorFlag_NoHighlight             = 0x0,
    MirrorFlag_CharacterRangeHighlight = 0x1,
    MirrorFlag_LineRangeHighlight      = 0x2,
    MirrorFlag_UnusedHighlight         = 0x3,
    MirrorFlag_HighlightMask           = 0x3,
    MirrorFlag_NewlinesAreJumps        = 0x4,
};

struct Mirror{
    Buffer_ID mirror_buffer_id;
    Mirror_Mode mode;
    Mirror_Flags flags;
    Managed_Scope mirror_scope;
    int32_t count;
    int32_t max;
    Managed_Object source_buffer_ids;
    Managed_Object mirror_ranges;
    Managed_Object source_ranges;
};

struct Mirror_Hot{
    int32_t count;
    Buffer_ID *source_buffer_ids;
    Marker *mirror_ranges;
    Managed_Object *source_ranges;
};

////////////////////////////////

// The primary API for mirrors.

static bool32 mirror_init(Application_Links *app, Buffer_ID buffer, Mirror_Flags flags, Managed_Object *mirror_object_out);
static bool32 mirror_end(Application_Links *app, Managed_Object mirror);
static bool32 mirror_add_range(Application_Links *app, Managed_Object mirror, Buffer_ID source,
                               int32_t mirror_first, int32_t source_first, int32_t length);
static bool32 mirror_set_mode(Application_Links *app, Managed_Object mirror, Mirror_Mode mode);
static bool32 mirror_get_mode(Application_Links *app, Managed_Object mirror, Mirror_Mode *mode_out);
static bool32 mirror_set_flags(Application_Links *app, Managed_Object mirror, Mirror_Flags flags);
static bool32 mirror_get_flags(Application_Links *app, Managed_Object mirror, Mirror_Flags *flags_out);

////////////////////////////////

// Extra helpers for mirrors (all implemented on the primary API)

static bool32 mirror_buffer_create(Application_Links *app, String buffer_name, Mirror_Flags flags, Buffer_ID *mirror_buffer_id_out);
static bool32 mirror_buffer_end(Application_Links *app, Buffer_ID mirror);
static bool32 mirror_buffer_add_range_exact(Application_Links *app, Buffer_ID mirror, Buffer_ID source,
                                            int32_t mirror_first, int32_t source_first, int32_t length);
static bool32 mirror_buffer_add_range_loose(Application_Links *app, Buffer_ID mirror, Buffer_ID source,
                                            int32_t mirror_first, int32_t source_first, int32_t max_length);
static bool32 mirror_buffer_insert_range(Application_Links *app, Buffer_ID mirror, Buffer_ID source,
                                         int32_t mirror_insert_pos, int32_t source_first, int32_t length);
static bool32 mirror_buffer_set_mode(Application_Links *app, Buffer_ID mirror, Mirror_Mode mode);
static bool32 mirror_buffer_get_mode(Application_Links *app, Buffer_ID mirror, Mirror_Mode *mode_out);
static bool32 mirror_buffer_set_flags(Application_Links *app, Buffer_ID mirror, Mirror_Flags flags);
static bool32 mirror_buffer_get_flags(Application_Links *app, Buffer_ID mirror, Mirror_Flags *flags_out);

static bool32 mirror_buffer_refresh(Application_Links *app, Buffer_ID mirror);

static void   mirror_quick_sort_mirror_ranges(Mirror_Range *ranges, int32_t first, int32_t one_past_last);

static bool32 mirror_buffer_add_range_exact_array(Application_Links *app, Buffer_ID mirror, Mirror_Range *ranges, int32_t count);
static bool32 mirror_buffer_add_range_loose_array(Application_Links *app, Buffer_ID mirror, Mirror_Range *ranges, int32_t count);
static bool32 mirror_buffer_insert_range_array(Application_Links *app, Buffer_ID mirror, Mirror_Range *ranges, int32_t count);

#endif

// TOP
