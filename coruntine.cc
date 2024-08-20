#include <iostream>
#include <queue>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>

#include "coroutine.h"

struct CoroutineScheduler {
  struct Coroutine *current;
  std::queue<struct Coroutine *> q;
  struct Coroutine idle;

  struct Coroutine *pick_next(void) {
    if (q.empty()) {
      return nullptr;
    }
    auto ret = q.front();
    q.pop();
    return ret;
  }

  void activate(struct Coroutine* co) {
    q.push(co);
  }
};

// thread_local
struct CoroutineScheduler gCoroutineScheduler;

static struct Coroutine *get_current_coroutine(void) {
  return gCoroutineScheduler.current;
}

static void set_current_coroutine(struct Coroutine *routine) {
  gCoroutineScheduler.current = routine;
}

static struct Coroutine *co_pick_next(void) {
  return gCoroutineScheduler.pick_next();
}

static void __CoroutineDestroy(struct Coroutine *coroutine) {
  free(coroutine->stack);
  free(coroutine);
}

static void co_finish_yield(struct Coroutine* prev) {
  if (prev->state & COROUTINE_STATE_DEAD) {
    if (0 == __atomic_sub_fetch(&prev->refcount, 1, __ATOMIC_ACQ_REL)) {
      __CoroutineDestroy(prev);
    }
  } else if (prev->state & COROUTINE_STATE_SUSPENDED) {
    // std::cout << "Interruptible: " << prev << std::endl;
  } else {
    gCoroutineScheduler.activate(prev);
  }
}

void CoroutineYield(void) {
  struct Coroutine *next;
  struct Coroutine *prev;

  next = co_pick_next();
  if (!next) {
    return;
  }

  prev = get_current_coroutine();
  set_current_coroutine(next);

  prev = __co_yield_to_asm(prev, next);
  co_finish_yield(prev);
  // next->resumeArg;
}

void CoroutineExit(void *status) {
  struct Coroutine *co = get_current_coroutine();
  __atomic_or_fetch(&co->state, COROUTINE_STATE_DEAD, __ATOMIC_RELEASE);
  CoroutineYield();
}

// This function will be called by __co_start_asm
extern "C" void __co_main(struct Coroutine *prev, struct Coroutine *self) {
  co_finish_yield(prev);
  self->func(self->data);
  CoroutineExit(nullptr);
}

struct Coroutine *CoroutineCreate(void (*func)(void *), void *data,
                                  size_t stackSize) {
  struct Coroutine *co = (struct Coroutine *)malloc(sizeof(*co));
  if (!co) {
    return nullptr;
  }
  void *stack = malloc(stackSize);
  if (!stack) {
    free(co);
    return nullptr;
  }
  co->sp = (unsigned long)stack + stackSize;
  co->state = 0;

  // one for caller
  // one for code
  co->refcount = 2;

  co->stack = stack;
  co->func = func;
  co->data = data;

#if defined(__i386__)
  *((void **)(co->sp - 4)) = co;
  *((void **)(co->sp - 8)) = (void *)__co_start_asm;
  *((void **)(co->sp - 12)) = 0; // ebp
  *((void **)(co->sp - 16)) = 0; // ebx
  *((void **)(co->sp - 20)) = 0; // edi
  *((void **)(co->sp - 24)) = 0; // esi
  co->sp -= 6 * 4;
#elif defined(__x86_64__)
  *((void **)(co->sp - 1 * 8)) = co;
  *((void **)(co->sp - 2 * 8)) = (void *)__co_start_asm;
  *((void **)(co->sp - 3 * 8)) = 0; // %rbp
  *((void **)(co->sp - 4 * 8)) = 0; // %rbx
  *((void **)(co->sp - 5 * 8)) = 0; // %r12
  *((void **)(co->sp - 6 * 8)) = 0; // %r13
  *((void **)(co->sp - 7 * 8)) = 0; // %r14
  *((void **)(co->sp - 8 * 8)) = 0; // %r15
  co->sp -= 8 * 8;
#else
#error "Unsupported architecture"
#endif

  return co;
}

void CoroutineDestroy(struct Coroutine *coroutine) {
  if (0 == __atomic_sub_fetch(&coroutine->refcount, 1, __ATOMIC_ACQ_REL)) {
    __CoroutineDestroy(coroutine);
  }
}

static struct Coroutine initCoroutine = {
    .sp = 0,
};

void my_co1(void *data) {
  for (int i = 0; i < 10; ++i) {
    CoroutineYield();

    std::cerr << "== my_co1: " << data << std::endl;
    CoroutineYield();
  }

  // A.submit
  // B.submit
  // do
  //A.wait();
  //B.wait();
  // C.submit
}

void my_cox(void *data) {
  //std::cout << "my_cox ===>>>>>" << std::endl;
  std::cout << "===============" << std::endl;

  auto cox = CoroutineCreate(my_cox, (char *)data + 1);
  gCoroutineScheduler.activate(cox);

  CoroutineDestroy(cox);
}

int main(int argc, char **argv) {
  // colist.push(&main_co);
  gCoroutineScheduler.current = &initCoroutine;

  auto co0 = CoroutineCreate(&my_co1, (void *)0x1);
  auto co1 = CoroutineCreate(&my_co1, (void *)0x2);
  auto co2 = CoroutineCreate(&my_co1, (void *)0x3);
  auto co3 = CoroutineCreate(&my_cox, (void *)0x3);

  // for_each_avaiable_conn(conn)
  //   co.resume();
  // co.func() { read() parse(); ...; co.yield(INT); }

  gCoroutineScheduler.activate(co0);
  gCoroutineScheduler.activate(co1);
  gCoroutineScheduler.activate(co2);
  gCoroutineScheduler.activate(co3);

  while (true) {
    CoroutineYield();
  }

  return 0;
}

/*
 *
 * ret_ip
 * data
 * --->>>
 */
