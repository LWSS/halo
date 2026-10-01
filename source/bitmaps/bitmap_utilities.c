/*
BITMAP_UTILITIES.C

*/

/* ---------- headers */

#include "cseries.h"
#include "bitmaps.h"
#include "bitmap_macros.h"
#include "s3tc.h"

/* ---------- constants */

enum
{
	MAXIMUM_FILTER_SIZE= 10,
	NUMBER_OF_SHARPEN_TABLE_ENTRIES= 256, // [fake name]
};

/* ---------- macros */

/* ---------- structures */

/* ---------- prototypes */

static struct bitmap_data *bitmap_2d_shrink(struct bitmap_data const *source_bitmap, short scale, short alpha_bias, boolean ignore_zero_alpha);
static struct bitmap_data *bitmap_3d_shrink(struct bitmap_data const *source_bitmap, short scale, short alpha_bias, boolean ignore_zero_alpha);
static struct bitmap_data *bitmap_cm_shrink(struct bitmap_data const *source_bitmap, short scale, short alpha_bias, boolean ignore_zero_alpha);
static void bitmap_2d_smooth(struct bitmap_data *bitmap, short filter_size, short const *filter_coefficients);
static void bitmap_3d_smooth(struct bitmap_data *bitmap, short filter_size, short const *filter_coefficients);
static void bitmap_cm_smooth(struct bitmap_data *bitmap, short filter_size, short const *filter_coefficients);
static void bitmap_2d_sharpen(struct bitmap_data *bitmap, real sharpen_amount, short const *positive_table, short const *negative_table);
static void bitmap_3d_sharpen(struct bitmap_data *bitmap, real sharpen_amount, short const *positive_table, short const *negative_table);
static void bitmap_cm_sharpen(struct bitmap_data *bitmap, real sharpen_amount, short const *positive_table, short const *negative_table);
static void bitmap_2d_alpha_bleed(struct bitmap_data *bitmap, short passes);
static void bitmap_3d_alpha_bleed(struct bitmap_data *bitmap, short passes);
static void bitmap_cm_alpha_bleed(struct bitmap_data *bitmap, short passes);
static void bitmap_2d_height_map(struct bitmap_data *bitmap, real bump_height);
static void bitmap_3d_height_map(struct bitmap_data *bitmap, real bump_height);
static void bitmap_cm_height_map(struct bitmap_data *bitmap, real bump_height);
static void bitmap_2d_vector_map(struct bitmap_data *bitmap);
static void bitmap_3d_vector_map(struct bitmap_data *bitmap);
static void bitmap_cm_vector_map(struct bitmap_data *bitmap);
static void bitmap_2d_compress_to_mipmap(struct bitmap_data const *source_bitmap, struct bitmap_data *destination_bitmap, short destination_mipmap_index, pixel32 const *transparent_color);
static void bitmap_3d_compress_to_mipmap(struct bitmap_data const *source_bitmap, struct bitmap_data *destination_bitmap, short destination_mipmap_index, pixel32 const *transparent_color);
static void bitmap_cm_compress_to_mipmap(struct bitmap_data const *source_bitmap, struct bitmap_data *destination_bitmap, short destination_mipmap_index, pixel32 const *transparent_color);
static void bitmap_2d_uncompress_from_mipmap(struct bitmap_data const *source_bitmap, struct bitmap_data *destination_bitmap, short source_mipmap_index);
static void bitmap_3d_uncompress_from_mipmap(struct bitmap_data const *source_bitmap, struct bitmap_data *destination_bitmap, short source_mipmap_index);
static void bitmap_cm_uncompress_from_mipmap(struct bitmap_data const *source_bitmap, struct bitmap_data *destination_bitmap, short source_mipmap_index);

/* ---------- globals */

static real const word_to_real_scale= 1.f/UNSIGNED_SHORT_MAX; // [fake name]

static short negative_table[NUMBER_OF_SHARPEN_TABLE_ENTRIES]= {0};
static short positive_table[NUMBER_OF_SHARPEN_TABLE_ENTRIES]= {0};
static short filter_coefficients[MAXIMUM_FILTER_SIZE]= {0};

/* ---------- public code */

void bitmap_fill(
	struct bitmap_data *bitmap,
	pixel32 color)
{
	pixel32 *pixels= (pixel32 *)bitmap_2d_address(bitmap, 0, 0, 0);
	long pixel_count= bitmap_get_pixel_count(bitmap);
	long pixel_index;

	for (pixel_index= 0; pixel_index<pixel_count; pixel_index++)
	{
		pixels[pixel_index]= color;
	}

	return;
}

void bitmap_alpha_to_rgb(
	struct bitmap_data *bitmap)
{
	pixel32 *pixel= (pixel32 *)bitmap_2d_address(bitmap, 0, 0, 0);
	long pixel_count= bitmap_get_pixel_count(bitmap);
	long pixel_index;

	for (pixel_index= 0; pixel_index<pixel_count; pixel_index++, pixel++)
	{
		pixel32 alpha= PIXEL32_ALPHA(*pixel);

		*pixel= PIXEL32_FROM_ARGB(alpha, alpha, alpha, alpha);
	}

	return;
}

struct bitmap_data *bitmap_clone(
	struct bitmap_data const *source_bitmap)
{
	struct bitmap_data *cloned_bitmap= NULL;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 103, source_bitmap);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 104, source_bitmap->base_address);

	switch (source_bitmap->type)
	{
	case _bitmap_type_2d:
		cloned_bitmap= bitmap_2d_new(source_bitmap->width, source_bitmap->height, source_bitmap->mipmap_count, source_bitmap->format);
		break;
	case _bitmap_type_3d:
		cloned_bitmap= bitmap_3d_new(source_bitmap->width, source_bitmap->height, source_bitmap->depth, source_bitmap->mipmap_count, source_bitmap->format);
		break;
	case _bitmap_type_cube_map:
		cloned_bitmap= bitmap_cube_map_new(source_bitmap->width, source_bitmap->mipmap_count, source_bitmap->format);
		break;
	default:
		match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 131, FALSE, "### ERROR unsupported bitmap type");
	}

	if (cloned_bitmap && cloned_bitmap->base_address)
	{
		void const *source= bitmap_mipmap_address(source_bitmap, 0);
		void *destination= bitmap_mipmap_address(cloned_bitmap, 0);
		long pixel_data_size= bitmap_get_pixel_data_size(source_bitmap);

		match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 141, bitmap_get_pixel_data_size(cloned_bitmap)==pixel_data_size);
		memcpy(destination, source, pixel_data_size);
		cloned_bitmap->flags= source_bitmap->flags;
	}
	else
	{
		error(_error_silent, "### ERROR failed to allocate temporary bitmap");
	}

	return cloned_bitmap;
}

struct bitmap_data *bitmap_shrink(
	struct bitmap_data const *source_bitmap,
	short scale,
	short alpha_bias,
	boolean ignore_zero_alpha)
{
	struct bitmap_data *result= NULL;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 225, bitmap_verify(source_bitmap, TRUE));

	if (scale<=1)
	{
		result= bitmap_clone(source_bitmap);
	}
	else
	{
		switch (source_bitmap->type)
		{
		case _bitmap_type_2d:
			result= bitmap_2d_shrink(source_bitmap, scale, alpha_bias, ignore_zero_alpha);
			break;
		case _bitmap_type_3d:
			result= bitmap_3d_shrink(source_bitmap, scale, alpha_bias, ignore_zero_alpha);
			break;
		case _bitmap_type_cube_map:
			result= bitmap_cm_shrink(source_bitmap, scale, alpha_bias, ignore_zero_alpha);
			break;
		default:
			match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 243, FALSE, "### ERROR unupported bitmap type");
		}
	}

	return result;
}

