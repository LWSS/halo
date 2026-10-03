/*
UNICODE.C

*/

/* ---------- headers */

#include "cseries.h"
#include "unicode.h"

/* ---------- constants */

#define MAXIMUM_MEMCMP_SIZE 0x10000000
#define MAXIMUM_MEMCPY_MEMMOVE_SIZE 0x10000000
#define MAXIMUM_MEMSET_SIZE 0x10000000
#define MAXIMUM_STRING_SIZE 0x8000
#define MAXIMUM_MODE_STRING_SIZE 4

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

/* ---------- public code */

void *umemchr(
	void const *buffer,
	int c,
	size_t count)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 84, buffer);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 85, count < MAXIMUM_MEMCMP_SIZE);

	return memchr(buffer, c, count);
}

void *umemcpy(
	void *dest,
	void const *src,
	size_t count)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 96, dest && src);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 97, (count >= 0) && (count < MAXIMUM_MEMCPY_MEMMOVE_SIZE));
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 98, (((char *)src+count) <= (char *)dest) || (((char *)dest+count) <= (char *)src));

	return memcpy(dest, src, count);
}

int umemcmp(
	void const *buffer1,
	void const *buffer2,
	size_t count)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 109, buffer1 && buffer2);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 110, (count >= 0) && (count <= MAXIMUM_MEMCMP_SIZE));

	return memcmp(buffer1, buffer2, count);
}

void *umemmove(
	void *dest,
	void const *src,
	size_t count)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 121, dest && src);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 122, (count >= 0) && (count <= MAXIMUM_MEMCPY_MEMMOVE_SIZE));

	return memmove(dest, src, count);
}

void *umemset(
	void *buffer,
	int c,
	size_t count)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 133, buffer);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 134, (count >= 0) && (count <= MAXIMUM_MEMSET_SIZE));

	return memset(buffer, c, count);
}

wchar_t *ustrcpy(
	wchar_t *dest,
	wchar_t const *src)
{
	size_t source_size = wcslen(src);

	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 146, (source_size >= 0) && (source_size < MAXIMUM_STRING_SIZE));
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 147, ((src+source_size) < dest) || ((dest + source_size) < src));

	return wcscpy(dest, src);
}

wchar_t *ustrcat(
	wchar_t *dest,
	wchar_t const *src)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 157, dest && src);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 158, wcslen(dest) < MAXIMUM_STRING_SIZE);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 159, wcslen(src) < MAXIMUM_STRING_SIZE);

	return wcscat(dest, src);
}

int ustrcmp(
	wchar_t const *string1,
	wchar_t const *string2)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 181, string1 && string2);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 182, wcslen(string1) < MAXIMUM_STRING_SIZE);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 183, wcslen(string2) < MAXIMUM_STRING_SIZE);

	return wcscmp(string1, string2);
}

size_t ustrlen(
	wchar_t const *string)
{
	size_t size;

	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 194, string);
	size = wcslen(string);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 196, size < MAXIMUM_STRING_SIZE);

	return size;
}

size_t ustrnlen(
	wchar_t const *string,
	size_t count)
{
	size_t size = 0;

	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 208, string);

	while (size < count && *string++)
	{
		size++;
	}
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 214, size < MAXIMUM_STRING_SIZE);

	return size;
}

wchar_t *ustrchr(
	wchar_t const *string,
	wchar_t c)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 224, string);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 225, wcslen(string) < MAXIMUM_STRING_SIZE);

	return wcschr(string, c);
}

int ustrcoll(
	wchar_t const *string1,
	wchar_t const *string2)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 235, string1 && string2);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 236, wcslen(string1) < MAXIMUM_STRING_SIZE);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 237, wcslen(string2) < MAXIMUM_STRING_SIZE);

	return wcscoll(string1, string2);
}

size_t ustrcspn(
	wchar_t const *string,
	wchar_t const *character_set)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 247, string && character_set);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 248, wcslen(string) < MAXIMUM_STRING_SIZE);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 249, wcslen(character_set) < MAXIMUM_STRING_SIZE);

	return wcscspn(string, character_set);
}

wchar_t *ustrerror(
	int err)
{
	static wchar_t error_string[256];

	error_string[0] = L'\0';
	usnprintf(error_string, NUMBEROF(error_string), L"%hs", strerror(err));

	return error_string;
}

