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
ERT_STUB(getcontext, -1)
ERT_STUB_SILENT(gnu_dev_major, 0)
ERT_STUB_SILENT(gnu_dev_minor, 0)
ERT_STUB(makecontext, 0)
ERT_STUB(mallinfo, 0)
ERT_STUB_SILENT(pthread_setname_np, 0)
ERT_STUB(pthread_yield, -1)
ERT_STUB(setcontext, -1)
ERT_STUB(__fdelt_chk, 0)

// musl implements POSIX which returns int, but we
// compile mariadb with glibc which returns char*
// see man strerror
char* strerror_r(int err) {
  char* strerror();
  // sufficient for mariadb to just return strerror() here
  return strerror(err);
}
// musl defines this in strerror_r.c. We must also do it to prevent multiple definition error.
OE_WEAK_ALIAS(strerror_r, __xpg_strerror_r);

// New stubs for newer glibc/libraries
char *__fgets_chk(char *s, size_t size, int n, FILE *stream) {
    if (size > n) {
        size = n;
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
    return pthread_cond_timedwait(cond, mutex, abstime);
}

int swapcontext(ucontext_t *oucp, const ucontext_t *ucp) {
    errno = ENOSYS;
    return -1;
}

char *__xpg_strerror_r(int errnum, char *buf, size_t buflen) {
    // Fallback to regular strerror_r
    int ret = strerror_r(errnum, buf, buflen);
    if (ret == 0) {
        return buf;
    }
    return NULL;
}