static struct bitmap_data *bitmap_2d_shrink(
	struct bitmap_data const *source_bitmap,
	short scale,
	short alpha_bias,
	boolean ignore_zero_alpha)
{
	struct bitmap_data *destination_bitmap;
	short scale_width, scale_height;
	short width, height;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 261, bitmap_verify(source_bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 262, source_bitmap->type==_bitmap_type_2d);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 263, scale>1);

	scale_width= MIN(scale, source_bitmap->width);
	scale_height= MIN(scale, source_bitmap->height);
	width= source_bitmap->width/scale_width;
	height= source_bitmap->height/scale_height;

	destination_bitmap= bitmap_2d_new(width, height, 0, _bitmap_format_a8r8g8b8);
	if (destination_bitmap && destination_bitmap->base_address)
	{
		short x, y;

		for (y= 0; y<height; y++)
		{
			for (x= 0; x<width; x++)
			{
				long alpha= 0, red= 0, green= 0, blue= 0;
				long count= 0;
				pixel32 *destination= (pixel32 *)bitmap_2d_address(destination_bitmap, x, y, 0);
				short i, j;

				for (j= 0; j<scale_height; j++)
				{
					for (i= 0; i<scale_width; i++)
					{
						pixel32 pixel= *(pixel32 *)bitmap_2d_address(source_bitmap, x*scale_width + i, y*scale_height + j, 0);

						if (PIXEL32_ALPHA(pixel) || !ignore_zero_alpha)
						{
							alpha+= PIXEL32_ALPHA(pixel);
							red+= PIXEL32_RED(pixel);
							green+= PIXEL32_GREEN(pixel);
							blue+= PIXEL32_BLUE(pixel);
							count++;
						}
					}
				}

				if (count)
				{
					long half_count= count/2;
					long average_alpha= PIN((half_count+alpha)/count + alpha_bias, 0, PIXEL32_COMPONENT_MASK);

					*destination= PIXEL32_FROM_ARGB(average_alpha, (half_count+red)/count, (half_count+green)/count, (half_count+blue)/count);
				}
				else
				{
					*destination= 0;
				}
			}
		}
	}
	else
	{
		error(_error_silent, "### ERROR failed to allocate temporary bitmap");
	}

	return destination_bitmap;
}

static struct bitmap_data *bitmap_3d_shrink(
	struct bitmap_data const *source_bitmap,
	short scale,
	short alpha_bias,
	boolean ignore_zero_alpha)
{
	struct bitmap_data *destination_bitmap;
	short scale_width, scale_height, scale_depth;
	short width, height, depth;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 349, bitmap_verify(source_bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 350, source_bitmap->type==_bitmap_type_3d);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 351, scale>1);

	scale_width= MIN(scale, source_bitmap->width);
	scale_height= MIN(scale, source_bitmap->height);
	scale_depth= MIN(scale, source_bitmap->depth);
	width= source_bitmap->width/scale_width;
	height= source_bitmap->height/scale_height;
	depth= source_bitmap->depth/scale_depth;

	destination_bitmap= bitmap_3d_new(width, height, depth, 0, _bitmap_format_a8r8g8b8);
	if (destination_bitmap && destination_bitmap->base_address)
	{
		short x, y, z;

		for (z= 0; z<depth; z++)
		{
			for (y= 0; y<height; y++)
			{
				for (x= 0; x<width; x++)
				{
					long alpha= 0, red= 0, green= 0, blue= 0;
					long count= 0;
					pixel32 *destination= (pixel32 *)bitmap_3d_address(destination_bitmap, x, y, z, 0);
					short i, j, k;

					for (k= 0; k<scale_depth; k++)
					{
						for (j= 0; j<scale_height; j++)
						{
							for (i= 0; i<scale_width; i++)
							{
								pixel32 pixel= *(pixel32 *)bitmap_3d_address(source_bitmap, x*scale_width + i, y*scale_height + j, z*scale_depth + k, 0);

								if (PIXEL32_ALPHA(pixel) || !ignore_zero_alpha)
								{
									alpha+= PIXEL32_ALPHA(pixel);
									red+= PIXEL32_RED(pixel);
									green+= PIXEL32_GREEN(pixel);
									blue+= PIXEL32_BLUE(pixel);
									count++;
								}
							}
						}
					}

					if (count)
					{
						long half_count= count/2;
						long average_alpha= PIN((half_count+alpha)/count + alpha_bias, 0, PIXEL32_COMPONENT_MASK);

						*destination= PIXEL32_FROM_ARGB(average_alpha, (half_count+red)/count, (half_count+green)/count, (half_count+blue)/count);
					}
					else
					{
						*destination= 0;
					}
				}
			}
		}
	}
	else
	{
		error(_error_silent, "### ERROR failed to allocate temporary bitmap");
	}

	return destination_bitmap;
}

static struct bitmap_data *bitmap_cm_shrink(
	struct bitmap_data const *source_bitmap,
	short scale,
	short alpha_bias,
	boolean ignore_zero_alpha)
{
	struct bitmap_data *destination_bitmap;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 443, bitmap_verify(source_bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 444, source_bitmap->type==_bitmap_type_cube_map);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 445, scale>1);

	destination_bitmap= bitmap_cube_map_new(source_bitmap->width/MIN(scale, source_bitmap->width), 0, _bitmap_format_a8r8g8b8);
	if (destination_bitmap && destination_bitmap->base_address)
	{
		struct bitmap_data *face_bitmap= bitmap_2d_new(source_bitmap->width, source_bitmap->height, 0, _bitmap_format_a8r8g8b8);

		if (face_bitmap && face_bitmap->base_address)
		{
			short face_index;

			for (face_index= 0; face_index<NUMBER_OF_CUBE_MAP_FACES; face_index++)
			{
				struct bitmap_data *shrunk_bitmap;

				bitmap_cube_map_face_extract(source_bitmap, 0, face_index, face_bitmap);
				shrunk_bitmap= bitmap_2d_shrink(face_bitmap, scale, alpha_bias, ignore_zero_alpha);
				if (shrunk_bitmap && shrunk_bitmap->base_address)
				{
					bitmap_cube_map_face_insert(shrunk_bitmap, destination_bitmap, 0, face_index);
				}
				bitmap_delete(shrunk_bitmap);
			}
		}
		else
		{
			error(_error_silent, "### ERROR failed to allocate temporary bitmap");
		}
		bitmap_delete(face_bitmap);
	}
	else
	{
		error(_error_silent, "### ERROR failed to allocate temporary bitmap");
	}

	return destination_bitmap;
}

void bitmap_fade(
	struct bitmap_data *bitmap,
	pixel32 fade_color,
	real fade_amount)
{
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 501, bitmap_verify(bitmap, TRUE));

	if (fade_amount>0.f)
	{
		long fade= (long)floor(PIN(fade_amount, 0.f, 1.f)*256.f + 0.5f);
		long inverse_fade= 256-fade;
		long fade_alpha= PIXEL32_ALPHA(fade_color)*fade;
		long fade_red= PIXEL32_RED(fade_color)*fade;
		long fade_green= PIXEL32_GREEN(fade_color)*fade;
		long fade_blue= PIXEL32_BLUE(fade_color)*fade;
		pixel32 *pixels= bitmap_mipmap_address(bitmap, 0);
		long pixel_count= bitmap_get_pixel_count(bitmap);
		long pixel_index;

		for (pixel_index= 0; pixel_index<pixel_count; pixel_index++)
		{
			pixel32 pixel= pixels[pixel_index];
			pixel32 alpha= (PIXEL32_ALPHA(pixel)*inverse_fade + fade_alpha + PIXEL32_ONE_HALF)>>8;
			pixel32 red= (PIXEL32_RED(pixel)*inverse_fade + fade_red + PIXEL32_ONE_HALF)>>8;
			pixel32 green= (PIXEL32_GREEN(pixel)*inverse_fade + fade_green + PIXEL32_ONE_HALF)>>8;
			pixel32 blue= (PIXEL32_BLUE(pixel)*inverse_fade + fade_blue + PIXEL32_ONE_HALF)>>8;

			pixels[pixel_index]= PIXEL32_FROM_ARGB(alpha, red, green, blue);
		}
	}

	return;
}

void bitmap_smooth(
	struct bitmap_data *bitmap,
	real filter_size)
{
	short integer_filter_size= (short)floor(filter_size);

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 542, bitmap_verify(bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 543, filter_size<=(float)MAXIMUM_FILTER_SIZE);

	if (integer_filter_size>0.f)
	{
		short i;

		memset(filter_coefficients, 0, sizeof(filter_coefficients));
		for (i= 2*integer_filter_size; i>=0; i--)
		{
			short j;

			for (j= MAXIMUM_FILTER_SIZE-1; j>0; j--)
			{
				filter_coefficients[j]+= filter_coefficients[j-1];
			}
			filter_coefficients[0]= 1;
		}

		switch (bitmap->type)
		{
		case _bitmap_type_2d:
			bitmap_2d_smooth(bitmap, integer_filter_size, filter_coefficients);
			break;
		case _bitmap_type_3d:
			bitmap_3d_smooth(bitmap, integer_filter_size, filter_coefficients);
			break;
		case _bitmap_type_cube_map:
			bitmap_cm_smooth(bitmap, integer_filter_size, filter_coefficients);
			break;
		default:
			match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 580, FALSE, "### ERROR unsupported bitmap type");
		}
	}

	return;
}

