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

#ifndef ARCH_X86_TIMING_CORE_H
#define ARCH_X86_TIMING_CORE_H

#include <vector>
#include <list>
#include <string>

#include <arch/x86/emulator/Uinst.h>

#include "Alu.h"
#include "Thread.h"


namespace x86
{

// Forward declaration
class Timing;
class Cpu;

// Class Core
class Core
{
private:

	// Name of this core
	std::string name;

	// CPU that it belongs to 
	Cpu *cpu;

	// Array of threads 
	std::vector<std::unique_ptr<Thread>> threads;

	// Unique identifier in the CPU
	int id;

	// Arithmetic-logic unit
	Alu alu;

	// Event queue
	std::list<std::shared_ptr<Uop>> event_queue;




	//
	// Counters per core
	//

	// Counter used to assign per-core identifiers to uops
	long long uop_id_counter = 0;

	// Number of occupied integer registers
	int num_integer_registers_occupied = 0;

	// Number of occupied float point registers
	int num_float_point_registers_occupied = 0;

	// Number of XMM registers
	int num_xmm_registers_occupied = 0;

	// Total number of instructions in all threads' ROBs
	int reorder_buffer_occupancy = 0;

	// Total number of instructions in all threads' IQs
	int instruction_queue_occupancy = 0;

	// Total number of instructions in all threads' LSQs
	int load_store_queue_occupancy = 0;




	//
	// Fetch stage
	//

	// Currently fetching thread
	int current_fetch_thread = 0;




	//
	// Dispatch stage
	//

	// Currently dispatching thread
	int current_dispatch_thread = 0;




	//
	// Issue stage
	//

	// Currently issued thread
	int current_issue_thread = 0;




	//
	// Statistics
	//

	// Number of stalled micro-instruction when dispatch divded by reason
	long long dispatch_stall[Thread::DispatchStallMax] = {};

	// Number of dispatched micro-instructions for every opcode
	long long num_dispatched_uinsts[Uinst::OpcodeCount] = {};

	// Number of issued micro-instructions for every opcode
	long long num_issued_uinsts[Uinst::OpcodeCount] = {};

	// Number of committed micro-instructions for every opcode
	long long num_committed_uinsts[Uinst::OpcodeCount] = {};

	// Number of squashed micro-instructions
	long long num_squashed_uinsts = 0;

	// Number of branch micro-instructions
	long long num_branches = 0;

	// Number of mis-predicted branch micro-instructions
	long long num_mispredicted_branches = 0;




	//
	// Statistics for shared structures 
	//

	long long reorder_buffer_reads = 0;
	long long reorder_buffer_writes = 0;

	long long instruction_queue_reads = 0;
	long long instruction_queue_writes = 0;

	long long load_store_queue_reads = 0;
	long long load_store_queue_writes = 0;

	long long integer_register_reads = 0;
	long long integer_register_writes = 0;

	long long floating_point_register_reads = 0;
	long long floating_point_register_writes = 0;
	
	long long xmm_register_reads = 0;
	long long xmm_register_writes = 0;

public:

	/// Constructor
	Core(Cpu *cpu, int index);

	/// Return the number of threads
	int getNumThreads() const { return threads.size(); }

	/// Return the thread with the given index
	Thread *getThread(int index) const
	{
		assert(index >= 0 && index < (int) threads.size());
		return threads[index].get();
	}

	/// Get the CPU object that this core belongs to
	Cpu *getCpu() const { return cpu; }

	/// Get core index within the CPU
	int getId() const { return id; }

	/// Return a new unique identifier for a uop in this core
	long long getUopId() { return ++uop_id_counter; }

	/// Return the core's arithmetic-logic unit
	Alu *getAlu() { return &alu; }




	//
	// Register file
	//

	/// Get the number of occupied physical integer registers
	int getNumIntegerRegistersOccupied() { return num_integer_registers_occupied; }

	/// Get the number of occupied physical float point registers
	int getNumFloatPointRegistersOccupied() { return num_float_point_registers_occupied; }

	/// Get the number of occupied physical xmm registers
	int getNumXmmRegistersOccupied() { return num_xmm_registers_occupied; }

	/// Increment the number of occupied physical integer registers
	void incNumIntegerRegistersOccupied() { num_integer_registers_occupied++; }

	/// Increment the number of occupied physical float point registers
	void incNumFloatPointRegistersOccupied() { num_float_point_registers_occupied++; }

	/// Increment the number of occupied physical xmm registers
	void incNumXmmRegistersOccupied() { num_xmm_registers_occupied++; }

	/// Decrement the number of occupied physical integer registers
	void decNumIntegerRegistersOccupied()
	{
		assert(num_integer_registers_occupied > 0);
		num_integer_registers_occupied--;
	}

	/// Decrement the number of occupied physical float point registers
	void decNumFloatPointRegistersOccupied()
	{
		assert(num_float_point_registers_occupied > 0);
		num_float_point_registers_occupied--;
	}

	/// Decrement the number of occupied physical xmm registers
	void decNumXmmRegistersOccupied()
	{
		assert(num_xmm_registers_occupied > 0);
		num_xmm_registers_occupied--;
	}





	//
	// Event queue
	//

