/*
FILES_WINDOWS.C
*/

/* ---------- headers */

#include "cseries.h"
#include "international_strings.h"

#include <ctype.h>

/* ---------- constants */

enum
{
	_find_files_recursive_bit = 0,
	_find_files_enumerate_directories_bit,
	NUMBER_OF_FIND_FILES_FLAGS
};

enum
{
	FIRST_DRIVE_LETTER = 'A',
	LAST_DRIVE_LETTER = 'Z',
	DRIVE_NAME_LENGTH = 4,
	DIRECTORY_SEPARATOR = '\\',
	EXTENSION_SEPARATOR = '.',
	BAD_FILE = 0xFF,
	MAXIMUM_SEARCH_DEPTH = 8
};

enum
{
	has_filename_bit = 0,
	NUMBER_OF_REFERENCE_INFO_FLAGS
};

/* ---------- structures */

struct find_files_state
{
	unsigned long flags;
	short depth;
	short location;
	char path[MAXIMUM_FILENAME_LENGTH+1];
	HANDLE handles[MAXIMUM_SEARCH_DEPTH];
	WIN32_FIND_DATA data;
};

struct file_reference_info
{
	unsigned long signature;
	word flags;
	short location;
	char path[MAXIMUM_FILENAME_LENGTH+1];
	HANDLE file_handle;
};

/* ---------- prototypes */

static void file_error(char const *function_name, struct file_reference const *file);

/* ---------- globals */

static char file_last_cd_drive_name[DRIVE_NAME_LENGTH] = "?:\\";

static struct find_files_state find_files_globals =
{
	0,
	NONE,
	0,
	"",
	{
		INVALID_HANDLE_VALUE,
		INVALID_HANDLE_VALUE,
		INVALID_HANDLE_VALUE,
		INVALID_HANDLE_VALUE,
		INVALID_HANDLE_VALUE,
		INVALID_HANDLE_VALUE,
		INVALID_HANDLE_VALUE,
		INVALID_HANDLE_VALUE
	}
};

/* ---------- public code */

boolean file_location_is_valid(
	short location)
{
	return TRUE;
}

boolean file_create(
	struct file_reference *file)
{
	struct file_reference_info *info = file_reference_get_info(file);
	char full_path[MAXIMUM_FILENAME_LENGTH+1] = {0};
	boolean success = FALSE;

	file_location_get_full_path(info->location, info->path, full_path);

	if (TEST_FLAG(info->flags, has_filename_bit))
	{
		HANDLE file_handle = CreateFile(
			full_path,
			GENERIC_WRITE,
			0,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (file_handle!=INVALID_HANDLE_VALUE)
		{
			CloseHandle(file_handle);
			success = TRUE;
		}
	}
	else if (CreateDirectory(info->path, NULL))
	{
		success = TRUE;
	}

	if (!success)
	{
		file_error("file_create", file);
	}

	return success;
}

boolean file_delete(
	struct file_reference *file)
{
	struct file_reference_info *info = file_reference_get_info(file);
	char full_path[MAXIMUM_FILENAME_LENGTH+1] = {0};
	boolean success = FALSE;

	file_location_get_full_path(info->location, info->path, full_path);

	if (TEST_FLAG(info->flags, has_filename_bit))
	{
		if (SetFileAttributes(full_path, FILE_ATTRIBUTE_NORMAL) && DeleteFile(full_path))
		{
			success = TRUE;
		}
	}
	else if (RemoveDirectory(full_path))
	{
		success = TRUE;
	}

	if (!success)
	{
		file_error("file_delete", file);
	}

	return success;
}

boolean file_exists(
	struct file_reference const *file)
{
	struct file_reference_info *info = file_reference_get_info((struct file_reference *)file);
	char full_path[MAXIMUM_FILENAME_LENGTH+1] = {0};
	boolean exists = FALSE;

	file_location_get_full_path(info->location, info->path, full_path);

