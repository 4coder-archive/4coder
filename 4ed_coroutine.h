/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 03.08.2019
 *
 * Coroutine implementation from thread+mutex+cv
 *
 */

// TOP

#if !defined(FRED_COROUTINE_H)
#define FRED_COROUTINE_H

typedef void Coroutine_Function(struct Coroutine *head);

typedef u32 Coroutine_State;
enum{
    CoroutineState_Dead,
    CoroutineState_Active,
    CoroutineState_Inactive,
    CoroutineState_Waiting,
};

typedef u32 Coroutine_Type;
enum{
    CoroutineType_Uninitialized,
    CoroutineType_Root,
    CoroutineType_Sub,
};

struct Coroutine{
    Coroutine *next;
    Thread_Context *tctx;
    void *in;
    void *out;
    System_Thread thread;
    System_Condition_Variable cv;
    struct Coroutine_Group *sys;
    Coroutine_Function *func;
    Coroutine *yield_ctx;
    Coroutine_State state;
    Coroutine_Type type;
    void *user_data;
};

struct Coroutine_Group{
    Arena arena;
    System_Mutex lock;
    System_Condition_Variable init_cv;
    b32 did_init;
    Coroutine *active;
    Coroutine *unused;
    Coroutine root;
};

////////////////////////////////

typedef i32 Coroutine_Pass_Control;
enum{
    CoroutinePassControl_ExitMe,
    CoroutinePassControl_BlockMe,
};

#endif

// BOTTOM
