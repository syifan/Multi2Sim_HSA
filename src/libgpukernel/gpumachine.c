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
#include <gpudisasm.h>

char *err_gpu_machine_note =
	"\tThe AMD Evergreen instruction set is partially supported by Multi2Sim. If\n"
	"\tyour program is using an unimplemented instruction, please email\n"
	"\t'development@multi2sim.org' to request support for it.\n";

#define NOT_IMPL() fatal("GPU instruction '%s' not implemented\n%s", \
	gpu_isa_inst->info->name, err_gpu_machine_note)


void amd_inst_ALU_impl() {
	/* FIXME: block constant cache sets */
}


void amd_inst_ALU_BREAK_impl() {
	NOT_IMPL();
}


void amd_inst_ALU_POP_AFTER_impl() {
	NOT_IMPL();
}


void amd_inst_ALU_PUSH_BEFORE_impl() {
	/* FIXME: block constant cache sets */
}


void amd_inst_ELSE_impl() {
	NOT_IMPL();
}


void amd_inst_JUMP_impl() {
	NOT_IMPL();
}


void amd_inst_LOOP_END_impl() {
	NOT_IMPL();
}


void amd_inst_LOOP_START_DX10_impl() {
	NOT_IMPL();
}


#define W0  gpu_isa_inst->words[0].cf_alloc_export_word0_rat
#define W1  gpu_isa_inst->words[1].cf_alloc_export_word1_buf
void amd_inst_MEM_RAT_CACHELESS_impl()
{
	switch (W0.rat_inst) {

	/* STORE_RAW */
	case 2: {
		uint32_t value, addr;

		GPU_PARAM_NOT_SUPPORTED_NEQ(W0.rat_id, 1);  /* FIXME: what does rat_id mean? */
		GPU_PARAM_NOT_SUPPORTED_NEQ(W0.rim, 0);  /* rat_index_mode */
		GPU_PARAM_NOT_SUPPORTED_NEQ(W0.type, 1);
		GPU_PARAM_NOT_SUPPORTED_NEQ(W1.comp_mask, 1);  /* x___ */
		GPU_PARAM_NOT_SUPPORTED_NEQ(W1.burst_count, 0);
		GPU_PARAM_NOT_SUPPORTED_NEQ(W1.m, 0);  /* mark */

		value = gpu_isa_read_gpr(W0.rw_gpr, W0.rr, 0, 0);
		addr = gpu_isa_read_gpr(W0.index_gpr, 0, 0, 0);  /* FIXME: only 1D - X coordinate */
		mem_write(gk->global_mem, addr, 4, &value);
		/* FIXME: array_size: ignored now, cause 'burst_count' = 0 */
		/* FIXME: valid_pixel_mode */
		/* FIXME: rat_id */
		break;
	}

	default:
		GPU_PARAM_NOT_SUPPORTED(W0.rat_inst);
	}
}
#undef W0
#undef W1


void amd_inst_TC_impl() {
	NOT_IMPL();
}


void amd_inst_ADD_impl() {
	NOT_IMPL();
}


void amd_inst_MUL_impl() {
	NOT_IMPL();
}


void amd_inst_MUL_IEEE_impl() {
	NOT_IMPL();
}


void amd_inst_MAX_impl() {
	NOT_IMPL();
}


void amd_inst_MIN_impl() {
	NOT_IMPL();
}


void amd_inst_MAX_DX10_impl() {
	NOT_IMPL();
}


void amd_inst_MIN_DX10_impl() {
	NOT_IMPL();
}


void amd_inst_SETE_impl() {
	NOT_IMPL();
}


void amd_inst_SETGT_impl() {
	NOT_IMPL();
}


void amd_inst_SETGE_impl() {
	NOT_IMPL();
}


void amd_inst_SETNE_impl() {
	NOT_IMPL();
}


void amd_inst_SETE_DX10_impl() {
	NOT_IMPL();
}


void amd_inst_SETGT_DX10_impl() {
	NOT_IMPL();
}


void amd_inst_SETGE_DX10_impl() {
	NOT_IMPL();
}


void amd_inst_SETNE_DX10_impl() {
	NOT_IMPL();
}