	if (GetFileAttributes(full_path)!=-1)
	{
		exists = TRUE;
	}
	else if (GetLastError()!=ERROR_FILE_NOT_FOUND && GetLastError()!=ERROR_PATH_NOT_FOUND)
	{
		file_error("file_exists", file);
	}

	return exists;
}

boolean file_rename(
	struct file_reference *file,
	char const *name)
{
	struct file_reference_info *info = file_reference_get_info(file);
	char old_full_path[MAXIMUM_FILENAME_LENGTH+1] = {0};
	char new_full_path[MAXIMUM_FILENAME_LENGTH+1] = {0};
	boolean success = FALSE;

	file_location_get_full_path(info->location, info->path, old_full_path);
	strcpy(new_full_path, old_full_path);
	file_path_remove_name(new_full_path);
	file_path_add_name(new_full_path, name);

	if (MoveFile(old_full_path, new_full_path))
	{
		file_path_remove_name(info->path);
		file_path_add_name(info->path, name);
		success = TRUE;
	}

	return success;
}

boolean file_open(
	struct file_reference *file,
	unsigned long flags)
{
	HANDLE file_handle;
	struct file_reference_info *info = file_reference_get_info(file);
	char full_path[MAXIMUM_FILENAME_LENGTH+1] = {0};
	unsigned long access = 0;
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\tag_files\\files_windows.c", 308, VALID_FLAGS(flags, NUMBER_OF_PERMISSION_FLAGS));
	match_assert("c:\\halo\\SOURCE\\tag_files\\files_windows.c", 309, flags & (FLAG(_permission_read_bit)|FLAG(_permission_write_bit)));
	match_assert("c:\\halo\\SOURCE\\tag_files\\files_windows.c", 310, TEST_FLAG(flags, _permission_write_bit) || !TEST_FLAG(flags, _permission_append_bit));

	file_location_get_full_path(info->location, info->path, full_path);

	if (TEST_FLAG(flags, _permission_read_bit))
	{
		access = GENERIC_READ;
	}

	if (TEST_FLAG(flags, _permission_write_bit))
	{
		access |= GENERIC_WRITE;
	}

	file_handle = CreateFile(
		full_path,
		access,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (file_handle!=INVALID_HANDLE_VALUE)
	{
		info->file_handle = file_handle;
		success = TRUE;
	}

	if (success && TEST_FLAG(flags, _permission_append_bit) && SetFilePointer(info->file_handle, 0, NULL, FILE_END)==INVALID_SET_FILE_POINTER)
	{
		CloseHandle(info->file_handle);
		info->file_handle = NULL;
		success = FALSE;
	}

	if (!success)
	{
		file_error("file_open", file);
	}

	return success;
}

boolean file_close(
	struct file_reference *file)
{
	struct file_reference_info *info = file_reference_get_info(file);
	boolean success = FALSE;

	if (CloseHandle(info->file_handle))
	{
		info->file_handle = NULL;
		success = TRUE;
	}

	if (!success)
	{
		file_error("file_close", file);
	}

	return success;
}

unsigned long file_get_position(
	struct file_reference const *file)
{
	struct file_reference_info *info = file_reference_get_info((struct file_reference *)file);
	unsigned long position = SetFilePointer(info->file_handle, 0, NULL, FILE_CURRENT);

	if (position==INVALID_SET_FILE_POINTER)
	{
		file_error("file_get_position", file);
	}

	return position;
}

boolean file_set_position(
	struct file_reference const *file,
	unsigned long position)
{
	struct file_reference_info *info = file_reference_get_info((struct file_reference *)file);
	boolean success = SetFilePointer(info->file_handle, position, NULL, FILE_BEGIN)!=INVALID_SET_FILE_POINTER;

	if (!success)
	{
		file_error("file_set_position", file);
	}

	return success;
}

unsigned long file_get_eof(
	struct file_reference const *file)
{
	struct file_reference_info *info = file_reference_get_info((struct file_reference *)file);
	unsigned long eof = GetFileSize(info->file_handle, NULL);

	if (eof==INVALID_FILE_SIZE)
	{
		file_error("file_get_eof", file);
	}

	return eof;
}

boolean file_set_eof(
	struct file_reference const *file,
	unsigned long position)
{
	struct file_reference_info *info = file_reference_get_info((struct file_reference *)file);
	boolean success = file_set_position(file, position) && SetEndOfFile(info->file_handle);

	if (!success)
	{
		file_error("file_set_eof", file);
	}

	return success;
}

boolean file_read(
	struct file_reference const *file,
	unsigned long count,
	void *buffer)
{
	unsigned long bytes_read;
	struct file_reference_info *info = file_reference_get_info((struct file_reference *)file);
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\tag_files\\files_windows.c", 423, buffer);

	if (ReadFile(info->file_handle, buffer, count, &bytes_read, NULL))
	{
		if (bytes_read==count)
		{
			success = TRUE;
		}
		else
		{
			SetLastError(ERROR_HANDLE_EOF);
		}
	}

	if (!success)
	{
		file_error("file_read", file);
	}

	return success;
}

boolean file_write(
	struct file_reference const *file,
	unsigned long count,
	void const *buffer)
{
	unsigned long bytes_written;
	struct file_reference_info *info = file_reference_get_info((struct file_reference *)file);
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\tag_files\\files_windows.c", 451, buffer);

	if (WriteFile(info->file_handle, buffer, count, &bytes_written, NULL) && bytes_written==count)
	{
		success = TRUE;
	}

	if (!success)
	{
		file_error("file_write", file);
	}

	return success;
}

boolean file_read_from_position(
	struct file_reference const *file,
	unsigned long position,
	unsigned long count,
	void *buffer)
{
	return file_set_position(file, position) && file_read(file, count, buffer);
}

boolean file_write_to_position(
	struct file_reference const *file,
	unsigned long position,
	unsigned long count,
	void const *buffer)
{
	return file_set_position(file, position) && file_write(file, count, buffer);
}

boolean file_get_last_modification_date(
	struct file_reference const *file,
	struct file_last_modification_date *date)
{
	WIN32_FILE_ATTRIBUTE_DATA data;
	struct file_reference_info *info = file_reference_get_info((struct file_reference *)file);
	char full_path[MAXIMUM_FILENAME_LENGTH+1] = {0};
	boolean success = FALSE;

	memset(date, 0, sizeof(*date));
	file_location_get_full_path(info->location, info->path, full_path);

	if (GetFileAttributesEx(full_path, GetFileExInfoStandard, &data))
	{
		memcpy(date, &data.ftLastWriteTime, sizeof(*date));
		success = TRUE;
	}

	if (!success)
	{
		file_error("file_get_last_modification_date", file);
	}

	return TRUE;
}

long file_compare_last_modification_dates(
	struct file_last_modification_date *date1,
	struct file_last_modification_date *date2)
{
	return memcmp(date1, date2, sizeof(*date1));
}

boolean file_get_size(
	struct file_reference const *file,
	unsigned long *size)
{
	WIN32_FILE_ATTRIBUTE_DATA data;
	struct file_reference_info *info = file_reference_get_info((struct file_reference *)file);
	char full_path[MAXIMUM_FILENAME_LENGTH+1] = {0};
	boolean success = FALSE;

	match_assert("c:\\halo\\SOURCE\\tag_files\\files_windows.c", 524, size);

	file_location_get_full_path(info->location, info->path, full_path);

	if (GetFileAttributesEx(full_path, GetFileExInfoStandard, &data))
	{
		*size = data.nFileSizeLow;
		success = TRUE;
	}

	if (!success)
	{
		file_error("file_get_size", file);
	}

	return success;
}

void find_files_start(
	unsigned long flags,
	struct file_reference const *directory)
{
	struct file_reference_info *info = file_reference_get_info((struct file_reference *)directory);
	short depth = find_files_globals.depth;

	match_assert("c:\\halo\\SOURCE\\tag_files\\files_windows.c", 548, VALID_FLAGS(flags, NUMBER_OF_FIND_FILES_FLAGS));
	match_assert("c:\\halo\\SOURCE\\tag_files\\files_windows.c", 549, !TEST_FLAG(info->flags, has_filename_bit));

	while (depth>=0)
	{
		if (find_files_globals.handles[depth]!=INVALID_HANDLE_VALUE)
		{
			CloseHandle(find_files_globals.handles[depth]);
			find_files_globals.handles[depth] = INVALID_HANDLE_VALUE;
		}

		depth--;
	}

	find_files_globals.flags = flags;
	find_files_globals.depth = 0;
	find_files_globals.location = info->location;
	strcpy(find_files_globals.path, info->path);

	return;
}

boolean find_files_next(
	struct file_reference *file,
	struct file_last_modification_date *date)
{
	char full_path[MAXIMUM_FILENAME_LENGTH+1] = {0};
	short depth = find_files_globals.depth;
	boolean found = FALSE;

	while (depth>=0)
	{
		if (find_files_globals.handles[depth]==INVALID_HANDLE_VALUE)
		{
			file_location_get_full_path(find_files_globals.location, find_files_globals.path, full_path);
			file_path_add_name(full_path, "*.*");
			find_files_globals.handles[depth] = FindFirstFile(full_path, &find_files_globals.data);

			if (find_files_globals.handles[depth]==INVALID_HANDLE_VALUE)
			{
				file_path_remove_name(find_files_globals.path);
				depth--;
				continue;
			}
		}
		else if (!FindNextFile(find_files_globals.handles[depth], &find_files_globals.data))
		{
			CloseHandle(find_files_globals.handles[depth]);
			find_files_globals.handles[depth] = INVALID_HANDLE_VALUE;
			file_path_remove_name(find_files_globals.path);
			depth--;
			continue;
		}

		if (find_files_globals.data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (strcmp(find_files_globals.data.cFileName, ".") && strcmp(find_files_globals.data.cFileName, ".."))
			{
				if (TEST_FLAG(find_files_globals.flags, _find_files_enumerate_directories_bit))
				{
					file_reference_create(file, find_files_globals.location);
					file_reference_add_directory(file, find_files_globals.path);
					file_reference_add_directory(file, find_files_globals.data.cFileName);
				}

				if (TEST_FLAG(find_files_globals.flags, _find_files_recursive_bit))
				{
					if (!TEST_FLAG(find_files_globals.flags, _find_files_enumerate_directories_bit))
					{
						file_path_add_name(find_files_globals.path, find_files_globals.data.cFileName);
					}

					depth++;
				}

				if (TEST_FLAG(find_files_globals.flags, _find_files_enumerate_directories_bit))
				{
					if (date)
					{
						memcpy(date, &find_files_globals.data.ftLastWriteTime, sizeof(*date));
					}

					found = TRUE;
					break;
				}
			}
		}
		else if (!TEST_FLAG(find_files_globals.flags, _find_files_enumerate_directories_bit))
		{
			file_reference_create(file, find_files_globals.location);
			file_reference_add_directory(file, find_files_globals.path);
			file_reference_set_name(file, find_files_globals.data.cFileName);

			if (date)
			{
				memcpy(date, &find_files_globals.data.ftLastWriteTime, sizeof(*date));
			}

			found = TRUE;
			break;
		}
	}

	find_files_globals.depth = depth;

	return found;
}

void file_path_add_name(
	char *path,
	char const *name)
{
	if (name[0]!='\0')
	{
		char *end;

		match_assert("c:\\halo\\SOURCE\\tag_files\\files_windows.c", 672, strlen(path)+1+strlen(name)<=MAXIMUM_FILENAME_LENGTH);
		end = path + strlen(path);

		if (end!=path)
		{
			*end++ = DIRECTORY_SEPARATOR;
			*end = '\0';
		}

		strncpy(end, name, MAXIMUM_FILENAME_LENGTH-strlen(path));
		path[MAXIMUM_FILENAME_LENGTH] = '\0';
	}

	return;
}

void file_path_add_extension(
	char *path,
	char const *extension)
{
	if (extension[0]!='\0')
	{
		char *end;

		match_assert("c:\\halo\\SOURCE\\tag_files\\files_windows.c", 696, strlen(path)+1+strlen(extension)<=MAXIMUM_FILENAME_LENGTH);
		end = path + strlen(path);

		if (end!=path)
		{
			*end++ = EXTENSION_SEPARATOR;
			*end = '\0';
		}

		strncpy(end, extension, MAXIMUM_FILENAME_LENGTH-strlen(path));
		path[MAXIMUM_FILENAME_LENGTH] = '\0';
	}

	return;
}

void file_path_remove_name(
	char *path)
{
	short index = strlen(path);

	while (index!=0 && get_previous_character((byte *)path, &index)!=DIRECTORY_SEPARATOR)
	{
	}

	if (get_next_character((byte *)path, &index)==DIRECTORY_SEPARATOR)
	{
		index--;
	}

	path[index] = '\0';

	return;
}

void file_path_split(
	char *path,
	char **directory,
	char **parent_directory,
	char **filename,
	char **extension,
	boolean has_filename)
{
	short index = strlen(path);

	*directory = &path[index];
	*parent_directory = &path[index];
	*filename = &path[index];
	*extension = &path[index];

	while (index!=0)
	{
		word character = get_previous_character((byte *)path, &index);

		if (character==EXTENSION_SEPARATOR)
		{
			if (has_filename && **filename=='\0' && **extension=='\0')
			{
				path[index] = '\0';
				*extension = &path[index+1];
			}
		}
		else if (character==DIRECTORY_SEPARATOR)
		{
			if (has_filename && **filename=='\0')
			{
				path[index] = '\0';
				*filename = &path[index+1];
			}
			else if (**parent_directory=='\0')
			{
				*parent_directory = &path[index+1];
			}
		}
	}

	if (has_filename && **filename=='\0')
	{
		*filename = path;
	}
	else if (*filename!=path)
	{
		*directory = path;
	}

	return;
}

void file_location_get_full_path(
	short location,
	char const *path,
	char *full_path)
{
	match_assert("c:\\halo\\SOURCE\\tag_files\\files_windows.c", 788, path && full_path);

	full_path[0] = '\0';

	if (path[0]=='\0' || path[1]=='\0' || path[2]=='\0' || !isalpha(path[0]) || path[1]!=':' || path[2]!=DIRECTORY_SEPARATOR)
	{
		strcpy(full_path, "d:\\");
	}

	strcat(full_path, path);

	return;
}

boolean file_read_only(
	struct file_reference const *file)
{
	char full_path[MAXIMUM_FILENAME_LENGTH+1];
	unsigned long attributes;
	struct file_reference_info *info = file_reference_get_info((struct file_reference *)file);
	boolean read_only = FALSE;

	file_location_get_full_path(info->location, info->path, full_path);
	attributes = GetFileAttributes(full_path);

	if (attributes!=-1 && (attributes & FILE_ATTRIBUTE_READONLY))
	{
		read_only = TRUE;
	}

	return read_only;
}

/* ---------- private code */

static void file_error(
	char const *function_name,
	struct file_reference const *file)
{
	struct file_reference_info *info = file_reference_get_info((struct file_reference *)file);
	unsigned long error_code = GetLastError();

	error(
		_error_silent,
		"%s('%s') error 0x%08x",
		function_name,
		info->path,
		error_code);
	SetLastError(ERROR_SUCCESS);

	return;
}
