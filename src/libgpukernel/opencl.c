/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal (ubal@gap.upv.es)
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

#include <gpukernel-local.h>
#include <m2skernel.h>
#include <assert.h>
#include <debug.h>


/* Debug info */
int opencl_debug_category;

void opencl_debug_array(int nelem, int *array)
{
	char *comma = "";
	int i;

	opencl_debug("{");
	for (i = 0; i < nelem; i++) {
		opencl_debug("%s%d", comma, array[i]);
		comma = ", ";
	}
	opencl_debug("}");
}


/* List of OpenCL function names */
char *opencl_func_names[] = {
#define DEF_OPENCL_FUNC(_name, _argc) #_name,
#include "opencl.dat"
#undef DEF_OPENCL_FUNC
	""
};


/* Number of arguments for each OpenCL function */
int opencl_func_argc[] = {
#define DEF_OPENCL_FUNC(_name, _argc) _argc,
#include "opencl.dat"
#undef DEF_OPENCL_FUNC
	0
};


/* Error messages */
char *err_opencl_note =
	"\tThe OpenCL interface is implemented in library 'm2s-opencl.so' as a set of\n"
	"\tsystem calls, intercepted by Multi2Sim and emulated in 'opencl.c'.\n"
	"\tHowever, only a subset of this interface is currently implemented in the simulator.\n"
	"\tTo request the implementation of a specific OpenCL call, please email\n"
	"\t'development@multi2sim.org'.\n";

char *err_opencl_param_note =
	"\tNote that a real OpenCL implementation would return an error code to the\n"
	"\tcalling program. However, the purpose of this OpenCL implementation is to\n"
	"\tsupport correctly written programs. Thus, a detailed implementation of OpenCL\n"
	"\terror handling is not provided, and any OpenCL error will cause the\n"
	"\tsimulation to stop.\n";

char *err_opencl_compiler =
	"\tThe Multi2Sim implementation of the OpenCL interface does not support runtime\n"
	"\tcompilation of kernel sources. To run OpenCL kernels, you should first compile\n"
	"\tthem off-line using an Evergreen-compatible target device. Then, you have two\n"
	"\toptions two load them:\n"
	"\t  1) Replace 'clCreateProgramWithSource' calls by 'clCreateProgramWithBinary'\n"
	"\t     in your source files, referencing the pre-compiled kernel.\n"
	"\t  2) Tell Multi2Sim to provide the application with your pre-compiled kernel\n"
	"\t     using command-line option '-opencl:binary'.\n";
	
char *err_opencl_binary_note =
	"\tYou have selected a pre-compiled OpenCL kernel binary to be passed to your\n"
	"\tOpenCL application when it requests a kernel compilation. It is your\n"
	"\tresponsibility to check that the chosen binary corresponds to the kernel\n"
	"\tthat your application is expecting to load.\n";


/* Error macros */