wchar_t *ustrncat(
	wchar_t *dest,
	wchar_t const *src,
	size_t count)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 273, dest && src);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 274, wcslen(dest) < MAXIMUM_STRING_SIZE);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 275, (count >= 0) && (count < MAXIMUM_STRING_SIZE));

	return wcsncat(dest, src, count);
}

int ustrncmp(
	wchar_t const *string1,
	wchar_t const *string2,
	size_t count)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 298, string1 && string2);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 299, (count >= 0) && (count < MAXIMUM_STRING_SIZE));

	return wcsncmp(string1, string2, count);
}

wchar_t *ustrncpy(
	wchar_t *dest,
	wchar_t const *src,
	size_t count)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 310, dest && src);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 311, (count >= 0) && (count < MAXIMUM_STRING_SIZE));

	return wcsncpy(dest, src, count);
}

wchar_t *ustrpbrk(
	wchar_t const *string,
	wchar_t const *character_set)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 321, string && character_set);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 322, wcslen(string) < MAXIMUM_STRING_SIZE);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 323, wcslen(character_set) < MAXIMUM_STRING_SIZE);

	return wcspbrk(string, character_set);
}

wchar_t *ustrrchr(
	wchar_t const *string,
	wchar_t c)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 333, string);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 334, wcslen(string) < MAXIMUM_STRING_SIZE);

	return wcsrchr(string, c);
}

size_t ustrspn(
	wchar_t const *string,
	wchar_t const *character_set)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 344, string && character_set);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 345, wcslen(string) < MAXIMUM_STRING_SIZE);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 346, wcslen(character_set) < MAXIMUM_STRING_SIZE);

	return wcsspn(string, character_set);
}

wchar_t *ustrstr(
	wchar_t const *string,
	wchar_t const *character_set)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 356, string && character_set);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 357, wcslen(string) < MAXIMUM_STRING_SIZE);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 358, wcslen(character_set) < MAXIMUM_STRING_SIZE);

	return wcsstr(string, character_set);
}

wchar_t *ustrtok(
	wchar_t *token,
	wchar_t const *delimiters)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 368, delimiters);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 369, wcslen(delimiters) < MAXIMUM_STRING_SIZE);

	return wcstok(token, delimiters);
}

size_t ustrxfrm(
	wchar_t *dest,
	wchar_t const *src,
	size_t count)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 380, dest && src);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 381, wcslen(dest) < MAXIMUM_STRING_SIZE);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 382, wcslen(src) < MAXIMUM_STRING_SIZE);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 383, count < MAXIMUM_STRING_SIZE);

	return wcsxfrm(dest, src, count);
}

wchar_t *ustrlwr(
	wchar_t *string)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 392, string);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 393, wcslen(string) < MAXIMUM_STRING_SIZE);

	return _wcslwr(string);
}

wchar_t *ustrupr(
	wchar_t *string)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 402, string);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 403, wcslen(string) < MAXIMUM_STRING_SIZE);

	return _wcsupr(string);
}

wchar_t *ustrnlwr(
	wchar_t *string,
	size_t count)
{
	wchar_t *character;

	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 415, string);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 416, wcslen(string) < MAXIMUM_STRING_SIZE);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 417, count < MAXIMUM_STRING_SIZE);

	for (character = string; *character; character++)
	{
		*character = towupper(*character);
	}

	return string;
}

wchar_t *ustrnupr(
	wchar_t *string,
	size_t count)
{
	wchar_t *character;

	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 435, string);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 436, wcslen(string) < MAXIMUM_STRING_SIZE);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 437, count < MAXIMUM_STRING_SIZE);

	for (character = string; *character; character++)
	{
		*character = towlower(*character);
	}

	return string;
}

int ustrcasecmp(
	wchar_t const *string1,
	wchar_t const *string2)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 455, string1 && string2);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 456, wcslen(string1) < MAXIMUM_STRING_SIZE);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 457, wcslen(string2) < MAXIMUM_STRING_SIZE);

	return _wcsicmp(string1, string2);
}

int ustrncasecmp(
	wchar_t const *string1,
	wchar_t const *string2,
	size_t count)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 472, string1 && string2);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 473, wcslen(string1) < MAXIMUM_STRING_SIZE);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 474, wcslen(string2) < MAXIMUM_STRING_SIZE);

	return _wcsnicmp(string1, string2, count);
}

