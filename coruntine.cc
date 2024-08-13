#include <iostream>
#include <queue>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>

enum CoroutineState {
  COROUTINE_STATE_DEAD = 0x01,
  COROUTINE_STATE_INTERRUPTIBLE = 0x02,
};

struct Coroutine {
  unsigned long esp;
  unsigned long state;

  void *stack;
  void (*func)(void *);
  void *data;

  // std::variant<int, double, float> resumeArg;
};

//void Resume(struct Coroutine* co, std::variant& arg) {
//    if (!(co->state & COROUTINE_STATE_INTERRUPTIBLE)) {
//        return;
//    }
//    co->resumeArg = arg;
//    co->state &= ~COROUTINE_STATE_INTERRUPTIBLE;
//    coSched.activate(co);
//}

extern "C" struct Coroutine *__co_yield_to(struct Coroutine *prev,
                                           struct Coroutine *next);

extern "C" void __co_start(void);

struct CoroutineScheduler {
  struct Coroutine *current;
  std::queue<struct Coroutine *> list;

  struct Coroutine *pick_next(void) {
    if (list.empty()) {
      return nullptr;
    }
    auto ret = list.front();
    list.pop();
    return ret;
  }

  void activate(struct Coroutine* co) {
    list.push(co);
  }
};

struct CoroutineScheduler coSched;

static struct Coroutine *co_pick_next(void) { return coSched.pick_next(); }

static void co_finish_yield(struct Coroutine* prev) {
  if (prev->state & COROUTINE_STATE_DEAD) {
    std::cout << "Free: " << prev << std::endl;
    free(prev->stack);
    free(prev);
  } else if (prev->state & COROUTINE_STATE_INTERRUPTIBLE) {
    std::cout << "Interruptible: " << prev << std::endl;
  } else {
    coSched.activate(prev);
  }
}

void yield(void) {
  struct Coroutine *next;
  struct Coroutine *prev;

  next = co_pick_next();
  if (!next) {
    return;
  }

  prev = coSched.current;
  coSched.current = next;

  prev = __co_yield_to(prev, next);
  co_finish_yield(prev);
  // next->resumeArg;
}

static void co_exit(void) {
  struct Coroutine *co = coSched.current;
  co->state |= COROUTINE_STATE_DEAD;
  yield();
}

extern "C" void __co_main(struct Coroutine *prev, struct Coroutine *self) {
  co_finish_yield(prev);
  self->func(self->data);
  co_exit();
}

struct Coroutine *CoroutineCreate(void (*func)(void *), void *data) {
  struct Coroutine *co = (struct Coroutine *)malloc(sizeof(*co));

  co->stack = malloc(1 << 20);
  co->esp = (unsigned long)co->stack + (1 << 20);
  co->func = func;
  co->data = data;
  co->state = 0;

  *((void **)(co->esp - 4)) = co;
  *((void **)(co->esp - 8)) = (void *)__co_start;
  *((void **)(co->esp - 12)) = 0; // esi
  *((void **)(co->esp - 16)) = 0; // edi
  *((void **)(co->esp - 20)) = 0; // ebx
  *((void **)(co->esp - 24)) = 0; // ebp

  co->esp -= 24;

  return co;
}

static struct Coroutine main_co = {
    .esp = 0,
};

void my_co1(void *data) {
  for (int i = 0; i < 10; ++i) {
    std::cout << "= my_co1: " << data << std::endl;
    // sleep(0.1);
    yield();

    std::cout << "== my_co1: " << data << std::endl;
    // sleep(0.1);
    yield();
  }
}

void my_cox(void *data) {
  std::cout << "my_cox ===>>>>>" << std::endl;
  std::cout << "===============" << std::endl;

  auto cox = CoroutineCreate(my_cox, (char *)data + 1);
  coSched.activate(cox);
}

int main(int argc, char **argv) {
  // colist.push(&main_co);
  coSched.current = &main_co;

  auto co0 = CoroutineCreate(&my_co1, (void *)0x1);
  auto co1 = CoroutineCreate(&my_co1, (void *)0x2);
  auto co2 = CoroutineCreate(&my_co1, (void *)0x3);
  auto co3 = CoroutineCreate(&my_cox, (void *)0x3);

  coSched.activate(co0);
  coSched.activate(co1);
  coSched.activate(co2);
  coSched.activate(co3);

  while (true) {
    // std::cout << "main_co: " << std::endl;
    yield();
  }

  return 0;
}

/*
 *
 * ret_ip
 * data
 * --->>>
 */
