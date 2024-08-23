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

void pd_mutex_init(pd_mutex_t *mux) {
    pthread_mutex_init(mux);
}


void pd_mutex_lock(pd_mutex_t *mux) {
    pthread_mutex_lock(mux);
}


void pd_mutex_unlock(pd_mutex_t *mux) {
    pthread_mutex_unlock(mux);
}


void pd_mutex_destroy(pd_mutex_t *mux) {
    pthread_mutex_destroy(mux);
}


void pd_cond_init(pd_cond_t *cond) {
    pthread_cond_init(cond);
}


void pd_cond_wait(pd_cond_t *cond, pd_mutex_t *mux) {
    pthread_cond_wait(cond, mux);
}


void pd_cond_signal(pd_cond_t *cond) {
    pthread_cond_signal(cond);
}


// On Windows it is not necessary.
void pd_cond_destroy(pd_cond_t *cond) {
    pthread_cond_destroy(cond);
}
