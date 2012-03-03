/*
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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


#include <mem-system.h>
#include <config.h>
#include <misc.h>
#include <cpuarch.h>
#include <gpuarch.h>


/*
 * Global Variables
 */

char *mem_config_file_name = "";
char *mem_report_file_name = "";

char *mem_config_help = "FIXME\n";
#if 0
	"The GPU memory hierarchy can be configured by using an IniFile formatted file,\n"
	"describing the cache and interconnect properties. This file is passed to\n"
	"Multi2Sim with option '--gpu-cache-config <file>', and should be always used\n"
	"together with option '--gpu-sim detailed' for an architectural GPU simulation.\n"
	"\n"
	"The sections and variables allowed in the cache configuration file are the\n"
	"following:\n"
	"\n"
	"Section '[ CacheGeometry <name> ]': defines a geometry for a cache. Caches\n"
	"using this geometry can be instantiated later.\n"
	"\n"
	"  Sets = <num_sets> (Required)\n"
	"      Number of sets in the cache.\n"
	"  Assoc = <num_ways> (Required)\n"
	"      Cache associativity. The total number of blocks contained in the cache\n"
	"      is given by the product Sets * Assoc.\n"
	"  BlockSize = <size> (Required)\n"
	"      Size of a cache block in bytes. The total size of the cache is given by\n"
	"      the product Sets * Assoc * BlockSize.\n"
	"  Latency = <num_cycles> (Required)\n"
	"      Hit latency for a cache in number of CPU cycles.\n"
	"  Policy = {LRU|FIFO|Random} (Default = LRU)\n"
	"      Block replacement policy.\n"
	"  Banks = <num> (Default = 1)\n"
	"      Number of banks.\n"
	"  ReadPorts = <num> (Default = 2)\n"
	"      Number of read ports per bank.\n"
	"  WritePorts = <num> (Default = 2)\n"
	"      Number of write ports per bank.\n"
	"\n"
	"Section '[ Net <name> ]': defines an interconnection network.\n"
	"\n"
	"  UpperLinkWidth = <width> (Default = 8)\n"
	"      Bandwidth in bytes per cycle for bidirectional links connecting upper\n"
	"      level caches with upper input/output buffers of a switch.\n"
	"  LowerLinkWidth = <width> (Default = 8)\n"
	"      Bandwidth in bytes per cycle for the bidirectional link connecting\n"
	"      the switch with the lower-level cache or global memory.\n"
	"  InnerLinkWidth = <width> (Default = 8)\n"
	"      Bandwidth in bytes per cycle for the links connecting input/output\n"
	"      buffers of a switch with the internal crossbar.\n"
	"  InputBufferSize = <bytes> (Default = (B + 8) * 2)\n"
	"      Size of the switch input buffers. This size must be at least B + 8,\n"
	"      being B the block size of the lower level cache (or memory), and 8\n"
	"      the metadata size attached to block transfers. By default, the\n"
	"      input buffer size can hold two maximum-length packages.\n"
	"  OutputBufferSize = <bytes> (Default = (B + 8) * 2)\n"
	"      Size of the switch output buffers. It must be at least the maximum\n"
	"      package size, and it defaults to twice this size.\n"
	"\n"
	"Section '[ Cache <name> ]': instantiates a cache based on a cache geometry\n"
	"defined in a section CacheGeometry.\n"
	"\n"
	"  Geometry = <geometry_name> (Required)\n"
	"      Cache geometry identifier, as specified in a previous section of type\n"
	"      '[ CacheGeometry <geometry_name> ]'.\n"
	"  HiNet = <net_name> (Required only for non-L1 caches)\n"
	"      Upper interconnect where the cache is connected to. <net_name> must\n"
	"      correspond to a network declared in a previous section of type\n"
	"      '[ Net <net_name> ]'.\n"
	"  LoNet = <net_name> (Required)\n"
	"      Lower interconnect where the cache is connected to.\n"
	"\n"
	"Section '[ GlobalMemory ]'\n"
	"\n"
	"  HiNet = <net_name> (Required if there are caches)\n"
	"      Upper interconnect where global memory is connected to.\n"
	"  BlockSize = <size> (Required)\n"
	"      Memory block size in bytes.\n"
	"  Latency = <num_cycles> (Required)\n"
	"      Main memory access latency.\n"
	"  Banks = <num> (Default = 8)\n"
	"      Number of banks.\n"
	"  ReadPorts = <num> (Default = 2)\n"
	"      Number of read ports per bank.\n"
	"  WritePorts = <num> (Default = 2)\n"
	"      Number of write ports per bank.\n"
	"\n"
	"Section '[ Node <name> ]': defines an entry to the memory hierarchy from a\n"
	"compute unit in the GPU into an L1 cache or global memory.\n"
	"\n"
	"  ComputeUnit = <id>\n"
	"      Identifier of a compute unit. This must be an integer between 0 and the\n"
	"      number of compute units minus 1.\n"
	"  DataCache = <cache_name> (Required)\n"
	"      Name of the cache accessed when the compute unit reads/writes data.\n"
	"      The cache must be declared in a previous section of type\n"
	"      '[ Cache <cache_name> ]'.\n"
	"\n";
#endif




/*
 * Private functions
 */

#define MEM_SYSTEM_MAX_LEVELS  10

static char *err_mem_config_note =
	"\tPlease run 'm2s --help-mem-config' or consult the Multi2Sim Guide for\n"
	"\ta description of the memory system configuration file format.\n";

static char *err_mem_config_net =
	"\tNetwork identifiers need to be declared either in the cache configuration\n"
	"\tfile, or in the network configuration file (option '--net-config').\n";

static char *err_mem_levels =
	"\tThe path from a cache into main memory exceeds 10 levels of cache.\n"
	"\tThis might be a symptom of a recursive reference in 'LowModules'\n"
	"\tlists. If you really intend to have a high number of cache levels,\n"
	"\tincrease variable MEM_SYSTEM_MAX_LEVELS in '" __FILE__ "'\n";

static char *err_mem_block_size =
	"\tBlock size in a cache must be greater or equal than its lower-level\n"
	"\tcache for correct behavior of directories and coherence protocols.\n";

static char *err_mem_ignored_entry =
	"\tThis entry in the file will be ignored, because it refers to a\n"
	"\tnon-existent CPU core/thread or GPU compute unit.\n";

static char *err_mem_connect =
	"\tAn external network is used that does not provide connectivity between\n"
	"\ta memory module and an associated low/high module. Please add the\n"
	"\tnecessary links in the network configuration file.\n";


