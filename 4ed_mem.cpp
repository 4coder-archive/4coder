/*
* Mr. 4th Dimention - Allen Webster
 *
 * 30.08.2016
 *
 * Replacements for common memory block managing functions.
 */

// TOP

// TODO(allen): Make these as fast as possible

internal void
block_zero(void *a, umem_4tech size){
    for (u8_4tech *ptr = (u8_4tech*)a, *e = ptr + size; ptr < e; ptr += 1){
        *ptr = 0;
    }
}

#define block_zero_struct(s) block_zero((s), sizeof(*(s)))

internal void
block_fill_ones(void *a, umem_4tech size){
    for (u8_4tech *ptr = (u8_4tech*)a, *e = ptr + size; ptr < e; ptr += 1){
        *ptr = 0xFF;
    }
}

internal void
block_copy(void *dst, void *src, umem_4tech size){
    for (u8_4tech *d = (u8_4tech*)dst, *s = (u8_4tech*)src, *e = s + size; s < e; d += 1, s += 1){
        *d = *s;
    }
}

internal i32_4tech
block_compare(void *a, void *b, umem_4tech size){
    for (u8_4tech *aptr = (u8_4tech*)a, *bptr = (u8_4tech*)b, *e = bptr + size; bptr < e; aptr += 1, bptr += 1){
        i32_4tech dif = (i32_4tech)*aptr - (i32_4tech)*bptr;
        if (dif != 0){
            return(dif > 0?1:-1);
        }
    }
    return(0);
}

internal void
block_fill_u8_4tech(void *a, umem_4tech size, u8_4tech val){
    for (u8_4tech *ptr = (u8_4tech*)a, *e = ptr + size; ptr < e; ptr += 1){
        *ptr = val;
    }
}

internal void
block_fill_u16(void *a, umem_4tech size, u16_4tech val){
    Assert(size%sizeof(u16_4tech) == 0);
    umem_4tech count = size/sizeof(u16_4tech);
    for (u16_4tech *ptr = (u16_4tech*)a, *e = ptr + count; ptr < e; ptr += 1){
        *ptr = val;
    }
}

internal void
block_fill_u32(void *a, umem_4tech size, u32_4tech val){
    Assert(size%sizeof(u32_4tech) == 0);
    umem_4tech count = size/sizeof(u32_4tech);
    for (u32_4tech *ptr = (u32_4tech*)a, *e = ptr + count; ptr < e; ptr += 1){
        *ptr = val;
    }
}

internal void
block_fill_u64(void *a, umem_4tech size, u64_4tech val){
    Assert(size%sizeof(u64_4tech) == 0);
    umem_4tech count = size/sizeof(u64_4tech);
    for (u64_4tech *ptr = (u64_4tech*)a, *e = ptr + count; ptr < e; ptr += 1){
        *ptr = val;
    }
}

// BOTTOM