static void bitmap_2d_smooth(
	struct bitmap_data *bitmap,
	short filter_size,
	short const *filter_coefficients)
{
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 592, bitmap_verify(bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 593, bitmap->type==_bitmap_type_2d);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 594, filter_coefficients);

	if (bitmap->width<filter_size || bitmap->height<filter_size)
	{
		fprintf(stdout, "### WARNING tried to smooth a bitmap with a filter which is too large", "\r\n");
		fflush(stdout);
	}
	else
	{
		long pixel_data_size= bitmap_get_pixel_data_size(bitmap);
		pixel32 *pixels= bitmap_mipmap_address(bitmap, 0);
		pixel32 *buffer= match_malloc("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 605, pixel_data_size);

		if (buffer)
		{
			short x, y;

			for (y= 0; y<bitmap->height; y++)
			{
				for (x= 0; x<bitmap->width; x++)
				{
					long alpha= 0, red= 0, green= 0, blue= 0;
					short i;

					for (i= -filter_size; i<=filter_size; i++)
					{
						short wrapped_x= (x+i+bitmap->width)%bitmap->width; // [fake name]
						pixel32 pixel= pixels[y*bitmap->width + wrapped_x];
						long coefficient= filter_coefficients[i+filter_size];

						alpha+= coefficient*PIXEL32_ALPHA(pixel);
						red+= coefficient*PIXEL32_RED(pixel);
						green+= coefficient*PIXEL32_GREEN(pixel);
						blue+= coefficient*PIXEL32_BLUE(pixel);
					}
					buffer[y*bitmap->width + x]= PIXEL32_FROM_ARGB(ROUNDED_SHIFT_RIGHT(alpha, 2*filter_size), ROUNDED_SHIFT_RIGHT(red, 2*filter_size), ROUNDED_SHIFT_RIGHT(green, 2*filter_size), ROUNDED_SHIFT_RIGHT(blue, 2*filter_size));
				}
			}

			for (y= 0; y<bitmap->height; y++)
			{
				for (x= 0; x<bitmap->width; x++)
				{
					long alpha= 0, red= 0, green= 0, blue= 0;
					short i;

					for (i= -filter_size; i<=filter_size; i++)
					{
						short wrapped_y= (y+i+bitmap->height)%bitmap->height; // [fake name]
						pixel32 pixel= buffer[wrapped_y*bitmap->width + x];
						long coefficient= filter_coefficients[i+filter_size];

						alpha+= coefficient*PIXEL32_ALPHA(pixel);
						red+= coefficient*PIXEL32_RED(pixel);
						green+= coefficient*PIXEL32_GREEN(pixel);
						blue+= coefficient*PIXEL32_BLUE(pixel);
					}
					pixels[y*bitmap->width + x]= PIXEL32_FROM_ARGB(ROUNDED_SHIFT_RIGHT(alpha, 2*filter_size), ROUNDED_SHIFT_RIGHT(red, 2*filter_size), ROUNDED_SHIFT_RIGHT(green, 2*filter_size), ROUNDED_SHIFT_RIGHT(blue, 2*filter_size));
				}
			}

			match_free("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 677, buffer);
		}
		else
		{
			error(_error_silent, "### ERROR failed to allocate temporary buffer");
		}
	}

	return;
}

static void bitmap_3d_smooth(
	struct bitmap_data *bitmap,
	short filter_size,
	short const *filter_coefficients)
{
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 698, bitmap_verify(bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 699, bitmap->type==_bitmap_type_3d);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 700, filter_coefficients);

	if (bitmap->width<filter_size || bitmap->height<filter_size || bitmap->depth<filter_size)
	{
		fprintf(stdout, "### WARNING tried to smooth a bitmap with a filter which is too large", "\r\n");
		fflush(stdout);
	}
	else
	{
		long pixel_data_size= bitmap_get_pixel_data_size(bitmap);
		pixel32 *pixels= bitmap_mipmap_address(bitmap, 0);
		pixel32 *buffer= match_malloc("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 711, pixel_data_size);

		if (buffer)
		{
			short x, y, z;

			for (z= 0; z<bitmap->depth; z++)
			{
				for (y= 0; y<bitmap->height; y++)
				{
					for (x= 0; x<bitmap->width; x++)
					{
						long alpha= 0, red= 0, green= 0, blue= 0;
						short i;

						for (i= -filter_size; i<=filter_size; i++)
						{
							short wrapped_x= (x+i+bitmap->width)%bitmap->width; // [fake name]
							pixel32 pixel= pixels[(z*bitmap->height + y)*bitmap->width + wrapped_x];
							long coefficient= filter_coefficients[i+filter_size];

							alpha+= coefficient*PIXEL32_ALPHA(pixel);
							red+= coefficient*PIXEL32_RED(pixel);
							green+= coefficient*PIXEL32_GREEN(pixel);
							blue+= coefficient*PIXEL32_BLUE(pixel);
						}
						buffer[(z*bitmap->height + y)*bitmap->width + x]= PIXEL32_FROM_ARGB(ROUNDED_SHIFT_RIGHT(alpha, 2*filter_size), ROUNDED_SHIFT_RIGHT(red, 2*filter_size), ROUNDED_SHIFT_RIGHT(green, 2*filter_size), ROUNDED_SHIFT_RIGHT(blue, 2*filter_size));
					}
				}
			}

			for (z= 0; z<bitmap->depth; z++)
			{
				for (y= 0; y<bitmap->height; y++)
				{
					for (x= 0; x<bitmap->width; x++)
					{
						long alpha= 0, red= 0, green= 0, blue= 0;
						short i;

						for (i= -filter_size; i<=filter_size; i++)
						{
							short wrapped_y= (y+i+bitmap->height)%bitmap->height; // [fake name]
							pixel32 pixel= buffer[(z*bitmap->height + wrapped_y)*bitmap->width + x];
							long coefficient= filter_coefficients[i+filter_size];

							alpha+= coefficient*PIXEL32_ALPHA(pixel);
							red+= coefficient*PIXEL32_RED(pixel);
							green+= coefficient*PIXEL32_GREEN(pixel);
							blue+= coefficient*PIXEL32_BLUE(pixel);
						}
						pixels[(z*bitmap->height + y)*bitmap->width + x]= PIXEL32_FROM_ARGB(ROUNDED_SHIFT_RIGHT(alpha, 2*filter_size), ROUNDED_SHIFT_RIGHT(red, 2*filter_size), ROUNDED_SHIFT_RIGHT(green, 2*filter_size), ROUNDED_SHIFT_RIGHT(blue, 2*filter_size));
					}
				}
			}

			for (z= 0; z<bitmap->depth; z++)
			{
				for (y= 0; y<bitmap->height; y++)
				{
					for (x= 0; x<bitmap->width; x++)
					{
						long alpha= 0, red= 0, green= 0, blue= 0;
						short i;

						for (i= -filter_size; i<=filter_size; i++)
						{
							short wrapped_z= (z+i+bitmap->depth)%bitmap->depth; // [fake name]
							pixel32 pixel= pixels[(wrapped_z*bitmap->height + y)*bitmap->width + x];
							long coefficient= filter_coefficients[i+filter_size];

							alpha+= coefficient*PIXEL32_ALPHA(pixel);
							red+= coefficient*PIXEL32_RED(pixel);
							green+= coefficient*PIXEL32_GREEN(pixel);
							blue+= coefficient*PIXEL32_BLUE(pixel);
						}
						buffer[(z*bitmap->height + y)*bitmap->width + x]= PIXEL32_FROM_ARGB(ROUNDED_SHIFT_RIGHT(alpha, 2*filter_size), ROUNDED_SHIFT_RIGHT(red, 2*filter_size), ROUNDED_SHIFT_RIGHT(green, 2*filter_size), ROUNDED_SHIFT_RIGHT(blue, 2*filter_size));
					}
				}
			}

			memcpy(pixels, buffer, pixel_data_size);
			match_free("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 830, buffer);
		}
		else
		{
			error(_error_silent, "### ERROR failed to allocate temporary buffer");
		}
	}

	return;
}

static void bitmap_cm_smooth(
	struct bitmap_data *bitmap,
	short filter_size,
	short const *filter_coefficients)
{
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 851, bitmap_verify(bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 852, bitmap->type==_bitmap_type_cube_map);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 853, filter_coefficients);

	fprintf(stdout, "### WARNING tried to smooth a cube map", "\r\n");
	fflush(stdout);

	return;
}

void bitmap_sharpen(
	struct bitmap_data *bitmap,
	real sharpen_amount)
{
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 865, bitmap_verify(bitmap, TRUE));

	if (sharpen_amount>0.f)
	{
		short percent= PIN((short)(sharpen_amount*100.f), 0, 100);
		short divisor= FLOOR(100-percent, 1);
		short i;

		for (i= 0; i<NUMBER_OF_SHARPEN_TABLE_ENTRIES; i++)
		{
			positive_table[i]= (100*i)/divisor;
			negative_table[i]= (percent*i/8)/divisor;
		}

		switch (bitmap->type)
		{
		case _bitmap_type_2d:
			bitmap_2d_sharpen(bitmap, sharpen_amount, positive_table, negative_table);
			break;
		case _bitmap_type_3d:
			bitmap_3d_sharpen(bitmap, sharpen_amount, positive_table, negative_table);
			break;
		case _bitmap_type_cube_map:
			bitmap_cm_sharpen(bitmap, sharpen_amount, positive_table, negative_table);
			break;
		default:
			match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 895, FALSE, "### ERROR unsupported bitmap type");
		}
	}

	return;
}

