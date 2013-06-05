/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#include <assert.h>
#include <stdlib.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "arg.h"
#include "si2bin.h"


struct str_map_t si2bin_arg_type_map =
{
	13,
	{
		{ "invalid", si2bin_arg_invalid },
		{ "sreg", si2bin_arg_scalar_register },
		{ "vreg", si2bin_arg_vector_register },
		{ "sreg_series", si2bin_arg_scalar_register_series },
		{ "vreg_series", si2bin_arg_vector_register_series },
		{ "mreg", si2bin_arg_mem_register },
		{ "special_reg", si2bin_arg_special_register },
		{ "const", si2bin_arg_literal },
		{ "const_float", si2bin_arg_literal_float },
		{ "waitcnt", si2bin_arg_waitcnt },
		{ "label", si2bin_arg_label },
		{ "maddr", si2bin_arg_maddr },
		{ "maddr_qual", si2bin_arg_maddr_qual }
	}
};


struct si2bin_arg_t *si2bin_arg_create(void)
{
	struct si2bin_arg_t *arg;
	
	/* Allocate */
	arg = xcalloc(1, sizeof(struct si2bin_arg_t));
	
	/* Return */
	return arg;
	
}


struct si2bin_arg_t *si2bin_arg_create_literal(int value)
{
	struct si2bin_arg_t *arg;

	arg = si2bin_arg_create();
	arg->type = si2bin_arg_literal;
	arg->value.literal.val = value;

	return arg;
}

struct si2bin_arg_t *si2bin_arg_create_literal_float(float value)
{
	struct si2bin_arg_t *arg;

	arg = si2bin_arg_create();
	arg->type = si2bin_arg_literal_float;
	arg->value.literal_float.val = value;

	return arg;
}


struct si2bin_arg_t *si2bin_arg_create_scalar_register(int id)
{
	struct si2bin_arg_t *arg;

	arg = si2bin_arg_create();
	arg->type = si2bin_arg_scalar_register;
	arg->value.scalar_register.id = id;

	if (!IN_RANGE(arg->value.scalar_register.id, 0, 255))
		si2bin_yyerror_fmt("scalar register out of range: s%d", id);

	return arg;
}


struct si2bin_arg_t *si2bin_arg_create_scalar_register_series(int low, int high)
{
	struct si2bin_arg_t *arg;

	arg = si2bin_arg_create();
	arg->type = si2bin_arg_scalar_register_series;
	arg->value.scalar_register_series.low = low;
	arg->value.scalar_register_series.high = high;
	assert(high >= low);

	return arg;
}


struct si2bin_arg_t *si2bin_arg_create_vector_register(int id)
{
	struct si2bin_arg_t *arg;

	arg = si2bin_arg_create();
	arg->type = si2bin_arg_vector_register;
	arg->value.vector_register.id = id;

	if (!IN_RANGE(arg->value.vector_register.id, 0, 255))
		si2bin_yyerror_fmt("vector register out of range: v%d", id);

	return arg;
}


struct si2bin_arg_t *si2bin_arg_create_vector_register_series(int low, int high)
{
	struct si2bin_arg_t *arg;

	arg = si2bin_arg_create();
	arg->type = si2bin_arg_vector_register_series;
	arg->value.vector_register_series.low = low;
	arg->value.vector_register_series.high = high;
	assert(high >= low);

	return arg;
}


struct si2bin_arg_t *si2bin_arg_create_special_register(enum si_inst_special_reg_t reg)
{
	struct si2bin_arg_t *arg;

	arg = si2bin_arg_create();
	arg->type = si2bin_arg_special_register;
	arg->value.special_register.reg = reg;

	return arg;
}


struct si2bin_arg_t *si2bin_arg_create_maddr(struct si2bin_arg_t *soffset,
		struct si2bin_arg_t *qual,
		enum si_inst_buf_data_format_t data_format,
		enum si_inst_buf_num_format_t num_format)
{
	struct si2bin_arg_t *arg;

	arg = si2bin_arg_create();
	arg->type = si2bin_arg_maddr;
	arg->value.maddr.soffset = soffset;
	arg->value.maddr.qual = qual;
	arg->value.maddr.data_format = data_format;
	arg->value.maddr.num_format = num_format;

