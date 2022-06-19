# libmsbtfont Changelog

## Version 0.2

- Implemented coordinate support for the `msbtfont_copy_to_surface` function.  This feature was supposed to be in the initial release, 
but was forgotten.

- Added two new parameters to the `msbtfont_copy_to_surface` function that allows for specifying the number of characters per row and 
adjusting the start offset based on the max font width.  Due to these changes, any existing application using this function will need
to make some modifications.

- Changed handling to `MSBTFONT_SURFACE_FORMAT_8`, `MSBTFONT_SURFACE_FORMAT_16_8`, and `MSBTFONT_SURFACE_FORMAT_24_8` surface 
formats for `msbtfont_copy_to_surface` in order to allow proper and optimal alignment.

- Added the `msbtfont_get_surface_memory_requirement` function to retrieve memory requirements useful for allocating surface data.

## Version 0.1

- Initial Release