void amd_inst_FRACT_impl() {
	NOT_IMPL();
}


void amd_inst_TRUNC_impl() {
	NOT_IMPL();
}


void amd_inst_CEIL_impl() {
	NOT_IMPL();
}


void amd_inst_RNDNE_impl() {
	NOT_IMPL();
}


void amd_inst_FLOOR_impl() {
	NOT_IMPL();
}


void amd_inst_ASHR_INT_impl() {
	NOT_IMPL();
}


void amd_inst_LSHR_INT_impl()
{
	uint32_t src0, src1, dst;

	src0 = gpu_isa_read_op_src(0);
	src1 = gpu_isa_read_op_src(1);
	dst = src0 >> src1;
	gpu_isa_write_op_dst(dst);
}


void amd_inst_LSHL_INT_impl()
{
	uint32_t src0, src1, dst;

	src0 = gpu_isa_read_op_src(0);
	src1 = gpu_isa_read_op_src(1);
	dst = src0 << src1;
	gpu_isa_write_op_dst(dst);
}


void amd_inst_MOV_impl() {
	NOT_IMPL();
}


void amd_inst_NOP_impl() {
	NOT_IMPL();
}


void amd_inst_MUL_64_impl() {
	NOT_IMPL();
}


void amd_inst_FLT64_TO_FLT32_impl() {
	NOT_IMPL();
}


void amd_inst_FLT32_TO_FLT64_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETGT_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETGE_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETE_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETGT_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETGE_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETNE_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SET_INV_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SET_POP_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SET_CLR_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SET_RESTORE_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETE_PUSH_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETGT_PUSH_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETGE_PUSH_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETNE_PUSH_impl() {
	NOT_IMPL();
}


void amd_inst_KILLE_impl() {
	NOT_IMPL();
}


void amd_inst_KILLGT_impl() {
	NOT_IMPL();
}


void amd_inst_KILLGE_impl() {
	NOT_IMPL();
}


void amd_inst_KILLNE_impl() {
	NOT_IMPL();
}


void amd_inst_AND_INT_impl() {
	NOT_IMPL();
}


void amd_inst_OR_INT_impl() {
	NOT_IMPL();
}


void amd_inst_XOR_INT_impl() {
	NOT_IMPL();
}


void amd_inst_NOT_INT_impl() {
	NOT_IMPL();
}


void amd_inst_ADD_INT_impl()
{
	uint32_t src0, src1, dst;

	src0 = gpu_isa_read_op_src(0);
	src1 = gpu_isa_read_op_src(1);
	dst = src0 + src1;
	gpu_isa_write_op_dst(dst);
}


void amd_inst_SUB_INT_impl() {
	NOT_IMPL();
}


void amd_inst_MAX_INT_impl() {
	NOT_IMPL();
}


void amd_inst_MIN_INT_impl() {
	NOT_IMPL();
}


void amd_inst_MAX_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_MIN_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_SETE_INT_impl() {
	NOT_IMPL();
}


void amd_inst_SETGT_INT_impl() {
	NOT_IMPL();
}


void amd_inst_SETGE_INT_impl() {
	NOT_IMPL();
}


void amd_inst_SETNE_INT_impl() {
	NOT_IMPL();
}


void amd_inst_SETGT_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_SETGE_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_KILLGT_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_KILLGE_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_PREDE_INT_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETGT_INT_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETGE_INT_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETNE_INT_impl() {
	NOT_IMPL();
}


void amd_inst_KILLE_INT_impl() {
	NOT_IMPL();
}


void amd_inst_KILLGT_INT_impl() {
	NOT_IMPL();
}


void amd_inst_KILLGE_INT_impl() {
	NOT_IMPL();
}


void amd_inst_KILLNE_INT_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETE_PUSH_INT_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETGT_PUSH_INT_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETGE_PUSH_INT_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETNE_PUSH_INT_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETLT_PUSH_INT_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETLE_PUSH_INT_impl() {
	NOT_IMPL();
}


void amd_inst_FLT_TO_INT_impl() {
	NOT_IMPL();
}