	return arg;
}


struct si2bin_arg_t *si2bin_arg_create_maddr_qual(void)
{
	struct si2bin_arg_t *arg;

	arg = si2bin_arg_create();
	arg->type = si2bin_arg_maddr_qual;

	return arg;
}


struct si2bin_arg_t *si2bin_arg_create_label(struct si2bin_symbol_t *symbol)
{
	struct si2bin_arg_t *arg;

	arg = si2bin_arg_create();
	arg->type = si2bin_arg_label;
	arg->value.label.symbol = symbol;

	return arg;
}


void si2bin_arg_free(struct si2bin_arg_t *arg)
{
	switch (arg->type)
	{

	case si2bin_arg_maddr:

		si2bin_arg_free(arg->value.maddr.soffset);
		si2bin_arg_free(arg->value.maddr.qual);
		break;

	default:
		break;
	}

	free(arg);
}


int si2bin_arg_encode_operand(struct si2bin_arg_t *arg)
{
	switch (arg->type)
	{

	case si2bin_arg_literal:
	{
		int value;

		value = arg->value.literal.val;
		if (IN_RANGE(value, 0, 64))
			return value + 128;
		if (IN_RANGE(value, -16, -1))
			return 192 - value;
		si2bin_yyerror_fmt("invalid integer constant: %d", value);
		break;
	}

	case si2bin_arg_literal_float:
	{
		float value;

		value = arg->value.literal_float.val;
		if (value == 0.5)
			return 240;
		if (value == -0.5)
			return 241;
		if (value == 1.0)
			return 242;
		if (value == -1.0)
			return 243;
		if (value == 2.0)
			return 244;
		if (value == -2.0)
			return 245;
		if (value == 4.0)
			return 246;
		if (value == -4.0)
			return 247;

		si2bin_yyerror_fmt("invalid float constant: %g", value);
		break;
	}

	case si2bin_arg_scalar_register:
	{
		int id;

		id = arg->value.scalar_register.id;
		if (IN_RANGE(id, 0, 103))
			return id;

		si2bin_yyerror_fmt("invalid scalar register: s%d", id);
		break;
	}

	/* Encode the low register */
	case si2bin_arg_scalar_register_series:
	{
		int id;

		id = arg->value.scalar_register_series.low;
		if (IN_RANGE(id, 0, 103))
			return id;

		si2bin_yyerror_fmt("invalid scalar register: s%d", id);
		break;
	}

	case si2bin_arg_vector_register:
	{
		int id;

		id = arg->value.vector_register.id;
		if (IN_RANGE(id, 0, 255))
			return id + 256;

		si2bin_yyerror_fmt("invalid vector register: v%d", id);
		break;
	}

	/* Encode the low register */
	case si2bin_arg_vector_register_series:
	{
		int id;

		id = arg->value.vector_register_series.low;
		if (IN_RANGE(id, 0, 255))
			return id + 256;

		si2bin_yyerror_fmt("invalid vector register: v%d", id);
		break;
	}

	/* Special register */
	case si2bin_arg_special_register:
	{
		switch (arg->value.special_register.reg)
		{
		case si_inst_special_reg_vcc:
			return 106;

		case si_inst_special_reg_exec:
			return 126;

		case si_inst_special_reg_scc:
			return 253;

		default:
			si2bin_yyerror_fmt("%s: unsupported special register (code=%d)",
				__FUNCTION__, arg->value.special_register.reg);
		}
		break;
	}

	default:
		si2bin_yyerror_fmt("invalid operand (code %d)", arg->type);
		break;
	}

	/* Unreachable */
	return 0;
}


