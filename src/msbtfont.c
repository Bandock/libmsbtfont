#include "../include/msbtfont.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MSBTFONT_FOURCC(a, b, c, d) (a | (b << 8) | (c << 16) | (d << 24))
#define MSBTFONT_MSBT MSBTFONT_FOURCC('M', 'S', 'B', 'T')
#define MSBTFONT_TBSM MSBTFONT_FOURCC('T', 'B', 'S', 'M')

msbtfont_retcode msbtfont_create_header(msbtfont_header *header, const msbtfont_header_descriptor *header_descriptor)
{
	if (header == NULL || header_descriptor == NULL)
	{
		return (header == NULL) ? MSBTFONT_MISSING_HEADER : MSBTFONT_MISSING_HEADER_DESCRIPTOR;
	}
	const char magicword_le_data[] = { 'M', 'S', 'B', 'T' };
	const char magicword_be_data[] = { 'T', 'B', 'S', 'M' };
	const unsigned char version_le_data[] = { 0, 0, 1, 0 };
	const unsigned char version_be_data[] = { 0, 0, 0, 1 };
	unsigned char endianness = 0; // 0 = Little Endian, 1 = Big Endian
	unsigned int font_character_count = header_descriptor->font_character_count;
	memcpy(&header->magicword_le, magicword_le_data, sizeof(unsigned int));
	if (header->magicword_le != MSBTFONT_MSBT)
	{
		endianness = 1;
	}
	memcpy(&header->version_le, version_le_data, sizeof(header->version_le));
	memcpy(&header->magicword_be, magicword_be_data, sizeof(unsigned int));
	memcpy(&header->version_be, version_be_data, sizeof(header->version_be));
	if (header_descriptor->palette_format > 7)
	{
		return MSBTFONT_INVALID_PALETTE_FORMAT;
	}
	header->palette_format = header_descriptor->palette_format;
	header->max_font_width = header_descriptor->max_font_width;
	header->max_font_height = header_descriptor->max_font_height;
	header->flags = header_descriptor->flags;
	if (endianness == 0)
	{
		memcpy(&header->font_character_count_le, &font_character_count, sizeof(unsigned int));
		font_character_count = ((font_character_count >> 24) | ((font_character_count & 0xFF0000) >> 8) | ((font_character_count & 0xFF00) << 8) | (font_character_count << 24));
		memcpy(&header->font_character_count_be, &font_character_count, sizeof(unsigned int));
	}
	else
	{
		memcpy(&header->font_character_count_be, &font_character_count, sizeof(unsigned int));
		font_character_count = ((font_character_count >> 24) | ((font_character_count & 0xFF0000) >> 8) | ((font_character_count & 0xFF00) << 8) | (font_character_count << 24));
		memcpy(&header->font_character_count_le, &font_character_count, sizeof(unsigned int));
	}
	memcpy(header->font_name, header_descriptor->font_name, sizeof(header->font_name));
	memcpy(header->language, header_descriptor->language, sizeof(header->language));
	return MSBTFONT_SUCCESS;
}

msbtfont_retcode msbtfont_create_filedata(const msbtfont_header *header, msbtfont_filedata *filedata)
{
	size_t filedata_size = 0;
	size_t font_data_size = 0;
	if (header == NULL || filedata == NULL)
	{
		return (header == NULL) ? MSBTFONT_MISSING_HEADER : MSBTFONT_MISSING_FILEDATA;
	}
	if (header->magicword_le == MSBTFONT_MSBT)
	{
		if (header->flags & 0x01)
		{
			filedata_size += header->font_character_count_le;
		}
		font_data_size = ((header->palette_format + 1) * (header->max_font_width + 1) * (header->max_font_height + 1) * header->font_character_count_le);
		filedata_size += font_data_size / 8;
		if (font_data_size % 8)
		{
			filedata_size += 1;
		}
		filedata->data = malloc(filedata_size);
		filedata->size = filedata_size;
		if (header->flags & 0x01)
		{
			filedata->variable_table = &filedata->data[0];
			filedata->font_data = &filedata->data[header->font_character_count_le];
			memset(filedata->variable_table, header->max_font_width, header->font_character_count_le);
			memset(filedata->font_data, 0, filedata->size - header->font_character_count_le);
		}
		else
		{
			filedata->font_data = &filedata->data[0];
			memset(filedata->font_data, 0, filedata->size);
		}
	}
	else if (header->magicword_be == MSBTFONT_TBSM)
	{
		if (header->flags & 0x01)
		{
			filedata_size += header->font_character_count_be;
		}
		font_data_size = ((header->max_font_width + 1) * (header->max_font_height + 1) * header->font_character_count_be);
		filedata_size += font_data_size / 8;
		if (font_data_size % 8)
		{
			filedata_size += 1;
		}
		filedata->data = malloc(filedata_size);
		filedata->size = filedata_size;
		if (header->flags & 0x01)
		{
			filedata->variable_table = &filedata->data[0];
			filedata->font_data = &filedata->data[header->font_character_count_be];
			memset(filedata->variable_table, header->max_font_width, header->font_character_count_be);
			memset(filedata->font_data, 0, filedata->size - header->font_character_count_be);
		}
		else
		{
			filedata->font_data = &filedata->data[0];
			memset(filedata->font_data, 0, filedata->size);
		}
		return MSBTFONT_SUCCESS;
	}
	else
	{
		return MSBTFONT_INVALID_HEADER;
	}
}

msbtfont_retcode msbtfont_delete_filedata(msbtfont_filedata *filedata)
{
	if (filedata != NULL)
	{
		if (filedata->data != NULL)
		{
			free(filedata->data);
			filedata->data = NULL;
			if (filedata->variable_table != NULL)
			{
				filedata->variable_table = NULL;
			}
			if (filedata->font_data != NULL)
			{
				filedata->font_data = NULL;
			}
			return MSBTFONT_NO_ERROR;
		}
		else
		{
			return MSBTFONT_FILEDATA_NOT_INITIALIZED;
		}
	}
	return MSBTFONT_MISSING_FILEDATA;
}

