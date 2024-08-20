#ifndef _COROUTINE_H_
#define _COROUTINE_H_
#include <stddef.h>
#include <stdint.h>

enum CoroutineState {
  COROUTINE_STATE_DEAD = 0x01,
  COROUTINE_STATE_SUSPENDED = 0x02,
};

struct CoroutineScheduler;

struct Coroutine {
  uintptr_t sp;

  int state;
  int refcount;

  struct CoroutineScheduler *scheduler;

  void *stack;
  void (*func)(void *);
  void *data;

  // std::variant<int, double, float> resumeArg;
};

/**
 * @brief Yield to \p next Coroutine.
 * @note This is implemented by assembly code.
 * @return Return the previous Coroutine.
 * @sa 
 ****************************************************/
extern "C" struct Coroutine *__co_yield_to_asm(struct Coroutine *prev,
                                               struct Coroutine *next);

/**
 * @brief The init routine of \b Coroutine.
 * @note This is implemented by assembly code.
 ****************************************************/
extern "C" void __co_start_asm(void);

struct Coroutine *CoroutineCreate(void (*func)(void *), void *data,
                                  size_t stackSize = 4096);

void CoroutineDestroy(struct Coroutine *coroutine);

void CoroutineYield(void);
void CoroutineExit(void *status);

#endif // _COROUTINE_H_