void amd_inst_BFREV_INT_impl() {
	NOT_IMPL();
}


void amd_inst_ADDC_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_SUBB_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_GROUP_BARRIER_impl() {
	NOT_IMPL();
}


void amd_inst_GROUP_SEQ_BEGIN_impl() {
	NOT_IMPL();
}


void amd_inst_GROUP_SEQ_END_impl() {
	NOT_IMPL();
}


void amd_inst_SET_MODE_impl() {
	NOT_IMPL();
}


void amd_inst_SET_CF_IDX0_impl() {
	NOT_IMPL();
}


void amd_inst_SET_CF_IDX1_impl() {
	NOT_IMPL();
}


void amd_inst_SET_LDS_SIZE_impl() {
	NOT_IMPL();
}


void amd_inst_EXP_IEEE_impl() {
	NOT_IMPL();
}


void amd_inst_LOG_CLAMPED_impl() {
	NOT_IMPL();
}


void amd_inst_LOG_IEEE_impl() {
	NOT_IMPL();
}


void amd_inst_RECIP_CLAMPED_impl() {
	NOT_IMPL();
}


void amd_inst_RECIP_FF_impl() {
	NOT_IMPL();
}


void amd_inst_RECIP_IEEE_impl() {
	NOT_IMPL();
}


void amd_inst_RECIPSQRT_CLAMPED_impl() {
	NOT_IMPL();
}


void amd_inst_RECIPSQRT_FF_impl() {
	NOT_IMPL();
}


void amd_inst_RECIPSQRT_IEEE_impl() {
	NOT_IMPL();
}


void amd_inst_SQRT_IEEE_impl() {
	NOT_IMPL();
}


void amd_inst_SIN_impl() {
	NOT_IMPL();
}


void amd_inst_COS_impl() {
	NOT_IMPL();
}


void amd_inst_MULLO_INT_impl() {
	NOT_IMPL();
}


void amd_inst_MULHI_INT_impl() {
	NOT_IMPL();
}


void amd_inst_MULLO_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_MULHI_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_RECIP_INT_impl() {
	NOT_IMPL();
}


void amd_inst_RECIP_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_RECIP_64_impl() {
	NOT_IMPL();
}


void amd_inst_RECIP_CLAMPED_64_impl() {
	NOT_IMPL();
}


void amd_inst_RECIPSQRT_64_impl() {
	NOT_IMPL();
}


void amd_inst_RECIPSQRT_CLAMPED_64_impl() {
	NOT_IMPL();
}


void amd_inst_SQRT_64_impl() {
	NOT_IMPL();
}


void amd_inst_FLT_TO_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_INT_TO_FLT_impl() {
	NOT_IMPL();
}


void amd_inst_UINT_TO_FLT_impl() {
	NOT_IMPL();
}


void amd_inst_BFM_INT_impl() {
	NOT_IMPL();
}


void amd_inst_FLT32_TO_FLT16_impl() {
	NOT_IMPL();
}


void amd_inst_FLT16_TO_FLT32_impl() {
	NOT_IMPL();
}


void amd_inst_UBYTE0_FLT_impl() {
	NOT_IMPL();
}


void amd_inst_UBYTE1_FLT_impl() {
	NOT_IMPL();
}


void amd_inst_UBYTE2_FLT_impl() {
	NOT_IMPL();
}


void amd_inst_UBYTE3_FLT_impl() {
	NOT_IMPL();
}


void amd_inst_BCNT_INT_impl() {
	NOT_IMPL();
}


void amd_inst_FFBH_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_FFBL_INT_impl() {
	NOT_IMPL();
}


void amd_inst_FFBH_INT_impl() {
	NOT_IMPL();
}


void amd_inst_FLT_TO_UINT4_impl() {
	NOT_IMPL();
}


void amd_inst_DOT_IEEE_impl() {
	NOT_IMPL();
}


void amd_inst_FLT_TO_INT_RPI_impl() {
	NOT_IMPL();
}


void amd_inst_FLT_TO_INT_FLOOR_impl() {
	NOT_IMPL();
}


void amd_inst_MULHI_UINT24_impl() {
	NOT_IMPL();
}


