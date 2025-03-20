/**
 * \file          Main.c
 * \brief         Main source file
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

#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/uio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>
#include <dirent.h>
#include <dlfcn.h>
#include <signal.h>
#include <errno.h>
 
#include "Memory.h"

/**
 * \brief          Process ID of the target process
 */
int g_pid;

/**
 * \brief          Main function for library injection
 * \param[in] argc Number of command line arguments
 * \param[in] argv Array of arguments
 * \return         0 on success, 1 on failure
 */
int main(int argc, char *argv[]) {
    char* library_path, * process_name = NULL;
    uintptr_t remote_addr = 0, dlopen_result = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            if (i + 1 < argc) {
                process_name = strdup(argv[i + 1]);
                if (process_name == NULL) {
                    fprintf(stderr, "Error: Memory allocation failed\n");
                    goto cleanup;
                }
                i++;
            } else {
                fprintf(stderr, "Error: Missing argument for -p option\n");
                goto cleanup;
            }
        } else if (strcmp(argv[i], "-l") == 0) {
            if (i + 1 < argc) {
                library_path = strdup(argv[i + 1]);
                if (library_path == NULL) {
                    fprintf(stderr, "Error: Memory allocation failed\n");
                    goto cleanup;
                }
                i++;
            } else {
                fprintf(stderr, "Error: Missing argument for -l option\n");
                goto cleanup;
            }
        }
    }

    if (process_name == NULL || library_path == NULL) {
        fprintf(stderr, "Error: Please provide both -p and -l arguments\n");
        fprintf(stderr, "Usage: %s -p <process_cmdline_content> -l <library_path>\n", argv[0]);
        goto cleanup;
    }

    g_pid = get_process_id(process_name);
    if (g_pid == 1) {
        fprintf(stderr, "Error: Could not find process '%s'\n", process_name);
        goto cleanup;
    }

    if (ptrace(PTRACE_ATTACH, (pid_t)g_pid, NULL, NULL) == -1) {
        fprintf(stderr, "Error: Couldn't attach using ptrace: %s\n", strerror(errno));
        goto cleanup;
    }

    printf("\n");
    
    remote_addr = remote_call((void*)malloc, 1, 256);
    if (remote_addr == 1) {
        fprintf(stderr, "Error: Remote malloc failed: %s\n\n", strerror(errno));
        goto detach;
    }
    
    if (write_memory(g_pid, remote_addr, (uintptr_t)library_path, strlen(library_path) + sizeof(char)) != 0) {
        fprintf(stderr, "Error: Writing library path failed: %s\n\n", strerror(errno));
        goto free_remote;
    }
    printf("Info: Memory allocation successful in target process.\n\n");

    dlopen_result = remote_call((void*)dlopen, 2, remote_addr, RTLD_NOW | RTLD_GLOBAL);
    if (dlopen_result == 1) {
        fprintf(stderr, "Error: dlopen call failed.\n\n");
    } else if (dlopen_result == 0) {
        printf("Info: Library successfully loaded.\n\n");
    } else {
        char error_string[512] = {0};
        uintptr_t error_addr = remote_call((void*)dlerror, 0);

        if (error_addr == 1) {
            fprintf(stderr, "Error: dlerror call failed.\n\n");
            goto free_remote;
        }
        
        if (read_memory(g_pid, error_addr, (uintptr_t)error_string, sizeof(error_string)) != 0) {
            fprintf(stderr, "Error: Reading dlerror output failed.\n\n");
        } else {
            fprintf(stderr, "Error: dlopen failed with error:\n\t%s\n\n", error_string);
        }
    }

free_remote:
    if (remote_call((void*)free, 1, remote_addr) == 1) {
        fprintf(stderr, "Error: Remote free call failed.\n\n");
    } else {
        printf("Info: Remote memory freed successfully.\n\n");
    }

detach:
    if (ptrace(PTRACE_DETACH, g_pid, NULL, NULL) == -1) {
        fprintf(stderr, "Error: Couldn't detach using ptrace: %s\n", strerror(errno));
        goto cleanup;
    }

cleanup:
    free(process_name);
    free(library_path);

    printf("Info: Operation completed.\n");
    return (errno) ? 1 : 0;
}