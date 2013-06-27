/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This module is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this module; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <string.h>

#include "../include/cuda.h"
#include "api.h"
#include "debug.h"
#include "elf-format.h"
#include "list.h"
#include "function-arg.h"
#include "function.h"
#include "mhandle.h"
#include "module.h"


struct list_t *function_list;

CUfunction cuda_function_create(CUmodule module, const char *function_name)
{
	CUfunction function;

	struct elf_file_t *dev_func_bin;
	struct elf_section_t *sec;
	char text_sec_name[1024];
	int text_sec_index;
	struct elf_section_t *text_sec;

	int i;
	unsigned char inst_buffer_byte;

	function = (CUfunction)xcalloc(1, sizeof(struct CUfunc_st));
	function->id = list_count(function_list);
	function->ref_count = 1;
	function->name = xstrdup(function_name);
	function->module_id = module->id;

	dev_func_bin = module->elf_file;

	/* Look for .text.device_function_name section */
	snprintf(text_sec_name, sizeof text_sec_name, ".text.%s",
			function_name);
	text_sec_index = 0;
	for (i = 0; i < list_count(dev_func_bin->section_list); ++i)
	{
		sec = (struct elf_section_t *)list_get(
				dev_func_bin->section_list, i);

		if (!strncmp(sec->name, text_sec_name, sizeof text_sec_name))
		{
			text_sec_index = i;
			break;
		}
	}
	if (text_sec_index == 0)
		fatal("\tCannot get .text.device_function_name section!\n"
		     "\tThis is probably because the cubin filename set in\n"
		     "\tM2S_CUDA_BINARY is not the device function identifier.\n"
		     "\tPlease check the cubin file.");

	/* Get .text.device_function_name section */
	text_sec = (struct elf_section_t *)list_get(
			dev_func_bin->section_list, text_sec_index);

	/* Get instruction binary */
	function->inst_buffer_size = text_sec->header->sh_size;
	function->inst_buffer = (unsigned long long int *)xcalloc(1,
			function->inst_buffer_size);
	for (i = 0; i < function->inst_buffer_size; ++i)
	{
		elf_buffer_seek(&(dev_func_bin->buffer),
				text_sec->header->sh_offset + i);
		elf_buffer_read(&(dev_func_bin->buffer), &inst_buffer_byte, 1);
		if (i % 8 == 0 || i % 8 == 1 || i % 8 == 2 || i % 8 == 3)
		{
			function->inst_buffer[i / 8] |= 
				(unsigned long long int)(inst_buffer_byte) << 
				(i * 8 + 32);
		}
		else
		{
			function->inst_buffer[i / 8] |= 
				(unsigned long long int)(inst_buffer_byte) << 
				(i * 8 - 32);
		}
	}

	/* Get GPR usage */
	function->num_gpr_used = text_sec->header->sh_info >> 24;

	function->arg_list = list_create();

	list_add(function_list, function);

	return function;
}

void cuda_function_free(CUfunction function)
{
	int i;

	list_remove(function_list, function);

	for (i = 0; i < list_count(function->arg_list); i++)
	{
		cuda_function_arg_free(function, 
				(struct cuda_function_arg_t *)list_get(
					function->arg_list, i));
	}
	list_free(function->arg_list);
	free(function->inst_buffer);
	free(function->name);
	function->ref_count--;
	free(function);
}

