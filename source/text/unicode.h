/*
UNICODE.H

header included in hcex build.
*/

#ifndef __UNICODE_H
#define __UNICODE_H
#pragma once

/* ---------- headers */

#include <wchar.h>
#include <time.h>

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes/UNICODE.C */

void *umemchr(void const *buffer, int c, size_t count);
void *umemcpy(void *dest, void const *src, size_t count);
int umemcmp(void const *buffer1, void const *buffer2, size_t count);
void *umemmove(void *dest, void const *src, size_t count);
void *umemset(void *buffer, int c, size_t count);

wchar_t *ustrcpy(wchar_t *dest, wchar_t const *src);
wchar_t *ustrcat(wchar_t *dest, wchar_t const *src);
int ustrcmp(wchar_t const *string1, wchar_t const *string2);
size_t ustrlen(wchar_t const *string);
size_t ustrnlen(wchar_t const *string, size_t count);
wchar_t *ustrchr(wchar_t const *string, wchar_t c);
int ustrcoll(wchar_t const *string1, wchar_t const *string2);
size_t ustrcspn(wchar_t const *string, wchar_t const *character_set);
wchar_t *ustrerror(int err);
wchar_t *ustrncat(wchar_t *dest, wchar_t const *src, size_t count);
int ustrncmp(wchar_t const *string1, wchar_t const *string2, size_t count);
wchar_t *ustrncpy(wchar_t *dest, wchar_t const *src, size_t count);
wchar_t *ustrpbrk(wchar_t const *string, wchar_t const *character_set);
wchar_t *ustrrchr(wchar_t const *string, wchar_t c);
size_t ustrspn(wchar_t const *string, wchar_t const *character_set);
wchar_t *ustrstr(wchar_t const *string, wchar_t const *character_set);
wchar_t *ustrtok(wchar_t *token, wchar_t const *delimiters);
size_t ustrxfrm(wchar_t *dest, wchar_t const *src, size_t count);
wchar_t *ustrlwr(wchar_t *string);
wchar_t *ustrupr(wchar_t *string);
wchar_t *ustrnlwr(wchar_t *string, size_t count);
wchar_t *ustrnupr(wchar_t *string, size_t count);
int ustrcasecmp(wchar_t const *string1, wchar_t const *string2);
int ustrncasecmp(wchar_t const *string1, wchar_t const *string2, size_t count);

int uisalpha(wchar_t c);
int uisupper(wchar_t c);
int uislower(wchar_t c);
int uisdigit(wchar_t c);
int uisxdigit(wchar_t c);
int uisspace(wchar_t c);
int uispunct(wchar_t c);
int uisalnum(wchar_t c);
int uisprint(wchar_t c);
int uisgraph(wchar_t c);
int uiscntrl(wchar_t c);
int utoupper(wchar_t c);
int utolower(wchar_t c);

wchar_t ufgetc(FILE *stream);
wchar_t ufputc(wchar_t c, FILE *stream);
wchar_t uungetc(wchar_t c, FILE *stream);
wchar_t *ufgets(wchar_t *string, int size, FILE *stream);
int ufputs(wchar_t const *string, FILE *stream);
wchar_t *ugets(wchar_t *string);
int uputs(wchar_t const *string);

int ufprintf(FILE *stream, wchar_t const *format, ...);
int uprintf(wchar_t const *format, ...);
int usnprintf(wchar_t *string, size_t size, wchar_t const *format, ...);
int usprintf(wchar_t *string, wchar_t const *format, ...);
int uvfprintf(FILE *stream, wchar_t const *format, va_list args);
int uvprintf(wchar_t const *format, va_list args);
int uvsnprintf(wchar_t *string, size_t size, wchar_t const *format, va_list args);
int uvsprintf(wchar_t *string, wchar_t const *format, va_list args);

FILE *ufdopen(int fd, wchar_t const *path);
FILE *ufopen(wchar_t const *path, wchar_t const *mode);
int ufclose(FILE *stream);
FILE *ufreopen(wchar_t const *path, wchar_t const *mode, FILE *stream);
void uperror(wchar_t const *string);
FILE *upopen(wchar_t const *command, wchar_t const *mode);
int uremove(wchar_t const *path);
wchar_t *utmpnam(wchar_t *string);

long ustrtol(wchar_t const *nptr, wchar_t **endptr, int base);
unsigned long ustrtoul(wchar_t const *nptr, wchar_t **endptr, int base);
double ustrtod(wchar_t const *nptr, wchar_t **endptr);
int uatoi(wchar_t const *string);

wchar_t *uctime(time_t const *timer);
wchar_t *uasctime(struct tm const *timeptr);

char *wide_to_ascii(wchar_t const *unicode, char *ascii, long ascii_length);
wchar_t *ascii_to_wide(char const *ascii, wchar_t *unicode, unsigned long unicode_length);

/* ---------- globals */

/* ---------- public code */

#endif // __UNICODE_H