int uisalpha(
	wchar_t c)
{
	return iswalpha(c);
}

int uisupper(
	wchar_t c)
{
	return iswupper(c);
}

int uislower(
	wchar_t c)
{
	return iswlower(c);
}

int uisdigit(
	wchar_t c)
{
	return iswdigit(c);
}

int uisxdigit(
	wchar_t c)
{
	return iswxdigit(c);
}

int uisspace(
	wchar_t c)
{
	return iswspace(c);
}

int uispunct(
	wchar_t c)
{
	return iswpunct(c);
}

int uisalnum(
	wchar_t c)
{
	return iswalnum(c);
}

int uisprint(
	wchar_t c)
{
	return iswprint(c);
}

int uisgraph(
	wchar_t c)
{
	return iswgraph(c);
}

int uiscntrl(
	wchar_t c)
{
	return iswcntrl(c);
}

int utoupper(
	wchar_t c)
{
	return towupper(c);
}

int utolower(
	wchar_t c)
{
	return towlower(c);
}

wchar_t ufgetc(
	FILE *stream)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 589, stream);

	return fgetwc(stream);
}

wchar_t ufputc(
	wchar_t c,
	FILE *stream)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 599, stream);

	return fputwc(c, stream);
}

wchar_t uungetc(
	wchar_t c,
	FILE *stream)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 609, stream);

	return ungetwc(c, stream);
}

wchar_t *ufgets(
	wchar_t *string,
	int size,
	FILE *stream)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 620, string);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 621, wcslen(string) < MAXIMUM_STRING_SIZE);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 622, size < MAXIMUM_STRING_SIZE);

	return fgetws(string, size, stream);
}

int ufputs(
	wchar_t const *string,
	FILE *stream)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 632, string);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 633, wcslen(string) < MAXIMUM_STRING_SIZE);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 634, stream);

	return fputws(string, stream);
}

wchar_t *ugets(
	wchar_t *string)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 643, string);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 644, wcslen(string) < MAXIMUM_STRING_SIZE);

	return _getws(string);
}

int uputs(
	wchar_t const *string)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 665, string);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 666, wcslen(string) < MAXIMUM_STRING_SIZE);

	return _putws(string);
}

int ufprintf(
	FILE *stream,
	wchar_t const *format,
	...)
{
	va_list arguments;

	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 680, stream);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 681, format);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 682, wcslen(format) < MAXIMUM_STRING_SIZE);

	va_start(arguments, format);

	return vfwprintf(stream, format, arguments);
}

int uprintf(
	wchar_t const *format,
	...)
{
	va_list arguments;

	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 699, format);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 700, wcslen(format) < MAXIMUM_STRING_SIZE);

	va_start(arguments, format);

	return vwprintf(format, arguments);
}

int usnprintf(
	wchar_t *string,
	size_t size,
	wchar_t const *format,
	...)
{
	va_list arguments;

	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 719, string);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 720, (size > 0) && (size <= MAXIMUM_STRING_SIZE));
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 721, wcslen(format) < MAXIMUM_STRING_SIZE);

	va_start(arguments, format);

	return _vsnwprintf(string, size, format, arguments);
}

int usprintf(
	wchar_t *string,
	wchar_t const *format,
	...)
{
	va_list arguments;

	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 751, string && format);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 752, wcslen(string) < MAXIMUM_STRING_SIZE);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 753, wcslen(format) < MAXIMUM_STRING_SIZE);

	va_start(arguments, format);

	return vswprintf(string, format, arguments);
}

int uvfprintf(
	FILE *stream,
	wchar_t const *format,
	va_list args)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 792, stream && format);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 793, wcslen(format) < MAXIMUM_STRING_SIZE);

	return vfwprintf(stream, format, args);
}

int uvprintf(
	wchar_t const *format,
	va_list args)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 803, format);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 804, wcslen(format) < MAXIMUM_STRING_SIZE);

	return vwprintf(format, args);
}

int uvsnprintf(
	wchar_t *string,
	size_t size,
	wchar_t const *format,
	va_list args)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 816, string && format);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 817, wcslen(string) < MAXIMUM_STRING_SIZE);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 818, wcslen(format) < MAXIMUM_STRING_SIZE);

	return _vsnwprintf(string, size, format, args);
}