	/// Insert uop into event queue, making it ready to be extract in
	/// \a latency cycles from now. The uop's field `complete_when` is
	/// set to the current cycle plus \a latency in the function.
	void InsertInEventQueue(std::shared_ptr<Uop> uop, int latency);

	/// Extract uop from event queue. The given uop must be placed at the
	/// head of the event queue.
	void ExtractFromEventQueue(Uop *uop);

	/// Return an iterator to the first element of the event queue
	std::list<std::shared_ptr<Uop>>::iterator getEventQueueBegin()
	{
		return event_queue.begin();
	}

	/// Return a past-the-end iterator to the event queue
	std::list<std::shared_ptr<Uop>>::iterator getEventQueueEnd()
	{
		return event_queue.end();
	}




	//
	// Reorder buffer
	//

	/// Increment the total number of instructions in all threads' ROBs
	void incReorderBufferOccupancy() { reorder_buffer_occupancy++; }

	/// Decrement the total number of instructions in all threads' ROBs
	void decReorderBufferOccupancy()
	{
		assert(reorder_buffer_occupancy > 0);
		reorder_buffer_occupancy--;
	}

	/// Return the total number of instructions in all threads' ROBs
	int getReorderBufferOccupancy() const { return reorder_buffer_occupancy; }




	//
	// Instruction queue
	//
	
	/// Increment the total number of instructions in all threads' IQs
	void incInstructionQueueOccupancy() { instruction_queue_occupancy++; }

	/// Decrement the total number of instructions in all threads' IQs
	void decInstructionQueueOccupancy()
	{
		assert(instruction_queue_occupancy > 0);
		instruction_queue_occupancy--;
	}

	/// Return the total number of instructions in all threads' IQs
	int getInstructionQueueOccupancy() const
	{
		return instruction_queue_occupancy;
	}




	//
	// Load-Store queue
	//
	
	/// Increment the total number of instructions in all threads' LSQs
	void incLoadStoreQueueOccupancy() { load_store_queue_occupancy++; }

	/// Decrement the total number of instructions in all threads' LSQs
	void decLoadStoreQueueOccupancy()
	{
		assert(load_store_queue_occupancy > 0);
		load_store_queue_occupancy--;
	}

	/// Return the total number of instructions in all threads' LSQs
	int getLoadStoreQueueOccupancy() const
	{
		return load_store_queue_occupancy;
	}




	//
	// Pipeline stages
	//

	/// Run one simulation cycle for all pipeline stages of the core.
	void Run();

	/// Fetch stage
	void Fetch();

	/// Decode stage
	void Decode();

	/// Dispatch stage
	void Dispatch();

	/// Issue stage
	void Issue();

	/// Writeback stage
	void Writeback();



	//
	// Statistics
	//

	/// Increment the counter for reasons of dispatch stalls by the given
	/// quantum.
	void incDispatchStall(Thread::DispatchStall stall, int quantum)
	{
		assert(stall > Thread::DispatchStallInvalid &&
				stall < Thread::DispatchStallMax);
		dispatch_stall[stall] += quantum;
	}

	/// Increment the number of reads to a thread's reorder buffer
	void incReorderBufferReads() { reorder_buffer_reads++; }

	/// Increment the number of writes to a thread's reorder buffer
	void incReorderBufferWrites() { reorder_buffer_writes++; }

	/// Increment the number of reads to a thread's instruction queue
	void incInstructionQueueReads() { instruction_queue_reads++; }

	/// Increment the number of writes to a thread's instruction queue
	void incInstructionQueueWrites() { instruction_queue_writes++; }

	/// Increment the number of reads from the load-store queue
	void incLoadStoreQueueReads() { load_store_queue_reads++; }

	/// Increment the number of writes to a thread's load-store queue
	void incLoadStoreQueueWrites() { load_store_queue_writes++; }
	
	/// Increment the number of dispatched micro-instructions of a given
	/// kind.
	void incNumDispatchedUinsts(Uinst::Opcode opcode)
	{
		assert(opcode < Uinst::OpcodeCount);
		num_dispatched_uinsts[opcode]++;
	}
	
	/// Increment the number of issued micro-instructions of a type
	void incNumIssuedUinsts(Uinst::Opcode opcode)
	{
		assert(opcode < Uinst::OpcodeCount);
		num_issued_uinsts[opcode]++;
	}

	/// Increment the number of reads to integer registers
	void incIntegerRegisterReads(int count = 1)
	{
		integer_register_reads += count;
	}

	/// Increment the number of writes to integer registers
	void incIntegerRegisterWrites(int count = 1)
	{
		integer_register_writes += count;
	}

	/// Increment the number of reads to floating-point registers
	void incFloatingPointRegisterReads(int count = 1)
	{
		floating_point_register_reads += count;
	}

	/// Increment the number of writes to floating-point registers
	void incFloatingPointRegisterWrites(int count = 1)
	{
		floating_point_register_writes += count;
	}

	/// Increment the number of reads to XMM registers
	void incXmmRegisterReads(int count = 1)
	{
		xmm_register_reads += count;
	}

	/// Increment the number of writes to XMM registers
	void incXmmRegisterWrites(int count = 1)
	{
		xmm_register_writes += count;
	}

	/// Increment the number of squashed micro-instructions
	void incNumSquashedUinsts() { num_squashed_uinsts++; }
};

}

#endif // ARCH_X86_TIMING_CORE_H