static void mem_config_cpu_default(struct config_t *config)
{
	char section[MAX_STRING_SIZE];
	char str[MAX_STRING_SIZE];

	int core;
	int thread;

	/* Not if we are doing CPU functional simulation */
	if (cpu_sim_kind == cpu_sim_kind_functional)
		return;

	/* Cache geometry for L1 */
	strcpy(section, "CacheGeometry cpu-geo-l1");
	config_write_int(config, section, "Sets", 16);
	config_write_int(config, section, "Assoc", 2);
	config_write_int(config, section, "BlockSize", 64);
	config_write_int(config, section, "Latency", 1);
	config_write_string(config, section, "Policy", "LRU");
	config_write_int(config, section, "Banks", 1);
	config_write_int(config, section, "ReadPorts", 2);
	config_write_int(config, section, "WritePorts", 2);

	/* Cache geometry for L2 */
	strcpy(section, "CacheGeometry cpu-geo-l2");
	config_write_int(config, section, "Sets", 64);
	config_write_int(config, section, "Assoc", 4);
	config_write_int(config, section, "BlockSize", 64);
	config_write_int(config, section, "Latency", 10);
	config_write_string(config, section, "Policy", "LRU");
	config_write_int(config, section, "Banks", 4);
	config_write_int(config, section, "ReadPorts", 2);
	config_write_int(config, section, "WritePorts", 2);

	/* L1 caches and entries */
	FOREACH_CORE
	{
		/* L1 cache */
		snprintf(section, sizeof section, "Module cpu-l1-%d", core);
		config_write_string(config, section, "Type", "Cache");
		config_write_string(config, section, "Geometry", "cpu-geo-l1");
		config_write_string(config, section, "LowNetwork", "cpu-net-l1-l2");
		config_write_string(config, section, "LowModules", "cpu-l2");

		/* Entry */
		snprintf(str, sizeof str, "cpu-l1-%d", core);
		FOREACH_THREAD
		{
			snprintf(section, sizeof section, "Entry cpu-core-%d-thread-%d",
				core, thread);
			config_write_string(config, section, "Type", "CPU");
			config_write_int(config, section, "Core", core);
			config_write_int(config, section, "Thread", thread);
			config_write_string(config, section, "DataModule", str);
			config_write_string(config, section, "InstModule", str);
		}
	}

	/* L2 cache */
	snprintf(section, sizeof section, "Module cpu-l2");
	config_write_string(config, section, "Type", "Cache");
	config_write_string(config, section, "Geometry", "cpu-geo-l2");
	config_write_string(config, section, "HighNetwork", "cpu-net-l1-l2");
	config_write_string(config, section, "LowNetwork", "cpu-net-l2-mm");
	config_write_string(config, section, "LowModules", "cpu-mm");

	/* Main memory */
	snprintf(section, sizeof section, "Module cpu-mm");
	config_write_string(config, section, "Type", "MainMemory");
	config_write_string(config, section, "HighNetwork", "cpu-net-l2-mm");
	config_write_int(config, section, "BlockSize", 64);
	config_write_int(config, section, "Latency", 100);
	config_write_int(config, section, "Banks", 8);
	config_write_int(config, section, "ReadPorts", 2);
	config_write_int(config, section, "WritePorts", 2);

	/* Network connecting L1 caches and L2 */
	snprintf(section, sizeof section, "Network cpu-net-l1-l2");
	config_write_int(config, section, "DefaultInputBufferSize", 144);
	config_write_int(config, section, "DefaultOutputBufferSize", 144);
	config_write_int(config, section, "DefaultBandwidth", 72);

	/* Network connecting L2 cache and global memory */
	snprintf(section, sizeof section, "Network cpu-net-l2-mm");
	config_write_int(config, section, "DefaultInputBufferSize", 528);
	config_write_int(config, section, "DefaultOutputBufferSize", 528);
	config_write_int(config, section, "DefaultBandwidth", 264);
}


static void mem_config_gpu_default(struct config_t *config)
{
	char section[MAX_STRING_SIZE];
	char str[MAX_STRING_SIZE];

	int compute_unit_id;

	/* Not if we doing GPU functional simulation */
	if (gpu_sim_kind == gpu_sim_kind_functional)
		return;

	/* Cache geometry for L1 */
	strcpy(section, "CacheGeometry gpu-geo-l1");
	config_write_int(config, section, "Sets", 16);
	config_write_int(config, section, "Assoc", 2);
	config_write_int(config, section, "BlockSize", 256);
	config_write_int(config, section, "Latency", 1);
	config_write_string(config, section, "Policy", "LRU");
	config_write_int(config, section, "Banks", 1);
	config_write_int(config, section, "ReadPorts", 2);
	config_write_int(config, section, "WritePorts", 2);

	/* Cache geometry for L2 */
	strcpy(section, "CacheGeometry gpu-geo-l2");
	config_write_int(config, section, "Sets", 64);
	config_write_int(config, section, "Assoc", 4);
	config_write_int(config, section, "BlockSize", 256);
	config_write_int(config, section, "Latency", 10);
	config_write_string(config, section, "Policy", "LRU");
	config_write_int(config, section, "Banks", 4);
	config_write_int(config, section, "ReadPorts", 2);
	config_write_int(config, section, "WritePorts", 2);

	/* L1 caches and entries */
	FOREACH_COMPUTE_UNIT(compute_unit_id)
	{
		snprintf(section, sizeof section, "Module gpu-l1-%d", compute_unit_id);
		config_write_string(config, section, "Type", "Cache");
		config_write_string(config, section, "Geometry", "gpu-geo-l1");
		config_write_string(config, section, "LowNetwork", "gpu-net-l1-l2");
		config_write_string(config, section, "LowModules", "gpu-l2");

		snprintf(section, sizeof section, "Entry gpu-cu-%d", compute_unit_id);
		snprintf(str, sizeof str, "gpu-l1-%d", compute_unit_id);
		config_write_string(config, section, "Type", "GPU");
		config_write_int(config, section, "ComputeUnit", compute_unit_id);
		config_write_string(config, section, "Module", str);
	}

	/* L2 cache */
	snprintf(section, sizeof section, "Module gpu-l2");
	config_write_string(config, section, "Type", "Cache");
	config_write_string(config, section, "Geometry", "gpu-geo-l2");
	config_write_string(config, section, "HighNetwork", "gpu-net-l1-l2");
	config_write_string(config, section, "LowNetwork", "gpu-net-l2-gm");
	config_write_string(config, section, "LowModules", "gpu-gm");

	/* Global memory */
	snprintf(section, sizeof section, "Module gpu-gm");
	config_write_string(config, section, "Type", "MainMemory");
	config_write_string(config, section, "HighNetwork", "gpu-net-l2-gm");
	config_write_int(config, section, "BlockSize", 256);
	config_write_int(config, section, "Latency", 100);
	config_write_int(config, section, "Banks", 8);
	config_write_int(config, section, "ReadPorts", 2);
	config_write_int(config, section, "WritePorts", 2);

	/* Network connecting L1 caches and L2 */
	snprintf(section, sizeof section, "Network gpu-net-l1-l2");
	config_write_int(config, section, "DefaultInputBufferSize", 528);
	config_write_int(config, section, "DefaultOutputBufferSize", 528);
	config_write_int(config, section, "DefaultBandwidth", 264);

	/* Network connecting L2 cache and global memory */
	snprintf(section, sizeof section, "Network gpu-net-l2-gm");
	config_write_int(config, section, "DefaultInputBufferSize", 528);
	config_write_int(config, section, "DefaultOutputBufferSize", 528);
	config_write_int(config, section, "DefaultBandwidth", 264);
}


