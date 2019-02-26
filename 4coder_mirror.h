/*
4coder_mirror.h - Types for the mirror buffer system.
*/

#if !defined(FCODER_MIRROR_H)
#define FCODER_MIRROR_H

struct Mirror_Range{
    Buffer_ID source_buffer_id;
    i32 mirror_first;
    i32 source_first;
    i32 length;
};

////////////////////////////////

typedef i32 Mirror_Mode;
enum{
    MirrorMode_Constructing,
    MirrorMode_Reflecting,
};

typedef u32 Mirror_Flags;
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
    i32 count;
    i32 max;
    Managed_Object source_buffer_ids;
    Managed_Object mirror_ranges;
    Managed_Object source_ranges;
};

struct Mirror_Hot{
    i32 count;
    Buffer_ID *source_buffer_ids;
    Marker *mirror_ranges;
    Managed_Object *source_ranges;
};

////////////////////////////////

// The primary API for mirrors.

static b32  mirror_init(Application_Links *app, Buffer_ID buffer, Mirror_Flags flags, Managed_Object *mirror_object_out);
static b32  mirror_end(Application_Links *app, Managed_Object mirror);
static b32  mirror_add_range(Application_Links *app, Managed_Object mirror, Buffer_ID source,
                             i32 mirror_first, i32 source_first, i32 length);
static b32  mirror_set_mode(Application_Links *app, Managed_Object mirror, Mirror_Mode mode);
static b32  mirror_get_mode(Application_Links *app, Managed_Object mirror, Mirror_Mode *mode_out);
static b32  mirror_set_flags(Application_Links *app, Managed_Object mirror, Mirror_Flags flags);
static b32  mirror_get_flags(Application_Links *app, Managed_Object mirror, Mirror_Flags *flags_out);

////////////////////////////////

// Extra helpers for mirrors (all implemented on the primary API)

static b32  mirror_buffer_create(Application_Links *app, String buffer_name, Mirror_Flags flags, Buffer_ID *mirror_buffer_id_out);
static b32  mirror_buffer_end(Application_Links *app, Buffer_ID mirror);
static b32  mirror_buffer_add_range_exact(Application_Links *app, Buffer_ID mirror, Buffer_ID source,
                                          i32 mirror_first, i32 source_first, i32 length);
static b32  mirror_buffer_add_range_loose(Application_Links *app, Buffer_ID mirror, Buffer_ID source,
                                          i32 mirror_first, i32 source_first, i32 max_length);
static b32  mirror_buffer_insert_range(Application_Links *app, Buffer_ID mirror, Buffer_ID source,
                                       i32 mirror_insert_pos, i32 source_first, i32 length);
static b32  mirror_buffer_set_mode(Application_Links *app, Buffer_ID mirror, Mirror_Mode mode);
static b32  mirror_buffer_get_mode(Application_Links *app, Buffer_ID mirror, Mirror_Mode *mode_out);
static b32  mirror_buffer_set_flags(Application_Links *app, Buffer_ID mirror, Mirror_Flags flags);
static b32  mirror_buffer_get_flags(Application_Links *app, Buffer_ID mirror, Mirror_Flags *flags_out);

static b32  mirror_buffer_refresh(Application_Links *app, Buffer_ID mirror);

static void mirror_quick_sort_mirror_ranges(Mirror_Range *ranges, i32 first, i32 one_past_last);

static b32  mirror_buffer_add_range_exact_array(Application_Links *app, Buffer_ID mirror, Mirror_Range *ranges, i32 count);
static b32  mirror_buffer_add_range_loose_array(Application_Links *app, Buffer_ID mirror, Mirror_Range *ranges, i32 count);
static b32  mirror_buffer_insert_range_array(Application_Links *app, Buffer_ID mirror, Mirror_Range *ranges, i32 count);

#endif

// TOP
