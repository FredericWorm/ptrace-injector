/**
 * \file          Memory.c
 * \brief         Memory source file
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
 * \brief  Finds process ID by command line content
 * \param[in] command_line_content  Command line content to match
 * \return PID on success, 1 if not found or error
 */
int get_process_id(const char* command_line_content) {
    int pid = -1;

    if (command_line_content[0] == '\0') {
        fprintf(stderr, "Error: Command line content is empty.\n");
        return 1;
    }

    DIR* dir = opendir("/proc/");
    if (dir == NULL) {
        fprintf(stderr, "Error: Couldn't open /proc/ directory: %s\n", strerror(errno));
        return 1;
    }
    
    struct dirent* entry = NULL;
    while ((entry = readdir(dir)) != NULL) {
        int entry_pid = atoi(entry->d_name);
        if (entry_pid > 0) {
            char file_path[64], cmdline[128] = {0};
            snprintf(file_path, sizeof(file_path), "/proc/%d/cmdline", entry_pid);
            
            FILE* fp = fopen(file_path, "r");
            if (fp == NULL) {
                continue;
            }

            fgets(cmdline, sizeof(cmdline), fp);
            fclose(fp);
            
            if (strcmp(cmdline, command_line_content) == 0) {
                pid = entry_pid;
                break;
            }
        }
    }
    
    closedir(dir);
    if (pid == -1) {
        fprintf(stderr, "Error: Couldn't find process with command line: %s\n", command_line_content);
        return 1;
    }
    printf("Info: Found process %s with PID %d.\n", command_line_content, pid);
    return pid;
}

/**
 * \brief                  Gets module base address for a given PID
 * \param[in] pid          Process ID
 * \param[in] module_name  Module name
 * \param[in] is_local     Search in self if 1, else remote
 * \return                 Base address or 1 on error
 */
uintptr_t get_base(int pid, const char* module_name, int8_t is_local) {
    char line[512], file_path[64];
    uintptr_t start_address = 0;

    printf("Info: Getting base of %s.\n", module_name);
    
    if (module_name[0] == '\0') {
        fprintf(stderr, "Error: Library name is empty.\n");
        return 1;
    }

    if (is_local == 1) {
        snprintf(file_path, sizeof(file_path), "/proc/self/maps");
    } else {
        snprintf(file_path, sizeof(file_path), "/proc/%d/maps", pid);
    }
    
    FILE* fp = fopen(file_path, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Couldn't open maps file.\n");
        return 1;
    }
    
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, module_name)) {
            sscanf(line, "%lx-", &start_address);
            break;
        }
    }

    fclose(fp);

    if (start_address == 0) {
        fprintf(stderr, "Error: Couldn't find start address.\n");
        return 1;
    }

    printf("Info: Found %s start address %p of module %s.\n", 
           (is_local == 1) ? "local" : "remote", 
           (void*)start_address, module_name);

    return start_address;
}

/**
 * \brief                  Reads memory from a process
 * \param[in] pid          PID to read from
 * \param[in] address      Source address
 * \param[in] out          Local buffer
 * \param[in] length       Number of bytes to read
 * \return                 0 on success, 1 on error
 */
int8_t read_memory(int pid, uintptr_t address, uintptr_t out, size_t length) {
    struct iovec local = {(void*)out, length}, remote = {(void*)address, length};

    if (process_vm_readv((pid_t)pid, &local, 1, &remote, 1, 0) == (ssize_t)length) {
        return 0;
    }
    return 1;
}

/**
 * \brief                  Writes memory to a process
 * \param[in] pid          PID to write to
 * \param[in] address      Remote address
 * \param[in] data         Local data pointer
 * \param[in] length       Number of bytes to write
 * \return                 0 on success, 1 on error
 */
int8_t write_memory(int pid, uintptr_t address, uintptr_t data, size_t length) {
    struct iovec local = {(void*)data, length}, remote = {(void*)address, length};

    if (process_vm_writev((pid_t)pid, &local, 1, &remote, 1, 0) == (ssize_t)length) {
        return 0;
    }
    return 1;
}

/**
 * \brief                  Gets local module name for an address
 * \param[in] address      Address to look up
 * \param[out] module_name Buffer for name
 * \return                 0 on success, 1 on error
 */
int8_t get_local_module_name(void* address, char* module_name) {
    char line[512], file_path[64];
    FILE* fp = NULL;

    snprintf(file_path, sizeof(file_path), "/proc/%d/maps", (int)getpid());

    fp = fopen(file_path, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Couldn't open maps file.\n");
        return 1;
    }

    while (fgets(line, sizeof(line), fp)) {
        uintptr_t start_address, end_address;
        sscanf(line, "%lx-%lx ", &start_address, &end_address);

        if ((uintptr_t)address >= start_address && (uintptr_t)address <= end_address) {
            char* lib_path = strchr(line, '/');
            
            if (lib_path) {
                char* new_line = strchr(lib_path, '\n');
                if (new_line) {
                    *new_line = '\0';
                }

                strcpy(module_name, lib_path);
                printf("Info: Found symbol in module %s.\n", module_name);

                fclose(fp);
                return 0;
            }
        }
    }

    fclose(fp);
    return 1;
}