static void mem_config_read_networks(struct config_t *config)
{
	struct net_t *net;
	int i;

	char buf[MAX_STRING_SIZE];
	char *section;

	/* Create networks */
	mem_debug("Creating internal networks:\n");
	for (section = config_section_first(config); section; section = config_section_next(config))
	{
		char *net_name;

		/* Network section */
		if (strncasecmp(section, "Network ", 8))
			continue;
		net_name = section + 8;

		/* Create network */
		net = net_create(net_name);
		mem_debug("\t%s\n", net_name);
		list_add(mem_system->net_list, net);
	}
	mem_debug("\n");

	/* Add network pointers to configuration file. This needs to be done separately,
	 * because configuration file writes alter enumeration of sections. */
	for (i = 0; i < list_count(mem_system->net_list); i++)
	{
		net = list_get(mem_system->net_list, i);
		snprintf(buf, sizeof buf, "Network %s", net->name);
		assert(config_section_exists(config, buf));
		config_write_ptr(config, buf, "ptr", net);
	}

}


static void mem_config_insert_module_in_network(struct config_t *config,
	struct mod_t *mod, char *net_name, char *net_node_name,
	struct net_t **net_ptr, struct net_node_t **net_node_ptr)
{
	struct net_t *net;
	struct net_node_t *node;

	int def_input_buffer_size;
	int def_output_buffer_size;

	char buf[MAX_STRING_SIZE];

	/* No network specified */
	*net_ptr = NULL;
	*net_node_ptr = NULL;
	if (!*net_name)
		return;

	/* Try to insert in private network */
	snprintf(buf, sizeof buf, "Network %s", net_name);
	net = config_read_ptr(config, buf, "ptr", NULL);
	if (!net)
		goto try_external_network;

	/* For private networks, 'net_node_name' should be empty */
	if (*net_node_name)
		fatal("%s: %s: network node name should be empty.\n%s",
			mem_config_file_name, mod->name,
			err_mem_config_note);

	/* Network should not have this module already */
	if (net_get_node_by_user_data(net, mod))
		fatal("%s: network '%s' already contains module '%s'.\n%s",
			mem_config_file_name, net->name,
			mod->name, err_mem_config_note);

	/* Read buffer sizes from network */
	def_input_buffer_size = config_read_int(config, buf, "DefaultInputBufferSize", 0);
	def_output_buffer_size = config_read_int(config, buf, "DefaultOutputBufferSize", 0);
	if (!def_input_buffer_size)
		fatal("%s: network %s: variable 'DefaultInputBufferSize' missing.\n%s",
			mem_config_file_name, net->name, err_mem_config_note);
	if (!def_output_buffer_size)
		fatal("%s: network %s: variable 'DefaultOutputBufferSize' missing.\n%s",
			mem_config_file_name, net->name, err_mem_config_note);
	if (def_input_buffer_size < mod->block_size + 8)
		fatal("%s: network %s: minimum input buffer size is %d for cache '%s'.\n%s",
			mem_config_file_name, net->name, mod->block_size + 8,
			mod->name, err_mem_config_note);
	if (def_output_buffer_size < mod->block_size + 8)
		fatal("%s: network %s: minimum output buffer size is %d for cache '%s'.\n%s",
			mem_config_file_name, net->name, mod->block_size + 8,
			mod->name, err_mem_config_note);

	/* Insert module in network */
	node = net_add_end_node(net, def_input_buffer_size, def_output_buffer_size,
		mod->name, mod);

	/* Return */
	*net_ptr = net;
	*net_node_ptr = node;
	return;


try_external_network:

	/* Get network */
	net = net_find(net_name);
	if (!net)
		fatal("%s: %s: invalid network name.\n%s%s",
			mem_config_file_name, net_name,
			err_mem_config_note, err_mem_config_net);

	/* Node name must be specified */
	if (!*net_node_name)
		fatal("%s: %s: network node name required for external network.\n%s%s",
			mem_config_file_name, mod->name,
			err_mem_config_note, err_mem_config_net);

	/* Get node */
	node = net_get_node_by_name(net, net_node_name);
	if (!node)
		fatal("%s: network %s: node %s: invalid node name.\n%s%s",
			mem_config_file_name, net_name, net_node_name,
			err_mem_config_note, err_mem_config_net);

	/* No module must have been assigned previously to this node */
	if (node->user_data)
		fatal("%s: network %s: node '%s' already assigned.\n%s",
			mem_config_file_name, net->name,
			net_node_name, err_mem_config_note);

	/* Network should not have this module already */
	if (net_get_node_by_user_data(net, mod))
		fatal("%s: network %s: module '%s' is already present.\n%s",
			mem_config_file_name, net->name,
			mod->name, err_mem_config_note);

	/* Assign module to network node and return */
	node->user_data = mod;
	*net_ptr = net;
	*net_node_ptr = node;
}


static struct mod_t *mem_config_read_cache(struct config_t *config, char *section)
{
	char buf[MAX_STRING_SIZE];
	char mod_name[MAX_STRING_SIZE];

	int num_sets;
	int assoc;
	int block_size;
	int latency;

	char *policy_str;
	enum cache_policy_t policy;

	int bank_count;
	int read_port_count;
	int write_port_count;

	char *net_name;
	char *net_node_name;

	struct mod_t *mod;
	struct net_t *net;
	struct net_node_t *net_node;

