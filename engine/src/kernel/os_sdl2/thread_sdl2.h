#include <cetech/kernel/os.h>

#include "include/SDL2/SDL.h"

#if defined(CETECH_LINUX)

#include <unistd.h>
#include <sched.h>

#endif

//! Create new thread
//! \param fce Thread fce
//! \param name Thread name
//! \param data Thread data
//! \return new thread
ct_thread_t *thread_create(ct_thread_fce_t fce,
                           const char *name,
                           void *data) {
    return (ct_thread_t *) SDL_CreateThread(fce, name, data);
}

//! Kill thread
//! \param thread thread
void thread_kill(ct_thread_t *thread) {
    SDL_DetachThread((SDL_Thread *) thread);
}

//! Wait for thread
//! \param thread Thread
//! \param status Thread exit status
void thread_wait(ct_thread_t *thread,
                 int *status) {
    SDL_WaitThread((SDL_Thread *) thread, status);
}

//! Get id for thread
//! \param thread Thread
//! \return ID
uint64_t thread_get_id(ct_thread_t *thread) {
    return SDL_GetThreadID((SDL_Thread *) thread);
}

//! Get actual thread id
//! \return Thread id
uint64_t thread_actual_id() {
    return SDL_ThreadID();
}

void thread_yield() {
#if defined(CETECH_DARWIN)
    sched_yield();
#elif defined(CETECH_LINUX)
    sched_yield();
#endif
}

void thread_spin_lock(ct_spinlock *lock) {
    SDL_AtomicLock((SDL_SpinLock *) lock);
}

void thread_spin_unlock(ct_spinlock *lock) {
    SDL_AtomicUnlock((SDL_SpinLock *) lock);
}
