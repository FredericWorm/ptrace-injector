/**
 * \file          Memory.h
 * \brief         Memory header file
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

#ifndef MEMORY_H
#define MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int get_process_id(const char* command_line_content);

int8_t read_memory(int pid, uintptr_t address, uintptr_t out, size_t length);
int8_t write_memory(int pid, uintptr_t address, uintptr_t data, size_t length);
int8_t get_local_module_name(void* address, char* module_name);

uintptr_t get_base(int pid, const char* module_name, int8_t is_local);
uintptr_t get_remote_function_address(char* module_name, void* local_function_address);
uintptr_t remote_call(void* function_pointer, int count, ...);

extern int g_pid;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MEMORY_H */