static void bitmap_2d_sharpen(
	struct bitmap_data *bitmap,
	real sharpen_amount,
	short const *positive_table,
	short const *negative_table)
{
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 908, bitmap_verify(bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 909, bitmap->type==_bitmap_type_2d);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 910, positive_table);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 911, negative_table);

	if (bitmap->width>=3 && bitmap->height>=3)
	{
		long pixel_data_size= bitmap_get_pixel_data_size(bitmap);
		byte *buffer= match_malloc("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 920, pixel_data_size);

		if (buffer)
		{
			short y;

			for (y= 0; y<bitmap->height; y++)
			{
				short previous_y= (y>0) ? y-1 : bitmap->height-1;
				short next_y= (y<bitmap->height-1) ? y+1 : 0;
				byte const *previous_row= (byte const *)bitmap_2d_address(bitmap, 0, previous_y, 0);
				byte const *row= (byte const *)bitmap_2d_address(bitmap, 0, y, 0);
				byte const *next_row= (byte const *)bitmap_2d_address(bitmap, 0, next_y, 0);
				byte *destination_row= buffer + 4*bitmap->width*y;
				short i= 0;
				short last_i;

				do
				{
					short wrapped_i= 4*bitmap->width + i;

					destination_row[i]= (byte)PIN(positive_table[row[i]]
						- negative_table[previous_row[wrapped_i-4]]
						- negative_table[previous_row[i]]
						- negative_table[previous_row[i+4]]
						- negative_table[row[wrapped_i-4]]
						- negative_table[row[i+4]]
						- negative_table[next_row[wrapped_i-4]]
						- negative_table[next_row[i]]
						- negative_table[next_row[i+4]], 0, PIXEL32_COMPONENT_MASK);
					i++;
				}
				while (i<4);

				for (last_i= 4*bitmap->width-4; i<last_i; i++)
				{
					destination_row[i]= (byte)PIN(positive_table[row[i]]
						- negative_table[previous_row[i-4]]
						- negative_table[previous_row[i]]
						- negative_table[previous_row[i+4]]
						- negative_table[row[i-4]]
						- negative_table[row[i+4]]
						- negative_table[next_row[i-4]]
						- negative_table[next_row[i]]
						- negative_table[next_row[i+4]], 0, PIXEL32_COMPONENT_MASK);
				}

				for (last_i+= 4; i<last_i; i++)
				{
					short wrapped_i= i - 4*bitmap->width;

					destination_row[i]= (byte)PIN(positive_table[row[i]]
						- negative_table[previous_row[i-4]]
						- negative_table[previous_row[i]]
						- negative_table[previous_row[wrapped_i+4]]
						- negative_table[row[i-4]]
						- negative_table[row[wrapped_i+4]]
						- negative_table[next_row[i-4]]
						- negative_table[next_row[i]]
						- negative_table[next_row[wrapped_i+4]], 0, PIXEL32_COMPONENT_MASK);
				}
			}

			memcpy(bitmap_mipmap_address(bitmap, 0), buffer, pixel_data_size);
			match_free("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 977, buffer);
		}
		else
		{
			error(_error_silent, "### ERROR failed to allocate temporary buffer");
		}
	}

	return;
}

static void bitmap_3d_sharpen(
	struct bitmap_data *bitmap,
	real sharpen_amount,
	short const *positive_table,
	short const *negative_table)
{
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 994, bitmap_verify(bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 995, bitmap->type==_bitmap_type_3d);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 996, positive_table);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 997, negative_table);

	fprintf(stdout, "### WARNING tried to sharpen a 3d bitmap", "\r\n");
	fflush(stdout);

	return;
}

static void bitmap_cm_sharpen(
	struct bitmap_data *bitmap,
	real sharpen_amount,
	short const *positive_table,
	short const *negative_table)
{
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1011, bitmap_verify(bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1012, bitmap->type==_bitmap_type_cube_map);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1013, positive_table);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1014, negative_table);

	fprintf(stdout, "### WARNING tried to sharpen a cube map", "\r\n");
	fflush(stdout);

	return;
}

void bitmap_alpha_bleed(
	struct bitmap_data *bitmap,
	short passes)
{
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1026, bitmap_verify(bitmap, TRUE));

	if (passes>0)
	{
		switch (bitmap->type)
		{
		case _bitmap_type_2d:
			bitmap_2d_alpha_bleed(bitmap, passes);
			break;
		case _bitmap_type_3d:
			bitmap_3d_alpha_bleed(bitmap, passes);
			break;
		case _bitmap_type_cube_map:
			bitmap_cm_alpha_bleed(bitmap, passes);
			break;
		default:
			match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1040, FALSE, "### ERROR unsupported bitmap type");
		}
	}

	return;
}

static void bitmap_2d_alpha_bleed(
	struct bitmap_data *bitmap,
	short passes)
{
	long pixel_data_size;
	pixel32 *buffer;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1053, bitmap_verify(bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1054, bitmap->type==_bitmap_type_2d);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1055, passes>0);

	pixel_data_size= bitmap_get_pixel_data_size(bitmap);
	buffer= match_malloc("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1058, pixel_data_size);
	if (buffer)
	{
		short pass;

		for (pass= 0; pass<passes; pass++)
		{
			short y;

			for (y= 0; y<bitmap->height; y++)
			{
				pixel32 const *row= (pixel32 const *)bitmap_2d_address(bitmap, 0, y, 0);
				pixel32 *destination_row= buffer + y*bitmap->width;
				short x;

				for (x= 0; x<bitmap->width; x++)
				{
					pixel32 pixel= row[x];

					if (!PIXEL32_ALPHA_BITS(pixel))
					{
						boolean found= FALSE;
						short dy;

						for (dy= -1; !found && dy<=1; dy++)
						{
							short dx;

							for (dx= -1; !found && dx<=1; dx++)
							{
								short neighbor_x= x+dx;
								short neighbor_y= y+dy;

								if (neighbor_x>=0 && neighbor_y>=0 && neighbor_x<bitmap->width && neighbor_y<bitmap->height)
								{
									pixel32 neighbor= *(pixel32 *)bitmap_2d_address(bitmap, neighbor_x, neighbor_y, 0);

									if (neighbor)
									{
										pixel= PIXEL32_RGB_BITS(neighbor);
										found= TRUE;
									}
								}
							}
						}
					}
					destination_row[x]= pixel;
				}
			}

			memcpy(bitmap_mipmap_address(bitmap, 0), buffer, pixel_data_size);
		}

		match_free("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1122, buffer);
	}
	else
	{
		error(_error_silent, "### ERROR failed to allocate temporary buffer");
	}

	return;
}

static void bitmap_3d_alpha_bleed(
	struct bitmap_data *bitmap,
	short passes)
{
	struct bitmap_data *slice_bitmap;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1138, bitmap_verify(bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1139, bitmap->type==_bitmap_type_3d);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1140, passes>0);

	slice_bitmap= bitmap_2d_new(bitmap->width, bitmap->height, 0, bitmap->format);
	if (slice_bitmap && slice_bitmap->base_address)
	{
		short slice_index;

		for (slice_index= 0; slice_index<bitmap->depth; slice_index++)
		{
			bitmap_3d_slice_extract(bitmap, 0, slice_index, slice_bitmap);
			bitmap_2d_alpha_bleed(slice_bitmap, passes);
			bitmap_3d_slice_insert(slice_bitmap, bitmap, 0, slice_index);
		}
	}
	else
	{
		error(_error_silent, "### ERROR failed to allocate temporary bitmap");
	}
	bitmap_delete(slice_bitmap);

	return;
}

static void bitmap_cm_alpha_bleed(
	struct bitmap_data *bitmap,
	short passes)
{
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1185, bitmap_verify(bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1186, bitmap->type==_bitmap_type_cube_map);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1187, passes>0);

	fprintf(stdout, "### WARNING tried to alpha-bleed a cube map (skipping)");
	fflush(stdout);

	return;
}

