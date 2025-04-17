#!/bin/bash
# benchmark=(des_perf_1 des_perf_a_md1  des_perf_b_md1  edit_dist_1_md1  edit_dist_a_md3  fft_a_md2  pci_bridge32_a_md1  pci_bridge32_b_md1  pci_bridge32_b_md3
# des_perf_a_md2  des_perf_b_md2  edit_dist_a_md2  fft_2_md2        fft_a_md3  pci_bridge32_a_md2  pci_bridge32_b_md2)
# ispd2014: (mgc_des_perf_1   mgc_des_perf_2   mgc_edit_dist_1   mgc_edit_dist_2   mgc_fft   mgc_matrix_mult   
# mgc_pci_bridge32_1   mgc_pci_bridge32_2   mgc_superblue11   mgc_superblue12   mgc_superblue16)
# benchmark="fft_2_md2"
# benchmark="mgc_pci_bridge32_1"
benchmark="mgc_pci_bridge32_2"
./build/MCHLG -doParallel false -lef ./benchmarks/$benchmark/tech.lef -lef ./benchmarks/$benchmark/cells_modified.lef -def ./benchmarks/$benchmark/placed.def -placement_constraints ./benchmarks/$benchmark/placement.constraints -output_def ./output/$benchmark.def
