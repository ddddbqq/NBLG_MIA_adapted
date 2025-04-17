import os
import re
import subprocess

# config path
BUILD_DIR = "/home/jhqiao/workspace/NBLG/Mixed-Cell-Height_legalizer/"
GLOBAL_H_PATH = os.path.join(BUILD_DIR, "include/global.h")
RUN_SH_PATH = os.path.join(BUILD_DIR, "run.sh")
LOG_DIR = os.path.join(BUILD_DIR, "logs")  # ÈÕÖ¾´æ·ÅÄ¿Â¼

# log dir
os.makedirs(LOG_DIR, exist_ok=True)

# benchmarks
ispd2014_benchmarks = [
    'mgc_des_perf_1', 'mgc_des_perf_2', 'mgc_edit_dist_1', 'mgc_edit_dist_2',
    'mgc_fft', 'mgc_matrix_mult', 'mgc_pci_bridge32_1', 'mgc_pci_bridge32_2',
    #'mgc_superblue11', 'mgc_superblue12', 'mgc_superblue16'
]

param_sets = [
    {
        'name': 'param_set1',
        'values': {
            'INTRA_MIA_ABUTT_WEIGHT': 4.5,
            'MIA_WEIGHT': 1.0,
            'FIXED_HARD_FILLER_WEIGHT': 4.8,
            'ABS_DISPLACEMENT_WEIGHT': 9.0,
            'INTRA_CELL_RIPUP_TIMES': 1,
            'INTRA_CELL_RIPUP_FREQ': 100
        }
    },
    {
        'name': 'param_set2',
        'values': {
            'INTRA_MIA_ABUTT_WEIGHT': 10.0,
            'MIA_WEIGHT': 1.0,
            'FIXED_HARD_FILLER_WEIGHT': 5.6,
            'ABS_DISPLACEMENT_WEIGHT': 9.0,
            'INTRA_CELL_RIPUP_TIMES': 1,
            'INTRA_CELL_RIPUP_FREQ': 200
        }
    }
]

def update_run_sh(benchmark):
    """update run.sh benchmark"""
    with open(RUN_SH_PATH, 'r') as f:
        lines = f.readlines()
    
    # uncomment benchmark line
    found = False
    for i in range(len(lines)):
        stripped_line = lines[i].strip()
        if stripped_line.startswith('benchmark=') and not stripped_line.startswith('#'):
            lines[i] = f'benchmark="{benchmark}"\n'
            found = True
            break
    
    if not found:  # not found, add new line
        lines.append(f'benchmark="{benchmark}"\n')
    
    with open(RUN_SH_PATH, 'w') as f:
        f.writelines(lines)

def update_global_h(params):
    with open(GLOBAL_H_PATH, 'r') as f:
        content = f.read()
    
    for param, value in params.items():
        pattern = re.compile(rf'#define {param}\s+\(([^)]+)\)')
        content, count = re.subn(pattern, f'#define {param} ({value})', content)
        
        if count == 0:
            content += f'\n#define {param} ({value})'
    
    with open(GLOBAL_H_PATH, 'w') as f:
        f.write(content)

def run_experiment(benchmark, param_set_name):
    log_file = os.path.join(LOG_DIR, f"{benchmark}_{param_set_name}.log")
    
    try:
        result = subprocess.run(
            "./build.sh && ./run.sh",
            shell=True,
            cwd=BUILD_DIR,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            #text=True
        )
        output_str = result.stdout.decode('utf-8')
        with open(log_file, 'w') as f:
            f.write(f"=== Parameters: {param_set_name} ===\n")
            f.write(output_str)
        
        print(f":) {benchmark} - {param_set_name} done")
    except Exception as e:
        print(f":( {benchmark} - {param_set_name} fail: {str(e)}")

def main():
    for benchmark in ispd2014_benchmarks:
        print(f"\n processing benchmark: {benchmark}")
        
        # ¸üÐÂrun.shÖÐµÄbenchmark
        update_run_sh(benchmark)
        
        for param_set in param_sets:
            # ±¸·ÝÔ­Ê¼global.hÄÚÈÝ
            with open(GLOBAL_H_PATH, 'r') as f:
                original_global = f.read()
            
            try:
                # ¸üÐÂÈ«¾Ö²ÎÊý
                update_global_h(param_set['values'])
                
                # ÔËÐÐÊµÑé
                run_experiment(benchmark, param_set['name'])
            finally:
                # »Ö¸´global.hÔ­Ê¼ÄÚÈÝ
                with open(GLOBAL_H_PATH, 'w') as f:
                    f.write(original_global)

if __name__ == "__main__":
    main()