/**
 * \brief                              Computes remote function address in a target
 * \param[in] module_name              Module name
 * \param[in] local_function_address   Local pointer to function
 * \return                             Address or 1 on error
 */
uintptr_t get_remote_function_address(char* module_name, void* local_function_address) {
    uintptr_t local_module_address, remote_module_address, remote_function_address;
    
    local_module_address = get_base(g_pid, module_name, 1);
    remote_module_address = get_base(g_pid, module_name, 0);
    if (remote_module_address == 1 || local_module_address == 1) {
        return 1;
    }
    
    remote_function_address = (uintptr_t)local_function_address - local_module_address + remote_module_address;
    return remote_function_address;
}

/**
 * \brief                              Calls a function in remote process
 * \param[in] function_pointer         Pointer to the function
 * \param[in] count                    Argument count
 * \param[in] ...                      Arguments
 * \return                             Return value from remote function, 1 on error
 */
uintptr_t remote_call(void* function_pointer, int count, ...) {
    char module_name[512];
    struct user_regs_struct return_registers, original_registers, temp_registers;
    uintptr_t space = sizeof(uintptr_t), return_address = 0, remote_symbol_address = 1;
    int status = 0;

    if (get_local_module_name(function_pointer, module_name) != 0) {
        return 1;
    }
    
    remote_symbol_address = get_remote_function_address(module_name, function_pointer);
    if (remote_symbol_address == 1) {
        return 1;
    }
    
    if (ptrace(PTRACE_GETREGS, (pid_t)g_pid, NULL, &temp_registers) == -1) {
        fprintf(stderr, "Error: Couldn't get registers.\n");
        return 1;
    }
    
    memcpy(&original_registers, &temp_registers, sizeof(temp_registers));

    while (((temp_registers.rsp - space - 8) & 0xF) != 0) {
        temp_registers.rsp--;
    }

    va_list arg_list;
    va_start(arg_list, count);
    for (int i = 0; i < count && i < 6; i++) {
        uintptr_t argument = va_arg(arg_list, uintptr_t);
        switch (i) {
            case 0: temp_registers.rdi = argument; break;
            case 1: temp_registers.rsi = argument; break;
            case 2: temp_registers.rdx = argument; break;
            case 3: temp_registers.rcx = argument; break;
            case 4: temp_registers.r8 = argument; break;
            case 5: temp_registers.r9 = argument; break;
        }
    }
    va_end(arg_list);

    temp_registers.rsp -= sizeof(uintptr_t);
    if (write_memory(g_pid, temp_registers.rsp, (uintptr_t)&return_address, sizeof(uintptr_t)) != 0) {
        fprintf(stderr, "Error: Couldn't set return address.\n");
        return 1;
    }
    
    temp_registers.rip = remote_symbol_address;
    temp_registers.rax = 1;
    temp_registers.orig_rax = 0;

    if (ptrace(PTRACE_SETREGS, (pid_t)g_pid, NULL, &temp_registers) == -1) {
        fprintf(stderr, "Error: Couldn't set registers.\n");
        return 1;
    }

    if (ptrace(PTRACE_CONT, (pid_t)g_pid, NULL, NULL) == -1) {
        fprintf(stderr, "Error: Couldn't continue process.\n");
        return 1;
    }

    printf("Info: Waiting for function to end.\n");
    for (;;) {
        pid_t wp = waitpid(g_pid, &status, WUNTRACED);
            
        if (wp != g_pid) {
            fprintf(stderr, "Error: waitpid failed.\n");
            return 1;
        }
        
        if (WIFSTOPPED(status) && (WSTOPSIG(status) == SIGSEGV || WSTOPSIG(status) == SIGILL)) {
            break;
        }
        
        if (WIFEXITED(status)) {
            fprintf(stderr, "Error: Process exited.\n");
            return 1;
        }
        
        if (WIFSIGNALED(status)) {
            fprintf(stderr, "Error: Process terminated.\n");
            return 1;
        }
        
        if (ptrace(PTRACE_CONT, (pid_t)g_pid, NULL, NULL) == -1) {
            fprintf(stderr, "Error: Couldn't continue process.\n");
            return 1;
        }
    }
    printf("Info: Function ended.\n");

    if (ptrace(PTRACE_GETREGS, (pid_t)g_pid, NULL, &return_registers) == -1) {
        fprintf(stderr, "Error: Couldn't get registers.\n");
        return 1;
    }

    if (ptrace(PTRACE_SETREGS, (pid_t)g_pid, NULL, &original_registers) == -1) {
        fprintf(stderr, "Error: Couldn't set registers.\n");
        return 1;
    }

    return return_registers.rax;
}