void si2bin_arg_dump(struct si2bin_arg_t *arg, FILE *f)
{
	switch (arg->type)
	{
	
	case si2bin_arg_invalid:

		fprintf(f, "<invalid>");
		break;
		
	case si2bin_arg_scalar_register:

		fprintf(f, "<sreg> s%d", arg->value.scalar_register.id);
		break;
		
	case si2bin_arg_vector_register:

		fprintf(f, "<vreg> v%d", arg->value.vector_register.id);
		break;
		
	case si2bin_arg_scalar_register_series:

		fprintf(f, "<sreg_series> s[%d:%d]",
			arg->value.scalar_register_series.low,
			arg->value.scalar_register_series.high);
		break;
			
	case si2bin_arg_vector_register_series:

		fprintf(f, "<vreg_series> v[%d:%d]",
			arg->value.vector_register_series.low,
			arg->value.vector_register_series.high);
		break;
			
	case si2bin_arg_literal:
	{
		int value;

		value = arg->value.literal.val;
		fprintf(f, "<const> %d", value);
		if (value)
			fprintf(f, " (0x%x)", value);
		break;
	}
		
	case si2bin_arg_literal_float:

		fprintf(f, "<const_float> %g", arg->value.literal_float.val);
		break;

	case si2bin_arg_waitcnt:
	{
		char buf[MAX_STRING_SIZE];

		fprintf(f, "<waitcnt>");

		snprintf(buf, sizeof buf, "%d", arg->value.wait_cnt.vmcnt_value);
		fprintf(f, " vmcnt=%s", arg->value.wait_cnt.vmcnt_active ? buf : "x");

		snprintf(buf, sizeof buf, "%d", arg->value.wait_cnt.expcnt_value);
		fprintf(f, " expcnt=%s", arg->value.wait_cnt.expcnt_active ? buf : "x");

		snprintf(buf, sizeof buf, "%d", arg->value.wait_cnt.lgkmcnt_value);
		fprintf(f, " lgkmcnt=%s", arg->value.wait_cnt.lgkmcnt_active ? buf : "x");

		break;
	}

	case si2bin_arg_special_register:

		fprintf(f, "<special_reg> %s", str_map_value(&si_inst_special_reg_map,
				arg->value.special_register.reg));
		break;
	
	case si2bin_arg_mem_register:

		fprintf(f, "<mreg> m%d", arg->value.mem_register.id);
		break;
	
	case si2bin_arg_maddr:

		fprintf(f, "<maddr>");

		fprintf(f, " soffs={");
		si2bin_arg_dump(arg->value.maddr.soffset, f);
		fprintf(f, "}");

		fprintf(f, " qual={");
		si2bin_arg_dump(arg->value.maddr.qual, f);
		fprintf(f, "}");

		fprintf(f, " dfmt=%s", str_map_value(&si_inst_buf_data_format_map,
				arg->value.maddr.data_format));
		fprintf(f, " nfmt=%s", str_map_value(&si_inst_buf_num_format_map,
				arg->value.maddr.num_format));

		break;

	case si2bin_arg_maddr_qual:

		fprintf(f, "offen=%c", arg->value.maddr_qual.offen ? 't' : 'f');
		fprintf(f, " idxen=%c", arg->value.maddr_qual.idxen ? 't' : 'f');
		fprintf(f, " offset=%d", arg->value.maddr_qual.offset);
		break;

	case si2bin_arg_label:
		fprintf(f, "<label>");
		break;
		
	default:
		panic("%s: invalid argument type", __FUNCTION__);
		break;
	}
}


void si2bin_arg_valid_types(struct si2bin_arg_t *arg,
		enum si2bin_arg_type_t *types, int num_types,
		const char *user_message)
{
	char msg[MAX_STRING_SIZE];
	char *msg_ptr;
	char *sep;

	int msg_size;
	int i;

	/* Check if argument type if valid */
	for (i = 0; i < num_types; i++)
		if (arg->type == types[i])
			return;

	/* Construct error message */
	msg[0] = '\0';
	msg_ptr = msg;
	msg_size = sizeof msg;
	str_printf(&msg_ptr, &msg_size, "argument of type %s found, {",
			str_map_value(&si2bin_arg_type_map, arg->type));

	/* List allowed types */
	sep = "";
	for (i = 0; i < num_types; i++)
	{
		str_printf(&msg_ptr, &msg_size, "%s%s", sep,
				str_map_value(&si2bin_arg_type_map, types[i]));
		sep = "|";
	}

	/* Message tail */
	str_printf(&msg_ptr, &msg_size, "} expected");
	fatal("%s: %s", user_message, msg);
}