	/* Cache parameters */
	snprintf(buf, sizeof buf, "CacheGeometry %s",
		config_read_string(config, section, "Geometry", ""));
	config_var_enforce(config, section, "Geometry");
	config_section_enforce(config, buf);
	config_var_enforce(config, buf, "Latency");
	config_var_enforce(config, buf, "Sets");
	config_var_enforce(config, buf, "Assoc");
	config_var_enforce(config, buf, "BlockSize");

	/* Read values */
	str_token(mod_name, sizeof mod_name, section, 1, " ");
	num_sets = config_read_int(config, buf, "Sets", 16);
	assoc = config_read_int(config, buf, "Assoc", 2);
	block_size = config_read_int(config, buf, "BlockSize", 256);
	latency = config_read_int(config, buf, "Latency", 1);
	policy_str = config_read_string(config, buf, "Policy", "LRU");
	bank_count = config_read_int(config, buf, "Banks", 1);
	read_port_count = config_read_int(config, buf, "ReadPorts", 2);
	write_port_count = config_read_int(config, buf, "WritePorts", 1);

	/* Checks */
	policy = map_string_case(&cache_policy_map, policy_str);
	if (policy == cache_policy_invalid)
		fatal("%s: cache '%s': %s: invalid block replacement policy.\n%s",
			mem_config_file_name, mod_name,
			policy_str, err_mem_config_note);
	if (num_sets < 1 || (num_sets & (num_sets - 1)))
		fatal("%s: cache '%s': number of sets must be a power of two greater than 1.\n%s",
			mem_config_file_name, mod_name, err_mem_config_note);
	if (assoc < 1 || (assoc & (assoc - 1)))
		fatal("%s: cache '%s': associativity must be power of two and > 1.\n%s",
			mem_config_file_name, mod_name, err_mem_config_note);
	if (block_size < 4 || (block_size & (block_size - 1)))
		fatal("%s: cache '%s': block size must be power of two and at least 4.\n%s",
			mem_config_file_name, mod_name, err_mem_config_note);
	if (latency < 1)
		fatal("%s: cache '%s': invalid value for variable 'Latency'.\n%s",
			mem_config_file_name, mod_name, err_mem_config_note);
	if (bank_count < 1 || (bank_count & (bank_count - 1)))
		fatal("%s: cache '%s': number of banks must be a power of two greater than 1.\n%s",
			mem_config_file_name, mod_name, err_mem_config_note);
	if (read_port_count < 1)
		fatal("%s: cache '%s': invalid value for variable 'ReadPorts'.\n%s",
			mem_config_file_name, mod_name, err_mem_config_note);
	if (write_port_count < 1)
		fatal("%s: cache '%s': invalid value for variable 'WritePorts'.\n%s",
			mem_config_file_name, mod_name, err_mem_config_note);

	/* Create module */
	mod = mod_create(mod_name, mod_kind_cache,
		bank_count, read_port_count, write_port_count,
		block_size, latency);

	/* Store directory size */
	mod->dir_assoc = assoc;
	mod->dir_num_sets = num_sets;
	mod->dir_size = num_sets * assoc;

	/* High network */
	net_name = config_read_string(config, section, "HighNetwork", "");
	net_node_name = config_read_string(config, section, "HighNetworkNode", "");
	mem_config_insert_module_in_network(config, mod, net_name, net_node_name,
		&net, &net_node);
	mod->high_net = net;
	mod->high_net_node = net_node;

	/* Low network */
	net_name = config_read_string(config, section, "LowNetwork", "");
	net_node_name = config_read_string(config, section, "LowNetworkNode", "");
	mem_config_insert_module_in_network(config, mod, net_name, net_node_name,
		&net, &net_node);
	mod->low_net = net;
	mod->low_net_node = net_node;

	/* Create cache */
	mod->cache = cache_create(num_sets, block_size, assoc, policy);

	/* Return */
	return mod;
}


static struct mod_t *mem_config_read_main_memory(struct config_t *config, char *section)
{
	char mod_name[MAX_STRING_SIZE];

	int block_size;
	int latency;
	int bank_count;
	int read_port_count;
	int write_port_count;
	int dir_size;
	int dir_assoc;

	char *net_name;
	char *net_node_name;

	struct mod_t *mod;
	struct net_t *net;
	struct net_node_t *net_node;

	/* Read parameters */
	str_token(mod_name, sizeof mod_name, section, 1, " ");
	config_var_enforce(config, section, "Latency");
	config_var_enforce(config, section, "BlockSize");
	block_size = config_read_int(config, section, "BlockSize", 64);
	latency = config_read_int(config, section, "Latency", 1);
	bank_count = config_read_int(config, section, "Banks", 4);
	read_port_count = config_read_int(config, section, "ReadPorts", 2);
	write_port_count = config_read_int(config, section, "WritePorts", 2);
	dir_size = config_read_int(config, section, "DirectorySize", 1024);
	dir_assoc = config_read_int(config, section, "DirectoryAssoc", 8);

	/* Check parameters */
	if (block_size < 1 || (block_size & (block_size - 1)))
		fatal("%s: global memory: block size must be power of two and > 1.\n%s",
			mem_config_file_name, err_mem_config_note);
	if (latency < 1)
		fatal("%s: global memory: invalid value for variable 'Latency'.\n%s",
			mem_config_file_name, err_mem_config_note);
	if (bank_count < 1 || (bank_count & (bank_count - 1)))
		fatal("%s: global_memory: number of banks must be a power of two greater than 1.\n%s",
			mem_config_file_name, err_mem_config_note);
	if (read_port_count < 1)
		fatal("%s: global memory: invalid value for variable 'ReadPorts'.\n%s",
			mem_config_file_name, err_mem_config_note);
	if (write_port_count < 1)
		fatal("%s: global memory: invalid value for variable 'WritePorts'.\n%s",
			mem_config_file_name, err_mem_config_note);
	if (dir_size < 1 || (dir_size & (dir_size - 1)))
		fatal("%s: directory size must be a power of two.\n%s",
			mem_config_file_name, err_mem_config_note);
	if (dir_assoc < 1 || (dir_assoc & (dir_assoc - 1)))
		fatal("%s: directory associativity must be a power of two.\n%s",
			mem_config_file_name, err_mem_config_note);
	if (dir_assoc > dir_size)
		fatal("%s: invalid directory associativity.\n%s",
			mem_config_file_name, err_mem_config_note);

	/* Create module */
	mod = mod_create(mod_name, mod_kind_main_memory,
			bank_count, read_port_count, write_port_count,
			block_size, latency);

	/* Store directory size */
	mod->dir_size = dir_size;
	mod->dir_assoc = dir_assoc;
	mod->dir_num_sets = dir_size / dir_assoc;