void bitmap_height_map(
	struct bitmap_data *bitmap,
	real bump_height)
{
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1199, bitmap_verify(bitmap, TRUE));

	if (bump_height>0.f)
	{
		switch (bitmap->type)
		{
		case _bitmap_type_2d:
			bitmap_2d_height_map(bitmap, bump_height);
			break;
		case _bitmap_type_3d:
			bitmap_3d_height_map(bitmap, bump_height);
			break;
		case _bitmap_type_cube_map:
			bitmap_cm_height_map(bitmap, bump_height);
			break;
		default:
			match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1213, FALSE, "### ERROR unsupported bitmap type");
		}
	}
	else
	{
		fprintf(stdout, "### WARNING importing special-effect bump map with zero-height\r\n");
		fflush(stdout);
	}

	return;
}

static void bitmap_2d_height_map(
	struct bitmap_data *bitmap,
	real bump_height)
{
	long pixel_data_size;
	pixel32 *buffer;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1231, bitmap_verify(bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1232, bitmap->type==_bitmap_type_2d);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1233, bump_height>0.0f);

	pixel_data_size= bitmap_get_pixel_data_size(bitmap);
	buffer= match_malloc("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1236, pixel_data_size);
	if (buffer)
	{
		real scale= bitmap->height*bump_height/255.f;
		short x, y;

		for (y= 0; y<bitmap->height; y++)
		{
			for (x= 0; x<bitmap->width; x++)
			{
				pixel32 center= *(pixel32 *)bitmap_2d_address(bitmap, x, y, 0);
				pixel32 left= *(pixel32 *)bitmap_2d_address(bitmap, (x==0) ? bitmap->width-1 : x-1, y, 0);
				pixel32 right= *(pixel32 *)bitmap_2d_address(bitmap, (x==bitmap->width-1) ? 0 : x+1, y, 0);
				pixel32 top= *(pixel32 *)bitmap_2d_address(bitmap, x, ((y==0) ? bitmap->height : y)-1, 0);
				pixel32 bottom= *(pixel32 *)bitmap_2d_address(bitmap, x, (y==bitmap->height-1) ? 0 : y+1, 0);
				real center_height= PIXEL32_RED(center)*scale;
				real left_height= PIXEL32_RED(left)*scale;
				real right_height= PIXEL32_RED(right)*scale;
				real top_height= PIXEL32_RED(top)*scale;
				real bottom_height= PIXEL32_RED(bottom)*scale;
				real_vector3d u, v, normal;

				u.j= 0.f;
				if (center_height>left_height && center_height>right_height)
				{
					u.i= 1.f;
					u.k= 0.f;
				}
				else if (left_height>right_height)
				{
					u.i= -1.f;
					u.k= left_height-center_height;
				}
				else
				{
					u.i= 1.f;
					u.k= right_height-center_height;
				}

				v.i= 0.f;
				if (center_height>top_height && center_height>bottom_height)
				{
					v.j= 1.f;
					v.k= 0.f;
				}
				else if (top_height>bottom_height)
				{
					v.j= -1.f;
					v.k= top_height-center_height;
				}
				else
				{
					v.j= 1.f;
					v.k= bottom_height-center_height;
				}

				cross_product3d(&u, &v, &normal);
				if (normal.k<0.f)
				{
					normal.i= -normal.i;
					normal.j= -normal.j;
					normal.k= -normal.k;
				}
				normalize3d(&normal);

				buffer[y*bitmap->width + x]= PIXEL32_ALPHA_BITS(center) | PIXEL32_FROM_ARGB(0, fast_ftol((normal.i+1.f)*127.5f), fast_ftol((normal.j+1.f)*127.5f), fast_ftol((normal.k+1.f)*127.5f));
			}
		}

		memcpy(bitmap_mipmap_address(bitmap, 0), buffer, pixel_data_size);
		match_free("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1309, buffer);
	}
	else
	{
		error(_error_silent, "### ERROR failed to allocate temporary buffer");
	}

	return;
}

static void bitmap_3d_height_map(
	struct bitmap_data *bitmap,
	real bump_height)
{
	struct bitmap_data *slice_bitmap;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1325, bitmap_verify(bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1326, bitmap->type==_bitmap_type_3d);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1327, bump_height>0.0f);

	slice_bitmap= bitmap_2d_new(bitmap->width, bitmap->height, 0, bitmap->format);
	if (slice_bitmap && slice_bitmap->base_address)
	{
		short slice_index;

		for (slice_index= 0; slice_index<bitmap->depth; slice_index++)
		{
			bitmap_3d_slice_extract(bitmap, 0, slice_index, slice_bitmap);
			bitmap_2d_height_map(slice_bitmap, bump_height);
			bitmap_3d_slice_insert(slice_bitmap, bitmap, 0, slice_index);
		}
	}
	else
	{
		error(_error_silent, "### ERROR failed to allocate temporary bitmap");
	}
	bitmap_delete(slice_bitmap);

	return;
}

static void bitmap_cm_height_map(
	struct bitmap_data *bitmap,
	real bump_height)
{
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1372, bitmap_verify(bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1373, bitmap->type==_bitmap_type_cube_map);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1374, bump_height>0.0f);

	fprintf(stdout, "### WARNING tried to use a cube map as a height map\r\n");
	fflush(stdout);

	return;
}

void bitmap_vector_map(
	struct bitmap_data *bitmap)
{
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1385, bitmap_verify(bitmap, TRUE));

	switch (bitmap->type)
	{
	case _bitmap_type_2d:
		bitmap_2d_vector_map(bitmap);
		break;
	case _bitmap_type_3d:
		bitmap_3d_vector_map(bitmap);
		break;
	case _bitmap_type_cube_map:
		bitmap_cm_vector_map(bitmap);
		break;
	default:
		match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1399, FALSE, "### ERROR unsupported bitmap type");
	}

	return;
}

static void bitmap_2d_vector_map(
	struct bitmap_data *bitmap)
{
	long pixel_data_size;
	pixel32 *buffer;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1411, bitmap_verify(bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1412, bitmap->type==_bitmap_type_2d);

	pixel_data_size= bitmap_get_pixel_data_size(bitmap);
	buffer= match_malloc("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1415, pixel_data_size);
	if (buffer)
	{
		short x, y;

		for (y= 0; y<bitmap->height; y++)
		{
			for (x= 0; x<bitmap->width; x++)
			{
				pixel32 pixel= *(pixel32 *)bitmap_2d_address(bitmap, x, y, 0);
				real_vector3d vector;

				vector.i= PIXEL32_RED(pixel)/127.5f - 1.f;
				vector.j= PIXEL32_GREEN(pixel)/127.5f - 1.f;
				vector.k= PIXEL32_BLUE(pixel)/127.5f - 1.f;
				normalize3d(&vector);

				buffer[y*bitmap->width + x]= PIXEL32_ALPHA_BITS(pixel) | PIXEL32_FROM_ARGB(0, fast_ftol((vector.i+1.f)*127.5f + 0.5f), fast_ftol((vector.j+1.f)*127.5f + 0.5f), fast_ftol((vector.k+1.f)*127.5f + 0.5f));
			}
		}

		memcpy(bitmap_mipmap_address(bitmap, 0), buffer, pixel_data_size);
		match_free("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1448, buffer);
	}
	else
	{
		error(_error_silent, "### ERROR failed to allocate temporary buffer");
	}

	return;
}

static void bitmap_3d_vector_map(
	struct bitmap_data *bitmap)
{
	struct bitmap_data *slice_bitmap;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1463, bitmap_verify(bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1464, bitmap->type==_bitmap_type_3d);

	slice_bitmap= bitmap_2d_new(bitmap->width, bitmap->height, 0, bitmap->format);
	if (slice_bitmap && slice_bitmap->base_address)
	{
		short slice_index;

		for (slice_index= 0; slice_index<bitmap->depth; slice_index++)
		{
			bitmap_3d_slice_extract(bitmap, 0, slice_index, slice_bitmap);
			bitmap_2d_vector_map(slice_bitmap);
			bitmap_3d_slice_insert(slice_bitmap, bitmap, 0, slice_index);
		}
	}
	else
	{
		error(_error_silent, "### ERROR failed to allocate temporary bitmap");
	}
	bitmap_delete(slice_bitmap);

	return;
}

static void bitmap_cm_vector_map(
	struct bitmap_data *bitmap)
{
	struct bitmap_data *face_bitmap;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1509, bitmap_verify(bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1510, bitmap->type==_bitmap_type_cube_map);

	face_bitmap= bitmap_2d_new(bitmap->width, bitmap->height, 0, bitmap->format);
	if (face_bitmap && face_bitmap->base_address)
	{
		short face_index;

		for (face_index= 0; face_index<NUMBER_OF_CUBE_MAP_FACES; face_index++)
		{
			bitmap_cube_map_face_extract(bitmap, 0, face_index, face_bitmap);
			bitmap_2d_vector_map(face_bitmap);
			bitmap_cube_map_face_insert(face_bitmap, bitmap, 0, face_index);
		}
	}
	else
	{
		error(_error_silent, "### ERROR failed to allocate temporary bitmap");
	}
	bitmap_delete(face_bitmap);

	return;
}

void bitmap_compress_to_mipmap(
	struct bitmap_data const *source_bitmap,
	struct bitmap_data *destination_bitmap,
	short destination_mipmap_index,
	pixel32 const *transparent_color)
{
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1561, bitmap_verify(source_bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1563, bitmap_verify(destination_bitmap, FALSE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1564, destination_mipmap_index>=0 && destination_mipmap_index<=destination_bitmap->mipmap_count);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1565, MAX(1, destination_bitmap->width >>destination_mipmap_index)==source_bitmap->width);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1566, MAX(1, destination_bitmap->height>>destination_mipmap_index)==source_bitmap->height);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1567, MAX(1, destination_bitmap->depth >>destination_mipmap_index)==source_bitmap->depth);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1568, TEST_FLAG(destination_bitmap->flags, _bitmap_compressed_bit));

	switch (source_bitmap->type)
	{
	case _bitmap_type_2d:
		bitmap_2d_compress_to_mipmap(source_bitmap, destination_bitmap, destination_mipmap_index, transparent_color);
		break;
	case _bitmap_type_3d:
		bitmap_3d_compress_to_mipmap(source_bitmap, destination_bitmap, destination_mipmap_index, transparent_color);
		break;
	case _bitmap_type_cube_map:
		bitmap_cm_compress_to_mipmap(source_bitmap, destination_bitmap, destination_mipmap_index, transparent_color);
		break;
	default:
		match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1584, FALSE, "### ERROR unsupported bitmap type");
	}

	return;
}

