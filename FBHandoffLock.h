//
//  FBHandoffLock.h
//  FBKVOController
//
//  Created by Mario Ströhlein on 30/12/15.
//  Copyright © 2015 Facebook, Inc. All rights reserved.
//
#import <mach/mach.h>
#import <mach/mach_types.h>
#import <mach/mach_traps.h>

typedef volatile uint64_t FBHandoffLock;
#define FB_HANDOFF_LOCK_STATE_LOCKED       1ull
#define FB_HANDOFF_LOCK_STATE_UNLOCKED     0ull

#define FB_HANDOFF_LOCK_MAXIMUM_SPINS      1000

#define FB_HANDOFF_LOCK_INIT              FB_HANDOFF_LOCK_STATE_UNLOCKED
#define FB_HANDOFF_LOCK_ALWAYS_INLINE   __attribute__((always_inline))

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"

FB_HANDOFF_LOCK_ALWAYS_INLINE
static inline void FBHandoffLockLock(FBHandoffLock* lock)
{
    uint32_t numberOfSpins;
    
    do
    {
        if(__builtin_expect(__sync_lock_test_and_set(lock, FB_HANDOFF_LOCK_STATE_LOCKED), FB_HANDOFF_LOCK_STATE_UNLOCKED) == FB_HANDOFF_LOCK_STATE_UNLOCKED)
        {
            break;
        }
        
        for(numberOfSpins = FB_HANDOFF_LOCK_MAXIMUM_SPINS; numberOfSpins; numberOfSpins--)
        {
            if(*lock == FB_HANDOFF_LOCK_STATE_UNLOCKED) continue;
        }
        
        thread_switch(THREAD_NULL, SWITCH_OPTION_DEPRESS, 1);
        // continue
        
    } while(YES);

}

FB_HANDOFF_LOCK_ALWAYS_INLINE
static inline void FBHandoffLockUnlock(FBHandoffLock* lock)
{
    __sync_lock_release(lock);
}

FB_HANDOFF_LOCK_ALWAYS_INLINE
static inline BOOL FBHandoffLockTryLock(FBHandoffLock* lock)
{
    return __sync_bool_compare_and_swap(lock, FB_HANDOFF_LOCK_STATE_UNLOCKED, FB_HANDOFF_LOCK_STATE_LOCKED);
}


#pragma clang diagnostic pop

