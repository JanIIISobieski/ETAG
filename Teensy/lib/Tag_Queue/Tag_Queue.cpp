#include "Tag_Queue.h"

void Tag_Queue::enqueue(volatile uint8_t* ptr) {
    __disable_irq();
    queue[to_write] = ptr;
    ++run;
    next(to_write);
    __enable_irq();
}

volatile uint8_t* Tag_Queue::dequeue() {
    __disable_irq();
    popped = queue[to_pop];
    next(to_pop);
    --run;
    __enable_irq();
    return popped;
}

inline void Tag_Queue::next(uint8_t &ind) {
    if (++ind == size)
        ind = 0;
}

inline void Tag_Queue::next(volatile uint8_t &ind) {
    if (++ind == size)
        ind = 0;
}

void Tag_Queue::reset() {
    popped = nullptr;

    run = 0;
    to_pop = 0;
    to_write = 0;

    for (size_t i = 0; i < size; i++) {
        queue[i] = nullptr;
    }
}