static void bitmap_2d_compress_to_mipmap(
	struct bitmap_data const *source_bitmap,
	struct bitmap_data *destination_bitmap,
	short destination_mipmap_index,
	pixel32 const *transparent_color)
{
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1596, bitmap_verify(source_bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1597, source_bitmap->type==_bitmap_type_2d);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1599, bitmap_verify(destination_bitmap, FALSE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1600, destination_bitmap->type==_bitmap_type_2d);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1601, destination_mipmap_index>=0 && destination_mipmap_index<=destination_bitmap->mipmap_count);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1602, MAX(1, destination_bitmap->width >>destination_mipmap_index)==source_bitmap->width);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1603, MAX(1, destination_bitmap->height>>destination_mipmap_index)==source_bitmap->height);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1604, MAX(1, destination_bitmap->depth >>destination_mipmap_index)==source_bitmap->depth);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1605, TEST_FLAG(destination_bitmap->flags, _bitmap_compressed_bit));

	match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1695, FALSE, NULL);

	return;
}

static void bitmap_3d_compress_to_mipmap(
	struct bitmap_data const *source_bitmap,
	struct bitmap_data *destination_bitmap,
	short destination_mipmap_index,
	pixel32 const *transparent_color)
{
	struct bitmap_data *source_slice_bitmap;
	struct bitmap_data *destination_slice_bitmap;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1711, bitmap_verify(source_bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1712, source_bitmap->type==_bitmap_type_3d);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1714, bitmap_verify(destination_bitmap, FALSE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1715, destination_bitmap->type==_bitmap_type_3d);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1716, destination_mipmap_index>=0 && destination_mipmap_index<=destination_bitmap->mipmap_count);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1717, MAX(1, destination_bitmap->width >>destination_mipmap_index)==source_bitmap->width);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1718, MAX(1, destination_bitmap->height>>destination_mipmap_index)==source_bitmap->height);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1719, MAX(1, destination_bitmap->depth >>destination_mipmap_index)==source_bitmap->depth);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1720, TEST_FLAG(destination_bitmap->flags, _bitmap_compressed_bit));

	source_slice_bitmap= bitmap_2d_new(source_bitmap->width, source_bitmap->height, 0, source_bitmap->format);
	destination_slice_bitmap= bitmap_2d_new(source_bitmap->width, source_bitmap->height, 0, destination_bitmap->format);
	if (source_slice_bitmap && source_slice_bitmap->base_address && destination_slice_bitmap && destination_slice_bitmap->base_address)
	{
		short slice_index;

		for (slice_index= 0; slice_index<source_bitmap->depth; slice_index++)
		{
			bitmap_3d_slice_extract(source_bitmap, 0, slice_index, source_slice_bitmap);
			bitmap_2d_compress_to_mipmap(source_slice_bitmap, destination_slice_bitmap, 0, transparent_color);
			bitmap_3d_slice_insert(destination_slice_bitmap, destination_bitmap, destination_mipmap_index, slice_index);
		}
	}
	else
	{
		error(_error_silent, "### ERROR failed to allocate temporary bitmap");
	}
	bitmap_delete(source_slice_bitmap);
	bitmap_delete(destination_slice_bitmap);

	return;
}

static void bitmap_cm_compress_to_mipmap(
	struct bitmap_data const *source_bitmap,
	struct bitmap_data *destination_bitmap,
	short destination_mipmap_index,
	pixel32 const *transparent_color)
{
	struct bitmap_data *source_face_bitmap;
	struct bitmap_data *destination_face_bitmap;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1786, bitmap_verify(source_bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1787, source_bitmap->type==_bitmap_type_cube_map);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1789, bitmap_verify(destination_bitmap, FALSE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1790, destination_bitmap->type==_bitmap_type_cube_map);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1791, destination_mipmap_index>=0 && destination_mipmap_index<=destination_bitmap->mipmap_count);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1792, MAX(1, destination_bitmap->width >>destination_mipmap_index)==source_bitmap->width);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1793, MAX(1, destination_bitmap->height>>destination_mipmap_index)==source_bitmap->height);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1794, MAX(1, destination_bitmap->depth >>destination_mipmap_index)==source_bitmap->depth);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1795, TEST_FLAG(destination_bitmap->flags, _bitmap_compressed_bit));

	source_face_bitmap= bitmap_2d_new(source_bitmap->width, source_bitmap->height, 0, source_bitmap->format);
	destination_face_bitmap= bitmap_2d_new(source_bitmap->width, source_bitmap->height, 0, destination_bitmap->format);
	if (source_face_bitmap && source_face_bitmap->base_address && destination_face_bitmap && destination_face_bitmap->base_address)
	{
		short face_index;

		for (face_index= 0; face_index<NUMBER_OF_CUBE_MAP_FACES; face_index++)
		{
			bitmap_cube_map_face_extract(source_bitmap, 0, face_index, source_face_bitmap);
			bitmap_2d_compress_to_mipmap(source_face_bitmap, destination_face_bitmap, 0, transparent_color);
			bitmap_cube_map_face_insert(destination_face_bitmap, destination_bitmap, destination_mipmap_index, face_index);
		}
	}
	else
	{
		error(_error_silent, "### ERROR failed to allocate temporary bitmap");
	}
	bitmap_delete(source_face_bitmap);
	bitmap_delete(destination_face_bitmap);

	return;
}

void bitmap_uncompress_from_mipmap(
	struct bitmap_data const *source_bitmap,
	struct bitmap_data *destination_bitmap,
	short source_mipmap_index)
{
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1862, bitmap_verify(source_bitmap, FALSE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1863, source_mipmap_index>=0 && source_mipmap_index<=source_bitmap->mipmap_count);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1864, MAX(1, source_bitmap->width >>source_mipmap_index)==destination_bitmap->width);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1865, MAX(1, source_bitmap->height>>source_mipmap_index)==destination_bitmap->height);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1866, MAX(1, source_bitmap->depth >>source_mipmap_index)==destination_bitmap->depth);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1867, TEST_FLAG(source_bitmap->flags, _bitmap_compressed_bit));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1869, bitmap_verify(destination_bitmap, TRUE));

	switch (source_bitmap->type)
	{
	case _bitmap_type_2d:
		bitmap_2d_uncompress_from_mipmap(source_bitmap, destination_bitmap, source_mipmap_index);
		break;
	case _bitmap_type_3d:
		bitmap_3d_uncompress_from_mipmap(source_bitmap, destination_bitmap, source_mipmap_index);
		break;
	case _bitmap_type_cube_map:
		bitmap_cm_uncompress_from_mipmap(source_bitmap, destination_bitmap, source_mipmap_index);
		break;
	default:
		match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1883, FALSE, "### ERROR unsupported bitmap type");
	}

	return;
}

