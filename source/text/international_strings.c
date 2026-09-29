/*
INTERNATIONAL_STRINGS.C

*/

/* ---------- headers */

#include "cseries.h"
#include "international_strings.h"

/* ---------- constants */

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

/* ---------- globals */

static short global_language_code;

/* ---------- public code */

void set_language_code(
	short language_code)
{
	if (language_code < 0 || language_code >= NUMBER_OF_LANGUAGE_CODES)
	{
		language_code = _language_roman;
	}
	global_language_code = language_code;

	return;
}

word get_next_character(
	byte *string,
	short *index)
{
	word character;

	match_vassert("c:\\halo\\SOURCE\\text\\international_strings.c", 32, *index >= 0 && *index <= strlen((char *)string),
		csprintf(temporary, "#%d is out of range in string @%p", *index, string));

	string += *index;
	if (double_byte_character(string))
	{
		character = (string[0] << 8) | string[1];
		*index += 2;
	}
	else
	{
		character = string[0];
		*index += 1;
	}

	return character;
}

word get_previous_character(
	byte *string,
	short *index)
{
	short next_index;
	short previous_index;
	word character;

	match_vassert("c:\\halo\\SOURCE\\text\\international_strings.c", 55, *index > 0 && *index <= strlen((char *)string),
		csprintf(temporary, "#%d is out of range in string @%p", *index, string));

	next_index = 0;
	do
	{
		previous_index = next_index;
		character = get_next_character(string, &next_index);
	}
	while (next_index < *index);

	match_vwarn("c:\\halo\\SOURCE\\text\\international_strings.c", 67, next_index == *index,
		csprintf(temporary, "index #%d is inbetween characters in string %p", *index, string));

	*index = previous_index;

	return character;
}

void align_to_character(
	byte *string,
	short *index)
{
	short next_index;

	match_vassert("c:\\halo\\SOURCE\\text\\international_strings.c", 85, *index >= 0 && *index <= strlen((char *)string),
		csprintf(temporary, "#%d is out of range in string @%p", *index, string));

	next_index = 0;
	while (next_index < *index)
	{
		get_next_character(string, &next_index);
	}
	*index = next_index;

	return;
}

boolean double_byte_character(
	byte *string)
{
	byte character = string[0];
	boolean result = FALSE;

	if (character != '\0')
	{
		byte next_character = string[1];

		if (character == '|' && next_character && strchr("ibukprlctn", next_character))
		{
			result = TRUE;
		}
		else
		{
			switch (global_language_code)
			{
			case _language_japanese:
				if ((character >= 0x81 && character <= 0x9f || character >= 0xe0 && character <= 0xfe) &&
					next_character >= 0x40 && next_character <= 0xfc && next_character != 0x7f)
				{
					result = TRUE;
				}
				break;
			case _language_simple_chinese:
				if (character >= 0xa1 && character <= 0xfe &&
					next_character >= 0xa1 && next_character <= 0xfe)
				{
					result = TRUE;
				}
				break;
			case _language_traditional_chinese:
				if (character >= 0x81 && character <= 0xfe &&
					(next_character >= 0x40 && next_character <= 0x7e || next_character >= 0xa1 && next_character <= 0xfe))
				{
					result = TRUE;
				}
				break;
			case _language_korean_wansung:
				if (character >= 0x81 && character <= 0xfe &&
					(next_character >= 'A' && next_character <= 'Z' || next_character >= 'a' && next_character <= 'z' || next_character >= 0x81 && next_character <= 0xfe))
				{
					result = TRUE;
				}
				break;
			case _language_korean_johab:
				if ((character >= 0x84 && character <= 0xd3 || character >= 0xd8 && character <= 0xde || character >= 0xe0 && character <= 0xf9) &&
					(next_character >= 0x41 && next_character <= 0x7e || next_character >= 0x81 && next_character <= 0xfe))
				{
					result = TRUE;
				}
				break;
			}
		}
	}

	return result;
}

boolean character_in_pattern(
	word character,
	char *pattern)
{
	boolean result = FALSE;
	boolean done = FALSE;
	short index = 0;

	while (!done)
	{
		word pattern_character = get_next_character((byte *)pattern, &index);

		if (pattern_character)
		{
			if (pattern_character == character)
			{
				result = TRUE;
				done = TRUE;
			}
		}
		else
		{
			done = TRUE;
		}
	}

	return result;
}

/* ---------- private code */
