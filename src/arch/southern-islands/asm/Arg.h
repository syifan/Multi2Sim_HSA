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

#ifndef ARCH_SOUTHERN_ISLANDS_ASM_ARG_H
#define ARCH_SOUTHERN_ISLANDS_ASM_ARG_H


namespace SI
{

enum ArgType
{
	ArgTypeInvalid = 0,
	ArgTypeValue,
	ArgTypePointer,
	ArgTypeImage,
	ArgTypeSampler
};

enum ArgAccessType
{
	ArgAccessTypeInvalid = 0,
	ArgAccessTypeReadOnly,
	ArgAccessTypeWriteOnly,
	ArgAccessTypeReadWrite
};

enum ArgScope
{
	ArgScopeInavlid = 0,
	ArgScopeGlobal,
	ArgScopeEmuPrivate,
	ArgScopeEmuLocal,
	ArgScopeUAV,
	ArgScopeEmuConstant,
	ArgScopeEmuGDS,
	ArgScopeHwLocal,
	ArgScopeHwPrivate,
	ArgScopeHwConstant,
	ArgScopeHwGDS
};

enum ArgDataType
{
	ArgDataTypeInvalid = 0,
	ArgDataTypeInt1,
	ArgDataTypeInt8,
	ArgDataTypeInt16,
	ArgDataTypeInt32,
	ArgDataTypeInt64,
	ArgDataTypeUInt1,
	ArgDataTypeUInt8,
	ArgDataTypeUInt16,
	ArgDataTypeUInt32,
	ArgDataTypeUInt64,
	ArgDataTypeFloat,
	ArgDataTypeDouble,
	ArgDataTypeStruct,
	ArgDataTypeUnion,
	ArgDataTypeEvent,
	ArgDataTypeOpaque
};


class Arg
{
	ArgType type;
	std::string name;

	bool set;  /* Set to true when it is assigned */
	int size; /* Inferred from metadata or user calls */
	bool constarg; /* Set to true when argument is constant */

public:
	Arg(ArgType type, const std::string &name);

	ArgType GetType() { return type; }
	std::string GetName() { return name; }
	
	static int GetDataSize(ArgDataType data_type);
};


class ArgPointer : public Arg
{
	/* Metadata info */
	ArgDataType data_type;
	int num_elems;
	int constant_buffer_num;
	int constant_offset;
	ArgScope scope;
	int buffer_num;
	int alignment;
	ArgAccessType access_type;

	/* Value set by user */
	unsigned int device_ptr;

public:
	ArgPointer(const std::string &name,
			ArgDataType data_type,
			int num_elems,
			int constant_buffer_num,
			int constant_offset,
			ArgScope scope,
			int buffer_num,
			int alignment,
			ArgAccessType access_type) :
				Arg(ArgTypePointer, name),
				data_type(data_type),
				num_elems(num_elems),
				constant_buffer_num(constant_buffer_num),
				scope(scope),
				buffer_num(buffer_num),
				alignment(alignment) { };

	void SetDevicePtr(unsigned int device_ptr) {
		this->device_ptr = device_ptr; }
};


class ArgValue : public Arg
{
	/* Metadata info */
	ArgDataType data_type;
	int num_elems;
	int constant_buffer_num;
	int constant_offset;

	/* Value set by user */
	std::unique_ptr<void> value;

public:
	ArgValue(const std::string &name,
			ArgDataType data_type,
			int num_elems,
			int constant_buffer_num,
			int constant_offset) :
				Arg(ArgTypeValue, name),
				data_type(data_type),
				num_elems(num_elems),
				constant_buffer_num(constant_buffer_num),
				constant_offset(constant_offset) { }

	void SetValue(void *value) { this->value.reset(value); }
};


class ArgImage : public Arg
{
	int dimension;  /* 2 or 3 */
	ArgAccessType access_type;
	int uav;
	int constant_buffer_num;
	int constant_offset;

public:

	ArgImage(const std::string &name,
			int dimension,
			ArgAccessType access_type,
			int uav,
			int constant_buffer_num,
			int constant_buffer_offset) :
				Arg(ArgTypeImage, name),
				dimension(dimension),
				access_type(access_type),
				uav(uav),
				constant_buffer_num(constant_buffer_num),
				constant_offset(constant_offset) { }
};


class ArgSampler : public Arg
{
	int id;
	unsigned int location;
	int value;

public:

	ArgSampler(const std::string &name,
			int id,
			unsigned int location,
			int value) :
				Arg(ArgTypeSampler, name),
				id(id),
				location(location),
				value(value) { }
};



/*
 * Public
 */

/*
struct StringMapWrap;
extern struct StringMapWrap *si_arg_dimension_map;
extern struct StringMapWrap *si_arg_access_type_map;
extern struct StringMapWrap *si_arg_data_type_map;
extern struct StringMapWrap *si_arg_scope_map;
extern struct StringMapWrap *si_arg_reflection_map;
*/

}  /* namespace SI */

#endif
