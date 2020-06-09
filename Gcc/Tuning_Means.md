<!-- TOC -->

- [1. Cost](#1-cost)
    - [1.1 cpu_cost_table](#11-cpu_cost_table)
    - [1.2 address_cost_table](#12-address_cost_table)
    - [1.3 regmove_cost_table](#13-regmove_cost_table)
    - [1.4 vector_cost_table](#14-vector_cost_table)
    - [1.5 branch_cost_table](#15-branch_cost_table)
- [2. Modes](#2-modes)
    - [2.1 Generic approximation modes](#21-generic-approximation-modes)
    - [2.2 Auto Prefetch modes](#22-auto-prefetch-modes)
    - [2.3 Generic prefetch settings](#23-generic-prefetch-settings)
- [3. Tunings](#3-tunings)
    - [3.1 generic tunings](#31-generic-tunings)
    - [3.2 Pipeline descriptions](#32-pipeline-descriptions)

<!-- /TOC -->

> ref: 《Cortex-A57 Software Optimization Guide.pdf》

# 1. Cost

## 1.1 cpu_cost_table

aarch-cost-tables.h
```
const struct cpu_cost_table cortexa57_extra_costs =
{
  /* ALU */
  {
    0,                 /* arith.  */
    0,                 /* logical.  */
    0,                 /* shift.  */
    COSTS_N_INSNS (1), /* shift_reg.  */
    COSTS_N_INSNS (1), /* arith_shift.  */
    COSTS_N_INSNS (1), /* arith_shift_reg.  */
    COSTS_N_INSNS (1), /* log_shift.  */
    COSTS_N_INSNS (1), /* log_shift_reg.  */
    0,                 /* extend.  */
    COSTS_N_INSNS (1), /* extend_arith.  */
    COSTS_N_INSNS (1), /* bfi.  */
    0,                 /* bfx.  */
    0,                 /* clz.  */
    0,                  /* rev.  */
    0,                 /* non_exec.  */
    true               /* non_exec_costs_exec.  */
  },
  {
    /* MULT SImode */
    {
      COSTS_N_INSNS (2),       /* simple.  */
      COSTS_N_INSNS (3),       /* flag_setting.  */
      COSTS_N_INSNS (2),       /* extend.  */
      COSTS_N_INSNS (2),       /* add.  */
      COSTS_N_INSNS (2),       /* extend_add.  */
      COSTS_N_INSNS (18)       /* idiv.  */
    },
    /* MULT DImode */
    {
      COSTS_N_INSNS (4),       /* simple.  */
      0,                       /* flag_setting (N/A).  */
      COSTS_N_INSNS (2),       /* extend.  */
      COSTS_N_INSNS (4),       /* add.  */
      COSTS_N_INSNS (2),       /* extend_add.  */
      COSTS_N_INSNS (34)       /* idiv.  */
    }
  },
  /* LD/ST */
  {
    COSTS_N_INSNS (3),         /* load.  */
    COSTS_N_INSNS (3),         /* load_sign_extend.  */
    COSTS_N_INSNS (3),         /* ldrd.  */
    COSTS_N_INSNS (2),         /* ldm_1st.  */
    1,                         /* ldm_regs_per_insn_1st.  */
    2,                         /* ldm_regs_per_insn_subsequent.  */
    COSTS_N_INSNS (4),         /* loadf.  */
    COSTS_N_INSNS (4),         /* loadd.  */
    COSTS_N_INSNS (5),         /* load_unaligned.  */
    0,                         /* store.  */
    0,                         /* strd.  */
    0,                         /* stm_1st.  */
    1,                         /* stm_regs_per_insn_1st.  */
    2,                         /* stm_regs_per_insn_subsequent.  */
    0,                         /* storef.  */
    0,                         /* stored.  */
    COSTS_N_INSNS (1),         /* store_unaligned.  */
    COSTS_N_INSNS (1),         /* loadv.  */
    COSTS_N_INSNS (1)          /* storev.  */
  },
  {
    /* FP SFmode */
    {
      COSTS_N_INSNS (6),      /* div.  */
      COSTS_N_INSNS (1),       /* mult.  */
      COSTS_N_INSNS (2),       /* mult_addsub.  */
      COSTS_N_INSNS (2),       /* fma.  */
      COSTS_N_INSNS (1),       /* addsub.  */
      0,                       /* fpconst.  */
      0,                       /* neg.  */
      0,                       /* compare.  */
      COSTS_N_INSNS (1),       /* widen.  */
      COSTS_N_INSNS (1),       /* narrow.  */
      COSTS_N_INSNS (1),       /* toint.  */
      COSTS_N_INSNS (1),       /* fromint.  */
      COSTS_N_INSNS (1)        /* roundint.  */
    },
    /* FP DFmode */
    {
      COSTS_N_INSNS (11),      /* div.  */
      COSTS_N_INSNS (1),       /* mult.  */
      COSTS_N_INSNS (2),       /* mult_addsub.  */
      COSTS_N_INSNS (2),       /* fma.  */
      COSTS_N_INSNS (1),       /* addsub.  */
      0,                       /* fpconst.  */
      0,                       /* neg.  */
      0,                       /* compare.  */
      COSTS_N_INSNS (1),       /* widen.  */
      COSTS_N_INSNS (1),       /* narrow.  */
      COSTS_N_INSNS (1),       /* toint.  */
      COSTS_N_INSNS (1),       /* fromint.  */
      COSTS_N_INSNS (1)        /* roundint.  */
    }
  },
  /* Vector */
  {
    COSTS_N_INSNS (1)  /* alu.  */
  }
};
```

## 1.2 address_cost_table
```
static const struct cpu_addrcost_table generic_addrcost_table =
{
    {
      1, /* hi  */
      0, /* si  */
      0, /* di  */
      1, /* ti  */
    },
  0, /* pre_modify  */
  0, /* post_modify  */
  0, /* register_offset  */
  0, /* register_sextend  */
  0, /* register_zextend  */
  0 /* imm_offset  */
};
```

## 1.3 regmove_cost_table
```
static const struct cpu_regmove_cost generic_regmove_cost =
{
  1, /* GP2GP  */
  /* Avoid the use of slow int<->fp moves for spilling by setting
     their cost higher than memmov_cost.  */
  5, /* GP2FP  */
  5, /* FP2GP  */
  2 /* FP2FP  */
};

```
## 1.4 vector_cost_table
```
static const struct cpu_vector_cost generic_vector_cost =
{
  1, /* scalar_int_stmt_cost  */
  1, /* scalar_fp_stmt_cost  */
  1, /* scalar_load_cost  */
  1, /* scalar_store_cost  */
  1, /* vec_int_stmt_cost  */
  1, /* vec_fp_stmt_cost  */
  2, /* vec_permute_cost  */
  2, /* vec_to_scalar_cost  */
  1, /* scalar_to_vec_cost  */
  1, /* vec_align_load_cost  */
  1, /* vec_unalign_load_cost  */
  1, /* vec_unalign_store_cost  */
  1, /* vec_store_cost  */
  3, /* cond_taken_branch_cost  */
  1 /* cond_not_taken_branch_cost  */
};
```

## 1.5 branch_cost_table
```
static const struct cpu_branch_cost generic_branch_cost =
{
  1,  /* Predictable.  */
  3   /* Unpredictable.  */
};
```

# 2. Modes

## 2.1 Generic approximation modes

```
static const cpu_approx_modes generic_approx_modes =
{
  AARCH64_APPROX_NONE,	/* division  */
  AARCH64_APPROX_NONE,	/* sqrt  */
  AARCH64_APPROX_NONE	/* recip_sqrt  */
};
```

## 2.2 Auto Prefetch modes
```
/* An enum specifying how to take into account CPU autoprefetch capabilities
   during instruction scheduling:
   - AUTOPREFETCHER_OFF: Do not take autoprefetch capabilities into account.
   - AUTOPREFETCHER_WEAK: Attempt to sort sequences of loads/store in order of
   offsets but allow the pipeline hazard recognizer to alter that order to
   maximize multi-issue opportunities.
   - AUTOPREFETCHER_STRONG: Attempt to sort sequences of loads/store in order of
   offsets and prefer this even if it restricts multi-issue opportunities.  */
   
  enum {
    AUTOPREFETCHER_OFF,
    AUTOPREFETCHER_WEAK,
    AUTOPREFETCHER_STRONG
  } autoprefetcher_model;
```

## 2.3 Generic prefetch settings

```
static const cpu_prefetch_tune generic_prefetch_tune =
{
  0,			/* num_slots  */
  -1,			/* l1_cache_size  */
  -1,			/* l1_cache_line_size  */
  -1,			/* l2_cache_size  */
  true,			/* prefetch_dynamic_strides */
  -1,			/* minimum_stride */
  -1			/* default_opt_level  */
};
```

# 3. Tunings

## 3.1 generic tunings
```
static const struct tune_params generic_tunings =
{
  &cortexa57_extra_costs,
  &generic_addrcost_table,
  &generic_regmove_cost,
  &generic_vector_cost,
  &generic_branch_cost,
  &generic_approx_modes,
  SVE_NOT_IMPLEMENTED, /* sve_width  */
  4, /* memmov_cost  */
  2, /* issue_rate  */
  (AARCH64_FUSE_AES_AESMC | AARCH64_FUSE_CMP_BRANCH), /* fusible_ops  */
  "16:12",	/* function_align.  */
  "4",	/* jump_align.  */
  "8",	/* loop_align.  */
  2,	/* int_reassoc_width.  */
  4,	/* fp_reassoc_width.  */
  1,	/* vec_reassoc_width.  */
  2,	/* min_div_recip_mul_sf.  */
  2,	/* min_div_recip_mul_df.  */
  0,	/* max_case_values.  */
  tune_params::AUTOPREFETCHER_WEAK,	/* autoprefetcher_model.  */
  (AARCH64_EXTRA_TUNE_NONE),	/* tune_flags.  */
  &generic_prefetch_tune
};

```

## 3.2 Pipeline descriptions

> -mtune=cortexa57 (include cortex-a57.md)

cortex-a57.md
```
;; ARM Cortex-A57 pipeline description
;; Copyright (C) 2014-2017 Free Software Foundation, Inc.
;;
;; This file is part of GCC.
;;
;; GCC is free software; you can redistribute it and/or modify it
;; under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 3, or (at your option)
;; any later version.
;;
;; GCC is distributed in the hope that it will be useful, but
;; WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;; General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with GCC; see the file COPYING3.  If not see
;; <http://www.gnu.org/licenses/>.

(define_automaton "cortex_a57")

(define_attr "cortex_a57_neon_type"
  "neon_abd, neon_abd_q, neon_arith_acc, neon_arith_acc_q,
   neon_arith_basic, neon_arith_complex,
   neon_reduc_add_acc, neon_multiply, neon_multiply_q,
   neon_multiply_long, neon_mla, neon_mla_q, neon_mla_long,
   neon_sat_mla_long, neon_shift_acc, neon_shift_imm_basic,
   neon_shift_imm_complex,
   neon_shift_reg_basic, neon_shift_reg_basic_q, neon_shift_reg_complex,
   neon_shift_reg_complex_q, neon_fp_negabs, neon_fp_arith,
   neon_fp_arith_q, neon_fp_reductions_q, neon_fp_cvt_int,
   neon_fp_cvt_int_q, neon_fp_cvt16, neon_fp_minmax, neon_fp_mul,
   neon_fp_mul_q, neon_fp_mla, neon_fp_mla_q, neon_fp_recpe_rsqrte,
   neon_fp_recpe_rsqrte_q, neon_fp_recps_rsqrts, neon_fp_recps_rsqrts_q,
   neon_bitops, neon_bitops_q, neon_from_gp,
   neon_from_gp_q, neon_move, neon_tbl3_tbl4, neon_zip_q, neon_to_gp,
   neon_load_a, neon_load_b, neon_load_c, neon_load_d, neon_load_e,
   neon_load_f, neon_store_a, neon_store_b, neon_store_complex,
   unknown"
  (cond [
          (eq_attr "type" "neon_abd, neon_abd_long")
            (const_string "neon_abd")
          (eq_attr "type" "neon_abd_q")
            (const_string "neon_abd_q")
          (eq_attr "type" "neon_arith_acc, neon_reduc_add_acc,\
                           neon_reduc_add_acc_q")
            (const_string "neon_arith_acc")
          (eq_attr "type" "neon_arith_acc_q")
            (const_string "neon_arith_acc_q")
          (eq_attr "type" "neon_add, neon_add_q, neon_add_long,\
                           neon_add_widen, neon_neg, neon_neg_q,\
                           neon_reduc_add, neon_reduc_add_q,\
                           neon_reduc_add_long, neon_sub, neon_sub_q,\
                           neon_sub_long, neon_sub_widen, neon_logic,\
                           neon_logic_q, neon_tst, neon_tst_q")
            (const_string "neon_arith_basic")
          (eq_attr "type" "neon_abs, neon_abs_q, neon_add_halve_narrow_q,\
                           neon_add_halve, neon_add_halve_q,\
                           neon_sub_halve, neon_sub_halve_q, neon_qabs,\
                           neon_qabs_q, neon_qadd, neon_qadd_q, neon_qneg,\
                           neon_qneg_q, neon_qsub, neon_qsub_q,\
                           neon_sub_halve_narrow_q,\
                           neon_compare, neon_compare_q,\
                           neon_compare_zero, neon_compare_zero_q,\
                           neon_minmax, neon_minmax_q, neon_reduc_minmax,\
                           neon_reduc_minmax_q")
            (const_string "neon_arith_complex")

          (eq_attr "type" "neon_mul_b, neon_mul_h, neon_mul_s,\
                           neon_mul_h_scalar, neon_mul_s_scalar,\
                           neon_sat_mul_b, neon_sat_mul_h,\
                           neon_sat_mul_s, neon_sat_mul_h_scalar,\
                           neon_sat_mul_s_scalar,\
                           neon_mul_b_long, neon_mul_h_long,\
                           neon_mul_s_long, neon_mul_d_long,\
                           neon_mul_h_scalar_long, neon_mul_s_scalar_long,\
                           neon_sat_mul_b_long, neon_sat_mul_h_long,\
                           neon_sat_mul_s_long, neon_sat_mul_h_scalar_long,\
                           neon_sat_mul_s_scalar_long")
            (const_string "neon_multiply")
          (eq_attr "type" "neon_mul_b_q, neon_mul_h_q, neon_mul_s_q,\
                           neon_mul_h_scalar_q, neon_mul_s_scalar_q,\
                           neon_sat_mul_b_q, neon_sat_mul_h_q,\
                           neon_sat_mul_s_q, neon_sat_mul_h_scalar_q,\
                           neon_sat_mul_s_scalar_q")
            (const_string "neon_multiply_q")
          (eq_attr "type" "neon_mla_b, neon_mla_h, neon_mla_s,\
                           neon_mla_h_scalar, neon_mla_s_scalar,\
                           neon_mla_b_long, neon_mla_h_long,\
                           neon_mla_s_long,\
                           neon_mla_h_scalar_long, neon_mla_s_scalar_long")
            (const_string "neon_mla")
          (eq_attr "type" "neon_mla_b_q, neon_mla_h_q, neon_mla_s_q,\
                           neon_mla_h_scalar_q, neon_mla_s_scalar_q")
            (const_string "neon_mla_q")
          (eq_attr "type" "neon_sat_mla_b_long, neon_sat_mla_h_long,\
                           neon_sat_mla_s_long, neon_sat_mla_h_scalar_long,\
                           neon_sat_mla_s_scalar_long")
            (const_string "neon_sat_mla_long")

          (eq_attr "type" "neon_shift_acc, neon_shift_acc_q")
            (const_string "neon_shift_acc")
          (eq_attr "type" "neon_shift_imm, neon_shift_imm_q,\
                           neon_shift_imm_narrow_q, neon_shift_imm_long")
            (const_string "neon_shift_imm_basic")
          (eq_attr "type" "neon_sat_shift_imm, neon_sat_shift_imm_q,\
                           neon_sat_shift_imm_narrow_q")
            (const_string "neon_shift_imm_complex")
          (eq_attr "type" "neon_shift_reg")
            (const_string "neon_shift_reg_basic")
          (eq_attr "type" "neon_shift_reg_q")
            (const_string "neon_shift_reg_basic_q")
          (eq_attr "type" "neon_sat_shift_reg")
            (const_string "neon_shift_reg_complex")
          (eq_attr "type" "neon_sat_shift_reg_q")
            (const_string "neon_shift_reg_complex_q")

          (eq_attr "type" "neon_fp_neg_s, neon_fp_neg_s_q,\
                           neon_fp_abs_s, neon_fp_abs_s_q,\
                           neon_fp_neg_d, neon_fp_neg_d_q,\
                           neon_fp_abs_d, neon_fp_abs_d_q")
            (const_string "neon_fp_negabs")
          (eq_attr "type" "neon_fp_addsub_s, neon_fp_abd_s,\
                           neon_fp_reduc_add_s, neon_fp_compare_s,\
                           neon_fp_minmax_s, neon_fp_round_s,\
                           neon_fp_addsub_d, neon_fp_abd_d,\
                           neon_fp_reduc_add_d, neon_fp_compare_d,\
                           neon_fp_minmax_d, neon_fp_round_d,\
                           neon_fp_reduc_minmax_s, neon_fp_reduc_minmax_d")
            (const_string "neon_fp_arith")
          (eq_attr "type" "neon_fp_addsub_s_q, neon_fp_abd_s_q,\
                           neon_fp_reduc_add_s_q, neon_fp_compare_s_q,\
                           neon_fp_minmax_s_q, neon_fp_round_s_q,\
                           neon_fp_addsub_d_q, neon_fp_abd_d_q,\
                           neon_fp_reduc_add_d_q, neon_fp_compare_d_q,\
                           neon_fp_minmax_d_q, neon_fp_round_d_q")
            (const_string "neon_fp_arith_q")
          (eq_attr "type" "neon_fp_reduc_minmax_s_q,\
                           neon_fp_reduc_minmax_d_q,\
                           neon_fp_reduc_add_s_q, neon_fp_reduc_add_d_q")
            (const_string "neon_fp_reductions_q")
          (eq_attr "type" "neon_fp_to_int_s, neon_int_to_fp_s,\
                           neon_fp_to_int_d, neon_int_to_fp_d")
            (const_string "neon_fp_cvt_int")
          (eq_attr "type" "neon_fp_to_int_s_q, neon_int_to_fp_s_q,\
                           neon_fp_to_int_d_q, neon_int_to_fp_d_q")
            (const_string "neon_fp_cvt_int_q")
          (eq_attr "type" "neon_fp_cvt_narrow_s_q, neon_fp_cvt_widen_h")
            (const_string "neon_fp_cvt16")
          (eq_attr "type" "neon_fp_mul_s, neon_fp_mul_s_scalar,\
                           neon_fp_mul_d")
            (const_string "neon_fp_mul")
          (eq_attr "type" "neon_fp_mul_s_q, neon_fp_mul_s_scalar_q,\
                           neon_fp_mul_d_q, neon_fp_mul_d_scalar_q")
            (const_string "neon_fp_mul_q")
          (eq_attr "type" "neon_fp_mla_s, neon_fp_mla_s_scalar,\
                           neon_fp_mla_d")
            (const_string "neon_fp_mla")
          (eq_attr "type" "neon_fp_mla_s_q, neon_fp_mla_s_scalar_q,
                           neon_fp_mla_d_q, neon_fp_mla_d_scalar_q")
            (const_string "neon_fp_mla_q")
          (eq_attr "type" "neon_fp_recpe_s, neon_fp_rsqrte_s,\
                           neon_fp_recpx_s,\
                           neon_fp_recpe_d, neon_fp_rsqrte_d,\
                           neon_fp_recpx_d")
            (const_string "neon_fp_recpe_rsqrte")
          (eq_attr "type" "neon_fp_recpe_s_q, neon_fp_rsqrte_s_q,\
                           neon_fp_recpx_s_q,\
                           neon_fp_recpe_d_q, neon_fp_rsqrte_d_q,\
                           neon_fp_recpx_d_q")
            (const_string "neon_fp_recpe_rsqrte_q")
          (eq_attr "type" "neon_fp_recps_s, neon_fp_rsqrts_s,\
                           neon_fp_recps_d, neon_fp_rsqrts_d")
            (const_string "neon_fp_recps_rsqrts")
          (eq_attr "type" "neon_fp_recps_s_q, neon_fp_rsqrts_s_q,\
                           neon_fp_recps_d_q, neon_fp_rsqrts_d_q")
            (const_string "neon_fp_recps_rsqrts_q")
          (eq_attr "type" "neon_bsl, neon_cls, neon_cnt,\
                           neon_rev, neon_permute, neon_rbit,\
                           neon_tbl1, neon_tbl2, neon_zip,\
                           neon_dup, neon_dup_q, neon_ext, neon_ext_q,\
                           neon_move, neon_move_q, neon_move_narrow_q")
            (const_string "neon_bitops")
          (eq_attr "type" "neon_bsl_q, neon_cls_q, neon_cnt_q,\
                           neon_rev_q, neon_permute_q, neon_rbit_q")
            (const_string "neon_bitops_q")
          (eq_attr "type" "neon_from_gp,f_mcr,f_mcrr")
            (const_string "neon_from_gp")
          (eq_attr "type" "neon_from_gp_q")
            (const_string "neon_from_gp_q")
          (eq_attr "type" "neon_tbl3, neon_tbl4")
            (const_string "neon_tbl3_tbl4")
          (eq_attr "type" "neon_zip_q")
            (const_string "neon_zip_q")
          (eq_attr "type" "neon_to_gp, neon_to_gp_q,f_mrc,f_mrrc")
            (const_string "neon_to_gp")

          (eq_attr "type" "f_loads, f_loadd,\
                           neon_load1_1reg, neon_load1_1reg_q,\
                           neon_load1_2reg, neon_load1_2reg_q")
            (const_string "neon_load_a")
          (eq_attr "type" "neon_load1_3reg, neon_load1_3reg_q,\
                           neon_load1_4reg, neon_load1_4reg_q")
            (const_string "neon_load_b")
          (eq_attr "type" "neon_ldp, neon_ldp_q,\
                           neon_load1_one_lane, neon_load1_one_lane_q,\
                           neon_load1_all_lanes, neon_load1_all_lanes_q,\
                           neon_load2_2reg, neon_load2_2reg_q,\
                           neon_load2_all_lanes, neon_load2_all_lanes_q")
            (const_string "neon_load_c")
          (eq_attr "type" "neon_load2_4reg, neon_load2_4reg_q,\
                           neon_load3_3reg, neon_load3_3reg_q,\
                           neon_load3_one_lane, neon_load3_one_lane_q,\
                           neon_load4_4reg, neon_load4_4reg_q")
            (const_string "neon_load_d")
          (eq_attr "type" "neon_load2_one_lane, neon_load2_one_lane_q,\
                           neon_load3_all_lanes, neon_load3_all_lanes_q,\
                           neon_load4_all_lanes, neon_load4_all_lanes_q")
            (const_string "neon_load_e")
          (eq_attr "type" "neon_load4_one_lane, neon_load4_one_lane_q")
            (const_string "neon_load_f")

          (eq_attr "type" "f_stores, f_stored,\
                           neon_store1_1reg")
            (const_string "neon_store_a")
          (eq_attr "type" "neon_store1_2reg, neon_store1_1reg_q")
            (const_string "neon_store_b")
          (eq_attr "type" "neon_stp, neon_stp_q,\
                           neon_store1_3reg, neon_store1_3reg_q,\
                           neon_store3_3reg, neon_store3_3reg_q,\
                           neon_store2_4reg, neon_store2_4reg_q,\
                           neon_store4_4reg, neon_store4_4reg_q,\
                           neon_store2_2reg, neon_store2_2reg_q,\
                           neon_store3_one_lane, neon_store3_one_lane_q,\
                           neon_store4_one_lane, neon_store4_one_lane_q,\
                           neon_store1_4reg, neon_store1_4reg_q,\
                           neon_store1_one_lane, neon_store1_one_lane_q,\
                           neon_store2_one_lane, neon_store2_one_lane_q")
            (const_string "neon_store_complex")]
          (const_string "unknown")))

;; The Cortex-A57 core is modelled as a triple issue pipeline that has
;; the following functional units.
;; 1.  Two pipelines for integer operations: SX1, SX2

(define_cpu_unit "ca57_sx1_issue" "cortex_a57")
(define_reservation "ca57_sx1" "ca57_sx1_issue")

(define_cpu_unit "ca57_sx2_issue" "cortex_a57")
(define_reservation "ca57_sx2" "ca57_sx2_issue")

;; 2.  One pipeline for complex integer operations: MX

(define_cpu_unit "ca57_mx_issue"
                 "cortex_a57")
(define_reservation "ca57_mx" "ca57_mx_issue")
(define_reservation "ca57_mx_block" "ca57_mx_issue")

;; 3.  Two asymmetric pipelines for Neon and FP operations: CX1, CX2
(define_automaton "cortex_a57_cx")

(define_cpu_unit "ca57_cx1_issue"
                 "cortex_a57_cx")
(define_cpu_unit "ca57_cx2_issue"
                 "cortex_a57_cx")

(define_reservation "ca57_cx1" "ca57_cx1_issue")

(define_reservation "ca57_cx2" "ca57_cx2_issue")
(define_reservation "ca57_cx2_block" "ca57_cx2_issue*2")

;; 4.  One pipeline for branch operations: BX

(define_cpu_unit "ca57_bx_issue" "cortex_a57")
(define_reservation "ca57_bx" "ca57_bx_issue")

;; 5.  Two pipelines for load and store operations: LS1, LS2.  The most
;;     valuable thing we can do is force a structural hazard to split
;;     up loads/stores.

(define_cpu_unit "ca57_ls_issue" "cortex_a57")
(define_cpu_unit "ca57_ldr, ca57_str" "cortex_a57")
(define_reservation "ca57_load_model" "ca57_ls_issue,ca57_ldr*2")
(define_reservation "ca57_store_model" "ca57_ls_issue,ca57_str")

;; Block all issue queues.

(define_reservation "ca57_block" "ca57_cx1_issue + ca57_cx2_issue
                                  + ca57_mx_issue + ca57_sx1_issue
                                  + ca57_sx2_issue + ca57_ls_issue")

;; Simple Execution Unit:
;;
;; Simple ALU without shift
(define_insn_reservation "cortex_a57_alu" 2
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "type" "alu_imm,alus_imm,logic_imm,logics_imm,\
                        alu_sreg,alus_sreg,logic_reg,logics_reg,\
                        adc_imm,adcs_imm,adc_reg,adcs_reg,\
                        adr,bfx,extend,clz,rbit,rev,alu_dsp_reg,\
                        rotate_imm,shift_imm,shift_reg,\
                        mov_imm,mov_reg,\
                        mvn_imm,mvn_reg,\
                        mrs,multiple,no_insn"))
  "ca57_sx1|ca57_sx2")

;; ALU ops with immediate shift
(define_insn_reservation "cortex_a57_alu_shift" 3
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "type" "bfm,\
                        alu_shift_imm,alus_shift_imm,\
                        crc,logic_shift_imm,logics_shift_imm,\
                        mov_shift,mvn_shift"))
  "ca57_mx")

;; Multi-Cycle Execution Unit:
;;
;; ALU ops with register controlled shift
(define_insn_reservation "cortex_a57_alu_shift_reg" 3
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "type" "alu_shift_reg,alus_shift_reg,\
                        logic_shift_reg,logics_shift_reg,\
                        mov_shift_reg,mvn_shift_reg"))
   "ca57_mx")

;; All multiplies
;; TODO: AArch32 and AArch64 have different behavior
(define_insn_reservation "cortex_a57_mult32" 3
  (and (eq_attr "tune" "cortexa57")
       (ior (eq_attr "mul32" "yes")
            (eq_attr "mul64" "yes")))
  "ca57_mx")

;; Integer divide
(define_insn_reservation "cortex_a57_div" 10
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "type" "udiv,sdiv"))
  "ca57_mx_issue,ca57_mx_block*3")

;; Block all issue pipes for a cycle
(define_insn_reservation "cortex_a57_block" 1
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "type" "block"))
  "ca57_block")

;; Branch execution Unit
;;
;; Branches take one issue slot.
;; No latency as there is no result
(define_insn_reservation "cortex_a57_branch" 0
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "type" "branch"))
  "ca57_bx")

;; Load-store execution Unit
;;
;; Loads of up to two words.
(define_insn_reservation "cortex_a57_load1" 5
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "type" "load_byte,load1,load2"))
  "ca57_load_model")

;; Loads of three or four words.
(define_insn_reservation "cortex_a57_load3" 5
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "type" "load3,load4"))
  "ca57_ls_issue*2,ca57_load_model")

;; Stores of up to two words.
(define_insn_reservation "cortex_a57_store1" 0
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "type" "store1,store2"))
  "ca57_store_model")

;; Stores of three or four words.
(define_insn_reservation "cortex_a57_store3" 0
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "type" "store3,store4"))
  "ca57_ls_issue*2,ca57_store_model")

;; Advanced SIMD Unit - Integer Arithmetic Instructions.

(define_insn_reservation  "cortex_a57_neon_abd" 5
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_abd"))
  "ca57_cx1|ca57_cx2")

(define_insn_reservation  "cortex_a57_neon_abd_q" 5
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_abd_q"))
  "ca57_cx1+ca57_cx2")

(define_insn_reservation  "cortex_a57_neon_aba" 7
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_arith_acc"))
  "ca57_cx2")

(define_insn_reservation  "cortex_a57_neon_aba_q" 8
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_arith_acc_q"))
  "ca57_cx2+(ca57_cx2_issue,ca57_cx2)")

(define_insn_reservation  "cortex_a57_neon_arith_basic" 4
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_arith_basic"))
  "ca57_cx1|ca57_cx2")

(define_insn_reservation  "cortex_a57_neon_arith_complex" 5
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_arith_complex"))
  "ca57_cx1|ca57_cx2")

;; Integer Multiply Instructions.

(define_insn_reservation "cortex_a57_neon_multiply" 6
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_multiply"))
  "ca57_cx1")

(define_insn_reservation "cortex_a57_neon_multiply_q" 7
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_multiply_q"))
  "ca57_cx1+(ca57_cx1_issue,ca57_cx1)")

(define_insn_reservation "cortex_a57_neon_mla" 6
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_mla"))
  "ca57_cx1")

(define_insn_reservation "cortex_a57_neon_mla_q" 7
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_mla_q"))
  "ca57_cx1+(ca57_cx1_issue,ca57_cx1)")

(define_insn_reservation "cortex_a57_neon_sat_mla_long" 6
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_sat_mla_long"))
  "ca57_cx1")

;; Integer Shift Instructions.

(define_insn_reservation
  "cortex_a57_neon_shift_acc" 7
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_shift_acc"))
  "ca57_cx2")

(define_insn_reservation
  "cortex_a57_neon_shift_imm_basic" 4
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_shift_imm_basic"))
  "ca57_cx2")

(define_insn_reservation
  "cortex_a57_neon_shift_imm_complex" 5
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_shift_imm_complex"))
  "ca57_cx2")

(define_insn_reservation
  "cortex_a57_neon_shift_reg_basic" 4
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_shift_reg_basic"))
  "ca57_cx2")

(define_insn_reservation
  "cortex_a57_neon_shift_reg_basic_q" 5
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_shift_reg_basic_q"))
  "ca57_cx2+(ca57_cx2_issue,ca57_cx2)")

(define_insn_reservation
  "cortex_a57_neon_shift_reg_complex" 5
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_shift_reg_complex"))
  "ca57_cx2")

(define_insn_reservation
  "cortex_a57_neon_shift_reg_complex_q" 6
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_shift_reg_complex_q"))
  "ca57_cx2+(ca57_cx2_issue,ca57_cx2)")

;; Floating Point Instructions.

(define_insn_reservation
  "cortex_a57_neon_fp_negabs" 4
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_fp_negabs"))
  "(ca57_cx1|ca57_cx2)")

(define_insn_reservation
  "cortex_a57_neon_fp_arith" 6
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_fp_arith"))
  "(ca57_cx1|ca57_cx2)")

(define_insn_reservation
  "cortex_a57_neon_fp_arith_q" 6
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_fp_arith_q"))
  "(ca57_cx1+ca57_cx2)")

(define_insn_reservation
  "cortex_a57_neon_fp_reductions_q" 10
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_fp_reductions_q"))
  "(ca57_cx1+ca57_cx2),(ca57_cx1|ca57_cx2)")

(define_insn_reservation
  "cortex_a57_neon_fp_cvt_int" 6
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_fp_cvt_int"))
  "(ca57_cx1|ca57_cx2)")

(define_insn_reservation
  "cortex_a57_neon_fp_cvt_int_q" 6
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_fp_cvt_int_q"))
  "(ca57_cx1+ca57_cx2)")

(define_insn_reservation
  "cortex_a57_neon_fp_cvt16" 10
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_fp_cvt16"))
  "(ca57_cx1_issue+ca57_cx2_issue),(ca57_cx1|ca57_cx2)")

(define_insn_reservation
  "cortex_a57_neon_fp_mul" 5
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_fp_mul"))
  "(ca57_cx1|ca57_cx2)")

(define_insn_reservation
  "cortex_a57_neon_fp_mul_q" 5
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_fp_mul_q"))
  "(ca57_cx1+ca57_cx2)")

(define_insn_reservation
  "cortex_a57_neon_fp_mla" 9
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_fp_mla"))
  "(ca57_cx1,ca57_cx1)|(ca57_cx2,ca57_cx2)")

(define_insn_reservation
  "cortex_a57_neon_fp_mla_q" 9
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_fp_mla_q"))
  "(ca57_cx1+ca57_cx2),(ca57_cx1,ca57_cx2)")

(define_insn_reservation
  "cortex_a57_neon_fp_recpe_rsqrte" 6
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_fp_recpe_rsqrte"))
  "(ca57_cx1|ca57_cx2)")

(define_insn_reservation
  "cortex_a57_neon_fp_recpe_rsqrte_q" 6
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_fp_recpe_rsqrte_q"))
  "(ca57_cx1+ca57_cx2)")

(define_insn_reservation
  "cortex_a57_neon_fp_recps_rsqrts" 10
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_fp_recps_rsqrts"))
  "(ca57_cx1|ca57_cx2)")

(define_insn_reservation
  "cortex_a57_neon_fp_recps_rsqrts_q" 10
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_fp_recps_rsqrts_q"))
  "(ca57_cx1+ca57_cx2)")

;; Miscellaneous Instructions.

(define_insn_reservation
  "cortex_a57_neon_bitops" 4
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_bitops"))
  "(ca57_cx1|ca57_cx2)")

(define_insn_reservation
  "cortex_a57_neon_bitops_q" 4
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_bitops_q"))
  "(ca57_cx1+ca57_cx2)")

(define_insn_reservation
  "cortex_a57_neon_from_gp" 9
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_from_gp"))
  "(ca57_ls_issue+ca57_cx1_issue,ca57_cx1)
               |(ca57_ls_issue+ca57_cx2_issue,ca57_cx2)")

(define_insn_reservation
  "cortex_a57_neon_from_gp_q" 9
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_from_gp_q"))
  "(ca57_ls_issue+ca57_cx1_issue,ca57_cx1)
               +(ca57_ls_issue+ca57_cx2_issue,ca57_cx2)")

(define_insn_reservation
  "cortex_a57_neon_tbl3_tbl4" 7
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_tbl3_tbl4"))
  "(ca57_cx1_issue,ca57_cx1)
               +(ca57_cx2_issue,ca57_cx2)")

(define_insn_reservation
  "cortex_a57_neon_zip_q" 7
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_zip_q"))
  "(ca57_cx1_issue,ca57_cx1)
               +(ca57_cx2_issue,ca57_cx2)")

(define_insn_reservation
  "cortex_a57_neon_to_gp" 7
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_to_gp"))
  "((ca57_ls_issue+ca57_sx1_issue),ca57_sx1)
   |((ca57_ls_issue+ca57_sx2_issue),ca57_sx2)")

;; Load Instructions.

(define_insn_reservation
  "cortex_a57_neon_load_a" 6
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_load_a"))
  "ca57_load_model")

(define_insn_reservation
  "cortex_a57_neon_load_b" 7
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_load_b"))
  "ca57_ls_issue,ca57_ls_issue+ca57_ldr,ca57_ldr*2")

(define_insn_reservation
  "cortex_a57_neon_load_c" 9
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_load_c"))
  "ca57_load_model+(ca57_cx1|ca57_cx2)")

(define_insn_reservation
  "cortex_a57_neon_load_d" 11
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_load_d"))
  "ca57_cx1_issue+ca57_cx2_issue,
   ca57_ls_issue+ca57_ls_issue,ca57_ldr*2")

(define_insn_reservation
  "cortex_a57_neon_load_e" 9
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_load_e"))
  "ca57_load_model+(ca57_cx1|ca57_cx2)")

(define_insn_reservation
  "cortex_a57_neon_load_f" 11
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_load_f"))
  "ca57_cx1_issue+ca57_cx2_issue,
   ca57_ls_issue+ca57_ls_issue,ca57_ldr*2")

;; Store Instructions.

(define_insn_reservation
  "cortex_a57_neon_store_a" 0
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_store_a"))
  "ca57_store_model")

(define_insn_reservation
  "cortex_a57_neon_store_b" 0
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_store_b"))
  "ca57_store_model")

;; These block issue for a number of cycles proportional to the number
;; of 64-bit chunks they will store, we don't attempt to model that
;; precisely, treat them as blocking execution for two cycles when
;; issued.
(define_insn_reservation
  "cortex_a57_neon_store_complex" 0
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "cortex_a57_neon_type" "neon_store_complex"))
  "ca57_block*2")

;; Floating-Point Operations.

(define_insn_reservation "cortex_a57_fp_const" 4
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "type" "fconsts,fconstd"))
  "(ca57_cx1|ca57_cx2)")

(define_insn_reservation "cortex_a57_fp_add_sub" 6
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "type" "fadds,faddd"))
  "(ca57_cx1|ca57_cx2)")

(define_insn_reservation "cortex_a57_fp_mul" 6
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "type" "fmuls,fmuld"))
  "(ca57_cx1|ca57_cx2)")

(define_insn_reservation "cortex_a57_fp_mac" 10
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "type" "fmacs,ffmas,fmacd,ffmad"))
  "(ca57_cx1,nothing,nothing,ca57_cx1) \
   |(ca57_cx2,nothing,nothing,ca57_cx2)")

(define_insn_reservation "cortex_a57_fp_cvt" 6
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "type" "f_cvt,f_cvtf2i,f_cvti2f"))
  "(ca57_cx1|ca57_cx2)")

(define_insn_reservation "cortex_a57_fp_cmp" 7
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "type" "fcmps,fcmpd,fccmps,fccmpd"))
  "ca57_cx2")

(define_insn_reservation "cortex_a57_fp_arith" 4
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "type" "ffariths,ffarithd"))
  "(ca57_cx1|ca57_cx2)")

(define_insn_reservation "cortex_a57_fp_cpys" 4
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "type" "fmov,fcsel"))
  "(ca57_cx1|ca57_cx2)")

(define_insn_reservation "cortex_a57_fp_divs" 12
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "type" "fdivs, fsqrts,\
                        neon_fp_div_s, neon_fp_sqrt_s"))
  "ca57_cx2_block*5")

(define_insn_reservation "cortex_a57_fp_divd" 16
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "type" "fdivd, fsqrtd, neon_fp_div_d, neon_fp_sqrt_d"))
  "ca57_cx2_block*3")

(define_insn_reservation "cortex_a57_neon_fp_div_q" 20
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "type" "fdivd, fsqrtd,\
                         neon_fp_div_s_q, neon_fp_div_d_q,\
                         neon_fp_sqrt_s_q, neon_fp_sqrt_d_q"))
  "ca57_cx2_block*3")

(define_insn_reservation "cortex_a57_crypto_simple" 3
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "type" "crypto_aese,crypto_aesmc,crypto_sha1_fast,crypto_sha256_fast"))
  "ca57_cx1")

(define_insn_reservation "cortex_a57_crypto_complex" 6
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "type" "crypto_sha1_slow,crypto_sha256_slow"))
  "ca57_cx1*2")

(define_insn_reservation "cortex_a57_crypto_xor" 6
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "type" "crypto_sha1_xor"))
  "(ca57_cx1*2)|(ca57_cx2*2)")

;; We lie with calls.  They take up all issue slots, but are otherwise
;; not harmful.
(define_insn_reservation "cortex_a57_call" 1
  (and (eq_attr "tune" "cortexa57")
       (eq_attr "type" "call"))
  "ca57_sx1_issue+ca57_sx2_issue+ca57_cx1_issue+ca57_cx2_issue\
    +ca57_mx_issue+ca57_bx_issue+ca57_ls_issue"
)

;; Simple execution unit bypasses
(define_bypass 1 "cortex_a57_alu"
                 "cortex_a57_alu,cortex_a57_alu_shift,cortex_a57_alu_shift_reg")
(define_bypass 2 "cortex_a57_alu_shift"
                 "cortex_a57_alu,cortex_a57_alu_shift,cortex_a57_alu_shift_reg")
(define_bypass 2 "cortex_a57_alu_shift_reg"
                 "cortex_a57_alu,cortex_a57_alu_shift,cortex_a57_alu_shift_reg")
(define_bypass 1 "cortex_a57_alu" "cortex_a57_load1,cortex_a57_load3")
(define_bypass 2 "cortex_a57_alu_shift" "cortex_a57_load1,cortex_a57_load3")
(define_bypass 2 "cortex_a57_alu_shift_reg"
                 "cortex_a57_load1,cortex_a57_load3")

;; An MLA or a MUL can feed a dependent MLA.
(define_bypass 5 "cortex_a57_neon_*mla*,cortex_a57_neon_*mul*"
                 "cortex_a57_neon_*mla*")

(define_bypass 5 "cortex_a57_fp_mul,cortex_a57_fp_mac"
                 "cortex_a57_fp_mac")

;; We don't need to care about control hazards, either the branch is
;; predicted in which case we pay no penalty, or the branch is
;; mispredicted in which case instruction scheduling will be unlikely to
;; help.
(define_bypass 1 "cortex_a57_*"
                 "cortex_a57_call,cortex_a57_branch")

;; AESE+AESMC and AESD+AESIMC pairs forward with zero latency
(define_bypass 0 "cortex_a57_crypto_simple"
                 "cortex_a57_crypto_simple"
                 "aarch_crypto_can_dual_issue")
```

- Extensions to the C Language Family
    - Atomic Builtins: Built-in functions for atomic memory access.  (GCC 需要对AARCH64支持）
        - type __sync_fetch_and_add (type *ptr, type value, ...)
        - type __sync_fetch_and_sub (type *ptr, type value, ...)
        - type __sync_fetch_and_or (type *ptr, type value, ...)
        - type __sync_fetch_and_and (type *ptr, type value, ...)
        - type __sync_fetch_and_xor (type *ptr, type value, ...)
        - type __sync_fetch_and_nand (type *ptr, type value, ...)
        - type __sync_add_and_fetch (type *ptr, type value, ...)
        - type __sync_sub_and_fetch (type *ptr, type value, ...)
        - type __sync_or_and_fetch (type *ptr, type value, ...)
        - type __sync_and_and_fetch (type *ptr, type value, ...)
        - type __sync_xor_and_fetch (type *ptr, type value, ...)
        - type __sync_nand_and_fetch (type *ptr, type value, ...)
        - bool __sync_bool_compare_and_swap (type *ptr, type oldval type newval, ...)
        - type __sync_val_compare_and_swap (type *ptr, type oldval type newval, ...)
        - __sync_synchronize (...)
        - type __sync_lock_test_and_set (type *ptr, type value, ...)
        - void __sync_lock_release (type *ptr, ...)
    - Other Builtins: Other built-in functions. （无须移植）
        - int __builtin_types_compatible_p (type1, type2)
        - type __builtin_choose_expr (const_exp, exp1, exp2)
        - int __builtin_constant_p (exp)
        - long __builtin_expect (long exp, long c)
        - void __builtin_prefetch (const void *addr, ...)
        - double __builtin_huge_val (void)
        - float __builtin_huge_valf (void)
        - long double __builtin_huge_vall (void)
        - double __builtin_inf (void)
        - float __builtin_inff (void)
        - long double __builtin_infl (void)
        - double __builtin_nan (const char *str)
        - float __builtin_nanf (const char *str)
        - long double __builtin_nanl (const char *str)
    - Target Builtins: Built-in functions specific to particular targets. （X86 需要一直到 AArch64）
        - X86(-mmmx):
            v8qi __builtin_ia32_paddb (v8qi, v8qi)
            v4hi __builtin_ia32_paddw (v4hi, v4hi)
            v2si __builtin_ia32_paddd (v2si, v2si)
            v8qi __builtin_ia32_psubb (v8qi, v8qi)
            v4hi __builtin_ia32_psubw (v4hi, v4hi)
            v2si __builtin_ia32_psubd (v2si, v2si)
            v8qi __builtin_ia32_paddsb (v8qi, v8qi)
            v4hi __builtin_ia32_paddsw (v4hi, v4hi)
            v8qi __builtin_ia32_psubsb (v8qi, v8qi)
            v4hi __builtin_ia32_psubsw (v4hi, v4hi)
            v8qi __builtin_ia32_paddusb (v8qi, v8qi)
            v4hi __builtin_ia32_paddusw (v4hi, v4hi)
            v8qi __builtin_ia32_psubusb (v8qi, v8qi)
            v4hi __builtin_ia32_psubusw (v4hi, v4hi)
            v4hi __builtin_ia32_pmullw (v4hi, v4hi)
            v4hi __builtin_ia32_pmulhw (v4hi, v4hi)
            di __builtin_ia32_pand (di, di)
            di __builtin_ia32_pandn (di,di)
            di __builtin_ia32_por (di, di)
            di __builtin_ia32_pxor (di, di)
            v8qi __builtin_ia32_pcmpeqb (v8qi, v8qi)
            v4hi __builtin_ia32_pcmpeqw (v4hi, v4hi)
            v2si __builtin_ia32_pcmpeqd (v2si, v2si)
            v8qi __builtin_ia32_pcmpgtb (v8qi, v8qi)
            v4hi __builtin_ia32_pcmpgtw (v4hi, v4hi)
            v2si __builtin_ia32_pcmpgtd (v2si, v2si)
            v8qi __builtin_ia32_punpckhbw (v8qi, v8qi)
            v4hi __builtin_ia32_punpckhwd (v4hi, v4hi)
            v2si __builtin_ia32_punpckhdq (v2si, v2si)
            v8qi __builtin_ia32_punpcklbw (v8qi, v8qi)
            v4hi __builtin_ia32_punpcklwd (v4hi, v4hi)
            v2si __builtin_ia32_punpckldq (v2si, v2si)
            v8qi __builtin_ia32_packsswb (v4hi, v4hi)
            v4hi __builtin_ia32_packssdw (v2si, v2si)
            v8qi __builtin_ia32_packuswb (v4hi, v4hi)
    - Built-in Functions for Memory Model Aware Atomic Operations
        -  __atomic_load 
        - __atomic_test_and_set 


if (!alloc_emulate_ctxt(vcpu))      // 软件模拟


"arch/x86/kvm/vmx/vmx.c" + 6823   --> intel posted interrupt  55d2375e58

if (!dump_invalid_vmcs) {
    pr_warn_ratelimited("set kvm_intel.dump_invalid_vmcs=1 to dump internal KVM state.\n");
    return;
}


https://www.dazhuanlan.com/2020/01/09/5e16cf12e11ea/

```
https://www.spinics.net/lists/kvm/maillist.html

PML is a new feature on Intel's Boardwell server platfrom targeted to reduce
overhead of dirty logging mechanism.


Intel VT 2015年推出page-modification logging(PML),
VMM可以利用EPT监控虚拟机在运行期间物理页面的修改。

在没有PML前，VMM要监控虚拟机中物理页面的修改，需要将EPT的页面结构设置为not-present或者read-only，这样会触发许多EPT violations,开销非常大。

PML建立在CPU对EPT中的accessed与dirty标志位支持上。

当启用PML时，对EPT中设置了dirty标志位的写操作都会产生一条in-memory记录，报告写操作的虚拟机物理地址，当记录写满时，触发一次VM Exit，然后VMM就可以监控被修改的页面。


https://www.spinics.net/lists/kvm/msg112904.html

```

x86_emulator_ops

TCG and KVM are entirely separate modes of operation for QEMU. If you're using KVM (via -enable-kvm on the command line) then all guest instructions are either natively executed by the host CPU or (for a few instructions mostly doing I/O to emulated devices) emulated inside the host kernel; QEMU's TCG instruction emulation (which is the code you're referring to above) is never used at all. Conversely, if you use QEMU in TCG mode (the default) then we are a pure emulator in userspace and make no use of the host CPU's hypervisor functionality. qemu-user-mode is always TCG emulation, and never KVM.

TCG和KVM是QEMU的完全独立的操作模式。如果您正在使用KVM（通过命令行上的-enable-kvm），则所有来宾指令要么由主机CPU本机执行，要么（在某些指令中主要对仿真设备进行I / O）在主机内核中进行仿真；完全不会使用QEMU的TCG指令仿真（这是您上面引用的代码）。相反，如果在TCG模式（默认）下使用QEMU，则我们是用户空间中的纯仿真器，并且不使用主机CPU的管理程序功能。qemu-user-mode始终是TCG仿真，而不是KVM。

```
I was able to carve the `arch/x86/kvm/emulate.c` code, but the emulator is
constructed in such a way that a lot of the code which enforces expected
behavior lives in the x86_emulate_ops supplied in `arch/x86/kvm/x86.c`.
Testing the emulator is still valuable, but a reproducible way to use the kvm
ops would be useful.

Any comments/suggestions are greatly appreciated.

Best,
Sam Caccavale
```



```
static const struct read_write_emulator_ops read_emultor = {
    .read_write_prepare = read_prepare,
    .read_write_emulate = read_emulate,
    .read_write_mmio = vcpu_mmio_read,
    .read_write_exit_mmio = read_exit_mmio,
};

static const struct read_write_emulator_ops write_emultor = {
    .read_write_emulate = write_emulate,
    .read_write_mmio = write_mmio,
    .read_write_exit_mmio = write_exit_mmio,
    .write = true,
};
```

QEMU:
    - User-mode emulation(qemu-aarch64): Run operating systems for any machine, on any supported architecture
    - Full-system emulation(qemu-system-aarch64): Run programs for another Linux/BSD target, on any supported architecture
    - Virtualization(qemu-kvm): Run KVM and Xen virtual machines with near native performance


 ./scripts/sign-file sha512 ./signing_key.priv ./signing_key.x509 /lib/modules/3.10.1/kernel/drivers/char/my_module.ko

```
diff --git a/arch/x86/kvm/vmx/vmx.c b/arch/x86/kvm/vmx/vmx.c
index 69536553446d..c7ee5ead1565 100644
--- a/arch/x86/kvm/vmx/vmx.c
+++ b/arch/x86/kvm/vmx/vmx.c
@@ -5829,6 +5829,7 @@  static int vmx_handle_exit(struct kvm_vcpu *vcpu)
 	}
 
 	if (unlikely(vmx->fail)) {
+		dump_vmcs();
 		vcpu->run->exit_reason = KVM_EXIT_FAIL_ENTRY;
 		vcpu->run->fail_entry.hardware_entry_failure_reason
 			= vmcs_read32(VM_INSTRUCTION_ERROR);
```


```
上面函数中vcpu_enter_guest()我们前面讲过，是在kvm内核中转入客户模式的函数，他处于while循环中，也就是如果不需要Qemu的协助，即r>0，那就继续循环，然后重新切换进客户系统运行，如果需要Qemu的协助，那返回值r<=0,退出循环，向上层返回r。
```

RCU.refs: https://blog.csdn.net/xueli1991/article/details/51741763


虚拟化中断 IPI优化：
https://cloud.tencent.com/developer/article/1400629


https://blog.csdn.net/carltraveler/article/details/76422829?locationNum=8&fps=1


首先根据delivery_mode选择添加的方式，基本都会调用下面两个函数：

- kvm_make_request(..,vcpu): 这个函数就是向目标vcpu添加一个reqest请求。当目标vcpu再次enter_guest时，check到这个中断请求，进行中断注入，vcpu重新运行，捕获这个中断。

- kvm_vcpu_kick(vcpu)： 这个函数的功能就是，判断vcpu是否正在物理cpu上运行，如果是，则让vcpu退出，以便进行中断注入。



http://blog.sciencenet.cn/blog-279072-1075607.html



```default_routing
{ .gsi = 0, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_IOAPIC, .pin = (0) } },
{ .gsi = 0, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_PIC_MASTER .pin = (0) } },
{ .gsi = 1, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_IOAPIC, .pin = (1) } },
{ .gsi = 1, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_PIC_MASTER .pin = (1) },
{ .gsi = 2, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_IOAPIC, .pin = (2) } },
{ .gsi = 2, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_PIC_MASTER, .pin = (2) } },
{ .gsi = 3, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_IOAPIC, .pin = (3) } },
{ .gsi = 3, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_PIC_MASTER, .pin = (3) } },
{ .gsi = 4, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_IOAPIC, .pin = (4) } },
{ .gsi = 4, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_PIC_MASTER, .pin = (4) } },
{ .gsi = 5, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_IOAPIC, .pin = (5) } },
{ .gsi = 5, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_PIC_MASTER, .pin = (5) } },
{ .gsi = 6, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_IOAPIC, .pin = (6) } },
{ .gsi = 6, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_PIC_MASTER, .pin = (6) } },
{ .gsi = 7, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_IOAPIC, .pin = (7) } },
{ .gsi = 7, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_PIC_MASTER, .pin = (7) } },

{ .gsi = 8, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_IOAPIC, .pin = (8) } },
{ .gsi = 8, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_PIC_SLAVE, .pin = (0) } },
{ .gsi = 9, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_IOAPIC, .pin = (9) } },
{ .gsi = 9, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip =  KVM_IRQCHIP_PIC_SLAVE, .pin = (1) } },
{ .gsi = 10, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_IOAPIC, .pin = (10) } } ,
{ .gsi = 10, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_PIC_SLAVE, .pin = (2) }},
{ .gsi = 11, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_IOAPIC, .pin = (11) } },
{ .gsi = 11, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_PIC_SLAVE, .pin = (3) } },
{ .gsi = 12, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_IOAPIC, .pin = (12) } },
{ .gsi = 12, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_PIC_SLAVE, .pin = (4) } },
{ .gsi = 13, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_IOAPIC, .pin = (13) } },
{ .gsi = 13, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_PIC_SLAVE, .pin = (5) } },
{ .gsi = 14, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_IOAPIC, .pin = (14) } },
{ .gsi = 14, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_PIC_SLAVE, .pin = (6) } },
{ .gsi = 15, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_IOAPIC, .pin = (15) } },
{ .gsi = 15, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_PIC_SLAVE, .pin = (7) } },

{ .gsi = 16, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_IOAPIC, .pin = (16) } },
{ .gsi = 17, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_IOAPIC, .pin = (17) } },
{ .gsi = 18, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_IOAPIC, .pin = (18) } },
{ .gsi = 19, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_IOAPIC, .pin = (19) } },
{ .gsi = 20, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_IOAPIC, .pin = (20) } },
{ .gsi = 21, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_IOAPIC, .pin = (21) } },
{ .gsi = 22, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_IOAPIC, .pin = (22) } },
{ .gsi = 23, .type = KVM_IRQ_ROUTING_IRQCHIP, .u.irqchip = { .irqchip = KVM_IRQCHIP_IOAPIC, .pin = (23) } },
```


```
vmx_handle_exit [kvm_intel]() {
  handle_wrmsr [kvm_intel]() {
    kvm_set_msr [kvm]() {
      vmx_set_msr [kvm_intel]() {
        kvm_set_msr_common [kvm]() {
          kvm_x2apic_msr_write [kvm]() {
            kvm_lapic_reg_write [kvm]();
            kvm_lapic_reg_write [kvm]() {
              kvm_irq_delivery_to_apic [kvm]() {
                kvm_irq_delivery_to_apic_fast [kvm]() {
                  kvm_apic_set_irq [kvm]() {
                    __apic_accept_irq [kvm]() {
                      vmx_deliver_posted_interrupt [kvm_intel]() {
                        physflat_send_IPI_mask() {
                          default_send_IPI_mask_sequence_phys();
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
    skip_emulated_instruction [kvm_intel]() {
      vmx_cache_reg [kvm_intel]();
      vmx_set_interrupt_shadow [kvm_intel]();
    }
  }
}
```

GVA - Guest Virtual Address，虚拟机的虚拟地址
GPA - Guest Physical Address，虚拟机的虚拟地址
GFN - Guest Frame Number，虚拟机的页框号
HVA - Host Virtual Address，宿主机虚拟地址
HPA - Host Physical Address，宿主机物理地址
HFN - Host Frame Number，宿主机的页框号

•GVA -> GPA (guest page table) •GPA -> HVA (memslot) •HVA -> HPA (QEMU page table) •GPA -> HPA (nested page table)

AArch64 System Register Descriptions

• The AArch64 Auxiliary Feature registers ID_AA64AFR0_EL1 and ID_AA64AFR1_EL1.
• The AArch64 Processor Feature registers ID_AA64PFR0_EL1 and ID_AA64PFR1_EL1.
• The AArch64 Debug Feature registers ID_AA64DFR0_EL1 and ID_AA64DFR1_EL1.
• The AArch64 Memory Model Feature registers ID_AA64MMFR0_EL1, 
ID_AA64MMFR1_EL1, and ID_AA64MMFR2_EL1.
• The AArch64 Instruction Set Attribute registers ID_AA64ISAR0_EL1 and 
ID_AA64ISAR1_EL1.


 objdump -d *.ko | perl /home/dave/kernels/linux-2.6.git-t61/scripts/checkstack.pl i386


 $> yum install ras-utils
 $> file /usr/sbin/mce-inject
  $> mce-inject /opt/mce-test/mce-test-a4c080bd/cases/coverage/soft-inj/panic/data/fatal
ls
No Memory errors.

No PCIe AER errors.

No Extlog errors.
MCE records summary:
        2 No Error errors