#define OPENCL_PARAM_NOT_SUPPORTED(p) \
	fatal("%s: not supported for '" #p "' = 0x%x\n%s", err_prefix, p, err_opencl_note);
#define OPENCL_PARAM_NOT_SUPPORTED_EQ(p, v) \
	{ if ((p) == (v)) fatal("%s: not supported for '" #p "' = 0x%x\n%s", err_prefix, (v), err_opencl_param_note); }
#define OPENCL_PARAM_NOT_SUPPORTED_NEQ(p, v) \
	{ if ((p) != (v)) fatal("%s: not supported for '" #p "' != 0x%x\n%s", err_prefix, (v), err_opencl_param_note); }
#define OPENCL_PARAM_NOT_SUPPORTED_LT(p, v) \
	{ if ((p) < (v)) fatal("%s: not supported for '" #p "' < %d\n%s", err_prefix, (v), err_opencl_param_note); }
#define OPENCL_PARAM_NOT_SUPPORTED_OOR(p, min, max) \
	{ if ((p) < (min) || (p) > (max)) fatal("%s: not supported for '" #p "' out of range [%d:%d]\n%s", \
	err_prefix, (min), (max), err_opencl_param_note); }
#define OPENCL_PARAM_NOT_SUPPORTED_FLAG(p, flag, name) \
	{ if ((p) & (flag)) fatal("%s: flag '" name "' not supported\n%s", err_prefix, err_opencl_param_note); }





/* OpenCL API Implementation */

int opencl_func_run(int code, unsigned int *args)
{
	char err_prefix[MAX_STRING_SIZE];
	char *func_name;
	uint32_t opencl_success = 0;
	int func_code, func_argc;
	int retval = 0;
	
	/* Decode OpenCL function */
	assert(code >= OPENCL_FUNC_FIRST && code <= OPENCL_FUNC_LAST);
	func_code = code - OPENCL_FUNC_FIRST;
	func_name = opencl_func_names[func_code];
	func_argc = opencl_func_argc[func_code];
	assert(func_argc <= OPENCL_MAX_ARGS);
	snprintf(err_prefix, MAX_STRING_SIZE, "OpenCL call '%s'", func_name);
	
	/* Execute */
	opencl_debug("%s\n", func_name);
	switch (func_code) {

	/* 1000 */
	case OPENCL_FUNC_clGetPlatformIDs:
	{
		int num_entries = args[0];  /* cl_uint num_entries */
		uint32_t platforms = args[1];  /* cl_platform_id *platforms */
		uint32_t num_platforms = args[2];  /* cl_uint *num_platforms */
		uint32_t one = 1;
		
		opencl_debug("  num_entries=%d, platforms=0x%x, num_platforms=0x%x\n",
			num_entries, platforms, num_platforms);
		if (num_platforms)
			mem_write(isa_mem, num_platforms, 4, &one);
		if (platforms && num_entries > 0)
			mem_write(isa_mem, platforms, 4, &opencl_platform->id);
		break;
	}


	/* 1001 */
	case OPENCL_FUNC_clGetPlatformInfo:
	{
		uint32_t platform_id = args[0];  /* cl_platform_id platform */
		uint32_t param_name = args[1];  /* cl_platform_info param_name */
		uint32_t param_value_size = args[2];  /* size_t param_value_size */
		uint32_t param_value = args[3];  /* void *param_value */
		uint32_t param_value_size_ret = args[4];  /* size_t *param_value_size_ret */

		struct opencl_platform_t *platform;
		uint32_t size_ret;

		opencl_debug("  platform=0x%x, param_name=0x%x, param_value_size=0x%x,\n"
			"  param_value=0x%x, param_value_size_ret=0x%x\n",
			platform_id, param_name, param_value_size, param_value, param_value_size_ret);

		platform = opencl_object_get(OPENCL_OBJ_PLATFORM, platform_id);
		size_ret = opencl_platform_get_info(platform, param_name, isa_mem, param_value, param_value_size);
		if (param_value_size_ret)
			mem_write(isa_mem, param_value_size_ret, 4, &size_ret);
		break;
	}


	/* 1002 */
	case OPENCL_FUNC_clGetDeviceIDs:
	{
		uint32_t platform = args[0];  /* cl_platform_id platform */
		int device_type = args[1];  /* cl_device_type device_type */
		int num_entries = args[2];  /* cl_uint num_entries */
		uint32_t devices = args[3];  /* cl_device_id *devices */
		uint32_t num_devices = args[4];  /* cl_uint *num_devices */
		uint32_t one = 1;
		struct opencl_device_t *device;

		opencl_debug("  platform=0x%x, device_type=%d, num_entries=%d\n",
			platform, device_type, num_entries);
		opencl_debug("  devices=0x%x, num_devices=%x\n",
			devices, num_devices);
		if (platform != opencl_platform->id)
			fatal("%s: invalid platform\n%s", err_prefix, err_opencl_param_note);
		if (device_type != 4)  /* CL_DEVICE_TYPE_GPU */
			fatal("%s: device_type only supported for CL_DEVICE_TYPE_GPU\n%s",
				err_prefix, err_opencl_note);
		
		/* Return 1 in 'num_devices' */
		if (num_devices)
			mem_write(isa_mem, num_devices, 4, &one);

		/* Return 'id' of the only existing device */
		if (devices && num_entries > 0) {
			if (!(device = (struct opencl_device_t *) opencl_object_get_type(OPENCL_OBJ_DEVICE)))
				panic("%s: no device", err_prefix);
			mem_write(isa_mem, devices, 4, &device->id);
		}
		break;
	}


	/* 1003 */
	case OPENCL_FUNC_clGetDeviceInfo:
	{
		uint32_t device_id = args[0];  /* cl_device_id device */
		uint32_t param_name = args[1];  /* cl_device_info param_name */
		uint32_t param_value_size = args[2];  /* size_t param_value_size */
		uint32_t param_value = args[3];  /* void *param_value */
		uint32_t param_value_size_ret = args[4];  /* size_t *param_value_size_ret */

		struct opencl_device_t *device;
		uint32_t size_ret;

		opencl_debug("  device=0x%x, param_name=0x%x, param_value_size=%d\n"
			"  param_value=0x%x, param_value_size_ret=0x%x\n",
			device_id, param_name, param_value_size, param_value, param_value_size_ret);

		device = opencl_object_get(OPENCL_OBJ_DEVICE, device_id);
		size_ret = opencl_device_get_info(device, param_name, isa_mem, param_value, param_value_size);
		if (param_value_size_ret)
			mem_write(isa_mem, param_value_size_ret, 4, &size_ret);
		break;
	}


	/* 1004 */
	case OPENCL_FUNC_clCreateContext:
	{
		uint32_t properties = args[0];  /* const cl_context_properties *properties */
		uint32_t num_devices = args[1];  /* cl_uint num_devices */
		uint32_t devices = args[2];  /* const cl_device_id *devices */
		uint32_t pfn_notify = args[3];  /* void (CL_CALLBACK *pfn_notify)(const char *errinfo,
						   const void *private_info, size_t cb, void *user_data) */
		uint32_t user_data = args[4];  /* void *user_data */
		uint32_t errcode_ret = args[5];  /* cl_int *errcode_ret */

		struct opencl_device_t *device;
		uint32_t device_id;
		struct opencl_context_t *context;

		opencl_debug("  properties=0x%x, num_devices=%d, devices=0x%x\n"
			"pfn_notify=0x%x, user_data=0x%x, errcode_ret=0x%x\n",
			properties, num_devices, devices, pfn_notify, user_data, errcode_ret);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(pfn_notify, 0);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(num_devices, 1);
		OPENCL_PARAM_NOT_SUPPORTED_EQ(devices, 0);

		/* Read device id */
		mem_read(isa_mem, devices, 4, &device_id);
		device = opencl_object_get(OPENCL_OBJ_DEVICE, device_id);
		if (!device)
			fatal("%s: invalid device\n%s", err_prefix, err_opencl_param_note);

		/* Create context and return id */
		context = opencl_context_create();
		opencl_context_set_properties(context, isa_mem, properties);
		context->device_id = device_id;
		retval = context->id;

		/* Return success */
		if (errcode_ret)
			mem_write(isa_mem, errcode_ret, 4, &opencl_success);
		break;
	}


	/* 1005 */
	case OPENCL_FUNC_clCreateContextFromType:
	{
		uint32_t properties = args[0];  /* const cl_context_properties *properties */
		uint32_t device_type = args[1];  /* cl_device_type device_type */
		uint32_t pfn_notify = args[2];  /* void (*pfn_notify)(const char *, const void *, size_t , void *) */
		uint32_t user_data = args[3];  /* void *user_data */
		uint32_t errcode_ret = args[4];  /* cl_int *errcode_ret */

		struct opencl_device_t *device;
		struct opencl_context_t *context;

		opencl_debug("  properties=0x%x, device_type=0x%x, pfn_notify=0x%x,\n"
			"  user_data=0x%x, errcode_ret=0x%x\n",
			properties, device_type, pfn_notify, user_data, errcode_ret);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(pfn_notify, 0);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(device_type, 4);  /* CL_DEVICE_TYPE_GPU */

		/* Get device */
		device = (struct opencl_device_t *) opencl_object_get_type(OPENCL_OBJ_DEVICE);
		assert(device);

		/* Create context */
		context = opencl_context_create();
		context->device_id = device->id;
		opencl_context_set_properties(context, isa_mem, properties);
		retval = context->id;

		/* Return success */
		if (errcode_ret)
			mem_write(isa_mem, errcode_ret, 4, &opencl_success);

		break;
	}


	/* 1008 */
	case OPENCL_FUNC_clGetContextInfo:
	{
		uint32_t context_id = args[0];  /* cl_context context */
		uint32_t param_name = args[1];  /* cl_context_info param_name */
		uint32_t param_value_size = args[2];  /* size_t param_value_size */
		uint32_t param_value = args[3];  /* void *param_value */
		uint32_t param_value_size_ret = args[4];  /* size_t *param_value_size_ret */

		struct opencl_context_t *context;
		uint32_t size_ret = 0;

		opencl_debug("  context=0x%x, param_name=0x%x, param_value_size=0x%x,\n"
			"  param_value=0x%x, param_value_size_ret=0x%x\n",
			context_id, param_name, param_value_size, param_value, param_value_size_ret);

		context = opencl_object_get(OPENCL_OBJ_CONTEXT, context_id);
		size_ret = opencl_context_get_info(context, param_name, isa_mem, param_value, param_value_size);
		if (param_value_size_ret)
			mem_write(isa_mem, param_value_size_ret, 4, &size_ret);
		break;
	}


	/* 1009 */
	case OPENCL_FUNC_clCreateCommandQueue:
	{
		uint32_t context_id = args[0];  /* cl_context context */
		uint32_t device_id = args[1];  /* cl_device_id device */
		uint32_t properties = args[2];  /* cl_command_queue_properties properties */
		uint32_t errcode_ret = args[3];  /* cl_int *errcode_ret */

		struct opencl_context_t *context;
		struct opencl_device_t *device;
		struct opencl_command_queue_t *command_queue;

		opencl_debug("  context=0x%x, device=0x%x, properties=0x%x, errcode_ret=0x%x\n",
			context_id, device_id, properties, errcode_ret);

		/* Check that context and device are valid */
		context = opencl_object_get(OPENCL_OBJ_CONTEXT, context_id);
		device = opencl_object_get(OPENCL_OBJ_DEVICE, device_id);

		/* Create command queue and return id */
		command_queue = opencl_command_queue_create();
		command_queue->context_id = context_id;
		command_queue->device_id = device_id;
		opencl_command_queue_read_properties(command_queue, isa_mem, properties);
		retval = command_queue->id;

		/* Return success */
		if (errcode_ret)
			mem_write(isa_mem, errcode_ret, 4, &opencl_success);
		break;
	}


	/* 1014 */
	case OPENCL_FUNC_clCreateBuffer:
	{
		uint32_t context_id = args[0];  /* cl_context context */
		uint32_t flags = args[1];  /* cl_mem_flags flags */
		uint32_t size = args[2];  /* size_t size */
		uint32_t host_ptr = args[3];  /* void *host_ptr */
		uint32_t errcode_ret = args[4];  /* cl_int *errcode_ret */

		struct opencl_mem_t *mem;

		opencl_debug("  context=0x%x, flags=0x%x, size=%d, host_ptr=0x%x, errcode_ret=0x%x\n",
			context_id, flags, size, host_ptr, errcode_ret);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(host_ptr, 0);

		/* Check flags */
		OPENCL_PARAM_NOT_SUPPORTED_FLAG(flags, 0x08, "CL_MEM_USE_HOST_PTR");
		OPENCL_PARAM_NOT_SUPPORTED_FLAG(flags, 0x10, "CL_MEM_ALLOC_HOST_PTR");
		OPENCL_PARAM_NOT_SUPPORTED_FLAG(flags, 0x20, "CL_MEM_COPY_HOST_PTR");

		/* Create memory object */
		mem = opencl_mem_create();
		mem->size = size;
		mem->flags = flags;

		/* Assign position in device global memory */
		mem->device_ptr = gk->global_mem_top;
		gk->global_mem_top += size;

		/* Return memory object */
		retval = mem->id;
		if (errcode_ret)
			mem_write(isa_mem, errcode_ret, 4, &opencl_success);
		break;
	}


	/* 1028 */
	case OPENCL_FUNC_clCreateProgramWithSource:
	{
		uint32_t context_id = args[0];  /* cl_context context */
		uint32_t count = args[1];  /* cl_uint count */
		uint32_t strings = args[2];  /* const char **strings */
		uint32_t lengths = args[3];  /* const size_t *lengths */
		uint32_t errcode_ret = args[4];  /* cl_int *errcode_ret */

		struct opencl_context_t *context;
		struct opencl_program_t *program;

		opencl_debug("  context=0x%x, count=%d, strings=0x%x, lengths=0x%x, errcode_ret=0x%x\n",
			context_id, count, strings, lengths, errcode_ret);

		/* Application tries to compile source, and no binary was passed to Multi2Sim */
		if (!*gk_opencl_binary_name)
			fatal("%s: kernel source compilation not supported.\n%s",
				err_prefix, err_opencl_compiler);

		/* Create program */
		context = opencl_object_get(OPENCL_OBJ_CONTEXT, context_id);
		program = opencl_program_create();
		retval = program->id;
		warning("%s: binary '%s' used as pre-compiled kernel.\n%s",
			err_prefix, gk_opencl_binary_name, err_opencl_binary_note);

		/* Load OpenCL binary passed to Multi2Sim */
		program->binary = read_buffer(gk_opencl_binary_name, &program->binary_size);
		if (!program->binary)
			fatal("%s: cannot read from file '%s'", err_prefix, gk_opencl_binary_name);
		break;
	}


	/* 1029 */
	case OPENCL_FUNC_clCreateProgramWithBinary:
	{
		uint32_t context_id = args[0];  /* cl_context context */
		uint32_t num_devices = args[1];  /* cl_uint num_devices */
		uint32_t device_list = args[2];  /* const cl_device_id *device_list */
		uint32_t lengths = args[3];  /* const size_t *lengths */
		uint32_t binaries = args[4];  /* const unsigned char **binaries */
		uint32_t binary_status = args[5];  /* cl_int *binary_status */
		uint32_t errcode_ret = args[6];  /* cl_int *errcode_ret */

		uint32_t length, binary;
		uint32_t device_id;
		struct opencl_context_t *context;
		struct opencl_device_t *device;
		struct opencl_program_t *program;

		opencl_debug("  context=0x%x, num_devices=%d, device_list=0x%x, lengths=0x%x\n"
			"  binaries=0x%x, binary_status=0x%x, errcode_ret=0x%x\n",
			context_id, num_devices, device_list, lengths, binaries,
			binary_status, errcode_ret);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(num_devices, 1);

		/* Get device and context */
		mem_read(isa_mem, device_list, 4, &device_id);
		device = opencl_object_get(OPENCL_OBJ_DEVICE, device_id);
		context = opencl_object_get(OPENCL_OBJ_CONTEXT, context_id);

		/* Create program */
		program = opencl_program_create();
		retval = program->id;

		/* Read binary length and pointer */
		mem_read(isa_mem, lengths, 4, &length);
		mem_read(isa_mem, binaries, 4, &binary);
		opencl_debug("    lengths[0] = %d\n", length);
		opencl_debug("    binaries[0] = 0x%x\n", binary);

		/* Read binary */
		program->binary = malloc(length);
		program->binary_size = length;
		assert(program->binary);
		mem_read(isa_mem, binary, length, program->binary);

		/* Return success */
		if (binary_status)
			mem_write(isa_mem, binary_status, 4, &opencl_success);
		if (errcode_ret)
			mem_write(isa_mem, errcode_ret, 4, &opencl_success);
		break;
	}


	/* 1032 */
	case OPENCL_FUNC_clBuildProgram:
	{
		uint32_t program_id = args[0];  /* cl_program program */
		uint32_t num_devices = args[1];  /* cl_uint num_devices */
		uint32_t device_list = args[2];  /* const cl_device_id *device_list */
		uint32_t options = args[3];  /* const char *options */
		uint32_t pfn_notify = args[4];  /* void (CL_CALLBACK *pfn_notify)(cl_program program,
							void *user_data) */
		uint32_t user_data = args[5];  /* void *user_data */

		struct opencl_program_t *program;

		opencl_debug("  program=0x%x, num_devices=%d, device_list=0x%x, options=0x%x\n"
			"  pfn_notify=0x%x, user_data=0x%x\n",
			program_id, num_devices, device_list, options, pfn_notify, user_data);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(num_devices, 1);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(pfn_notify, 0);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(user_data, 0);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(options, 0);

		/* Get program */
		program = opencl_object_get(OPENCL_OBJ_PROGRAM, program_id);
		if (!program->binary)
			fatal("%s: program binary must be loaded first.\n%s",
				err_prefix, err_opencl_param_note);

		/* Build program */
		opencl_program_build(program);
		break;
	}


	/* 1036 */
	case OPENCL_FUNC_clCreateKernel:
	{
		uint32_t program_id = args[0];  /* cl_program program */
		uint32_t kernel_name = args[1];  /* const char *kernel_name */
		uint32_t errcode_ret = args[2];  /* cl_int *errcode_ret */

		char kernel_name_str[MAX_STRING_SIZE];
		struct opencl_kernel_t *kernel;
		struct opencl_program_t *program;

		opencl_debug("  program=0x%x, kernel_name=0x%x, errcode_ret=0x%x\n",
			program_id, kernel_name, errcode_ret);
		if (mem_read_string(isa_mem, kernel_name, MAX_STRING_SIZE, kernel_name_str) == MAX_STRING_SIZE)
			fatal("%s: 'kernel_name' string is too long", err_prefix);
		opencl_debug("    kernel_name='%s'\n", kernel_name_str);

		/* Get program */
		program = opencl_object_get(OPENCL_OBJ_PROGRAM, program_id);

		/* Create the kernel */
		kernel = opencl_kernel_create();
		kernel->program_id = program_id;

		/* Load kernel information by reading the '.rodata' section
		 * buffered in the 'opencl_program' object */
		opencl_kernel_load_rodata(kernel, kernel_name_str);

		/* Return kernel id */
		retval = kernel->id;
		break;
	}


	/* 1040 */
	case OPENCL_FUNC_clSetKernelArg:
	{
		uint32_t kernel_id = args[0];  /* cl_kernel kernel */
		uint32_t arg_index = args[1];  /* cl_uint arg_index */
		uint32_t arg_size = args[2];  /* size_t arg_size */
		uint32_t arg_value = args[3];  /* const void *arg_value */

		struct opencl_kernel_t *kernel;
		struct opencl_kernel_arg_t *arg;

		opencl_debug("  kernel_id=0x%x, arg_index=%d, arg_size=%d, arg_value=0x%x\n",
			kernel_id, arg_index, arg_size, arg_value);

		/* Check */
		kernel = opencl_object_get(OPENCL_OBJ_KERNEL, kernel_id);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(arg_size, 4);
		if (arg_index >= list_count(kernel->arg_list))
			fatal("%s: argument index out of bounds.\n%s", err_prefix,
				err_opencl_param_note);

		/* Copy to kernel object */
		arg = list_get(kernel->arg_list, arg_index);
		assert(arg);
		mem_read(isa_mem, arg_value, 4, &arg->value);

		/* Return success */
		break;
	}


	/* 1052 */
	case OPENCL_FUNC_clFinish:
	{
		uint32_t command_queue = args[0];  /* cl_command_queue command_queue */

		opencl_debug("  command_queue=0x%x\n", command_queue);
		/* FIXME: block until command queue empty */
		break;
	}


	/* 1053 */
	case OPENCL_FUNC_clEnqueueReadBuffer:
	{
		uint32_t command_queue = args[0];  /* cl_command_queue command_queue */
		uint32_t buffer = args[1];  /* cl_mem buffer */
		uint32_t blocking_read = args[2];  /* cl_bool blocking_read */
		uint32_t offset = args[3];  /* size_t offset */
		uint32_t cb = args[4];  /* size_t cb */
		uint32_t ptr = args[5];  /* void *ptr */
		uint32_t num_events_in_wait_list = args[6];  /* cl_uint num_events_in_wait_list */
		uint32_t event_wait_list = args[7];  /* const cl_event *event_wait_list */
		uint32_t event = args[8];  /* cl_event *event */
		
		struct opencl_mem_t *mem;
		void *buf;

		opencl_debug("  command_queue=0x%x, buffer=0x%x, blocking_read=0x%x,\n"
			"  offset=0x%x, cb=0x%x, ptr=0x%x, num_events_in_wait_list=0x%x,\n"
			"  event_wait_list=0x%x, event=0x%x\n",
			command_queue, buffer, blocking_read, offset, cb, ptr,
			num_events_in_wait_list, event_wait_list, event);

		/* FIXME: 'blocking_read' ignored */
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(num_events_in_wait_list, 0);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(event_wait_list, 0);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(event, 0);

		/* Get memory object */
		mem = opencl_object_get(OPENCL_OBJ_MEM, buffer);

		/* Check that device buffer storage is not exceeded */
		if (offset + cb > mem->size)
			fatal("%s: buffer storage exceeded\n%s", err_prefix, err_opencl_param_note);

		/* Copy buffer from device memory to host memory */
		buf = malloc(cb);
		assert(buf);
		mem_read(gk->global_mem, mem->device_ptr + offset, cb, buf);
		mem_write(isa_mem, ptr, cb, buf);
		free(buf);

		/* Return success */
		opencl_debug("    %d bytes copied from device memory (0x%x) to host memory (0x%x)\n",
			cb, mem->device_ptr + offset, ptr);
		break;
	}


	/* 1055 */
	case OPENCL_FUNC_clEnqueueWriteBuffer:
	{
		uint32_t command_queue = args[0];  /* cl_command_queue command_queue */
		uint32_t buffer = args[1];  /* cl_mem buffer */
		uint32_t blocking_write = args[2];  /* cl_bool blocking_write */
		uint32_t offset = args[3];  /* size_t offset */
		uint32_t cb = args[4];  /* size_t cb */
		uint32_t ptr = args[5];  /* const void *ptr */
		uint32_t num_events_in_wait_list = args[6];  /* cl_uint num_events_in_wait_list */
		uint32_t event_wait_list = args[7];  /* const cl_event *event_wait_list */
		uint32_t event = args[8];  /* cl_event *event */

		struct opencl_mem_t *mem;
		void *buf;

		opencl_debug("  command_queue=0x%x, buffer=0x%x, blocking_write=0x%x,\n"
			"  offset=0x%x, cb=0x%x, ptr=0x%x, num_events_in_wait_list=0x%x,\n"
			"  event_wait_list=0x%x, event=0x%x\n",
			command_queue, buffer, blocking_write, offset, cb,
			ptr, num_events_in_wait_list, event_wait_list, event);

		/* FIXME: 'blocking_write' ignored */
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(num_events_in_wait_list, 0);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(event_wait_list, 0);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(event, 0);

		/* Get memory object */
		mem = opencl_object_get(OPENCL_OBJ_MEM, buffer);

		/* Check that device buffer storage is not exceeded */
		if (offset + cb > mem->size)
			fatal("%s: buffer storage exceeded\n%s", err_prefix, err_opencl_param_note);

		/* Copy buffer from host memory to device memory */
		buf = malloc(cb);
		assert(buf);
		mem_read(isa_mem, ptr, cb, buf);
		mem_write(gk->global_mem, mem->device_ptr + offset, cb, buf);
		free(buf);

		/* Return success */
		opencl_debug("    %d bytes copied from host memory (0x%x) to device memory (0x%x)\n",
			cb, ptr, mem->device_ptr + offset);
		break;
	}


	/* 1064 */
	case OPENCL_FUNC_clEnqueueMapBuffer:
	{
		uint32_t command_queue = args[0];  /* cl_command_queue command_queue */
		uint32_t buffer = args[1];  /* cl_mem buffer */
		uint32_t blocking_map = args[2];  /* cl_bool blocking_map */
		uint32_t map_flags = args[3];  /* cl_map_flags map_flags */
		uint32_t offset = args[4];  /* size_t offset */
		uint32_t cb = args[5];  /* size_t cb */
		uint32_t num_events_in_wait_list = args[6];  /* cl_uint num_events_in_wait_list */
		uint32_t event_wait_list = args[7];  /* const cl_event *event_wait_list */
		uint32_t event = args[8];  /* cl_event *event */
		uint32_t errcode_ret = args[9];  /* cl_int *errcode_ret */

		struct opencl_mem_t *mem;

		opencl_debug("  command_queue=0x%x, buffer=0x%x, blocking_map=0x%x, map_flags=0x%x,\n"
			"  offset=0x%x, cb=0x%x, num_events_in_wait_list=0x%x, event_wait_list=0x%x,\n"
			"  event=0x%x, errcode_ret=0x%x\n",
			command_queue, buffer, blocking_map, map_flags, offset, cb,
			num_events_in_wait_list, event_wait_list, event, errcode_ret);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(num_events_in_wait_list, 0);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(event_wait_list, 0);
		OPENCL_PARAM_NOT_SUPPORTED_EQ(blocking_map, 0);

		/* Get memory object */
		mem = opencl_object_get(OPENCL_OBJ_MEM, buffer);

		/* Return success */
		if (errcode_ret)
			mem_write(isa_mem, errcode_ret, 4, &opencl_success);
		fatal("clEnqueueMapBuffer: not implemented");
		break;
	}


	/* 1067 */
	case OPENCL_FUNC_clEnqueueNDRangeKernel:
	{
		uint32_t command_queue = args[0];  /* cl_command_queue command_queue */
		uint32_t kernel_id = args[1];  /* cl_kernel kernel */
		uint32_t work_dim = args[2];  /* cl_uint work_dim */
		uint32_t global_work_offset_ptr = args[3];  /* const size_t *global_work_offset */
		uint32_t global_work_size_ptr = args[4];  /* const size_t *global_work_size */
		uint32_t local_work_size_ptr = args[5];  /* const size_t *local_work_size */
		uint32_t num_events_in_wait_list = args[6];  /* cl_uint num_events_in_wait_list */
		uint32_t event_wait_list = args[7];  /* const cl_event *event_wait_list */
		uint32_t event = args[8];  /* cl_event *event */

		struct opencl_kernel_t *kernel;
		int i;

		opencl_debug("  command_queue=0x%x, kernel=0x%x, work_dim=%d,\n"
			"  global_work_offset=0x%x, global_work_size_ptr=0x%x, local_work_size_ptr=0x%x,\n"
			"  num_events_in_wait_list=0x%x, event_wait_list=0x%x, event=0x%x\n",
			command_queue, kernel_id, work_dim, global_work_offset_ptr, global_work_size_ptr,
			local_work_size_ptr, num_events_in_wait_list, event_wait_list, event);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(num_events_in_wait_list, 0);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(event_wait_list, 0);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(event, 0);
		OPENCL_PARAM_NOT_SUPPORTED_NEQ(global_work_offset_ptr, 0);
		OPENCL_PARAM_NOT_SUPPORTED_OOR(work_dim, 1, 3);

		/* Get kernel */
		kernel = opencl_object_get(OPENCL_OBJ_KERNEL, kernel_id);
		kernel->work_dim = work_dim;

		/* Global work sizes */
		kernel->global_size3[1] = 1;
		kernel->global_size3[2] = 1;
		for (i = 0; i < work_dim; i++)
			mem_read(isa_mem, global_work_size_ptr + i * 4, 4, &kernel->global_size3[i]);
		kernel->global_size = kernel->global_size3[0] * kernel->global_size3[1] * kernel->global_size3[2];
		opencl_debug("    global_work_size=");
		opencl_debug_array(work_dim, kernel->global_size3);
		opencl_debug("\n");

		/* Local work sizes.
		 * If no pointer provided, assign the same as global size - FIXME: can be done better. */
		memcpy(kernel->local_size3, kernel->global_size3, 12);
		if (local_work_size_ptr)
			for (i = 0; i < work_dim; i++)
				mem_read(isa_mem, local_work_size_ptr + i * 4, 4, &kernel->local_size3[i]);
		kernel->local_size = kernel->local_size3[0] * kernel->local_size3[1] * kernel->local_size3[2];
		opencl_debug("    local_work_size=");
		opencl_debug_array(work_dim, kernel->local_size3);
		opencl_debug("\n");

		/* Calculate number of groups */
		for (i = 0; i < 3; i++)
			kernel->group_count3[i] = (kernel->global_size3[i] + kernel->local_size3[i] - 1)
				/ kernel->local_size3[i];
		kernel->group_count = kernel->group_count3[0] * kernel->group_count3[1] * kernel->group_count3[2];
		opencl_debug("    group_count=");
		opencl_debug_array(work_dim, kernel->group_count3);
		opencl_debug("\n");

		/* FIXME: enqueue kernel execution in command queue */
		gpu_isa_run(kernel);
		break;
	}



	default:
		
		fatal("opencl_func_run: function '%s' (code=%d) not implemented.\n%s",
			func_name, code, err_opencl_note);
	}

	return retval;
}

