/*
	This is free and unencumbered software released into the public domain.

	Anyone is free to copy, modify, publish, use, compile, sell, or
	distribute this software, either in source code form or as a compiled
	binary, for any purpose, commercial or non-commercial, and by any
	means.

	In jurisdictions that recognize copyright laws, the author or authors
	of this software dedicate any and all copyright interest in the
	software to the public domain. We make this dedication for the benefit
	of the public at large and to the detriment of our heirs and
	successors. We intend this dedication to be an overt act of
	relinquishment in perpetuity of all present and future rights to this
	software under copyright law.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
	OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
	ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.

	For more information, please refer to <http://unlicense.org>
*/

#ifndef SCHEDULER_H_WTDXB0FE
#define SCHEDULER_H_WTDXB0FE

#include <process.h>
#include <isr.h>

void scheduler_add_task(task_t *task);
void scheduler_remove_task(task_t *task);
void scheduler_set_zombie(task_t *task);
void scheduler_switch_task(registers_t *regs);
void schedule();
task_t *get_current_task();
void scheduler_start();
void save_registers(task_t *current_task, registers_t *regs);

/* defined in scheduler.s */
void run_user_task(task_registers_t *registers);
void run_kernel_task(task_registers_t *registers);

#endif /* end of include guard: SCHEDULER_H_WTDXB0FE */
