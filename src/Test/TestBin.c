/**
 * \file          TestBin.c
 * \brief         Test source file for the binary
 */

/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 Frederic
 
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

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

/**
 * \brief          Loop funtion for printing
 * \return         0
 */
void *print_loop(void *arg) {
    (void)arg;
    while (1) {
        sleep(10);
        printf("Hello, World!\n");
    }
    return NULL;
}

/**
 * \brief          Main function for test binary, creates async loop for printing
 * \return         0
 */
int main() {
    pthread_t tid;
    pthread_create(&tid, NULL, print_loop, NULL);

    while (1) {
        sleep(1);
    }

    return 0;
}