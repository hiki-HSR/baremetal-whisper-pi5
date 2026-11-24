#include <pthread.h>
#include <__external_threading>
#include <cerrno>
#include <cstdint>
#include <new>

namespace {

constexpr unsigned MAX_THREADS = 64;
constexpr unsigned MAX_MUTEXES = 128;
constexpr unsigned MAX_CONDS   = 128;

struct ThreadSlot {
    bool used = false;
    std::__libcpp_thread_t native{};
    void *result = nullptr;
    void *(*function)(void *) = nullptr;
    void *argument = nullptr;
};

struct MutexSlot {
    std::__libcpp_mutex_t native{};
};

struct CondSlot {
    std::__libcpp_condvar_t native{};
};

ThreadSlot threads[MAX_THREADS];
MutexSlot *mutexes[MAX_MUTEXES] = {};
CondSlot *conds[MAX_CONDS] = {};

template <typename T, unsigned N>
unsigned allocate_slot(T *(&slots)[N], T *value) {
    for (unsigned i = 0; i < N; ++i) {
        if (slots[i] == nullptr) {
            slots[i] = value;
            return i + 1;
        }
    }
    return 0;
}

template <typename T, unsigned N>
T *lookup(T *(&slots)[N], unsigned id) {
    if (id == 0 || id > N) {
        return nullptr;
    }
    return slots[id - 1];
}

void *thread_entry(void *arg) {
    ThreadSlot *slot = static_cast<ThreadSlot *>(arg);
    slot->result = slot->function(slot->argument);
    return slot->result;
}

} // namespace

extern "C" {

int pthread_mutex_init(
    pthread_mutex_t *m,
    const pthread_mutexattr_t *attr
) {
    if (!m || attr) return EINVAL;

    auto *slot = new (std::nothrow) MutexSlot{};
    if (!slot) return ENOMEM;

    unsigned id = allocate_slot(mutexes, slot);

    if (!id) {
        delete slot;
        return EAGAIN;
    }

    *m = static_cast<pthread_mutex_t>(id);
    return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *m) {
    if (!m) return EINVAL;

    unsigned id = static_cast<unsigned>(*m);
    auto *slot = lookup(mutexes, id);
    if (!slot) return EINVAL;

    std::__libcpp_mutex_destroy(&slot->native);
    mutexes[id - 1] = nullptr;
    delete slot;
    *m = 0;
    return 0;
}

int pthread_mutex_lock(pthread_mutex_t *m) {
    if (!m) return EINVAL;

    auto *slot = lookup(mutexes, static_cast<unsigned>(*m));
    if (!slot) return EINVAL;

    return std::__libcpp_mutex_lock(&slot->native);
}

int pthread_mutex_unlock(pthread_mutex_t *m) {
    if (!m) return EINVAL;

    auto *slot = lookup(mutexes, static_cast<unsigned>(*m));
    if (!slot) return EINVAL;

    return std::__libcpp_mutex_unlock(&slot->native);
}

int pthread_cond_init(
    pthread_cond_t *c,
    const pthread_condattr_t *attr
) {
    if (!c || attr) return EINVAL;

    auto *slot = new (std::nothrow) CondSlot{};
    if (!slot) return ENOMEM;

    unsigned id = allocate_slot(conds, slot);

    if (!id) {
        delete slot;
        return EAGAIN;
    }

    *c = static_cast<pthread_cond_t>(id);
    return 0;
}

int pthread_cond_destroy(pthread_cond_t *c) {
    if (!c) return EINVAL;

    unsigned id = static_cast<unsigned>(*c);
    auto *slot = lookup(conds, id);
    if (!slot) return EINVAL;

    std::__libcpp_condvar_destroy(&slot->native);
    conds[id - 1] = nullptr;
    delete slot;
    *c = 0;
    return 0;
}

int pthread_cond_wait(
    pthread_cond_t *c,
    pthread_mutex_t *m
) {
    if (!c || !m) return EINVAL;

    auto *cv = lookup(conds, static_cast<unsigned>(*c));
    auto *mx = lookup(mutexes, static_cast<unsigned>(*m));

    if (!cv || !mx) return EINVAL;

    return std::__libcpp_condvar_wait(
        &cv->native,
        &mx->native
    );
}

int pthread_cond_broadcast(pthread_cond_t *c) {
    if (!c) return EINVAL;

    auto *slot = lookup(conds, static_cast<unsigned>(*c));
    if (!slot) return EINVAL;

    return std::__libcpp_condvar_broadcast(&slot->native);
}

int pthread_create(
    pthread_t *t,
    const pthread_attr_t *attr,
    void *(*start)(void *),
    void *arg
) {
    if (!t || !start || attr) return EINVAL;

    for (unsigned i = 0; i < MAX_THREADS; ++i) {
        if (threads[i].used) continue;

        ThreadSlot &slot = threads[i];
        slot.used = true;
        slot.function = start;
        slot.argument = arg;
        slot.result = nullptr;
        slot.native = {};

        int rc = std::__libcpp_thread_create(
            &slot.native,
            thread_entry,
            &slot
        );

        if (rc != 0) {
            slot.used = false;
            return rc;
        }

        *t = static_cast<pthread_t>(i + 1);
        return 0;
    }

    return EAGAIN;
}

int pthread_join(pthread_t t, void **result) {
    unsigned id = static_cast<unsigned>(t);

    if (id == 0 || id > MAX_THREADS) return EINVAL;

    ThreadSlot &slot = threads[id - 1];

    if (!slot.used) return EINVAL;

    int rc = std::__libcpp_thread_join(&slot.native);
    if (rc != 0) return rc;

    if (result) {
        *result = slot.result;
    }

    slot.used = false;
    return 0;
}

} // extern "C"
