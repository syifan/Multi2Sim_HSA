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

#ifndef M2C_LLVM2SI_BASIC_BLOCK_H_OLD
#define M2C_LLVM2SI_BASIC_BLOCK_H_OLD

#include <llvm-c/Core.h>

#include <lib/class/class.h>
#include <m2c/common/basic-block.h>


/*
 * Class 'Llvm2siBasicBlock'
 */

CLASS_BEGIN(Llvm2siBasicBlock, BasicBlock)

	/* Comment stored temporarily in the basic block to be attached to the
	 * next instruction added. */
	String *comment;

	/* Function where the basic block belongs. This field is populated
	 * automatically when function 'llvm2si_function_add' is called. */
	Llvm2siFunction *function;

	/* List list of instructions forming the basic block. Each element is of
	 * type 'Si2binInst'. */
	List *inst_list;

	Bitmap *def;
	Bitmap *use;

	Bitmap *in;
	Bitmap *out;

CLASS_END(Llvm2siBasicBlock)


void Llvm2siBasicBlockCreate(Llvm2siBasicBlock *self,
		Llvm2siFunction *function, LeafNode *node);
void Llvm2siBasicBlockDestroy(Llvm2siBasicBlock *self);

/* Virtual function from class Object */
void Llvm2siBasicBlockDump(Object *self, FILE *f);

/* Add one SI instruction to the 'inst_list' field of the basic block. */
void Llvm2siBasicBlockAddInst(Llvm2siBasicBlock *basic_block, Si2binInst *inst);

/* Add a comment to a basic block. The comment will be attached to the next
 * instruction added with 'Llvm2siBasicBlockAddInst'. If not other
 * instruction is added to the basic block, the comment won't have any effect. */
void Llvm2siBasicBlockAddComment(Llvm2siBasicBlock *basic_block,
		char *comment);

/* Emit SI code for the basic block into field 'inst_list'. */
void Llvm2siBasicBlockEmit(Llvm2siBasicBlock *basic_block,
		LLVMBasicBlockRef llbb);


#endif

