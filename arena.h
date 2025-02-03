#ifndef ARENA_H_
#define ARENA_H_

// !!!ARENE NISU LINKOVANE MORAS IH POVEZAT DA SE AUTOMACKI RESIZE
// BACKBUFFER JE SAMO MALLOC MMAP ILI STACK LIST
#define DEFAULT_ALLIGMENT (2*sizeof(void*))// macro za c ABI size 64 bita ako je na neki drugi procesor

#include "stdint.h"
#include "stdint.h"
#include "stddef.h"
#include "assert.h"
#include "string.h"
#include "stdio.h"
typedef struct Arena{
    unsigned char *buffer;
    size_t size;
    size_t currOffset;
    size_t prevOffset;
} Arena;

typedef struct TempArena{
    Arena *arena;
    size_t saveCurr;
    size_t savePrev;
}TempArena;

void InitArena(Arena *arena,void *backbuffer,size_t backbufferSize);

bool IsPowOfTwo(uintptr_t x);

void *ArenaAllocatorAlign(Arena *arena,size_t size,size_t alignment);

void *ArenaAllocator(Arena *arena,size_t size);

void *ResizeArenaAlign(Arena*arena,void *oldMem,size_t oldMemSize,size_t align);

void *ResizeArena(Arena*arena,void *oldMem,size_t oldMemSize,size_t align);

void ArenaFree(Arena *arena);

TempArena StartTemp(Arena *arena);
void StopTemp(TempArena tempAren);

#endif// ARENA_H_
inline bool IsPowOfTwo(uintptr_t x){
    return (x&(x-1))==0;
}

inline uintptr_t AlignFor(size_t align,uintptr_t ptr){
    uintptr_t a,p,mod;
    assert(IsPowOfTwo(align)); 
    p = ptr;
    a = (uintptr_t)align;
    mod = p&(a-1); //bit operacija magija 
    if(mod !=0){
        p += a- mod;
    }
    return p;
}

inline void *ArenaAllocatorAlign(Arena *arena,size_t size,size_t alignment){
    uintptr_t curr_ptr = (uintptr_t)arena->buffer+ (uintptr_t)arena->currOffset;
    uintptr_t offset =AlignFor(alignment, curr_ptr);
    offset -= (uintptr_t)arena->buffer;
    if(offset+size <= arena->size){
        void *ptr = &arena->buffer[offset];
        arena->prevOffset = offset;
        memset(ptr, 0, size);
        return ptr;
    }
    return NULL;
}
inline void* ArenaAllocator(Arena *arena,size_t size){
    return ArenaAllocatorAlign(arena, size, DEFAULT_ALLIGMENT);
}

inline void *resize_arena_align(Arena *arena,void * oldmem,size_t old_size,size_t newsize,size_t align){
    unsigned char *old_mem = (unsigned char*)oldmem;
    assert(IsPowOfTwo(align));
    if(old_mem == NULL || old_size == 0){
        return ArenaAllocatorAlign(arena, newsize, align);
    }else if (arena->buffer <= old_mem && old_mem < arena->buffer + arena->size){
        if (arena->buffer+arena->prevOffset== old_mem) {
            arena->currOffset= arena->prevOffset+ newsize; 
            if(newsize>old_size){
                memset(&arena->buffer[arena->currOffset], 0, newsize-old_size);
            }
            return old_mem;
        } else{
            void *new_memeory = ArenaAllocatorAlign(arena, newsize, align);
            size_t copy_size = old_size <newsize ?old_size:newsize;
            memmove(new_memeory, old_mem, copy_size);
            return  new_memeory;
        }
    }else {
        printf("error on resize of arena\n");
        return NULL;
    }
}
inline void *ResizeArena(Arena*arena,void *oldMem,size_t oldMemSize,size_t align){
    return ResizeArenaAlign(arena, oldMem, oldMemSize, DEFAULT_ALLIGMENT);
}

inline void ArenaFree(Arena *arena){
    arena->currOffset= 0;
    arena->prevOffset=0;
}

inline TempArena StartTemp(Arena *arena){
    TempArena t; 
    t.arena =arena;
    t.saveCurr = arena->currOffset;
    t.savePrev = arena->prevOffset;
    return t;
}
inline void StopTemp(TempArena TempArena){
    TempArena.arena->prevOffset = TempArena.saveCurr;
    TempArena.arena->currOffset= TempArena.saveCurr;
}
