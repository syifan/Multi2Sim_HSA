/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <lib/mhandle/mhandle.h>

#include "init-list-elem.h"
#include "val.h"

struct cl2llvm_init_list_elem_t *cl2llvm_create_init_list_elem(char *name)
{
	struct cl2llvm_init_list_elem_t *elem;
	elem = xcalloc(1,sizeof(struct cl2llvm_init_list_elem_t));
	elem->elem_name = xstrdup(name);
	return elem;
}

void cl2llvm_init_list_elem_free(struct cl2llvm_init_list_elem_t *elem)
{
	free(elem->elem_name);
	free(elem);
}