msbtfont_retcode msbtfont_store_font_character_data(const msbtfont_header *header, msbtfont_filedata *filedata, const unsigned char *srcdata, unsigned int index)
{
	if (header != NULL)
	{
		if (filedata != NULL)
		{
			if (srcdata != NULL)
			{
				if (filedata->data != NULL)
				{
					if (header->magicword_le == MSBTFONT_MSBT)
					{
						if (index < header->font_character_count_le)
						{
							unsigned short max_font_width = header->max_font_width + 1;
							unsigned short max_font_height = header->max_font_height + 1;
							size_t c_doffset = ((index * (header->palette_format + 1) * max_font_width * max_font_height) / 8);
							size_t c_soffset = 0;
							unsigned char dbit_offset = ((index * (header->palette_format + 1) * max_font_width * max_font_height) % 8);
							unsigned char sbit_offset = 0;
							unsigned char *base_chardata = &filedata->font_data[c_doffset];
							if (header->palette_format < 7)
							{
								c_doffset = 0;
								unsigned char bitmask = (0xFF << (7 - header->palette_format));
								for (unsigned int i = 0; i < max_font_width * max_font_height; ++i)
								{
									base_chardata[c_doffset] &= ~(bitmask >> dbit_offset);
									unsigned char sbit_offset_s = sbit_offset;
									size_t s_bits_left = (header->palette_format + 1);
									unsigned char src = ((srcdata[c_soffset] & (bitmask >> sbit_offset)) << sbit_offset);
									sbit_offset += s_bits_left;
									if (sbit_offset > 7)
									{
										s_bits_left -= (8 - sbit_offset_s);
										sbit_offset -= 8;
										++c_soffset;
										if (s_bits_left > 0)
										{
											src |= ((srcdata[c_soffset] >> (8 - s_bits_left)) << (7 - header->palette_format));
										}
									}
									base_chardata[c_doffset] |= (src >> dbit_offset);
									unsigned char dbit_offset_s = dbit_offset;
									size_t d_bits_left = (header->palette_format + 1);
									dbit_offset += d_bits_left;
									if (dbit_offset > 7)
									{
										d_bits_left -= (8 - dbit_offset_s);
										dbit_offset -= 8;
										++c_doffset;
										if (d_bits_left > 0)
										{
											base_chardata[c_doffset] &= ~(bitmask << ((header->palette_format + 1) - d_bits_left));
											base_chardata[c_doffset] |= (src << ((header->palette_format + 1) - d_bits_left));
										}
									}
								}
							}
							else
							{
								memcpy(base_chardata, srcdata, max_font_width * max_font_height);
							}
							return MSBTFONT_SUCCESS;
						}
						else
						{
							return MSBTFONT_INDEX_OUT_OF_BOUNDS;
						}
					}
					else if (header->magicword_be == MSBTFONT_TBSM)
					{
						if (index < header->font_character_count_be)
						{
							unsigned short max_font_width = header->max_font_width + 1;
							unsigned short max_font_height = header->max_font_height + 1;
							size_t c_doffset = ((index * (header->palette_format + 1) * max_font_width * max_font_height) / 8);
							size_t c_soffset = 0;
							unsigned char dbit_offset = ((index * (header->palette_format + 1) * max_font_width * max_font_height) % 8);
							unsigned char sbit_offset = 0;
							unsigned char *base_chardata = &filedata->font_data[c_doffset];
							if (header->palette_format < 7)
							{
								c_doffset = 0;
								unsigned char bitmask = (0xFF << (7 - header->palette_format));
								for (unsigned int i = 0; i < max_font_width * max_font_height; ++i)
								{
									base_chardata[c_soffset] &= ~(bitmask >> dbit_offset);
									unsigned char sbit_offset_s = sbit_offset;
									size_t s_bits_left = (header->palette_format + 1);
									unsigned char src = ((srcdata[c_soffset] & (bitmask >> sbit_offset)) << sbit_offset);
									sbit_offset += s_bits_left;
									if (sbit_offset > 7)
									{
										s_bits_left -= (8 - sbit_offset_s);
										sbit_offset -= 8;
										++c_soffset;
										if (s_bits_left > 0)
										{
											src |= ((srcdata[c_soffset] >> (8 - s_bits_left)) << (7 - header->palette_format));
										}	
									}
									base_chardata[c_doffset] |= (src >> dbit_offset);
									unsigned char dbit_offset_s = dbit_offset;
									size_t d_bits_left = (header->palette_format + 1);
									dbit_offset += d_bits_left;
									if (dbit_offset > 7)
									{
										d_bits_left -= (8 - dbit_offset_s);
										dbit_offset -= 8;
										++c_doffset;
										if (d_bits_left > 0)
										{
											base_chardata[c_doffset] &= ~(bitmask << ((header->palette_format + 1) - d_bits_left));
											base_chardata[c_doffset] |= (src << ((header->palette_format + 1) - d_bits_left));
										}
									}
								}
							}
							else
							{
								memcpy(base_chardata, srcdata, max_font_width * max_font_height);
							}
							return MSBTFONT_SUCCESS;
						}
						else
						{
							return MSBTFONT_INDEX_OUT_OF_BOUNDS;
						}
					}
					else
					{
						return MSBTFONT_INVALID_HEADER;
					}
				}
				else
				{
					return MSBTFONT_FILEDATA_NOT_INITIALIZED;
				}
			}
			else
			{
				return MSBTFONT_MISSING_SOURCE_DATA;
			}
		}
		else
		{
			return MSBTFONT_MISSING_FILEDATA;
		}
	}
	else
	{
		return MSBTFONT_MISSING_HEADER;
	}
}