	/* High network */
	net_name = config_read_string(config, section, "HighNetwork", "");
	net_node_name = config_read_string(config, section, "HighNetworkNode", "");
	mem_config_insert_module_in_network(config, mod, net_name, net_node_name,
			&net, &net_node);
	mod->high_net = net;
	mod->high_net_node = net_node;

	/* Create cache and directory */
	mod->cache = cache_create(dir_size / dir_assoc, block_size, dir_assoc, cache_policy_lru);

	/* Return */
	return mod;
}


static void mem_config_read_module_address_range(struct config_t *config,
	struct mod_t *mod, char *section)
{
	char *range_str;
	char *token;
	char *delim;

	/* Read address range */
	range_str = config_read_string(config, section, "AddressRange", "");
	if (!*range_str)
	{
		mod->range_kind = mod_range_bounds;
		mod->range.bounds.low = 0;
		mod->range.bounds.high = -1;
		return;
	}

	/* Duplicate string */
	range_str = strdup(range_str);
	if (!range_str)
		fatal("%s: out of memory", __FUNCTION__);

	/* Split in tokens */
	delim = " ";
	token = strtok(range_str, delim);
	if (!token)
		goto invalid_format;

	/* First token - ADDR or BOUNDS */
	if (!strcasecmp(token, "BOUNDS"))
	{
		/* Format is: BOUNDS <low> <high> */
		mod->range_kind = mod_range_bounds;

		/* Low bound */
		if (!(token = strtok(NULL, delim)))
			goto invalid_format;
		mod->range.bounds.low = str_to_int(token);

		/* High bound */
		if (!(token = strtok(NULL, delim)))
			goto invalid_format;
		mod->range.bounds.high = str_to_int(token);

		/* No more tokens */
		if ((token = strtok(NULL, delim)))
			goto invalid_format;
	}
	else if (!strcasecmp(token, "ADDR"))
	{
		/* Format is: ADDR DIV <div> MOD <mod> EQ <eq> */
		mod->range_kind = mod_range_interleaved;

		/* Token 'DIV' */
		if (!(token = strtok(NULL, delim)) || strcasecmp(token, "DIV"))
			goto invalid_format;

		/* Field <div> */
		if (!(token = strtok(NULL, delim)))
			goto invalid_format;
		mod->range.interleaved.div = str_to_int(token);
		if (mod->range.interleaved.div < 1)
			goto invalid_format;

		/* Token 'MOD' */
		if (!(token = strtok(NULL, delim)) || strcasecmp(token, "MOD"))
			goto invalid_format;

		/* Field <mod> */
		if (!(token = strtok(NULL, delim)))
			goto invalid_format;
		mod->range.interleaved.mod = str_to_int(token);
		if (mod->range.interleaved.mod < 1)
			goto invalid_format;

		/* Token 'EQ' */
		if (!(token = strtok(NULL, delim)) || strcasecmp(token, "EQ"))
			goto invalid_format;

		/* Field <eq> */
		if (!(token = strtok(NULL, delim)))
			goto invalid_format;
		mod->range.interleaved.eq = str_to_int(token);
		if (mod->range.interleaved.eq >= mod->range.interleaved.mod)
			goto invalid_format;

		/* No more tokens */
		if ((token = strtok(NULL, delim)))
			goto invalid_format;
	}
	else
	{
		goto invalid_format;
	}

	/* Free string duplicate */
	free(range_str);
	return;

invalid_format:

	fatal("%s: %s: invalid format for 'AddressRange'.\n%s",
		mem_config_file_name, mod->name, err_mem_config_note);
}


static void mem_config_read_modules(struct config_t *config)
{
	struct mod_t *mod;

	char *section;
	char *mod_type;

	char buf[MAX_STRING_SIZE];
	char mod_name[MAX_STRING_SIZE];
	int i;

	/* Create modules */
	mem_debug("Creating modules:\n");
	for (section = config_section_first(config); section; section = config_section_next(config))
	{
		/* Section for a module */
		if (strncasecmp(section, "Module ", 7))
			continue;

		/* Create module, depending on the type. */
		str_token(mod_name, sizeof mod_name, section, 1, " ");
		mod_type = config_read_string(config, section, "Type", "");
		if (!strcasecmp(mod_type, "Cache"))
			mod = mem_config_read_cache(config, section);
		else if (!strcasecmp(mod_type, "MainMemory"))
			mod = mem_config_read_main_memory(config, section);
		else
			fatal("%s: %s: invalid or missing value for 'Type'.\n%s",
				mem_config_file_name, mod_name,
				err_mem_config_note);

		/* Read module address range */
		mem_config_read_module_address_range(config, mod, section);

		/* Add module */
		list_add(mem_system->mod_list, mod);
		mem_debug("\t%s\n", mod_name);
	}

	/* Debug */
	mem_debug("\n");

	/* Add module pointers to configuration file. This needs to be done separately,
	 * because configuration file writes alter enumeration of sections. */
	for (i = 0; i < list_count(mem_system->mod_list); i++)
	{
		mod = list_get(mem_system->mod_list, i);
		snprintf(buf, sizeof buf, "Module %s", mod->name);
		assert(config_section_exists(config, buf));
		config_write_ptr(config, buf, "ptr", mod);
	}
}


static void mem_config_check_route_to_main_memory(struct mod_t *mod, int block_size, int level)
{
	struct mod_t *low_mod;
	int i;

	/* Maximum level */
	if (level > MEM_SYSTEM_MAX_LEVELS)
		fatal("%s: %s: too many cache levels.\n%s%s",
			mem_config_file_name, mod->name,
			err_mem_levels, err_mem_config_note);

	/* Check block size */
	if (mod->block_size < block_size)
		fatal("%s: %s: decreasing block size.\n%s%s",
			mem_config_file_name, mod->name,
			err_mem_block_size, err_mem_config_note);
	block_size = mod->block_size;

	/* Dump current module */
	mem_debug("\t");
	for (i = 0; i < level * 2; i++)
		mem_debug(" ");
	mem_debug("%s\n", mod->name);

	/* Check that cache has a way to main memory */
	if (!linked_list_count(mod->low_mod_list) && mod->kind == mod_kind_cache)
		fatal("%s: %s: main memory not accessible from cache.\n%s",
			mem_config_file_name, mod->name,
			err_mem_config_note);

	/* Dump children */
	for (linked_list_head(mod->low_mod_list); !linked_list_is_end(mod->low_mod_list);
		linked_list_next(mod->low_mod_list))
	{
		low_mod = linked_list_get(mod->low_mod_list);
		mem_config_check_route_to_main_memory(low_mod, block_size, level + 1);
	}
}

