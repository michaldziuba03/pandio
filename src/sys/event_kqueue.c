/* Copyright (c) Michał Dziuba
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "core.h"
#include "timers.h"
#include <fcntl.h>

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


uint64_t pd_now() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    return now.tv_sec * 1000 + now.tv_nsec / 1000000;
}


int pd_set_nonblocking(pd_fd_t fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) == -1) {
        perror("fcntl");
        return -1;
    }

    return 0;
}


void pd_event_modify(pd_io_t *ctx, pd_event_t *event, int fd, int operation, unsigned flags) {
    struct kevent kev;
    event->flags = flags;

    EV_SET(&kev, fd, event->flags, operation, 0, 0, event);
    if (kevent(ctx->poll_fd, &kev, 1, NULL, 0, NULL) == -1) {
        perror("pd_event_modify");
    }
}


void pd_event_del(pd_io_t *ctx, pd_event_t *event, int fd) {
    struct kevent kev;

    EV_SET(&kev, fd, event->flags, EV_DELETE, 0, 0, NULL);
    if (kevent(ctx->poll_fd, &kev, 1, NULL, 0, NULL) == -1) {
        perror("pd_event_del");
    }
}


void pd_event_add_readable(pd_io_t *ctx, pd_event_t *event, pd_fd_t fd) {
    event->flags |= EVFILT_READ;
    //event->ctx->handles++;
    pd_event_modify(ctx, event, fd, EV_ADD, event->flags);
}


void pd_event_read_start(pd_io_t *ctx, pd_event_t *event, pd_fd_t fd) {
    event->flags |= EVFILT_READ;
    pd_event_modify(ctx, event, fd, EV_ADD | EV_ENABLE, event->flags);
}


#define MAX_EVENTS 1024

void pd_io_run(pd_io_t *ctx) {
    struct kevent events[MAX_EVENTS];
    struct kevent ev;
    int timeout = pd_timers_next(ctx);
    pd_event_t *pev;

    // TODO: break loop if there is no active handles
    while (true) {
        struct timespec ts;
        if (timeout >= 0) {
            ts.tv_sec = timeout / 1000;
            ts.tv_nsec = (timeout % 1000) * 1000000;
        }

        int nevents = kevent(ctx->poll_fd, NULL, 0, events, MAX_EVENTS, timeout >= 0 ? &ts : NULL);

        if (nevents == -1) {
            perror("kevent");
            exit(EXIT_FAILURE);
        }

        ctx->now = pd_now();

        for (int i = 0; i < nevents; ++i) {
            ev = events[i];
            unsigned pevents = 0;
            pev = (pd_event_t *)ev.udata;

            if (ev.filter == EVFILT_READ)
                pevents |= PD_POLLIN;

            if (ev.filter == EVFILT_WRITE)
                pevents |= PD_POLLOUT;

            if ((ev.flags & EV_ERROR) || (ev.flags & EV_EOF)) {
                // usually we have to close handle in this scenario, so let's simplify detection
                pevents |= PD_CLOSE;

                if (ev.flags & EV_ERROR)
                    pevents |= PD_POLLERR;

                if (ev.flags & EV_EOF)
                    pevents |= PD_POLLHUP;
            }

            pev->handler(pev, pevents);
        }

        pd_timers_run(ctx);
        timeout = pd_timers_next(ctx);
    }
}