int uvsprintf(
	wchar_t *string,
	wchar_t const *format,
	va_list args)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 841, string && format);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 842, wcslen(string) < MAXIMUM_STRING_SIZE);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 843, wcslen(format) < MAXIMUM_STRING_SIZE);

	return vswprintf(string, format, args);
}

FILE *ufdopen(
	int fd,
	wchar_t const *path)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 877, path);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 878, wcslen(path) < MAXIMUM_STRING_SIZE);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 879, fd > 0);

	return _wfdopen(fd, path);
}

FILE *ufopen(
	wchar_t const *path,
	wchar_t const *mode)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 889, path && mode);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 890, wcslen(path) < MAXIMUM_STRING_SIZE);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 891, wcslen(mode) < MAXIMUM_MODE_STRING_SIZE);

	return _wfopen(path, mode);
}

int ufclose(
	FILE *stream)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 900, stream);

	return fclose(stream);
}

FILE *ufreopen(
	wchar_t const *path,
	wchar_t const *mode,
	FILE *stream)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 911, path && mode);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 912, wcslen(path) < MAXIMUM_STRING_SIZE);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 913, wcslen(mode) < MAXIMUM_MODE_STRING_SIZE);

	return _wfreopen(path, mode, stream);
}

void uperror(
	wchar_t const *string)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 922, string);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 923, wcslen(string) < MAXIMUM_STRING_SIZE);

	_wperror(string);

	return;
}

FILE *upopen(
	wchar_t const *command,
	wchar_t const *mode)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 935, command && mode);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 936, wcslen(command) < MAXIMUM_STRING_SIZE);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 937, wcslen(mode) < MAXIMUM_MODE_STRING_SIZE);

	return NULL;
}

int uremove(
	wchar_t const *path)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 946, path);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 947, wcslen(path) < MAXIMUM_STRING_SIZE);

	return _wremove(path);
}

wchar_t *utmpnam(
	wchar_t *string)
{
	return _wtmpnam(string);
}

long ustrtol(
	wchar_t const *nptr,
	wchar_t **endptr,
	int base)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 967, nptr);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 968, wcslen(nptr) < MAXIMUM_STRING_SIZE);

	return wcstol(nptr, endptr, base);
}

unsigned long ustrtoul(
	wchar_t const *nptr,
	wchar_t **endptr,
	int base)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 979, nptr);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 980, wcslen(nptr) < MAXIMUM_STRING_SIZE);

	return wcstoul(nptr, endptr, base);
}

double ustrtod(
	wchar_t const *nptr,
	wchar_t **endptr)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 990, nptr);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 991, wcslen(nptr) < MAXIMUM_STRING_SIZE);

	return wcstod(nptr, endptr);
}

int uatoi(
	wchar_t const *string)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 1002, string);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 1003, wcslen(string) < MAXIMUM_STRING_SIZE);

	return _wtoi(string);
}

wchar_t *uctime(
	time_t const *timer)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 1014, timer);

	return _wctime(timer);
}

wchar_t *uasctime(
	struct tm const *timeptr)
{
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 1023, timeptr);

	return _wasctime(timeptr);
}

char *wide_to_ascii(
	wchar_t const *unicode,
	char *ascii,
	long ascii_length)
{
	size_t length;
	size_t index;

	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 1040, unicode && ascii);
	length = wcslen(unicode);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 1042, length < MAXIMUM_STRING_SIZE);

	if (length > ascii_length - 1)
	{
		return NULL;
	}

	for (index = 0; index < length; index++)
	{
		if (unicode[index] & 0xff80)
		{
			return NULL;
		}
	}

	for (index = 0; index < length; index++)
	{
		ascii[index] = (char)unicode[index];
	}
	ascii[index] = '\0';

	return ascii;
}

wchar_t *ascii_to_wide(
	char const *ascii,
	wchar_t *unicode,
	unsigned long unicode_length)
{
	size_t length;

	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 1087, ascii && unicode);
	length = strlen(ascii);
	match_assert("c:\\halo\\SOURCE\\text\\unicode.c", 1089, length < MAXIMUM_STRING_SIZE);

	if (unicode_length >= (length + 1) * sizeof(wchar_t))
	{
		long index;

		unicode[length] = L'\0';
		for (index = length - 1; index >= 0; index--)
		{
			unicode[index] = ascii[index];
		}

		return unicode;
	}

	return NULL;
}

/* ---------- private code */