static void mem_config_read_low_modules(struct config_t *config)
{
	char buf[MAX_STRING_SIZE];
	char *delim;

	char *low_mod_name;
	char *low_mod_name_list;

	struct mod_t *mod;
	struct mod_t *low_mod;

	int i;

	/* Lower level modules */
	for (i = 0; i < list_count(mem_system->mod_list); i++)
	{
		/* Get cache module */
		mod = list_get(mem_system->mod_list, i);
		if (mod->kind != mod_kind_cache)
			continue;

		/* Section name */
		snprintf(buf, sizeof buf, "Module %s", mod->name);
		assert(config_section_exists(config, buf));

		/* Low module name list */
		low_mod_name_list = config_read_string(config, buf, "LowModules", "");
		if (!*low_mod_name_list)
			continue;

		/* Create copy of low module name list */
		low_mod_name_list = strdup(low_mod_name_list);
		if (!low_mod_name_list)
			fatal("%s: out of memory", __FUNCTION__);

		/* For each element in the list */
		delim = ", ";
		for (low_mod_name = strtok(low_mod_name_list, delim);
			low_mod_name; low_mod_name = strtok(NULL, delim))
		{
			/* Check valid module name */
			snprintf(buf, sizeof buf, "Module %s", low_mod_name);
			if (!config_section_exists(config, buf))
				fatal("%s: %s: invalid module name in 'LowModules'.\n%s",
					mem_config_file_name, mod->name,
					err_mem_config_note);

			/* Get low cache and assign */
			low_mod = config_read_ptr(config, buf, "ptr", NULL);
			assert(low_mod);
			linked_list_add(mod->low_mod_list, low_mod);
			linked_list_add(low_mod->high_mod_list, mod);
		}

		/* Free copy of low module name list */
		free(low_mod_name_list);
	}

	/* Check paths to main memory */
	mem_debug("Checking paths between caches and main memories:\n");
	for (i = 0; i < list_count(mem_system->mod_list); i++)
	{
		mod = list_get(mem_system->mod_list, i);
		mem_config_check_route_to_main_memory(mod, mod->block_size, 1);
	}
	mem_debug("\n");
}


static void mem_config_read_cpu_entries(struct config_t *config)
{
	struct mod_t *mod;

	char *section;
	char *value;
	char *entry_name;

	char buf[MAX_STRING_SIZE];

	int core;
	int thread;

	struct entry_t
	{
		char data_mod_name[MAX_STRING_SIZE];
		char inst_mod_name[MAX_STRING_SIZE];
	} *entry, *entry_list;

	/* Allocate entry list */
	entry_list = calloc(cpu_cores * cpu_threads, sizeof(struct entry_t));
	if (!entry_list)
		fatal("%s: out of memory", __FUNCTION__);

	/* Read memory system entries */
	for (section = config_section_first(config); section; section = config_section_next(config))
	{
		/* Section is a node */
		if (strncasecmp(section, "Entry ", 6))
			continue;

		/* Name for the entry */
		entry_name = section + 6;
		if (!*entry_name)
			fatal("%s: entry %s: bad name", mem_config_file_name, entry_name);

		/* Get type */
		value = config_read_string(config, section, "Type", "");
		if (strcasecmp(value, "CPU") && strcasecmp(value, "GPU"))
			fatal("%s: entry %s: wrong or missing value for 'Type'",
				mem_config_file_name, entry_name);

		/* Only handle CPU */
		if (strcasecmp(value, "CPU"))
			continue;

		/* Read core */
		core = config_read_int(config, section, "Core", -1);
		if (core < 0)
			fatal("%s: entry %s: wrong or missing value for 'Core'",
				mem_config_file_name, entry_name);

		/* Read thread */
		thread = config_read_int(config, section, "Thread", -1);
		if (thread < 0)
			fatal("%s: entry %s: wrong or missing value for 'Thread'",
				mem_config_file_name, entry_name);

		/* Check bounds */
		if (core >= cpu_cores || thread >= cpu_threads)
		{
			config_var_allow(config, section, "DataModule");
			config_var_allow(config, section, "InstModule");
			warning("%s: entry %s ignored.\n%s",
				mem_config_file_name, entry_name, err_mem_ignored_entry);
			continue;
		}

		/* Check that entry was not assigned before */
		entry = &entry_list[core * cpu_threads + thread];
		if (entry->data_mod_name[0])
			fatal("%s: duplicated entry for CPU core %d - thread %d",
				mem_config_file_name, core, thread);

		/* Get entry data module */
		value = config_read_string(config, section, "DataModule", "");
		if (!*value)
			fatal("%s: entry %s: wrong or missing value for 'DataModule'",
				mem_config_file_name, entry_name);
		snprintf(entry->data_mod_name, MAX_STRING_SIZE, "%s", value);

		/* Get entry instruction module */
		value = config_read_string(config, section, "InstModule", "");
		if (!*value)
			fatal("%s: entry %s: wrong of missing value for 'InstModule'",
				mem_config_file_name, entry_name);
		snprintf(entry->inst_mod_name, MAX_STRING_SIZE, "%s", value);
	}

	/* Stop here if we are doing CPU functional simulation */
	if (cpu_sim_kind == cpu_sim_kind_functional)
		goto out;

	/* Assign entry modules */
	mem_debug("Assigning CPU entries to memory system:\n");
	FOREACH_CORE FOREACH_THREAD
	{
		/* Check that entry was set */
		entry = &entry_list[core * cpu_threads + thread];
		if (!*entry->data_mod_name)
			fatal("%s: no entry given for CPU core %d - thread %d",
				mem_config_file_name, core, thread);

		/* Look for data module */
		snprintf(buf, sizeof buf, "Module %s", entry->data_mod_name);
		if (!config_section_exists(config, buf))
			fatal("%s: invalid data module for CPU core %d - thread %d",
				mem_config_file_name, core, thread);

		/* Assign data module */
		mod = config_read_ptr(config, buf, "ptr", NULL);
		assert(mod);
		THREAD.data_mod = mod;
		mem_debug("\tCPU core %d - thread %d - data -> %s\n",
			core, thread, mod->name);

		/* Look for instructions module */
		snprintf(buf, sizeof buf, "Module %s", entry->inst_mod_name);
		if (!config_section_exists(config, buf))
			fatal("%s: invalid instructions module for CPU core %d - thread %d",
				mem_config_file_name, core, thread);

		/* Assign data module */
		mod = config_read_ptr(config, buf, "ptr", NULL);
		assert(mod);
		THREAD.inst_mod = mod;
		mem_debug("\tCPU core %d - thread %d - instructions -> %s\n",
			core, thread, mod->name);
	}

	/* Debug */
	mem_debug("\n");

out:
	/* Free entry list */
	free(entry_list);
}


