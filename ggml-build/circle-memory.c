#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>

int posix_memalign(void **memptr, size_t alignment, size_t size)
{
    if (memptr == NULL) {
        return EINVAL;
    }

    if (alignment < sizeof(void *) ||
        (alignment & (alignment - 1)) != 0) {
        return EINVAL;
    }

    // aligned_alloc requires size to be a multiple of alignment.
    if (size > SIZE_MAX - (alignment - 1)) {
        return ENOMEM;
    }

    size_t rounded_size =
        (size + alignment - 1) & ~(alignment - 1);

    void *ptr = aligned_alloc(alignment, rounded_size);

    if (ptr == NULL) {
        return ENOMEM;
    }

    *memptr = ptr;
    return 0;
}