static void bitmap_2d_uncompress_from_mipmap(
	struct bitmap_data const *source_bitmap,
	struct bitmap_data *destination_bitmap,
	short source_mipmap_index)
{
	byte *source;
	short height;
	short y;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1894, bitmap_verify(source_bitmap, FALSE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1895, source_bitmap->type==_bitmap_type_2d);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1896, source_mipmap_index>=0 && source_mipmap_index<=source_bitmap->mipmap_count);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1897, MAX(1, source_bitmap->width >>source_mipmap_index)==destination_bitmap->width);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1898, MAX(1, source_bitmap->height>>source_mipmap_index)==destination_bitmap->height);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1899, MAX(1, source_bitmap->depth >>source_mipmap_index)==destination_bitmap->depth);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1900, TEST_FLAG(source_bitmap->flags, _bitmap_compressed_bit));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1902, bitmap_verify(destination_bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1903, destination_bitmap->type==_bitmap_type_2d);

	source= bitmap_mipmap_address(source_bitmap, source_mipmap_index);
	height= bitmap_mipmap_get_height(source_bitmap, source_mipmap_index);
	for (y= 0; y<height; y+= 4)
	{
		short width= bitmap_mipmap_get_width(source_bitmap, source_mipmap_index);
		short x;

		for (x= 0; x<width; x+= 4)
		{
			struct S3TC_COLOR colors[16];
			short color_index= 0;
			short i, j;

			switch (source_bitmap->format)
			{
			case _bitmap_format_dxt1:
				DecodeBlockRGB((struct S3TCBlockRGB *)source, colors);
				source+= sizeof(struct S3TCBlockRGB);
				break;
			case _bitmap_format_dxt3:
				DecodeBlockAlpha4((struct S3TCBlockAlpha4 *)source, colors);
				source+= sizeof(struct S3TCBlockAlpha4);
				break;
			case _bitmap_format_dxt5:
				DecodeBlockAlpha3((struct S3TCBlockAlpha3 *)source, colors);
				source+= sizeof(struct S3TCBlockAlpha3);
				break;
			default:
				match_vassert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1932, FALSE, "### ERROR unsupported bitmap format");
			}

			for (j= 0; j<4; j++)
			{
				for (i= 0; i<4; i++)
				{
					if (x+i<destination_bitmap->width && y+j<destination_bitmap->height)
					{
						struct S3TC_COLOR *destination= (struct S3TC_COLOR *)bitmap_2d_address(destination_bitmap, x+i, y+j, 0);

						*destination= colors[color_index++];
					}
				}
			}
		}
	}

	return;
}

static void bitmap_3d_uncompress_from_mipmap(
	struct bitmap_data const *source_bitmap,
	struct bitmap_data *destination_bitmap,
	short source_mipmap_index)
{
	struct bitmap_data *source_slice_bitmap;
	struct bitmap_data *destination_slice_bitmap;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1968, bitmap_verify(source_bitmap, FALSE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1969, source_bitmap->type==_bitmap_type_3d);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1970, source_mipmap_index>=0 && source_mipmap_index<=source_bitmap->mipmap_count);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1971, MAX(1, source_bitmap->width >>source_mipmap_index)==destination_bitmap->width);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1972, MAX(1, source_bitmap->height>>source_mipmap_index)==destination_bitmap->height);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1973, MAX(1, source_bitmap->depth >>source_mipmap_index)==destination_bitmap->depth);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1974, TEST_FLAG(source_bitmap->flags, _bitmap_compressed_bit));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1976, bitmap_verify(destination_bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 1977, destination_bitmap->type==_bitmap_type_3d);

	source_slice_bitmap= bitmap_2d_new(destination_bitmap->width, destination_bitmap->height, 0, source_bitmap->format);
	destination_slice_bitmap= bitmap_2d_new(destination_bitmap->width, destination_bitmap->height, 0, destination_bitmap->format);
	if (source_slice_bitmap && source_slice_bitmap->base_address && destination_slice_bitmap && destination_slice_bitmap->base_address)
	{
		short slice_index;

		for (slice_index= 0; slice_index<source_bitmap->depth; slice_index++)
		{
			bitmap_3d_slice_extract(source_bitmap, source_mipmap_index, slice_index, source_slice_bitmap);
			bitmap_2d_uncompress_from_mipmap(source_slice_bitmap, destination_slice_bitmap, 0);
			bitmap_3d_slice_insert(destination_slice_bitmap, destination_bitmap, 0, slice_index);
		}
	}
	else
	{
		error(_error_silent, "### ERROR failed to allocate temporary bitmap");
	}
	bitmap_delete(source_slice_bitmap);
	bitmap_delete(destination_slice_bitmap);

	return;
}

static void bitmap_cm_uncompress_from_mipmap(
	struct bitmap_data const *source_bitmap,
	struct bitmap_data *destination_bitmap,
	short source_mipmap_index)
{
	struct bitmap_data *source_face_bitmap;
	struct bitmap_data *destination_face_bitmap;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 2041, bitmap_verify(source_bitmap, FALSE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 2042, source_bitmap->type==_bitmap_type_cube_map);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 2043, source_mipmap_index>=0 && source_mipmap_index<=source_bitmap->mipmap_count);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 2044, MAX(1, source_bitmap->width >>source_mipmap_index)==destination_bitmap->width);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 2045, MAX(1, source_bitmap->height>>source_mipmap_index)==destination_bitmap->height);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 2046, MAX(1, source_bitmap->depth >>source_mipmap_index)==destination_bitmap->depth);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 2047, TEST_FLAG(source_bitmap->flags, _bitmap_compressed_bit));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 2049, bitmap_verify(destination_bitmap, TRUE));
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 2050, destination_bitmap->type==_bitmap_type_cube_map);

	source_face_bitmap= bitmap_2d_new(destination_bitmap->width, destination_bitmap->height, 0, source_bitmap->format);
	destination_face_bitmap= bitmap_2d_new(destination_bitmap->width, destination_bitmap->height, 0, destination_bitmap->format);
	if (source_face_bitmap && source_face_bitmap->base_address && destination_face_bitmap && destination_face_bitmap->base_address)
	{
		short face_index;

		for (face_index= 0; face_index<NUMBER_OF_CUBE_MAP_FACES; face_index++)
		{
			bitmap_cube_map_face_extract(source_bitmap, source_mipmap_index, face_index, source_face_bitmap);
			bitmap_2d_uncompress_from_mipmap(source_face_bitmap, destination_face_bitmap, 0);
			bitmap_cube_map_face_insert(destination_face_bitmap, destination_bitmap, 0, face_index);
		}
	}
	else
	{
		error(_error_silent, "### ERROR failed to allocate temporary bitmap");
	}
	bitmap_delete(source_face_bitmap);
	bitmap_delete(destination_face_bitmap);

	return;
}

real real_rgb_color_brightness(
	union real_rgb_color const *color)
{
	return 0.299f*color->red + 0.587f*color->green + 0.114f*color->blue;
}

union hsv_color *rgb_color_to_hsv_color(
	union rgb_color const *rgb,
	union hsv_color *hsv)
{
	real red= rgb->red/65535.f;
	real green= rgb->green/65535.f;
	real blue= rgb->blue/65535.f;
	real maximum= MAX(red, MAX(green, blue));
	real minimum= MIN(red, MIN(green, blue));
	real delta= maximum-minimum;
	real hue, saturation, value;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 2130, hsv);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 2131, rgb!=(rgb_color *)hsv);

	value= maximum;
	saturation= (maximum==0.f) ? 0.f : delta/maximum;
	if (saturation==0.f)
	{
		hue= 0.f;
	}
	else
	{
		if (red==maximum)
		{
			hue= (green-blue)/delta;
		}
		else if (green==maximum)
		{
			hue= 2.f + (blue-red)/delta;
		}
		else
		{
			hue= 4.f + (red-green)/delta;
		}
		hue/= 6.f;
		if (hue<0.f)
		{
			hue+= 1.f;
		}
	}

	hsv->hue= (word)(hue*65536.f);
	hsv->saturation= (word)(saturation*65535.f);
	hsv->value= (word)(value*65535.f);

	return hsv;
}

