/* MisbitFont Library V0.2
 * By Joshua Moss
 *
 * This library is designed to take advantage of the MisbitFont bitmap format,
 * which at present uses the 0.1 specifications.  Can be used either as a static
 * or shared library.  Any text encoding standard can be used in conjunction with
 * this library (as per the specs).  UTF-8 is only necessary for the 'font_name'
 * and 'language' members of the 'msbtfont_header' structure if they are used.
 * 
 * Useful applications include bitmap font editors (for producing MisbitFont files),
 * games, emulators, fantasy computers/consoles, and more. 
 *
 */

#ifndef _MSBTFONT_H_
#define _MSBTFONT_H_

#include <stddef.h>

#if (defined(_WIN32) || defined(_WIN64)) && defined(MSBTFONT_SHARED)
#if defined(MSBTFONT_DEVELOPMENT_DLL)
#define MSBTFONT_SPEC __declspec(dllexport)
#else
#define MSBTFONT_SPEC __declspec(dllimport)
#endif
#else
#define MSBTFONT_SPEC
#endif

#ifdef __cplusplus
extern "C"
{
#endif
typedef struct msbtfont_header
{
	unsigned int magicword_le; // MSBT Magic Word in Little Endian
	struct
	{
		unsigned short major;
		unsigned short minor;
	} version_le; // Version in Little Endian
	unsigned int magicword_be; // MSBT Magic Word in Big Endian
	struct
	{
		unsigned short major;
		unsigned short minor;
	} version_be; // Version in Big Endian
	unsigned char palette_format;
	unsigned char max_font_width;
	unsigned char max_font_height;
	unsigned char flags;
	unsigned int font_character_count_le; // Font Character Count in Little Endian
	unsigned int font_character_count_be; // Font Character Count in Big Endian
	unsigned char font_name[64]; // Uses UTF-8; Optional
	unsigned char language[64]; // Uses UTF-8; Optional
} msbtfont_header;

typedef struct msbtfont_filedata
{
	unsigned char *data;
	unsigned char *variable_table;
	unsigned char *font_data;
	size_t size;
} msbtfont_filedata;

typedef struct msbtfont_rect
{
	unsigned short width;
	unsigned short height;
	unsigned short x;
	unsigned short y;
} msbtfont_rect;

typedef enum 
{
	MSBTFONT_SURFACE_FORMAT_8, // Direct 8-bit Surface Format
	MSBTFONT_SURFACE_FORMAT_16_8, // 16-bit (8-bit) Surface Format (Generally used to store in the first component)
	MSBTFONT_SURFACE_FORMAT_24_8, // 24-bit (8-bit) Surface Format (Generally used to store in the first component)
	MSBTFONT_SURFACE_FORMAT_32_8, // 32-bit (8-bit) Surface Format (Generally used to store in the first component)
} msbtfont_surface_format;

typedef enum
{
	MSBTFONT_SURFACE_ORIGIN_UPPERLEFT,
	MSBTFONT_SURFACE_ORIGIN_LOWERLEFT
} msbtfont_surface_origin;

typedef enum
{
	MSBTFONT_NO_ERROR = 0,
	MSBTFONT_SUCCESS = MSBTFONT_NO_ERROR,
	MSBTFONT_FAILED = -1,
	MSBTFONT_MISSING_HEADER_DESCRIPTOR = -2,
	MSBTFONT_MISSING_HEADER = -3,
	MSBTFONT_MISSING_FILEDATA = -4,
	MSBTFONT_INVALID_HEADER = -5,
	MSBTFONT_INVALID_PALETTE_FORMAT = -6,
	MSBTFONT_FILEDATA_NOT_INITIALIZED = -7,
	MSBTFONT_MISSING_SOURCE_DATA = -8,
	MSBTFONT_INDEX_OUT_OF_BOUNDS = -9,
	MSBTFONT_MISSING_SURFACE_DESCRIPTOR = -10,
	MSBTFONT_MISSING_RECT = -11,
	MSBTFONT_NO_CHARACTERS = -12,
	MSBTFONT_MISSING_SURFACE_DATA = -13,
	MSBTFONT_NO_SURFACE_AREA = -14,
	MSBTFONT_UNSUPPORTED_SURFACE_FORMAT = -15
} msbtfont_retcode;

typedef struct msbtfont_header_descriptor
{
	unsigned char palette_format;
	unsigned char max_font_width;
	unsigned char max_font_height;
	unsigned char flags;
	unsigned int font_character_count;
	unsigned char font_name[64];
	unsigned char language[64];
} msbtfont_header_descriptor;

typedef struct msbtfont_surface_descriptor
{
	msbtfont_rect rect;
	msbtfont_surface_format format;
	msbtfont_surface_origin origin;
} msbtfont_surface_descriptor;

/**
 *  Function:  msbtfont_create_header
 *
 *  Description:  Initializes a valid MisbitFont header by setting up all the necessary data
 *  without needing to specify data manually.  Automatically handles endianness for later
 *  file storage.
 *
 *  Parameters:
 *  	header = Pointer to an existing MisbitFont header structure (created either statically or dynamically).  Must not be NULL.
 *  	header_descriptor = Pointer to an existing MisbitFont header descriptor (created either statically or dynamically).  Necessary for setting up the header properly.  Must not be NULL.
 *
 *  Returns:
 *  	MSBTFONT_SUCCESS = Successfully initialized MisbitFont header.
 *  	MSBTFONT_MISSING_HEADER = Pointer to a MisbitFont header structure was not provided.
 *  	MSBTFONT_MISSING_HEADER_DESCRIPTOR = Pointer to a MisbitFont header descriptor was not provided.
 *  	MSBTFONT_INVALID_PALETTE_FORMAT = Invalid palette format was provided in the descriptor (outside the 0-7 range).
 **/
extern MSBTFONT_SPEC msbtfont_retcode msbtfont_create_header(msbtfont_header *header, const msbtfont_header_descriptor *header_descriptor);

/**
 *  Function:  msbtfont_create_filedata
 *
 *  Description:  Initializes file data to be used with a MisbitFont file.  Header must be
 *  provided in order to properly initialize it.  Memory is dynamically allocated if
 *  successful.  A variable spacing size table pointer is also provided if the header happens
 *  to be using the variable spacing type.  Make sure to call 'msbtfont_delete_filedata' when
 *  you're done with this data to prevent memory leaks.
 *
 *  Parameters:
 *  	header = Pointer to an existing MisbitFont header structure (created either statically or dynamically).  Must not be NULL as it is necessary to setup the file data properly.
 *  	filedata = Pointer to an existing MisbitFont file data structure (created either statically or dynamically).  Must not be NULL.
 *  
 *  Returns:
 *  	MSBTFONT_SUCCESS = Successfully initialized MisbitFont file data structure.
 *  	MSBTFONT_MISSING_HEADER = Pointer to a MisbitFont header structure was not provided.
 *  	MSBTFONT_MISSING_FILEDATA = Pointer to a MisbitFont file data structure was not provided.
 *  	MSBTFONT_INVALID_HEADER = Invalid header was provided, more than likely failed a magic word check.
 **/
extern MSBTFONT_SPEC msbtfont_retcode msbtfont_create_filedata(const msbtfont_header *header, msbtfont_filedata *filedata);

/**
 *  Function:  msbtfont_delete_filedata
 *
 *  Description:  Frees file data from an existing MisbitFont file data structure.  You should
 *  call this function if 'msbtfont_create_filedata' was used to create that data.  Resets
 *  the data (if successfully freed), variable_table (if not NULL) and font_data pointers
 *  to NULL automatically.
 *
 *  Parameters:
 *  	filedata = Pointer to an existing MisbitFont file data structure (created either statically or dynamically).  Must not be NULL in order to properly free memory.
 *
 *  Returns:
 *  	MSBTFONT_NO_ERROR = File data was successfully freed without incident.
 *  	MSBTFONT_MISSING_FILEDATA = Pointer to a MisbitFont file data structure was not provided.
 *  	MSBTFONT_FILEDATA_NOT_INITIALIZED = MisbitFont file data structure was not initialized.
 **/
extern MSBTFONT_SPEC msbtfont_retcode msbtfont_delete_filedata(msbtfont_filedata *filedata);

/**
 *  Function:  msbtfont_store_font_character_data
 *
 *  Description:  Stores font character data for a single character from application memory
 *  straight to the file data; guided by the header in order to ensure proper storage.
 *
 *  Parameters:
 *  	header = Pointer to an existing MisbitFont header structure (created either statically or dynamically).  Must not be NULL in order to ensure proper storage.
 *  	filedata = Pointer to an existing MisbitFont file data structure (created either statically or dynamically).  Must not be NULL in order to store any kind of data.
 *  	srcdata = Pointer to source data located usually in application memory (either statically or dynamically allocated).  Must not be NULL in order to copy data.
 *  	index = Index to a certain font character (provided it is less than the font character count) for storage.
 *
 *  Returns:
 *  	MSBTFONT_SUCCESS = Font character data was successfully stored.
 *  	MSBTFONT_MISSING_HEADER = Pointer to a MisbitFont header structure was not provided.
 *  	MSBTFONT_MISSING_FILEDATA = Pointer to a MisbitFont file data structure was not provided.
 *  	MSBTFONT_MISSING_SOURCE_DATA = Pointer to source data was not provided.
 *  	MSBTFONT_INVALID_HEADER = Invalid header was provided, more than likely failed a magic word check.
 *  	MSBTFONT_FILEDATA_NOT_INITIALIZED = MisbitFont file data structre was not initialized.
 *  	MSBTFONT_INDEX_OUT_OF_BOUNDS = Index provided was outside the range (greater than or equal to the font character count).
 **/
extern MSBTFONT_SPEC msbtfont_retcode msbtfont_store_font_character_data(const msbtfont_header *header, msbtfont_filedata *filedata, const unsigned char *srcdata, unsigned int index);

/**
 *  Function:  msbtfont_get_surface_size
 *
 *  Description:  Retrieves the dimensions of a possible surface based on the font data and
 *  the number of characters per row.  Useful for surface creation and copying data back to
 *  the surface. 
 *
 *  Parameters:
 *  	header = Pointer to an existing MisbitFont header structure (created either statically or dynamically).  Must not be NULL in order to properly retrieve the dimensions of a possible surface.
 *  	surface_size = Pointer to an existing MisbitFont rect structure (created either statically or dynamically).  Must not be NULL in order to properly fill the data retrieved.  Retrieves only the width and height.
 *  	characters_per_row = Number of characters per row in the possible surface.  Must be at least 1 in order to properly retrieve the size.
 *
 *  Returns:
 *  	MSBTFONT_NO_ERROR = Successfully able to retrieve surface size data.
 *  	MSBTFONT_MISSING_HEADER = Pointer to a MisbitFont header structure was not provided.
 *  	MSBTFONT_MISSING_RECT = Pointer to a MisbitFont rect structure was not provided for 'surface_size'.
 *  	MSBTFONT_INVALID_HEADER = Invalid header was provided, more than likely failed a magic word check.
 *  	MSBTFONT_NO_CHARACTERS = Characters per row was set to 0.
 **/
extern MSBTFONT_SPEC msbtfont_retcode msbtfont_get_surface_size(const msbtfont_header *header, msbtfont_rect *surface_size, unsigned int characters_per_row);

/**
 *  Function:  msbtfont_get_surface_memory_requirement
 *
 *  Description:  Retrieves the amount of memory required to setup that surface in system
 *  memory based on surface size and surface format.  This function takes memory alignment into
 *  account based on the surface format.  Useful for allocating memory in regards to surface
 *  data and later using it.
 *
 *  Parameters:
 *  	surface_descriptor = Pointer to an existing surface descriptor (created either statically or dynamically).  Must not be NULL in order to properly retrieve memory size data.
 *
 *  Returns:
 *  	Surface memory required to allocate.  Otherwise, it's 0 if an invalid or no surface descriptor was provided.
 **/
extern MSBTFONT_SPEC size_t msbtfont_get_surface_memory_requirement(const msbtfont_surface_descriptor *surface_descriptor);

/**
 *  Function:  msbtfont_copy_to_surface
 *
 *  Description:  Copies all the font character data and store them in a surface-friendly
 *  form.  Using a surface descriptor, you can specify what format the surface uses for
 *  proper copying (conversion if necessary).  You can also specify the origin for different
 *  coordinate systems.  You can copy anywhere on the surface by specifying the coordinates
 *  within the surface descriptor.
 *
 *  Parameters:
 *  	header = Pointer to an existing MisbitFont header structure (created either statically or dynamically).  Must not be NULL.
 *  	filedata = Pointer to an existing MisbitFont file data structure (created either statically or dynamically).  Must not be NULL in order to retrieve (and convert if necessary).
 *  	characters_per_row = Number of characters per row in the surface.  If 0 is specified, it will fill based on the available width of the surface.
 *  	character_start_offset = If 'characters_per_row' is non-zero, this shifts the starting position by a number of characters.  Otherwise, it does nothing.  Useful for copying multiple fonts together into one surface.
 *  	surface_descriptor = Pointer to an existing surface descriptor (created either statically or dynamically).  Must not be NULL in order to ensure proper copying (and conversion if necessary).
 *  	surface_data = Pointer to an existing surface (created either statically or dynamically).  Must not be NULL in order to store data onto the surface.  Must also make sure there is enough memory before storage.
 *  
 *  Returns:
 *  	MSBTFONT_SUCCESS = Font character data was successfully copied.
 *  	MSBTFONT_MISSING_HEADER = Pointer to a MisbitFont header structure was not provided.
 *  	MSBTFONT_MISSING_FILEDATA = Pointer to a MisbitFont file data structure was not provided.
 *  	MSBTFONT_MISSING_SURFACE_DESCRIPTOR = Pointer to a surface descriptor was not provided.
 *  	MSBTFONT_MISSING_SURFACE_DATA = Pointer to surface data was not provided.
 *  	MSBTFONT_INVALID_HEADER = Invalid header was provided, more than likely failed a magic word check.
 *  	MSBTFONT_NO_SURFACE_AREA = There is no surface to copy due to either 0 width or height on the surface descriptor.
 *  	MSBTFONT_FILEDATA_NOT_INITIALIZED = MisbitFont file data structre was not initialized.
 *  	MSBTFONT_UNSUPPORTED_SURFACE_FORMAT = Surface format supplied by the descriptor is currently unsupported or invalid.
 **/
extern MSBTFONT_SPEC msbtfont_retcode msbtfont_copy_to_surface(const msbtfont_header *header, const msbtfont_filedata *filedata, unsigned int characters_per_row, unsigned int character_start_offset, const msbtfont_surface_descriptor *surface_descriptor, unsigned char *surface_data);

#ifdef __cplusplus
}
#endif
#endif