static void mem_config_read_gpu_entries(struct config_t *config)
{
	struct mod_t *mod;

	int compute_unit_id;

	char *section;
	char *value;
	char *entry_name;

	char buf[MAX_STRING_SIZE];

	struct entry_t
	{
		char mod_name[MAX_STRING_SIZE];
	} *entry, *entry_list;

	/* Allocate entry list */
	entry_list = calloc(gpu_num_compute_units, sizeof(struct entry_t));
	if (!entry_list)
		fatal("%s: out of memory", __FUNCTION__);

	/* Read memory system entries */
	for (section = config_section_first(config); section; section = config_section_next(config))
	{
		/* Section is a node */
		if (strncasecmp(section, "Entry ", 6))
			continue;

		/* Name for the entry */
		entry_name = section + 6;
		if (!*entry_name)
			fatal("%s: entry %s: bad name", mem_config_file_name, entry_name);

		/* Only handle GPU */
		value = config_read_string(config, section, "Type", "");
		if (strcasecmp(value, "GPU"))
			continue;

		/* Read compute unit */
		compute_unit_id = config_read_int(config, section, "ComputeUnit", -1);
		if (compute_unit_id < 0)
			fatal("%s: entry %s: wrong or missing value for 'ComputeUnit'",
				mem_config_file_name, entry_name);

		/* Check bounds */
		if (compute_unit_id >= gpu_num_compute_units)
		{
			config_var_allow(config, section, "Module");
			warning("%s: entry %s ignored.\n%s",
				mem_config_file_name, entry_name, err_mem_ignored_entry);
			continue;
		}

		/* Check that entry was not assigned before */
		entry = &entry_list[compute_unit_id];
		if (entry->mod_name[0])
			fatal("%s: duplicated entry for GPU compute unit %d",
				mem_config_file_name, compute_unit_id);

		/* Get entry data module */
		value = config_read_string(config, section, "Module", "");
		if (!*value)
			fatal("%s: entry %s: wrong or missing value for 'Module'",
				mem_config_file_name, entry_name);
		snprintf(entry->mod_name, MAX_STRING_SIZE, "%s", value);
	}

	/* Do not continue if we are doing GPU functional simulation */
	if (gpu_sim_kind == gpu_sim_kind_functional)
		goto out;

	/* Assign entry modules */
	mem_debug("Assigning GPU entries to memory system:\n");
	FOREACH_COMPUTE_UNIT(compute_unit_id)
	{
		/* Check that entry was set */
		entry = &entry_list[compute_unit_id];
		if (!*entry->mod_name)
			fatal("%s: no entry given for GPU compute unit %d",
				mem_config_file_name, compute_unit_id);

		/* Look for module */
		snprintf(buf, sizeof buf, "Module %s", entry->mod_name);
		if (!config_section_exists(config, buf))
			fatal("%s: invalid entry for compute unit %d",
				mem_config_file_name, compute_unit_id);

		/* Assign module */
		mod = config_read_ptr(config, buf, "ptr", NULL);
		assert(mod);
		gpu->compute_units[compute_unit_id]->global_mod = mod;
		mem_debug("\tGPU compute unit %d -> %s\n", compute_unit_id, mod->name);
	}

	/* Debug */
	mem_debug("\n");

out:
	/* Free entry list */
	free(entry_list);
}


static void mem_config_create_switches(struct config_t *config)
{
	struct net_t *net;
	struct net_node_t *net_node;
	struct net_node_t *net_switch;

	char buf[MAX_STRING_SIZE];
	char net_switch_name[MAX_STRING_SIZE];

	int def_bandwidth;
	int def_input_buffer_size;
	int def_output_buffer_size;

	int i;
	int j;

	/* For each network, add a switch and create node connections */
	mem_debug("Creating network switches and links for internal networks:\n");
	for (i = 0; i < list_count(mem_system->net_list); i++)
	{
		/* Get network and lower level cache */
		net = list_get(mem_system->net_list, i);

		/* Get switch bandwidth */
		snprintf(buf, sizeof buf, "Network %s", net->name);
		assert(config_section_exists(config, buf));
		def_bandwidth = config_read_int(config, buf, "DefaultBandwidth", 0);
		if (def_bandwidth < 1)
			fatal("%s: %s: invalid or missing value for 'DefaultBandwidth'.\n%s",
				mem_config_file_name, net->name, err_mem_config_note);

		/* Get input/output buffer sizes.
		 * Checks for these variables has done before. */
		def_input_buffer_size = config_read_int(config, buf, "DefaultInputBufferSize", 0);
		def_output_buffer_size = config_read_int(config, buf, "DefaultOutputBufferSize", 0);
		assert(def_input_buffer_size > 0);
		assert(def_output_buffer_size > 0);

		/* Create switch */
		snprintf(net_switch_name, sizeof net_switch_name, "Switch");
		net_switch = net_add_switch(net, def_input_buffer_size, def_output_buffer_size,
			def_bandwidth, net_switch_name);
		mem_debug("\t%s.Switch ->", net->name);

		/* Create connections between switch and every end node */
		for (j = 0; j < list_count(net->node_list); j++)
		{
			net_node = list_get(net->node_list, j);
			if (net_node != net_switch)
			{
				net_add_bidirectional_link(net, net_node, net_switch,
					def_bandwidth);
				mem_debug(" %s", net_node->name);
			}
		}

		/* Calculate routes */
		net_routing_table_calculate(net->routing_table);

		/* Debug */
		mem_debug("\n");
	}
	mem_debug("\n");

}


