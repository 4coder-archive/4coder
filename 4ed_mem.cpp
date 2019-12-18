/*
* Mr. 4th Dimention - Allen Webster
 *
 * 30.08.2016
 *
 * Replacements for common memory block managing functions.
 */

// TOP

// TODO(allen): Make these as fast as possible

#if 0
internal void
block_zero(void *a, u64 size){
    for (u8 *ptr = (u8*)a, *e = ptr + size; ptr < e; ptr += 1){
        *ptr = 0;
    }
}
internal void
block_fill_ones(void *a, u64 size){
    for (u8 *ptr = (u8*)a, *e = ptr + size; ptr < e; ptr += 1){
        *ptr = 0xFF;
    }
}
internal void
block_copy(void *dst, void *src, u64 size){
    for (u8 *d = (u8*)dst, *s = (u8*)src, *e = s + size; s < e; d += 1, s += 1){
        *d = *s;
    }
}
internal i32
block_compare(void *a, void *b, u64 size){
    for (u8 *aptr = (u8*)a, *bptr = (u8*)b, *e = bptr + size; bptr < e; aptr += 1, bptr += 1){
        i32 dif = (i32)*aptr - (i32)*bptr;
        if (dif != 0){
            return(dif > 0?1:-1);
        }
    }
    return(0);
}
internal void
block_fill_u8(void *a, u64 size, u8 val){
    for (u8 *ptr = (u8*)a, *e = ptr + size; ptr < e; ptr += 1){
        *ptr = val;
    }
}
internal void
block_fill_u16(void *a, u64 size, u16 val){
    Assert(size%sizeof(u16) == 0);
    u64 count = size/sizeof(u16);
    for (u16 *ptr = (u16*)a, *e = ptr + count; ptr < e; ptr += 1){
        *ptr = val;
    }
}
internal void
block_fill_u32(void *a, u64 size, u32 val){
    Assert(size%sizeof(u32) == 0);
    u64 count = size/sizeof(u32);
    for (u32 *ptr = (u32*)a, *e = ptr + count; ptr < e; ptr += 1){
        *ptr = val;
    }
}
internal void
block_fill_u64(void *a, u64 size, u64 val){
    Assert(size%sizeof(u64) == 0);
    u64 count = size/sizeof(u64);
    for (u64 *ptr = (u64*)a, *e = ptr + count; ptr < e; ptr += 1){
        *ptr = val;
    }
}

#define block_zero_struct(s) block_zero((s), sizeof(*(s)))
#endif

// BOTTOM