void amd_inst_MBCNT_32HI_INT_impl() {
	NOT_IMPL();
}


void amd_inst_OFFSET_TO_FLT_impl() {
	NOT_IMPL();
}


void amd_inst_MUL_UINT24_impl() {
	NOT_IMPL();
}


void amd_inst_BCNT_ACCUM_PREV_INT_impl() {
	NOT_IMPL();
}


void amd_inst_MBCNT_32LO_ACCUM_PREV_INT_impl() {
	NOT_IMPL();
}


void amd_inst_SETE_64_impl() {
	NOT_IMPL();
}


void amd_inst_SETNE_64_impl() {
	NOT_IMPL();
}


void amd_inst_SETGT_64_impl() {
	NOT_IMPL();
}


void amd_inst_SETGE_64_impl() {
	NOT_IMPL();
}


void amd_inst_MIN_64_impl() {
	NOT_IMPL();
}


void amd_inst_MAX_64_impl() {
	NOT_IMPL();
}


void amd_inst_DOT4_impl() {
	NOT_IMPL();
}


void amd_inst_DOT4_IEEE_impl() {
	NOT_IMPL();
}


void amd_inst_CUBE_impl() {
	NOT_IMPL();
}


void amd_inst_MAX4_impl() {
	NOT_IMPL();
}


void amd_inst_FREXP_64_impl() {
	NOT_IMPL();
}


void amd_inst_LDEXP_64_impl() {
	NOT_IMPL();
}


void amd_inst_FRACT_64_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETGT_64_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETE_64_impl() {
	NOT_IMPL();
}


void amd_inst_PRED_SETGE_64_impl() {
	NOT_IMPL();
}


void amd_inst_MUL_64_VEC_impl() {
	NOT_IMPL();
}


void amd_inst_ADD_64_impl() {
	NOT_IMPL();
}


void amd_inst_MOVA_INT_impl() {
	NOT_IMPL();
}


void amd_inst_FLT64_TO_FLT32_VEC_impl() {
	NOT_IMPL();
}


void amd_inst_FLT32_TO_FLT64_VEC_impl() {
	NOT_IMPL();
}


void amd_inst_SAD_ACCUM_PREV_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_DOT_impl() {
	NOT_IMPL();
}


void amd_inst_MUL_PREV_impl() {
	NOT_IMPL();
}


void amd_inst_MUL_IEEE_PREV_impl() {
	NOT_IMPL();
}


void amd_inst_ADD_PREV_impl() {
	NOT_IMPL();
}


void amd_inst_MULADD_PREV_impl() {
	NOT_IMPL();
}


void amd_inst_MULADD_IEEE_PREV_impl() {
	NOT_IMPL();
}


void amd_inst_INTERP_XY_impl() {
	NOT_IMPL();
}


void amd_inst_INTERP_ZW_impl() {
	NOT_IMPL();
}


void amd_inst_INTERP_X_impl() {
	NOT_IMPL();
}


void amd_inst_INTERP_Z_impl() {
	NOT_IMPL();
}


void amd_inst_STORE_FLAGS_impl() {
	NOT_IMPL();
}


void amd_inst_LOAD_STORE_FLAGS_impl() {
	NOT_IMPL();
}


void amd_inst_LDS_1A_impl() {
	NOT_IMPL();
}


void amd_inst_LDS_1A1D_impl() {
	NOT_IMPL();
}


void amd_inst_LDS_2A_impl() {
	NOT_IMPL();
}


void amd_inst_INTERP_LOAD_P0_impl() {
	NOT_IMPL();
}


void amd_inst_INTERP_LOAD_P10_impl() {
	NOT_IMPL();
}


void amd_inst_INTERP_LOAD_P20_impl() {
	NOT_IMPL();
}


void amd_inst_BFE_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_BFE_INT_impl() {
	NOT_IMPL();
}


void amd_inst_BFI_INT_impl() {
	NOT_IMPL();
}


void amd_inst_FMA_impl() {
	NOT_IMPL();
}


void amd_inst_CNDNE_64_impl() {
	NOT_IMPL();
}


void amd_inst_FMA_64_impl() {
	NOT_IMPL();
}