union rgb_color *hsv_color_to_rgb_color(
	union hsv_color const *hsv,
	union rgb_color *rgb)
{
	real hue= (hsv->hue/65536.f)*6.f;
	real saturation= hsv->saturation/65535.f;
	real value= hsv->value/65535.f;
	real red, green, blue;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 2182, rgb);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 2184, rgb!=(rgb_color *)hsv);

	if (saturation==0.f)
	{
		red= green= blue= value;
	}
	else
	{
		long integer_hue= (long)hue; // [fake name]
		long sector= (integer_hue>hue) ? integer_hue-1 : integer_hue; // [fake name]
		real fraction, p, q, t;

		fraction= hue-sector;
		p= value*(1.f-saturation);
		q= value*(1.f-saturation*fraction);
		t= value*(1.f-saturation*(1.f-fraction));
		switch (sector)
		{
		case 0: red= value; green= t; blue= p; break;
		case 1: red= q; green= value; blue= p; break;
		case 2: red= p; green= value; blue= t; break;
		case 3: red= p; green= q; blue= value; break;
		case 4: red= t; green= p; blue= value; break;
		case 5: red= value; green= p; blue= q; break;
		}
	}

	rgb->red= (word)(red*65535.f);
	rgb->green= (word)(green*65535.f);
	rgb->blue= (word)(blue*65535.f);

	return rgb;
}

union real_hsv_color *real_rgb_color_to_real_hsv_color(
	union real_rgb_color const *rgb,
	union real_hsv_color *hsv)
{
	real maximum= MAX(rgb->red, MAX(rgb->green, rgb->blue));
	real minimum= MIN(rgb->red, MIN(rgb->green, rgb->blue));
	real delta= maximum-minimum;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 2226, hsv);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 2227, rgb!=(real_rgb_color *)hsv);

	hsv->value= maximum;
	hsv->saturation= (maximum==0.f) ? 0.f : delta/maximum;
	if (hsv->saturation==0.f)
	{
		hsv->hue= 0.f;
	}
	else
	{
		if (rgb->red==maximum)
		{
			hsv->hue= (rgb->green-rgb->blue)/delta;
		}
		else if (rgb->green==maximum)
		{
			hsv->hue= 2.f + (rgb->blue-rgb->red)/delta;
		}
		else
		{
			hsv->hue= 4.f + (rgb->red-rgb->green)/delta;
		}
		hsv->hue/= 6.f;
		if (hsv->hue<0.f)
		{
			hsv->hue+= 1.f;
		}
	}

	return hsv;
}

union real_rgb_color *real_hsv_color_to_real_rgb_color(
	union real_hsv_color const *hsv,
	union real_rgb_color *rgb)
{
	real hue= hsv->hue*6.f;

	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 2271, rgb);
	match_assert("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 2273, rgb!=(real_rgb_color *)hsv);

	if (hsv->saturation==0.f)
	{
		rgb->red= rgb->green= rgb->blue= hsv->value;
	}
	else
	{
		long integer_hue= (long)hue; // [fake name]
		long sector= (integer_hue>hue) ? integer_hue-1 : integer_hue; // [fake name]
		real fraction, p, q, t;

		fraction= hue-sector;
		p= hsv->value*(1.f-hsv->saturation);
		q= hsv->value*(1.f-hsv->saturation*fraction);
		t= hsv->value*(1.f-hsv->saturation*(1.f-fraction));
		switch (sector)
		{
		case 0: rgb->red= hsv->value; rgb->green= t; rgb->blue= p; break;
		case 1: rgb->red= q; rgb->green= hsv->value; rgb->blue= p; break;
		case 2: rgb->red= p; rgb->green= hsv->value; rgb->blue= t; break;
		case 3: rgb->red= p; rgb->green= q; rgb->blue= hsv->value; break;
		case 4: rgb->red= t; rgb->green= p; rgb->blue= hsv->value; break;
		case 5: rgb->red= hsv->value; rgb->green= p; rgb->blue= q; break;
		}
	}

	return rgb;
}

union real_argb_color *argb_color_to_real_argb_color(
	union argb_color const *argb,
	union real_argb_color *real_argb)
{
	real_argb->alpha= argb->alpha*word_to_real_scale;
	real_argb->red= argb->red*word_to_real_scale;
	real_argb->green= argb->green*word_to_real_scale;
	real_argb->blue= argb->blue*word_to_real_scale;

	return real_argb;
}

union real_rgb_color *rgb_color_to_real_rgb_color(
	union rgb_color const *rgb,
	union real_rgb_color *real_rgb)
{
	real_rgb->red= rgb->red*word_to_real_scale;
	real_rgb->green= rgb->green*word_to_real_scale;
	real_rgb->blue= rgb->blue*word_to_real_scale;

	return real_rgb;
}

union real_argb_color *pixel32_to_real_argb_color(
	pixel32 pixel,
	union real_argb_color *color)
{
	color->alpha= PIXEL32_ALPHA(pixel)/255.f;
	color->red= PIXEL32_RED(pixel)/255.f;
	color->green= PIXEL32_GREEN(pixel)/255.f;
	color->blue= PIXEL32_BLUE(pixel)/255.f;

	return color;
}

union real_rgb_color *pixel32_to_real_rgb_color(
	pixel32 pixel,
	union real_rgb_color *color)
{
	color->red= PIXEL32_RED(pixel)/255.f;
	color->green= PIXEL32_GREEN(pixel)/255.f;
	color->blue= PIXEL32_BLUE(pixel)/255.f;

	return color;
}

union real_rgb_color *rgb_colors_interpolate(
	union real_rgb_color *rgb_result,
	unsigned long flags,
	union real_rgb_color const *rgb_lower_bound,
	union real_rgb_color const *rgb_upper_bound,
	real u)
{
	real one_minus_u= 1.f-u;

	match_assert_valid_real_rgb_color("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 2361, rgb_lower_bound);
	match_assert_valid_real_rgb_color("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 2362, rgb_upper_bound);

	if (TEST_FLAG(flags, 0))
	{
		real_hsv_color hsv_lower_bound, hsv_upper_bound, hsv_result;

		real_rgb_color_to_real_hsv_color(rgb_lower_bound, &hsv_lower_bound);
		real_rgb_color_to_real_hsv_color(rgb_upper_bound, &hsv_upper_bound);
		if ((fabs(hsv_lower_bound.hue-hsv_upper_bound.hue)>0.5f)!=TEST_FLAG(flags, 1))
		{
			if (hsv_lower_bound.hue<hsv_upper_bound.hue)
			{
				hsv_lower_bound.hue+= 1.f;
			}
			else
			{
				hsv_upper_bound.hue+= 1.f;
			}
		}
		hsv_result.hue= one_minus_u*hsv_lower_bound.hue + u*hsv_upper_bound.hue;
		if (hsv_result.hue>1.f)
		{
			hsv_result.hue-= 1.f;
		}
		hsv_result.saturation= one_minus_u*hsv_lower_bound.saturation + u*hsv_upper_bound.saturation;
		hsv_result.value= one_minus_u*hsv_lower_bound.value + u*hsv_upper_bound.value;
		real_hsv_color_to_real_rgb_color(&hsv_result, rgb_result);
	}
	else
	{
		rgb_result->red= one_minus_u*rgb_lower_bound->red + u*rgb_upper_bound->red;
		rgb_result->green= one_minus_u*rgb_lower_bound->green + u*rgb_upper_bound->green;
		rgb_result->blue= one_minus_u*rgb_lower_bound->blue + u*rgb_upper_bound->blue;
	}

	match_assert_valid_real_rgb_color("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 2397, rgb_result);

	return rgb_result;
}

union real_rgb_color *rgb_colors_interpolate_and_scale(
	union real_rgb_color *rgb_result,
	unsigned long flags,
	union real_argb_color const *argb_lower_bound,
	union real_argb_color const *argb_upper_bound,
	union real_rgb_color const *rgb_scale,
	real u)
{
	rgb_colors_interpolate(rgb_result, flags, &argb_lower_bound->rgb, &argb_upper_bound->rgb, u);
	if (rgb_scale)
	{
		match_assert_valid_real_rgb_color("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 2414, rgb_scale);
		if (argb_lower_bound->alpha>_real_epsilon || argb_upper_bound->alpha>_real_epsilon)
		{
			real alpha= (1.f-u)*argb_lower_bound->alpha + u*argb_upper_bound->alpha;

			rgb_result->red= alpha*rgb_result->red + (1.f-alpha)*rgb_scale->red;
			rgb_result->green= alpha*rgb_result->green + (1.f-alpha)*rgb_scale->green;
			rgb_result->blue= alpha*rgb_result->blue + (1.f-alpha)*rgb_scale->blue;
		}
		else
		{
			rgb_result->red*= rgb_scale->red;
			rgb_result->green*= rgb_scale->green;
			rgb_result->blue*= rgb_scale->blue;
		}
	}

	match_assert_valid_real_rgb_color("c:\\halo\\SOURCE\\bitmaps\\bitmap_utilities.c", 2434, rgb_result);

	return rgb_result;
}