void mem_config_check_routes(void)
{
	struct mod_t *mod;
	struct net_routing_table_entry_t *entry;
	int i;

	/* For each module, check accessibility to low/high modules */
	mem_debug("Checking accessibility to low and high modules:\n");
	for (i = 0; i < list_count(mem_system->mod_list); i++)
	{
		struct mod_t *low_mod;

		/* Get module */
		mod = list_get(mem_system->mod_list, i);
		mem_debug("\t%s\n", mod->name);

		/* List of low modules */
		mem_debug("\t\tLow modules:");
		for (linked_list_head(mod->low_mod_list); !linked_list_is_end(mod->low_mod_list);
			linked_list_next(mod->low_mod_list))
		{
			/* Get low module */
			low_mod = linked_list_get(mod->low_mod_list);
			mem_debug(" %s", low_mod->name);

			/* Check that nodes are in the same network */
			if (mod->low_net != low_mod->high_net)
				fatal("%s: %s: low node '%s' is not in the same network.\n%s",
					mem_config_file_name, mod->name, low_mod->name,
					err_mem_config_note);

			/* Check that there is a route */
			entry = net_routing_table_lookup(mod->low_net->routing_table,
				mod->low_net_node, low_mod->high_net_node);
			if (!entry->output_buffer)
				fatal("%s: %s: network does not connect '%s' with '%s'.\n%s",
					mem_config_file_name, mod->low_net->name,
					mod->name, low_mod->name, err_mem_connect);
		}

		/* List of high modules */
		mem_debug("\n\t\tHigh modules:");
		for (linked_list_head(mod->high_mod_list); !linked_list_is_end(mod->high_mod_list);
			linked_list_next(mod->high_mod_list))
		{
			struct mod_t *high_mod;

			/* Get high module */
			high_mod = linked_list_get(mod->high_mod_list);
			mem_debug(" %s", high_mod->name);

			/* Check that nodes are in the same network */
			if (mod->high_net != high_mod->low_net)
				fatal("%s: %s: high node '%s' is not in the same network.\n%s",
					mem_config_file_name, mod->name, high_mod->name,
					err_mem_config_note);

			/* Check that there is a route */
			entry = net_routing_table_lookup(mod->high_net->routing_table,
				mod->high_net_node, high_mod->low_net_node);
			if (!entry->output_buffer)
				fatal("%s: %s: network does not connect '%s' with '%s'.\n%s",
					mem_config_file_name, mod->high_net->name,
					mod->name, high_mod->name, err_mem_connect);
		}

		/* Debug */
		mem_debug("\n");
	}

	/* Debug */
	mem_debug("\n");
}


/* Set a color for a module and all its lower-level modules */
static void mem_config_set_mod_color(struct mod_t *mod, int color)
{
	struct mod_t *low_mod;

	/* Already set */
	if (mod->color == color)
		return;

	/* Set color */
	mod->color = color;
	for (linked_list_head(mod->low_mod_list); !linked_list_is_end(mod->low_mod_list);
		linked_list_next(mod->low_mod_list))
	{
		low_mod = linked_list_get(mod->low_mod_list);
		mem_config_set_mod_color(low_mod, color);
	}
}


/* Check if a module or any of its lower-level modules has a color */
static int mem_config_check_mod_color(struct mod_t *mod, int color)
{
	struct mod_t *low_mod;

	/* This module has the color */
	if (mod->color == color)
		return 1;

	/* Check lower-level modules */
	for (linked_list_head(mod->low_mod_list); !linked_list_is_end(mod->low_mod_list);
		linked_list_next(mod->low_mod_list))
	{
		low_mod = linked_list_get(mod->low_mod_list);
		if (mem_config_check_mod_color(low_mod, color))
			return 1;
	}

	/* Not found */
	return 0;
}


static void mem_config_check_disjoint(void)
{
	int compute_unit_id;
	int core;
	int thread;

	/* No need if we do not have both CPU and GPU detailed simulation */
	if (cpu_sim_kind == cpu_sim_kind_functional || gpu_sim_kind == gpu_sim_kind_functional)
		return;

	/* Color CPU modules */
	FOREACH_CORE FOREACH_THREAD
	{
		mem_config_set_mod_color(THREAD.data_mod, 1);
		mem_config_set_mod_color(THREAD.inst_mod, 1);
	}

	/* Check color of GPU modules */
	FOREACH_COMPUTE_UNIT(compute_unit_id)
	{
		if (mem_config_check_mod_color(gpu->compute_units[compute_unit_id]->global_mod, 1))
			fatal("%s: non-disjoint CPU/GPU memory hierarchies",
				mem_config_file_name);
	}
}


static void mem_config_read_sub_block_sizes(void)
{
	struct mod_t *mod;
	struct mod_t *high_mod;

	int num_nodes;
	int i;

	mem_debug("Creating directories:\n");
	for (i = 0; i < list_count(mem_system->mod_list); i++)
	{
		/* Get module */
		mod = list_get(mem_system->mod_list, i);

		/* Calculate sub-block size */
		mod->sub_block_size = mod->block_size;
		for (linked_list_head(mod->high_mod_list); !linked_list_is_end(mod->high_mod_list);
			linked_list_next(mod->high_mod_list))
		{
			high_mod = linked_list_get(mod->high_mod_list);
			mod->sub_block_size = MIN(mod->sub_block_size, high_mod->block_size);
		}

		/* Get number of nodes for directory */
		if (mod->high_net && list_count(mod->high_net->node_list))
			num_nodes = list_count(mod->high_net->node_list);
		else
			num_nodes = 1;

		/* Create directory */
		mod->num_sub_blocks = mod->block_size / mod->sub_block_size;
		mod->dir = dir_create(mod->dir_num_sets, mod->dir_assoc, mod->num_sub_blocks, num_nodes);
		mem_debug("\t%s - %dx%dx%d (%dx%dx%d effective) - %d entries, %d sub-blocks\n",
			mod->name, mod->dir_num_sets, mod->dir_assoc, num_nodes,
			mod->dir_num_sets, mod->dir_assoc, linked_list_count(mod->high_mod_list),
			mod->dir_size, mod->num_sub_blocks);
	}

	/* Debug */
	mem_debug("\n");
}




/*
 * Public Functions
 */

void mem_system_config_read(void)
{
	struct config_t *config;

	/* Load memory system configuration file */
	config = config_create(mem_config_file_name);
	if (!*mem_config_file_name)
	{
		mem_config_cpu_default(config);
		mem_config_gpu_default(config);
	}
	else
	{
		if (!config_load(config))
			fatal("%s: cannot read memory system configuration file",
				mem_config_file_name);
	}

	/* Read networks */
	mem_config_read_networks(config);

	/* Read modules */
	mem_config_read_modules(config);

	/* Read low level caches */
	mem_config_read_low_modules(config);

	/* Read memory system entries */
	mem_config_read_cpu_entries(config);
	mem_config_read_gpu_entries(config);

	/* Create switches in internal networks */
	mem_config_create_switches(config);

	/* Check that all enforced sections and variables were specified */
	config_check(config);
	config_free(config);

	/* Check routes to low and high modules */
	mem_config_check_routes();

	/* Check for disjoint CPU/GPU memory hierarchies */
	mem_config_check_disjoint();

	/* Compute sub-block sizes, based on high modules */
	mem_config_read_sub_block_sizes();
}