msbtfont_retcode msbtfont_get_surface_size(const msbtfont_header *header, msbtfont_rect *surface_size, unsigned int characters_per_row)
{
	if (header != NULL)
	{
		if (surface_size != NULL)
		{
			if (characters_per_row > 0)
			{
				unsigned int font_character_count = 0;
				if (header->magicword_le == MSBTFONT_MSBT)
				{
					font_character_count = header->font_character_count_le;
				}
				else if (header->magicword_be == MSBTFONT_TBSM)
				{
					font_character_count = header->font_character_count_be;
				}
				else
				{
					return MSBTFONT_INVALID_HEADER;
				}
				surface_size->width = (header->max_font_width + 1) * characters_per_row;
				surface_size->height = 0;
				for (unsigned int i = 0; i < font_character_count; i += characters_per_row)
				{
					surface_size->height += (header->max_font_height + 1);
				}
				return MSBTFONT_NO_ERROR;
			}
			else
			{
				return MSBTFONT_NO_CHARACTERS;
			}
		}
		else
		{
			return MSBTFONT_MISSING_RECT;
		}
	}
	else
	{
		return MSBTFONT_MISSING_HEADER;
	}
}

size_t msbtfont_get_surface_memory_requirement(const msbtfont_surface_descriptor *surface_descriptor)
{
	if (surface_descriptor != NULL)
	{
		if (surface_descriptor->rect.width == 0 || surface_descriptor->rect.height == 0)
		{
			return 0;
		}
		switch (surface_descriptor->format)
		{
			case MSBTFONT_SURFACE_FORMAT_8:
			{
				size_t stride_check = surface_descriptor->rect.width % 4;
				return (surface_descriptor->rect.width + (stride_check ? 4 - stride_check : 0)) * surface_descriptor->rect.height;
			}
			case MSBTFONT_SURFACE_FORMAT_16_8:
			{
				size_t stride_check = (surface_descriptor->rect.width * 2) % 4;
				return (surface_descriptor->rect.width + (stride_check ? 4 - stride_check : 0)) * surface_descriptor->rect.height * 2;
			}
			case MSBTFONT_SURFACE_FORMAT_24_8:
			{
				size_t stride_check = (surface_descriptor->rect.width * 3) % 4;
				return (surface_descriptor->rect.width + (stride_check ? 4 - stride_check : 0)) * surface_descriptor->rect.height * 3;
			}
			case MSBTFONT_SURFACE_FORMAT_32_8:
			{
				return surface_descriptor->rect.width * surface_descriptor->rect.height * 4;
			}
			default:
			{
				return 0;
			}
		}
	}
	else
	{
		return 0;
	}
}

