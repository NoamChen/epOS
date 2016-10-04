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

#include <printk.h>
#include <lib/varargs.h>
#include <lib/ctype.h>
#include <lib/list.h>
#include <lib/kmalloc.h>
#include <video/VIDEO_textmode.h>
#include <types.h>

#define MAX_LOGGERS 5
static char buffer[256] = {0};

typedef struct
{
	bool enabled;
	log_func func;
} logger_t;

static logger_t loggers[MAX_LOGGERS] = {{0, NULL}};
static char *itoa(int value, char *str, int base)
{
	char *ptr;
	char *low;
	char *end;

	// Check for supported base.
	if (base < 2 || base > 36)
	{
		*str = '\0';
		return str;
	}

	ptr = str;

	// Set '-' for negative decimals.
	if (value < 0 && base == 10)
	{
		*ptr++ = '-';
	}

	// Remember where the numbers start.
	low = ptr;

	// The actual conversion.
	do
	{
		// Modulo is negative for negative value. This trick makes abs() unnecessary.
		*ptr++ = "ZYXWVUTSRQPONMLKJIHGFEDCBA9876543210123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[35 + value % base];
		value /= base;
	}
	while (value);

	// Terminating the string.
	*ptr-- = '\0';
	end = ptr + 1;

	// Invert the numbers.
	while (low < ptr)
	{
		char tmp = *low;
		*low++ = *ptr;
		*ptr-- = tmp;
	}

	return end;
}

static char *uitoa(unsigned int value, char *str, int base)
{
	char *ptr;
	char *low;
	char *end;

	// Check for supported base.
	if (base < 2 || base > 36)
	{
		*str = '\0';
		return str;
	}

	ptr = str;
	// Remember where the numbers start.
	low = ptr;

	// The actual conversion.
	do
	{
		// Modulo is negative for negative value. This trick makes abs() unnecessary.
		*ptr++ = "ZYXWVUTSRQPONMLKJIHGFEDCBA9876543210123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[35 + value % base];
		value /= base;
	}
	while (value);

	// Terminating the string.
	*ptr-- = '\0';
	end = ptr + 1;

	// Invert the numbers.
	while (low < ptr)
	{
		char tmp = *low;
		*low++ = *ptr;
		*ptr-- = tmp;
	}

	return end;
}

static char *lluitoa(unsigned long long value, char *str, int base)
{
	char *ptr;
	char *low;
	char *end;

	// Check for supported base.
	if (base < 2 || base > 36)
	{
		*str = '\0';
		return str;
	}

	ptr = str;
	// Remember where the numbers start.
	low = ptr;

	// The actual conversion.
	do
	{
		// Modulo is negative for negative value. This trick makes abs() unnecessary.
		*ptr++ = "ZYXWVUTSRQPONMLKJIHGFEDCBA9876543210123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[35 + value % base];
		value /= base;
	}
	while (value);

	// Terminating the string.
	*ptr-- = '\0';
	end = ptr + 1;

	// Invert the numbers.
	while (low < ptr)
	{
		char tmp = *low;
		*low++ = *ptr;
		*ptr-- = tmp;
	}

	return end;
}
// very simple printf, only supports basic stuff
static int do_printk(char *buffer, const char *fmt, va_list args)
{
	char *ptr;
	int len = 0;

	bool ll = false;
	char c;

	while (*fmt)
	{
		if (*fmt != '%')
		{
			*buffer++ = *fmt++;
		}
		else // Formatting in place... :)
		{
			fmt++;

			if ((*fmt == 'l') && (*(fmt + 1) == 'l'))
			{
				ll = true;
				fmt += 2;
			}

			switch (*fmt)
			{
			case 'd':
			case 'i':
				buffer = itoa(va_arg(args, int), buffer, 10);
				break;

			case 'u':
				if (!ll)
				{
					buffer = uitoa(va_arg(args, int), buffer, 10);
				}
				else
				{
					buffer = lluitoa(va_arg(args, long long), buffer, 10);
				}

				break;

			case 's':
				ptr = va_arg(args, char *);

				while (*ptr)
				{
					(*buffer++ = *ptr++);
				}

				break;

			case 'c':
				c = (char) va_arg(args, int);

				if (isascii(c))
				{
					*buffer++ = c;
				}

				break;

			case 'x':
			case 'X':
			case 'p':
				if (!ll)
				{
					buffer = uitoa(va_arg(args, int), buffer, 16);
				}
				else
				{
					buffer = lluitoa(va_arg(args, long long), buffer, 16);
				}

				break;

			default:
				break;
			}

			fmt++;
			len++;
			ll = false;
		}
	}

	*buffer = '\0';
	return len;
}


void register_logger(log_func func)
{
	int i;

	for (i = 0; i < MAX_LOGGERS; i++)
		if (!loggers[i].enabled)
		{
			break;
		}

	if (i == MAX_LOGGERS)
	{
		pr_error("No place for additional logger\r\n");
		return;
	}

	loggers[i].func = func;
	loggers[i].enabled = true;
}

int vprintk(const char *format, va_list arg)
{
	int done;
	done = do_printk(buffer, format, arg);

	for (int i = 0; i < MAX_LOGGERS; i++)
		if (loggers[i].enabled)
		{
			loggers[i].func(buffer);
		}

	return done;

}

int printk(const char *format, ...)
{
	va_list arg;
	int done;
	va_start(arg, format);
	done = do_printk(buffer, format, arg);
	va_end(arg);

	for (int i = 0; i < MAX_LOGGERS; i++)
		if (loggers[i].enabled)
		{
			loggers[i].func(buffer);
		}

	return done;
}