void amd_inst_LERP_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_BIT_ALIGN_INT_impl() {
	NOT_IMPL();
}


void amd_inst_BYTE_ALIGN_INT_impl() {
	NOT_IMPL();
}


void amd_inst_SAD_ACCUM_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_SAD_ACCUM_HI_UINT_impl() {
	NOT_IMPL();
}


void amd_inst_MULADD_UINT24_impl() {
	NOT_IMPL();
}


void amd_inst_LDS_IDX_OP_impl() {
	NOT_IMPL();
}


void amd_inst_MULADD_impl() {
	NOT_IMPL();
}


void amd_inst_MULADD_M2_impl() {
	NOT_IMPL();
}


void amd_inst_MULADD_M4_impl() {
	NOT_IMPL();
}


void amd_inst_MULADD_D2_impl() {
	NOT_IMPL();
}


void amd_inst_MULADD_IEEE_impl() {
	NOT_IMPL();
}


void amd_inst_CNDE_impl() {
	NOT_IMPL();
}


void amd_inst_CNDGT_impl() {
	NOT_IMPL();
}


void amd_inst_CNDGE_impl() {
	NOT_IMPL();
}


void amd_inst_CNDE_INT_impl() {
	NOT_IMPL();
}


void amd_inst_CMNDGT_INT_impl() {
	NOT_IMPL();
}


void amd_inst_CMNDGE_INT_impl() {
	NOT_IMPL();
}


void amd_inst_MUL_LIT_impl() {
	NOT_IMPL();
}


void amd_inst_FETCH_impl() {
	NOT_IMPL();
}


void amd_inst_GET_BUFFER_RESINFO_impl() {
	NOT_IMPL();
}


void amd_inst_SEMANTIC_impl() {
	NOT_IMPL();
}


void amd_inst_GATHER4_impl() {
	NOT_IMPL();
}


void amd_inst_GATHER4_C_impl() {
	NOT_IMPL();
}


void amd_inst_GATHER4_C_O_impl() {
	NOT_IMPL();
}


void amd_inst_GATHER4_O_impl() {
	NOT_IMPL();
}


void amd_inst_GET_GRADIENTS_H_impl() {
	NOT_IMPL();
}


void amd_inst_GET_GRADIENTS_V_impl() {
	NOT_IMPL();
}


void amd_inst_GET_LOD_impl() {
	NOT_IMPL();
}


void amd_inst_GET_NUMBER_OF_SAMPLES_impl() {
	NOT_IMPL();
}


void amd_inst_GET_TEXTURE_RESINFO_impl() {
	NOT_IMPL();
}


void amd_inst_KEEP_GRADIENTS_impl() {
	NOT_IMPL();
}


void amd_inst_LD_impl() {
	NOT_IMPL();
}


void amd_inst_SAMPLE_impl() {
	NOT_IMPL();
}


void amd_inst_SAMPLE_C_impl() {
	NOT_IMPL();
}


void amd_inst_SAMPLE_C_G_impl() {
	NOT_IMPL();
}


void amd_inst_SAMPLE_C_G_LB_impl() {
	NOT_IMPL();
}


void amd_inst_SAMPLE_C_L_impl() {
	NOT_IMPL();
}


void amd_inst_SAMPLE_C_LB_impl() {
	NOT_IMPL();
}


void amd_inst_SAMPLE_C_LZ_impl() {
	NOT_IMPL();
}


void amd_inst_SAMPLE_G_impl() {
	NOT_IMPL();
}


void amd_inst_SAMPLE_G_LB_impl() {
	NOT_IMPL();
}


void amd_inst_SAMPLE_L_impl() {
	NOT_IMPL();
}


void amd_inst_SAMPLE_LB_impl() {
	NOT_IMPL();
}


void amd_inst_SAMPLE_LZ_impl() {
	NOT_IMPL();
}


void amd_inst_SET_GRADIENTS_H_impl() {
	NOT_IMPL();
}


void amd_inst_SET_GRADIENTS_V_impl() {
	NOT_IMPL();
}


void amd_inst_SET_TEXTURE_OFFSETS_impl() {
	NOT_IMPL();
}