msbtfont_retcode msbtfont_copy_to_surface(const msbtfont_header *header, const msbtfont_filedata *filedata, unsigned int characters_per_row, unsigned int character_start_offset, const msbtfont_surface_descriptor *surface_descriptor, unsigned char *surface_data)
{
	if (header != NULL)
	{
		if (filedata != NULL)
		{
			if (surface_descriptor != NULL)
			{
				if (surface_data != NULL)
				{
					unsigned int font_character_count = 0;
					unsigned short max_font_width = header->max_font_width + 1;
					unsigned short max_font_height = header->max_font_height + 1;
					size_t font_offset_x = surface_descriptor->rect.x;
					size_t font_offset_y = surface_descriptor->rect.y;
					if (header->magicword_le == MSBTFONT_MSBT)
					{
						font_character_count = header->font_character_count_le;
					}
					else if (header->magicword_be == MSBTFONT_TBSM)
					{
						font_character_count = header->font_character_count_be;
					}
					else
					{
						return MSBTFONT_INVALID_HEADER;
					}
					if (surface_descriptor->rect.width == 0 || surface_descriptor->rect.height == 0)
					{
						return MSBTFONT_NO_SURFACE_AREA;
					}
					if (filedata->font_data == NULL)
					{
						return MSBTFONT_FILEDATA_NOT_INITIALIZED;
					}
					if (character_start_offset != 0)
					{
						font_offset_x += (character_start_offset * max_font_width);
						if (font_offset_x >= surface_descriptor->rect.width)
						{
							size_t new_font_offset_x = font_offset_x % surface_descriptor->rect.width;
							size_t row_count = font_offset_x / surface_descriptor->rect.width;
							font_offset_x = new_font_offset_x;
							font_offset_y += row_count * max_font_height;
							if (font_offset_y >= surface_descriptor->rect.height)
							{
								return MSBTFONT_SUCCESS;
							}
						}
					}
					switch (surface_descriptor->format)
					{
						case MSBTFONT_SURFACE_FORMAT_8:
						{
							size_t stride_check = surface_descriptor->rect.width % 4;
							switch (surface_descriptor->origin)
							{
								case MSBTFONT_SURFACE_ORIGIN_UPPERLEFT:
								{
									if (header->palette_format < 7)
									{
										unsigned char bitmask = (0xFF << (7 - header->palette_format));
										for (unsigned int i = 0; i < font_character_count; ++i)
										{
											if (characters_per_row != 0)
											{
												size_t index_mod = character_start_offset + i;
												if (index_mod != 0 && (index_mod % characters_per_row == 0))
												{
													font_offset_x = surface_descriptor->rect.x;
													font_offset_y += max_font_height;
													if (font_offset_y >= surface_descriptor->rect.height)
													{
														break;
													}
												}
											}
											size_t c_offset = 0;
											unsigned char bit_offset = ((i * (header->palette_format + 1) * max_font_width * max_font_height) % 8);
											unsigned char *base_chardata = &filedata->font_data[((i * (header->palette_format + 1) * max_font_width * max_font_height) / 8)];
											for (unsigned short y = 0; y < max_font_height; ++y)
											{
												if (font_offset_y + y >= surface_descriptor->rect.height)
												{
													break;
												}
												for (unsigned short x = 0; x < max_font_width; ++x)
												{
													size_t bits_left = (header->palette_format + 1);
													unsigned char pixel = ((base_chardata[c_offset] & (bitmask >> bit_offset)) << bit_offset);
													unsigned char bit_offset_s = bit_offset;
													bit_offset += bits_left;
													if (bit_offset > 7)
													{
														bits_left -= (8 - bit_offset_s);
														bit_offset -= 8;
														++c_offset;
														if (bits_left > 0)
														{
															pixel |= ((base_chardata[c_offset] >> (8 - bits_left)) << (7 - header->palette_format));
														}
													}
													pixel >>= (7 - header->palette_format);
													if (font_offset_x + x >= surface_descriptor->rect.width)
													{
														continue;
													}
													surface_data[((y + font_offset_y) * surface_descriptor->rect.width) + font_offset_x + x + ((stride_check ? (4 - stride_check) : 0) * (y + font_offset_y))] = pixel;
												}
											}
											font_offset_x += max_font_width;
											if (characters_per_row == 0 && font_offset_x >= surface_descriptor->rect.width)
											{
												font_offset_x = surface_descriptor->rect.x;
												font_offset_y += max_font_height;
											}
											if (font_offset_y >= surface_descriptor->rect.height)
											{
												break;
											}
										}
									}
									else
									{
										for (unsigned int i = 0; i < font_character_count; ++i)
										{
											if (characters_per_row != 0)
											{
												size_t index_mod = character_start_offset + i;
												if (index_mod != 0 && (index_mod % characters_per_row == 0))
												{
													font_offset_x = surface_descriptor->rect.x;
													font_offset_y += max_font_height;
													if (font_offset_y >= surface_descriptor->rect.height)
													{
														break;
													}
												}
											}
											for (unsigned short y = 0; y < max_font_height; ++y)
											{
												if (font_offset_y + y >= surface_descriptor->rect.height)
												{
													break;
												}
												for (unsigned short x = 0; x < max_font_width; ++x)
												{
													if (font_offset_x + x >= surface_descriptor->rect.width)
													{
														break;
													}
													surface_data[((y + font_offset_y) * surface_descriptor->rect.width) + font_offset_x + x + ((stride_check ? (4 - stride_check) : 0) * (y + font_offset_y))] = filedata->font_data[(i * max_font_width * max_font_height) + (y * max_font_width) + x];
												}
											}
											font_offset_x += max_font_width;
											if (characters_per_row == 0 && font_offset_x >= surface_descriptor->rect.width)
											{
												font_offset_x = surface_descriptor->rect.x;
												font_offset_y += max_font_height;
											}
											if (font_offset_y >= surface_descriptor->rect.height)
											{
												break;
											}
										}
									}
									break;
								}
								case MSBTFONT_SURFACE_ORIGIN_LOWERLEFT:
								{
									if (header->palette_format < 7)
									{
										unsigned char bitmask = (0xFF << (7 - header->palette_format));
										for (unsigned int i = 0; i < font_character_count; ++i)
										{
											if (characters_per_row != 0)
											{
												size_t index_mod = character_start_offset + i;
												if (index_mod != 0 && (index_mod % characters_per_row == 0))
												{
													font_offset_x = surface_descriptor->rect.x;
													font_offset_y += max_font_height;
													if (font_offset_y >= surface_descriptor->rect.height)
													{
														break;
													}
												}
											}
											size_t c_offset = 0;
											unsigned char bit_offset = ((i * (header->palette_format + 1) * max_font_width * max_font_height) % 8);
											unsigned char *base_chardata = &filedata->font_data[((i * (header->palette_format + 1) * max_font_width * max_font_height) / 8)];
											for (unsigned short y = 0; y < max_font_height; ++y)
											{
												if (font_offset_y + y >= surface_descriptor->rect.height)
												{
													break;
												}
												for (unsigned short x = 0; x < max_font_width; ++x)
												{
													size_t bits_left = (header->palette_format + 1);
													unsigned char pixel = ((base_chardata[c_offset] & (bitmask >> bit_offset)) << bit_offset);
													unsigned char bit_offset_s = bit_offset;
													bit_offset += bits_left;
													if (bit_offset > 7)
													{
														bits_left -= (8 - bit_offset_s);
														bit_offset -= 8;
														++c_offset;
														if (bits_left > 0)
														{
															pixel |= ((base_chardata[c_offset] >> (8 - bits_left)) << (7 - header->palette_format));
														}
													}
													pixel >>= (7 - header->palette_format);
													if (font_offset_x + x >= surface_descriptor->rect.width)
													{
														continue;
													}
													surface_data[((surface_descriptor->rect.height - 1 - font_offset_y - y) * surface_descriptor->rect.width) + font_offset_x + x + ((stride_check ? (4 - stride_check) : 0) * (surface_descriptor->rect.height - 1 - font_offset_y - y))] = pixel;
												}
											}
											font_offset_x += max_font_width;
											if (characters_per_row == 0 && font_offset_x >= surface_descriptor->rect.width)
											{
												font_offset_x = surface_descriptor->rect.x;
												font_offset_y += max_font_height;
											}
											if (font_offset_y >= surface_descriptor->rect.height)
											{
												break;
											}
										}
									}
									else
									{
										for (unsigned int i = 0; i < font_character_count; ++i)
										{
											for (unsigned short y = 0; y < max_font_height; ++y)
											{
												if (characters_per_row != 0)
												{
													size_t index_mod = character_start_offset + i;
													if (index_mod != 0 && (index_mod % characters_per_row == 0))
													{
														font_offset_x = surface_descriptor->rect.x;
														font_offset_y += max_font_height;
														if (font_offset_y >= surface_descriptor->rect.height)
														{
															break;
														}
													}
												}
												if (font_offset_y + y >= surface_descriptor->rect.height)
												{
													break;
												}
												for (unsigned short x = 0; x < max_font_width; ++x)
												{
													if (font_offset_x + x >= surface_descriptor->rect.width)
													{
														break;
													}
													surface_data[((max_font_height - 1 - font_offset_y - y) * surface_descriptor->rect.width) + font_offset_x + x + ((stride_check ? (4 - stride_check) : 0) * (surface_descriptor->rect.height - 1 - font_offset_y - y))] = filedata->font_data[(i * max_font_width * max_font_height) + (y * max_font_width) + x];
												}
											}
											font_offset_x += max_font_width;
											if (characters_per_row == 0 && font_offset_x >= surface_descriptor->rect.width)
											{
												font_offset_x = surface_descriptor->rect.x;
												font_offset_y += max_font_height;
											}
											if (font_offset_y >= surface_descriptor->rect.height)
											{
												break;
											}
										}
									}
									break;
								}
							}
							break;
						}
						case MSBTFONT_SURFACE_FORMAT_16_8:
						{
							typedef struct rg8
							{
								unsigned char r;
								unsigned char g;
							} rg8;
							rg8 *surface_data_rg8 = (rg8 *)(surface_data);
							size_t stride_check = surface_descriptor->rect.width % 2;
							switch (surface_descriptor->origin)
							{
								case MSBTFONT_SURFACE_ORIGIN_UPPERLEFT:
								{
									if (header->palette_format < 7)
									{
										unsigned char bitmask = (0xFF << (7 - header->palette_format));
										for (unsigned int i = 0; i < font_character_count; ++i)
										{
											if (characters_per_row != 0)
											{
												size_t index_mod = character_start_offset + i;
												if (index_mod != 0 && (index_mod % characters_per_row == 0))
												{
													font_offset_x = surface_descriptor->rect.x;
													font_offset_y += max_font_height;
													if (font_offset_y >= surface_descriptor->rect.height)
													{
														break;
													}
												}
											}
											size_t c_offset = 0;
											unsigned char bit_offset = ((i * (header->palette_format + 1) * max_font_width * max_font_height) % 8);
											unsigned char *base_chardata = &filedata->font_data[((i * (header->palette_format + 1) * max_font_width * max_font_height) / 8)];
											for (unsigned short y = 0; y < max_font_height; ++y)
											{
												if (font_offset_y + y >= surface_descriptor->rect.height)
												{
													break;
												}
												for (unsigned short x = 0; x < max_font_width; ++x)
												{
													size_t bits_left = (header->palette_format + 1);
													unsigned char pixel = ((base_chardata[c_offset] & (bitmask >> bit_offset)) << bit_offset);
													unsigned char bit_offset_s = bit_offset;
													bit_offset += bits_left;
													if (bit_offset > 7)
													{
														bits_left -= (8 - bit_offset_s);
														bit_offset -= 8;
														++c_offset;
														if (bits_left > 0)
														{
															pixel |= ((base_chardata[c_offset] >> (8 - bits_left)) << (7 - header->palette_format));
														}
													}
													pixel >>= (7 - header->palette_format);
													if (font_offset_x + x >= surface_descriptor->rect.width)
													{
														continue;
													}
													surface_data_rg8[((y + font_offset_y) * surface_descriptor->rect.width) + font_offset_x + x + (stride_check * (font_offset_y + y))].r = pixel;
												}
											}
											font_offset_x += max_font_width;
											if (characters_per_row == 0 && font_offset_x >= surface_descriptor->rect.width)
											{
												font_offset_x = surface_descriptor->rect.x;
												font_offset_y += max_font_height;
											}
											if (font_offset_y >= surface_descriptor->rect.height)
											{
												break;
											}
										}
									}
									else
									{
										for (unsigned int i = 0; i < font_character_count; ++i)
										{
											if (characters_per_row != 0)
											{
												size_t index_mod = character_start_offset + i;
												if (index_mod != 0 && (index_mod % characters_per_row == 0))
												{
													font_offset_x = surface_descriptor->rect.x;
													font_offset_y += max_font_height;
												}
											}
											for (unsigned short y = 0; y < max_font_height; ++y)
											{
												if (font_offset_y + y >= surface_descriptor->rect.height)
												{
													break;
												}
												for (unsigned short x = 0; x < max_font_width; ++x)
												{
													if (font_offset_x + x >= surface_descriptor->rect.width)
													{
														break;
													}
													surface_data_rg8[((y + font_offset_y) * surface_descriptor->rect.width) + font_offset_x + x].r = filedata->font_data[(i * max_font_width * max_font_height) + (y * max_font_width) + x + (stride_check * (font_offset_y + y))];
												}
											}
											font_offset_x += max_font_width;
											if (characters_per_row == 0 && font_offset_x >= surface_descriptor->rect.width)
											{
												font_offset_x = surface_descriptor->rect.x;
												font_offset_y += max_font_height;
											}
											if (font_offset_y >= surface_descriptor->rect.height)
											{
												break;
											}
										}
									}
									break;
								}
								case MSBTFONT_SURFACE_ORIGIN_LOWERLEFT:
								{
									if (header->palette_format < 7)
									{
										unsigned char bitmask = (0xFF << (7 - header->palette_format));
										for (unsigned int i = 0; i < font_character_count; ++i)
										{
											if (characters_per_row != 0)
											{
												size_t index_mod = character_start_offset + i;
												if (index_mod != 0 && (index_mod % characters_per_row == 0))
												{
													font_offset_x = surface_descriptor->rect.x;
													font_offset_y += max_font_height;
													if (font_offset_y >= surface_descriptor->rect.height)
													{
														break;
													}
												}
											}
											size_t c_offset = 0;
											unsigned char bit_offset = ((i * (header->palette_format + 1) * max_font_width * max_font_height) % 8);
											unsigned char *base_chardata = &filedata->font_data[((i * (header->palette_format + 1) * max_font_width * max_font_height) / 8)];
											for (unsigned short y = 0; y < max_font_height; ++y)
											{
												if (font_offset_y + y >= surface_descriptor->rect.height)
												{
													break;
												}
												for (unsigned short x = 0; x < max_font_width; ++x)
												{
													size_t bits_left = (header->palette_format + 1);
													unsigned char pixel = ((base_chardata[c_offset] & (bitmask >> bit_offset)) << bit_offset);
													unsigned char bit_offset_s = bit_offset;
													bit_offset += bits_left;
													if (bit_offset > 7)
													{
														bits_left -= (8 - bit_offset_s);
														bit_offset -= 8;
														++c_offset;
														if (bits_left > 0)
														{
															pixel |= ((base_chardata[c_offset] >> (8 - bits_left)) << (7 - header->palette_format));
														}
													}
													pixel >>= (7 - header->palette_format);
													if (font_offset_x + x >= surface_descriptor->rect.width)
													{
														continue;
													}
													surface_data_rg8[((surface_descriptor->rect.height - 1 - font_offset_y - y) * surface_descriptor->rect.width) + font_offset_x + x + (stride_check * (surface_descriptor->rect.height - 1 - font_offset_y - y))].r = pixel;
												}
											}
											font_offset_x += max_font_width;
											if (characters_per_row == 0 && font_offset_x >= surface_descriptor->rect.width)
											{
												font_offset_x = surface_descriptor->rect.x;
												font_offset_y += max_font_height;
											}
											if (font_offset_y >= surface_descriptor->rect.height)
											{
												break;
											}
										}
									}
									else
									{
										for (unsigned int i = 0; i < font_character_count; ++i)
										{
											if (characters_per_row != 0)
											{
												size_t index_mod = character_start_offset + i;
												if (index_mod != 0 && (index_mod % characters_per_row == 0))
												{
													font_offset_x = surface_descriptor->rect.x;
													font_offset_y += max_font_height;
													if (font_offset_y >= surface_descriptor->rect.height)
													{
														break;
													}
												}
											}
											for (unsigned short y = 0; y < max_font_height; ++y)
											{
												if (font_offset_y + y >= surface_descriptor->rect.height)
												{
													break;
												}
												for (unsigned short x = 0; x < max_font_width; ++x)
												{
													if (font_offset_x + x >= surface_descriptor->rect.width)
													{
														break;
													}
													surface_data_rg8[((surface_descriptor->rect.height - 1 - font_offset_y - y) * surface_descriptor->rect.width) + font_offset_x + x].r = filedata->font_data[(i * max_font_width * max_font_height) + (y * max_font_width) + x + (stride_check * (surface_descriptor->rect.height - 1 - font_offset_y - y))];
												}
											}
											font_offset_x += max_font_width;
											if (characters_per_row == 0 && font_offset_x >= surface_descriptor->rect.width)
											{
												font_offset_x = surface_descriptor->rect.x;
												font_offset_y += max_font_height;
											}
											if (font_offset_y >= surface_descriptor->rect.height)
											{
												break;
											}
										}
									}
									break;
								}
							}
							break;
						}
						case MSBTFONT_SURFACE_FORMAT_24_8:
						{
							size_t stride_check = (surface_descriptor->rect.width * 3) % 4;
							switch (surface_descriptor->origin)
							{
								case MSBTFONT_SURFACE_ORIGIN_UPPERLEFT:
								{
									if (header->palette_format < 7)
									{
										unsigned char bitmask = (0xFF << (7 - header->palette_format));
										for (unsigned int i = 0; i < font_character_count; ++i)
										{
											if (characters_per_row != 0)
											{
												size_t index_mod = character_start_offset + i;
												if (index_mod != 0 && (index_mod % characters_per_row == 0))
												{
													font_offset_x = surface_descriptor->rect.x;
													font_offset_y += max_font_height;
													if (font_offset_y >= surface_descriptor->rect.height)
													{
														break;
													}
												}
											}
											size_t c_offset = 0;
											unsigned char bit_offset = ((i * (header->palette_format + 1) * max_font_width * max_font_height) % 8);
											unsigned char *base_chardata = &filedata->font_data[((i * (header->palette_format + 1) * max_font_width * max_font_height) / 8)];
											for (unsigned short y = 0; y < max_font_height; ++y)
											{
												if (font_offset_y + y >= surface_descriptor->rect.height)
												{
													break;
												}
												for (unsigned short x = 0; x < max_font_width; ++x)
												{
													size_t bits_left = (header->palette_format + 1);
													unsigned char pixel = ((base_chardata[c_offset] & (bitmask >> bit_offset)) << bit_offset);
													unsigned char bit_offset_s = bit_offset;
													bit_offset += bits_left;
													if (bit_offset > 7)
													{
														bits_left -= (8 - bit_offset_s);
														bit_offset -= 8;
														++c_offset;
														if (bits_left > 0)
														{
															pixel |= ((base_chardata[c_offset] >> (8 - bits_left)) << (7 - header->palette_format));
														}
													}
													pixel >>= (7 - header->palette_format);
													if (font_offset_x + x >= surface_descriptor->rect.width)
													{
														continue;
													}
													surface_data[(((y + font_offset_y) * surface_descriptor->rect.width) + font_offset_x + x) * 3 + ((stride_check ? 4 - stride_check : 0) * (y + font_offset_y))] = pixel;
												}
											}
											font_offset_x += max_font_width;
											if (characters_per_row == 0 && font_offset_x >= surface_descriptor->rect.width)
											{
												font_offset_x = surface_descriptor->rect.x;
												font_offset_y += max_font_height;
											}
											if (font_offset_y >= surface_descriptor->rect.height)
											{
												break;
											}
										}
									}
									else
									{
										for (unsigned int i = 0; i < font_character_count; ++i)
										{
											if (characters_per_row != 0)
											{
												size_t index_mod = character_start_offset + i;
												if (index_mod != 0 && (index_mod % characters_per_row == 0))
												{
													font_offset_x = surface_descriptor->rect.x;
													font_offset_y += max_font_height;
													if (font_offset_y >= surface_descriptor->rect.height)
													{
														break;
													}
												}
											}
											for (unsigned short y = 0; y < max_font_height; ++y)
											{
												if (font_offset_y + y >= surface_descriptor->rect.height)
												{
													break;
												}
												for (unsigned short x = 0; x < max_font_width; ++x)
												{
													if (font_offset_x + x >= surface_descriptor->rect.width)
													{
														break;
													}
													surface_data[(((y + font_offset_y) * surface_descriptor->rect.width) + font_offset_x + x) * 3 + ((stride_check ? (4 - stride_check) : 0) * (y + font_offset_y))] = filedata->font_data[(i * max_font_width * max_font_height) + (y * max_font_width) + x];
												}
											}
											font_offset_x += max_font_width;
											if (characters_per_row == 0 && font_offset_x >= surface_descriptor->rect.width)
											{
												font_offset_x = surface_descriptor->rect.x;
												font_offset_y += max_font_height;
											}
											if (font_offset_y >= surface_descriptor->rect.height)
											{
												break;
											}
										}
									}
									break;
								}
								case MSBTFONT_SURFACE_ORIGIN_LOWERLEFT:
								{
									if (header->palette_format < 7)
									{
										unsigned char bitmask = (0xFF << (7 - header->palette_format));
										for (unsigned int i = 0; i < font_character_count; ++i)
										{
											if (characters_per_row != 0)
											{
												size_t index_mod = character_start_offset + i;
												if (index_mod != 0 && (index_mod % characters_per_row == 0))
												{
													font_offset_x = surface_descriptor->rect.x;
													font_offset_y += max_font_height;
													if (font_offset_y >= surface_descriptor->rect.height)
													{
														break;
													}
												}
											}
											size_t c_offset = 0;
											unsigned char bit_offset = ((i * (header->palette_format + 1) * max_font_width * max_font_height) % 8);
											unsigned char *base_chardata = &filedata->font_data[((i * (header->palette_format + 1) * max_font_width * max_font_height) / 8)];
											for (unsigned short y = 0; y < max_font_height; ++y)
											{
												if (font_offset_y + y >= surface_descriptor->rect.height)
												{
													break;
												}
												for (unsigned short x = 0; x < max_font_width; ++x)
												{
													size_t bits_left = (header->palette_format + 1);
													unsigned char pixel = ((base_chardata[c_offset] & (bitmask >> bit_offset)) << bit_offset);
													unsigned char bit_offset_s = bit_offset;
													bit_offset += bits_left;
													if (bit_offset > 7)
													{
														bits_left -= (8 - bit_offset_s);
														bit_offset -= 8;
														++c_offset;
														if (bits_left > 0)
														{
															pixel |= ((base_chardata[c_offset] >> (8 - bits_left)) << (7 - header->palette_format));
														}
													}
													pixel >>= (7 - header->palette_format);
													if (font_offset_x + x >= surface_descriptor->rect.width)
													{
														continue;
													}
													surface_data[(((surface_descriptor->rect.height - 1 - font_offset_y - y) * surface_descriptor->rect.width) + font_offset_x + x) * 3 + ((stride_check ? (4 - stride_check) : 0) * (surface_descriptor->rect.height - 1 - font_offset_y - y))] = pixel;
												}
											}
											font_offset_x += max_font_width;
											if (characters_per_row == 0 && font_offset_x >= surface_descriptor->rect.width)
											{
												font_offset_x = surface_descriptor->rect.x;
												font_offset_y += max_font_height;
											}
											if (font_offset_y >= surface_descriptor->rect.height)
											{
												break;
											}
										}
									}
									else
									{
										for (unsigned int i = 0; i < font_character_count; ++i)
										{
											if (characters_per_row != 0)
											{
												size_t index_mod = character_start_offset + i;
												if (index_mod != 0 && (index_mod % characters_per_row == 0))
												{
													font_offset_x = surface_descriptor->rect.x;
													font_offset_y += max_font_height;
													if (font_offset_y >= surface_descriptor->rect.height)
													{
														break;
													}
												}
											}
											for (unsigned short y = 0; y < max_font_height; ++y)
											{
												if (font_offset_y + y >= surface_descriptor->rect.height)
												{
													break;
												}
												for (unsigned short x = 0; x < max_font_width; ++x)
												{
													if (font_offset_x + x >= surface_descriptor->rect.width)
													{
														break;
													}
													surface_data[(((surface_descriptor->rect.height - 1 - font_offset_y - y) * surface_descriptor->rect.width) + font_offset_x + x) * 3 + ((stride_check ? 4 - stride_check : 0) * (surface_descriptor->rect.height - 1 - font_offset_y - y))] = filedata->font_data[(i * max_font_width * max_font_height) + (y * max_font_width) + x];
												}
											}
											font_offset_x += max_font_width;
											if (characters_per_row == 0 && font_offset_x >= surface_descriptor->rect.width)
											{
												font_offset_x = surface_descriptor->rect.x;
												font_offset_y += max_font_height;
											}
											if (font_offset_y >= surface_descriptor->rect.height)
											{
												break;
											}
										}
									}
									break;
								}
							}
							break;
						}
						case MSBTFONT_SURFACE_FORMAT_32_8:
						{
							typedef struct rgba8
							{
								unsigned char r;
								unsigned char g;
								unsigned char b;
								unsigned char a;
							} rgba8;
							rgba8 *surface_data_rgba8 = (rgba8 *)(surface_data);
							switch (surface_descriptor->origin)
							{
								case MSBTFONT_SURFACE_ORIGIN_UPPERLEFT:
								{
									if (header->palette_format < 7)
									{
										unsigned char bitmask = (0xFF << (7 - header->palette_format));
										for (unsigned int i = 0; i < font_character_count; ++i)
										{
											if (characters_per_row != 0)
											{
												size_t index_mod = character_start_offset + i;
												if (index_mod != 0 && (index_mod % characters_per_row == 0))
												{
													font_offset_x = surface_descriptor->rect.x;
													font_offset_y += max_font_height;
													if (font_offset_y >= surface_descriptor->rect.height)
													{
														break;
													}
												}
											}
											size_t c_offset = 0;
											unsigned char bit_offset = ((i * (header->palette_format + 1) * max_font_width * max_font_height) % 8);
											unsigned char *base_chardata = &filedata->font_data[((i * (header->palette_format + 1) * max_font_width * max_font_height) / 8)];
											for (unsigned short y = 0; y < max_font_height; ++y)
											{
												if (font_offset_y + y >= surface_descriptor->rect.height)
												{
													break;
												}
												for (unsigned short x = 0; x < max_font_width; ++x)
												{
													size_t bits_left = (header->palette_format + 1);
													unsigned char pixel = ((base_chardata[c_offset] & (bitmask >> bit_offset)) << bit_offset);
													unsigned char bit_offset_s = bit_offset;
													bit_offset += bits_left;
													if (bit_offset > 7)
													{
														bits_left -= (8 - bit_offset_s);
														bit_offset -= 8;
														++c_offset;
														if (bits_left > 0)
														{
															pixel |= ((base_chardata[c_offset] >> (8 - bits_left)) << (7 - header->palette_format));
														}
													}
													pixel >>= (7 - header->palette_format);
													if (font_offset_x + x >= surface_descriptor->rect.width)
													{
														continue;
													}
													surface_data_rgba8[((y + font_offset_y) * surface_descriptor->rect.width) + font_offset_x + x].r = pixel;
												}
											}
											font_offset_x += max_font_width;
											if (characters_per_row == 0 && font_offset_x >= surface_descriptor->rect.width)
											{
												font_offset_x = surface_descriptor->rect.x;
												font_offset_y += max_font_height;
											}
											if (font_offset_y >= surface_descriptor->rect.height)
											{
												break;
											}
										}
									}
									else
									{
										for (unsigned int i = 0; i < font_character_count; ++i)
										{
											if (characters_per_row != 0)
											{
												size_t index_mod = character_start_offset + i;
												if (index_mod != 0 && (index_mod % characters_per_row == 0))
												{
													font_offset_x = surface_descriptor->rect.x;
													font_offset_y += max_font_height;
													if (font_offset_y >= surface_descriptor->rect.height)
													{
														break;
													}
												}
											}
											for (unsigned short y = 0; y < max_font_height; ++y)
											{
												if (font_offset_y + y >= surface_descriptor->rect.height)
												{
													break;
												}
												for (unsigned short x = 0; x < max_font_width; ++x)
												{
													if (font_offset_x + x >= surface_descriptor->rect.width)
													{
														break;
													}
													surface_data_rgba8[((y + font_offset_y) * surface_descriptor->rect.width) + font_offset_x + x].r = filedata->font_data[(i * max_font_width * max_font_height) + (y * max_font_width) + x];
												}
											}
											font_offset_x += max_font_width;
											if (characters_per_row == 0 && font_offset_x >= surface_descriptor->rect.width)
											{
												font_offset_x = surface_descriptor->rect.x;
												font_offset_y += max_font_height;
											}
											if (font_offset_y >= surface_descriptor->rect.height)
											{
												break;
											}
										}
									}
									break;
								}
								case MSBTFONT_SURFACE_ORIGIN_LOWERLEFT:
								{
									if (header->palette_format < 7)
									{
										unsigned char bitmask = (0xFF << (7 - header->palette_format));
										for (unsigned int i = 0; i < font_character_count; ++i)
										{
											if (characters_per_row != 0)
											{
												size_t index_mod = character_start_offset + i;
												if (index_mod != 0 && (index_mod % characters_per_row == 0))
												{
													font_offset_x = surface_descriptor->rect.x;
													font_offset_y += max_font_height;
													if (font_offset_y >= surface_descriptor->rect.height)
													{
														break;
													}
												}
											}
											size_t c_offset = 0;
											unsigned char bit_offset = ((i * (header->palette_format + 1) * max_font_width * max_font_height) % (8 - header->palette_format));
											unsigned char *base_chardata = &filedata->font_data[((i * (header->palette_format + 1) * max_font_width * max_font_height) / (8 - header->palette_format))];
											for (unsigned short y = 0; y < max_font_height; ++y)
											{
												if (font_offset_y + y >= surface_descriptor->rect.height)
												{
													break;
												}
												for (unsigned short x = 0; x < max_font_width; ++x)
												{
													size_t bits_left = (header->palette_format + 1);
													unsigned char pixel = ((base_chardata[c_offset] & (bitmask >> bit_offset)) << bit_offset);
													unsigned char bit_offset_s = bit_offset;
													bit_offset += bits_left;
													if (bit_offset > 7)
													{
														bits_left -= (8 - bit_offset_s);
														bit_offset -= 8;
														++c_offset;
														if (bits_left > 0)
														{
															pixel |= ((base_chardata[c_offset] >> (8 - bits_left)) << (7 - header->palette_format));
														}
													}
													pixel >>= (7 - header->palette_format);
													if (font_offset_x + x >= surface_descriptor->rect.width)
													{
														continue;
													}
													surface_data_rgba8[((surface_descriptor->rect.height - 1 - font_offset_y - y) * surface_descriptor->rect.width) + font_offset_x + x].r = pixel;
												}
											}
											font_offset_x += max_font_width;
											if (characters_per_row == 0 && font_offset_x >= surface_descriptor->rect.width)
											{
												font_offset_x = surface_descriptor->rect.x;
												font_offset_y += max_font_height;
											}
											if (font_offset_y >= surface_descriptor->rect.height)
											{
												break;
											}
										}
									}
									else
									{
										for (unsigned int i = 0; i < font_character_count; ++i)
										{
											if (characters_per_row != 0)
											{
												size_t index_mod = character_start_offset + i;
												if (index_mod != 0 && (index_mod % characters_per_row == 0))
												{
													font_offset_x = surface_descriptor->rect.x;
													font_offset_y += max_font_height;
													if (font_offset_y >= surface_descriptor->rect.height)
													{
														break;
													}
												}
											}
											for (unsigned short y = 0; y < max_font_height; ++y)
											{
												if (font_offset_y + y >= surface_descriptor->rect.height)
												{
													break;
												}
												for (unsigned short x = 0; x < max_font_width; ++x)
												{
													if (font_offset_x + x >= surface_descriptor->rect.width)
													{
														break;
													}
													surface_data_rgba8[((surface_descriptor->rect.height - 1 - font_offset_y - y) * surface_descriptor->rect.width) + font_offset_x + x].r = filedata->font_data[(i * max_font_width * max_font_height) + (y * max_font_width) + x];
												}
											}
											font_offset_x += max_font_width;
											if (characters_per_row == 0 && font_offset_x >= surface_descriptor->rect.width)
											{
												font_offset_x = surface_descriptor->rect.x;
												font_offset_y += max_font_height;
											}
											if (font_offset_y >= surface_descriptor->rect.height)
											{
												break;
											}
										}
									}
									break;
								}
							}
							break;
						}
						default:
						{
							return MSBTFONT_UNSUPPORTED_SURFACE_FORMAT;
						}
					}
					return MSBTFONT_SUCCESS;
				}
				else
				{
					return MSBTFONT_MISSING_SURFACE_DATA;
				}
			}
			else
			{
				return MSBTFONT_MISSING_SURFACE_DESCRIPTOR;
			}
		}
		else
		{
			return MSBTFONT_MISSING_FILEDATA;
		}
	}
	else
	{
		return MSBTFONT_MISSING_HEADER;
	}
}
