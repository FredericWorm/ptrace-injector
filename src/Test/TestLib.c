/**
 * \file          TestLib.c
 * \brief         Test source file for the library
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
void *log_loop(void *arg) {
    FILE *fp = fopen("./testlib.log", "a");
    (void)arg;
    if (fp) {
        while (1) {
            sleep(10);
            fprintf(fp, "TestLib has been loaded!\n");
            fflush(fp);
        }
        fclose(fp);
    }
    return NULL;
}

/**
 * \brief          Main function for test library, creates async loop for printing
 */
__attribute__((constructor)) void onLoad() {
    pthread_t tid;
    pthread_create(&tid, NULL, log_loop, NULL);
    pthread_detach(tid);
}
