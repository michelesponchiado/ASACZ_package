/*
 * main.c
 *
 *  Created on: Mar 29, 2017
 *      Author: michele
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "/home/michele/workspace/OLinuxino_RF_data_send_receive/inc/ASACZ_firmware_version.h"
#include "/home/michele/workspace/OLinuxino_RF_data_send_receive/src/ASACZ_firmware_version.c"


#define def_magic_name_OLinuxino_ASACZ_header "OLinuxino_ASACZ_header"
typedef struct _type_OLinuxino_ASACZ_fw_update_header
{
	uint8_t magic_name[32];				// must be set to "OLinuxino_ASACZ_header", and filled up with 0x00 in the remaining bytes
	uint32_t major_number;				//!< the version major number
	uint32_t middle_number;				//!< the version middle number
	uint32_t minor_number;				//!< the version minor number
	uint32_t build_number;				//!< the build number
	uint8_t	date_and_time[64];			//!< the version date and time
	uint8_t	patch[64];					//!< the version patch
	uint8_t	notes[64];					//!< the version notes
	uint8_t	string[256];				//!< the version string
	uint32_t firmware_body_size;		// the expected number of bytes in the firmware body
	uint32_t firmware_body_CRC32_CC2650;// the CRC32 of the firmware body calculated as CC2650 does it, please see the calcCrcLikeChip routine
	uint32_t header_CRC32_CC2650;		// the CRC32 of the header (this field excluded), calculated as CC2650 does it, please see the calcCrcLikeChip routine

}__attribute__((__packed__)) type_OLinuxino_ASACZ_fw_update_header;


uint32_t calcCrcLikeChip(const unsigned char *pData, unsigned long ulByteCount)
{
	uint32_t d, ind;
	uint32_t acc = 0xFFFFFFFF;
	const uint32_t ulCrcRand32Lut[] =
	{
		0x00000000, 0x1DB71064, 0x3B6E20C8, 0x26D930AC,
		0x76DC4190, 0x6B6B51F4, 0x4DB26158, 0x5005713C,
		0xEDB88320, 0xF00F9344, 0xD6D6A3E8, 0xCB61B38C,
		0x9B64C2B0, 0x86D3D2D4, 0xA00AE278, 0xBDBDF21C
	};

	while ( ulByteCount-- )
	{
		d = *pData++;
		ind = (acc & 0x0F) ^ (d & 0x0F);
		acc = (acc >> 4) ^ ulCrcRand32Lut[ind];
		ind = (acc & 0x0F) ^ (d >> 4);
		acc = (acc >> 4) ^ ulCrcRand32Lut[ind];
	}

	return (acc ^ 0xFFFFFFFF);
}

typedef enum
{
	enum_asacz_package_retcode_OK =0,
	enum_asacz_package_retcode_ERR_unable_to_open_file,
	enum_asacz_package_retcode_ERR_unable_to_seek_end_input_file,
	enum_asacz_package_retcode_ERR_unable_to_get_input_file_size,
	enum_asacz_package_retcode_ERR_unable_to_seek_begin_input_file,
	enum_asacz_package_retcode_ERR_unable_to_alloc_input_file_body_buffer,
	enum_asacz_package_retcode_ERR_unable_to_read_input_file_body,
	enum_asacz_package_retcode_ERR_close_input_file,
	enum_asacz_package_retcode_ERR_magic_name_too_long,
	enum_asacz_package_retcode_ERR_unable_to_sprint_output_filename,
	enum_asacz_package_retcode_ERR_unable_to_open_output_file,
	enum_asacz_package_retcode_ERR_unable_to_write_header,
	enum_asacz_package_retcode_ERR_unable_to_write_body,
	enum_asacz_package_retcode_ERR_unable_to_close_output_file,

	enum_asacz_package_retcode_numof
}enum_asacz_package_retcode;

typedef struct _type_enum_asacz_package_retcode_table
{
	enum_asacz_package_retcode e;
	const char *s;
}type_enum_asacz_package_retcode_table;

static const type_enum_asacz_package_retcode_table asacz_package_retcode_table[]=
{
	{.e = enum_asacz_package_retcode_OK, .s ="OK"},
	{.e = enum_asacz_package_retcode_ERR_unable_to_open_file, .s ="ERR_unable_to_open_file"},
	{.e = enum_asacz_package_retcode_ERR_unable_to_seek_end_input_file,.s ="ERR_unable_to_seek_end_input_file"},
	{.e = enum_asacz_package_retcode_ERR_unable_to_get_input_file_size,.s ="ERR_unable_to_get_input_file_size"},
	{.e = enum_asacz_package_retcode_ERR_unable_to_seek_begin_input_file,.s ="ERR_unable_to_seek_begin_input_file"},
	{.e = enum_asacz_package_retcode_ERR_unable_to_alloc_input_file_body_buffer,.s ="ERR_unable_to_alloc_input_file_body_buffer"},
	{.e = enum_asacz_package_retcode_ERR_unable_to_read_input_file_body,.s ="ERR_unable_to_read_input_file_body"},
	{.e = enum_asacz_package_retcode_ERR_close_input_file,.s ="ERR_close_input_file"},
	{.e = enum_asacz_package_retcode_ERR_magic_name_too_long,.s ="ERR_magic_name_too_long"},
	{.e = enum_asacz_package_retcode_ERR_unable_to_sprint_output_filename,.s ="ERR_unable_to_sprint_output_filename"},
	{.e = enum_asacz_package_retcode_ERR_unable_to_open_output_file,.s ="ERR_unable_to_open_output_file"},
	{.e = enum_asacz_package_retcode_ERR_unable_to_write_header,.s ="ERR_unable_to_write_header"},
	{.e = enum_asacz_package_retcode_ERR_unable_to_write_body,.s ="ERR_unable_to_write_body"},
	{.e = enum_asacz_package_retcode_ERR_unable_to_close_output_file,.s ="ERR_unable_to_close_output_file"},
};

const char *get_enum_asacz_package_retcode_string(enum_asacz_package_retcode e)
{
	const char *p_return_string = "unknown return code";
	unsigned int found = 0;
	unsigned int i;
	for (i = 0; !found && i < sizeof(asacz_package_retcode_table)/sizeof(asacz_package_retcode_table[0]); i++)
	{
		const type_enum_asacz_package_retcode_table *p = &asacz_package_retcode_table[i];
		if (p->e == e)
		{
			p_return_string = p->s;
		}
	}

	return p_return_string;
}

#define ASACZ_PACKAGER_VERSION "0.1"

int main(int argc, char *argv[])
{
	printf ("ASACZ packager version %s %s %s\n", ASACZ_PACKAGER_VERSION, __DATE__, __TIME__);
	init_ASACZ_firmware_version();

	char *filename_in = "/home/michele/workspace/OLinuxino_RF_data_send_receive/OLinuxino/ASACZ";
	FILE *fin = NULL;
	FILE *fout = NULL;

	enum_asacz_package_retcode r  = enum_asacz_package_retcode_OK;
	if (r  == enum_asacz_package_retcode_OK)
	{
		fin = fopen(filename_in, "rb");
		if (!fin)
		{
			r = enum_asacz_package_retcode_ERR_unable_to_open_file;
		}
	}
	if (r  == enum_asacz_package_retcode_OK)
	{
		int result = fseek(fin, 0L, SEEK_END);
		if (result)
		{
			r = enum_asacz_package_retcode_ERR_unable_to_seek_end_input_file;
		}
	}
	long l_filesize = 0;
	if (r == enum_asacz_package_retcode_OK)
	{
		l_filesize = ftell(fin);
		if (l_filesize < 0)
		{
			r = enum_asacz_package_retcode_ERR_unable_to_get_input_file_size;
		}
	}
	if (r == enum_asacz_package_retcode_OK)
	{
		int result = fseek(fin, 0L, SEEK_SET);
		if (result)
		{
			r = enum_asacz_package_retcode_ERR_unable_to_seek_begin_input_file;
		}
	}
	uint8_t * p_bin_file_body = NULL;
	if (r == enum_asacz_package_retcode_OK)
	{
		p_bin_file_body = (uint8_t *)malloc(l_filesize);
		if (!p_bin_file_body)
		{
			r = enum_asacz_package_retcode_ERR_unable_to_alloc_input_file_body_buffer;
		}
		else
		{
			memset(p_bin_file_body, 0, l_filesize);
		}
	}
	if (r == enum_asacz_package_retcode_OK)
	{
		size_t n_read = fread(p_bin_file_body, l_filesize, 1, fin);
		if (n_read != 1)
		{
			r = enum_asacz_package_retcode_ERR_unable_to_read_input_file_body;
		}
	}
	if (fin)
	{
		if (fclose(fin))
		{
			if (r == enum_asacz_package_retcode_OK)
			{
				r = enum_asacz_package_retcode_ERR_close_input_file;
			}
		}
	}
	type_OLinuxino_ASACZ_fw_update_header OLinuxino_ASACZ_fw_update_header;

	memset(&OLinuxino_ASACZ_fw_update_header, 0, sizeof(OLinuxino_ASACZ_fw_update_header));

	if (r == enum_asacz_package_retcode_OK)
	{
		int n_needed = snprintf((char*)OLinuxino_ASACZ_fw_update_header.magic_name, sizeof(OLinuxino_ASACZ_fw_update_header.magic_name), "%s", def_magic_name_OLinuxino_ASACZ_header);
		if (n_needed >= (int)sizeof(OLinuxino_ASACZ_fw_update_header.magic_name))
		{
			r = enum_asacz_package_retcode_ERR_magic_name_too_long;
		}
	}
	if (r == enum_asacz_package_retcode_OK)
	{
		type_ASACZ_firmware_version ASACZ_firmware_version;
		get_ASACZ_firmware_version_whole_struct(&ASACZ_firmware_version);
		OLinuxino_ASACZ_fw_update_header.major_number 	= ASACZ_firmware_version.major_number;
		OLinuxino_ASACZ_fw_update_header.middle_number 	= ASACZ_firmware_version.middle_number;
		OLinuxino_ASACZ_fw_update_header.minor_number 	= ASACZ_firmware_version.minor_number;
		OLinuxino_ASACZ_fw_update_header.build_number  	= ASACZ_firmware_version.build_number;

		snprintf( (char*)OLinuxino_ASACZ_fw_update_header.notes,
					sizeof(OLinuxino_ASACZ_fw_update_header.notes),
					"%s",
					ASACZ_firmware_version.notes
				);
		snprintf( 	(char*)OLinuxino_ASACZ_fw_update_header.patch,
					sizeof(OLinuxino_ASACZ_fw_update_header.patch),
					"%s",
					ASACZ_firmware_version.patch
				);
		snprintf( 	(char*)OLinuxino_ASACZ_fw_update_header.date_and_time,
					sizeof(OLinuxino_ASACZ_fw_update_header.date_and_time),
					"%s",
					ASACZ_firmware_version.date_and_time
				);
		snprintf( 	(char*)OLinuxino_ASACZ_fw_update_header.string,
					sizeof(OLinuxino_ASACZ_fw_update_header.string),
					"%s",
					ASACZ_firmware_version.string
				);
	}
	if (r == enum_asacz_package_retcode_OK)
	{
		OLinuxino_ASACZ_fw_update_header.firmware_body_size = l_filesize;
		OLinuxino_ASACZ_fw_update_header.firmware_body_CRC32_CC2650 = calcCrcLikeChip((const unsigned char *)p_bin_file_body, OLinuxino_ASACZ_fw_update_header.firmware_body_size);

	}
	if (r == enum_asacz_package_retcode_OK)
	{
		int header_CRC_size = sizeof(OLinuxino_ASACZ_fw_update_header) - sizeof(OLinuxino_ASACZ_fw_update_header.header_CRC32_CC2650);
		OLinuxino_ASACZ_fw_update_header.header_CRC32_CC2650 = calcCrcLikeChip((const unsigned char *)&OLinuxino_ASACZ_fw_update_header, header_CRC_size);
	}
	char outfile_name[1024];
	memset(outfile_name, 0, sizeof(outfile_name));
	if (r == enum_asacz_package_retcode_OK)
	{
		int n = snprintf(outfile_name, sizeof(outfile_name), "OLINUXINO_ASACZ.%u_%u_%u_build%u"
				, OLinuxino_ASACZ_fw_update_header.major_number
				, OLinuxino_ASACZ_fw_update_header.middle_number
				, OLinuxino_ASACZ_fw_update_header.minor_number
				, OLinuxino_ASACZ_fw_update_header.build_number
				);
		if (n >= (int)sizeof(outfile_name))
		{
			r = enum_asacz_package_retcode_ERR_unable_to_sprint_output_filename;
		}
		else
		{
			fout = fopen(outfile_name, "wb");
			if (!fout)
			{
				r = enum_asacz_package_retcode_ERR_unable_to_open_output_file;
			}
		}
	}
	if (r == enum_asacz_package_retcode_OK)
	{
		size_t n = fwrite(&OLinuxino_ASACZ_fw_update_header, sizeof(OLinuxino_ASACZ_fw_update_header), 1, fout);
		if (n != 1)
		{
			r = enum_asacz_package_retcode_ERR_unable_to_write_header;
		}
	}
	if (r == enum_asacz_package_retcode_OK)
	{
		size_t n = fwrite(p_bin_file_body, l_filesize, 1, fout);
		if (n != 1)
		{
			r = enum_asacz_package_retcode_ERR_unable_to_write_body;
		}
	}
	if (fout)
	{
		if (fclose(fout))
		{
			if (r == enum_asacz_package_retcode_OK)
			{
				r = enum_asacz_package_retcode_ERR_unable_to_close_output_file;
			}
		}
	}
	if (r == enum_asacz_package_retcode_OK)
	{
		printf("Output generated OK to %s\n", outfile_name);
		printf("Header info:\n");
		printf("\t magic key: %s\n"	, OLinuxino_ASACZ_fw_update_header.magic_name);
		printf("\t ASACZ:\n");
		printf("\t   version: %u.%u.%u build %u\n"	, OLinuxino_ASACZ_fw_update_header.major_number, OLinuxino_ASACZ_fw_update_header.middle_number, OLinuxino_ASACZ_fw_update_header.minor_number, OLinuxino_ASACZ_fw_update_header.build_number);
		printf("\t   patch: %s\n"	, OLinuxino_ASACZ_fw_update_header.patch);
		printf("\t   date and time: %s\n"	, OLinuxino_ASACZ_fw_update_header.date_and_time);
		printf("\t   notes: %s\n"	, OLinuxino_ASACZ_fw_update_header.notes);
		printf("\t   full string: %s\n"	, OLinuxino_ASACZ_fw_update_header.string);
		printf("\t body size  %u bytes\n"			, OLinuxino_ASACZ_fw_update_header.firmware_body_size);
		printf("\t body CRC   0x%08X\n"		, OLinuxino_ASACZ_fw_update_header.firmware_body_CRC32_CC2650);
		printf("\t header CRC 0x%08X\n"	, OLinuxino_ASACZ_fw_update_header.header_CRC32_CC2650);
	}
	else
	{
		printf("ERROR %u: %s\n", (unsigned int)r, get_enum_asacz_package_retcode_string(r));
	}


	return r;
}
