/* Copyright (c) Edgeless Systems GmbH

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1335  USA */

#include <openenclave/ert_stubs.h>
#include <stdio.h>
#include <malloc.h>
#include <pthread.h>
#include <ucontext.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <sys/select.h>

ERT_STUB(backtrace_symbols_fd, 0)
ERT_STUB_SILENT(fedisableexcept, -1)
ERT_STUB_SILENT(gnu_dev_major, 0)
ERT_STUB_SILENT(gnu_dev_minor, 0)
ERT_STUB(mallinfo, 0)
ERT_STUB(pthread_yield, -1)
ERT_STUB(__fdelt_chk, 0)

int getcontext(ucontext_t *ucp) {
    (void)ucp;
    errno = ENOSYS;
    return -1;
}

int setcontext(const ucontext_t *ucp) {
    (void)ucp;
    errno = ENOSYS;
    return -1;
}

void makecontext(ucontext_t *ucp, void (*func)(void), int argc, ...) {
    // This is a stub implementation - does nothing
    // In a real implementation, this would set up the context to call func
    (void)ucp;
    (void)func;
    (void)argc;
}

int pthread_setname_np(pthread_t thread, const char *name) {
    // Silent stub - return success without doing anything
    (void)thread;
    (void)name;
    return 0;
}

// musl implements POSIX which returns int, but we
// compile mariadb with glibc which returns char*
// see man strerror
int strerror_r(int errnum, char *buf, size_t buflen) {
    (void)errnum;   // Mark as intentionally unused
    (void)buf;
    (void)buflen;
    return 0;
}


// GNU version of strerror_r that returns char*
// We provide this as __xpg_strerror_r and create an alias
char* __xpg_strerror_r(int errnum, char *buf, size_t buflen) {
    // Use the POSIX strerror_r that's already declared
    int ret = strerror_r(errnum, buf, buflen);
    if (ret == 0) {
        return buf;
    }
    return NULL;
}

// New stubs for newer glibc/libraries
char *__fgets_chk(char *s, size_t size, int n, FILE *stream) {
    // Handle negative n (invalid buffer size)
    if (n <= 0) {
        return NULL;  // Invalid buffer size
    }

    // Safe comparison after checking n > 0
    if (size > (size_t)n) {
        size = (size_t)n;
    }
    return fgets(s, size, stream);
}

// Define mallinfo2 struct since it might not be available in enclave
struct mallinfo2 {
    size_t arena;
    size_t ordblks;
    size_t smblks;
    size_t hblks;
    size_t hblkhd;
    size_t usmblks;
    size_t fsmblks;
    size_t uordblks;
    size_t fordblks;
    size_t keepcost;
};

struct mallinfo2 mallinfo2(void) {
    struct mallinfo2 info = {0};
    return info;
}

int pthread_cond_clockwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
                          clockid_t clockid, const struct timespec *abstime) {
    // Fallback to regular pthread_cond_timedwait
    (void)clockid;  // Mark parameter as intentionally unused
    return pthread_cond_timedwait(cond, mutex, abstime);
}

int swapcontext(ucontext_t *oucp, const ucontext_t *ucp) {
    (void)oucp;
    (void)ucp;
    errno = ENOSYS;
    return -1;
}
