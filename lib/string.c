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

#include <lib/string.h>

void *memset(void *s, int c, size_t n)
{
	uint8_t *p = s;

	for (unsigned int i = 0; i < n; i++)
	{
		p[i] = (uint8_t) c;
	}

	return s;
}

void *memcpy(void *restrict s1, const void *restrict s2, size_t n)
{
	const uint8_t *src = s2;
	uint8_t *dst = s1;

	while (n--)
	{
		*dst++ = *src++;
	}

	return s1;
}

int   memcmp(const void *s1, const void *s2, size_t n)
{
	const uint8_t *p1 = s1, *p2 = s2;

	while (n--)
		if (*p1 != *p2)
		{
			return *p1 - *p2;
		}
		else
		{
			*p1++, *p2++;
		}

	return 0;
}

void *memmove(void *s1, const void *s2, size_t n)
{
// this only work on c99, VLA needed.
	unsigned char tmp[n];
	memcpy(tmp, s2, n);
	memcpy(s1, tmp, n);
	return s1;

}



void *realloc(void *ptr, size_t size)
{
// TODO: Implement
	return NULL;
}

void free(void *ptr)
{
// TODO: Implement
}

void *malloc(size_t size)
{
// TODO: Implement
	return NULL;
}

void *calloc(size_t nmemb, size_t size)
{
// TODO: Implement
	return NULL;
}
