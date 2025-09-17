import re
import subprocess
from itertools import product
import csv
from pprint import pprint

"""
best congfig only considering avg disp: 
{'INTRA_CELL_RIPUP_TIMES': 3, 
'INTRA_CELL_RIPUP_FREQ': 100, 
'INTRA_MIA_ABUTT_WEIGHT': 6.5, 
'MIA_WEIGHT': 1.35, 
'FIXED_HARD_FILLER_WEIGHT': 2.9, 
'ABS_DISPLACEMENT_WEIGHT': 14.0}, 
Score: 8.75743
"""

"""
best config consigdering avg disp, hpwl, strict overflow:
{'INTRA_MIA_ABUTT_WEIGHT': 4.0, 
'MIA_WEIGHT': 1.1, 
'FIXED_HARD_FILLER_WEIGHT': 2.4, 
'ABS_DISPLACEMENT_WEIGHT': 8.0, 
'INTRA_CELL_RIPUP_TIMES': 3, 
'INTRA_CELL_RIPUP_FREQ': 100}
Best score: 10.1866
score = (1 + delta_rate/100) * temp_avg_disp + of_cnt * 100;
"""

"""
best config considering avg disp, hpwl, loose overflow:
{'INTRA_MIA_ABUTT_WEIGHT': 4.5, 
'MIA_WEIGHT': 1.0, 
'FIXED_HARD_FILLER_WEIGHT': 2.4, 
'ABS_DISPLACEMENT_WEIGHT': 9.0, 
'INTRA_CELL_RIPUP_TIMES': 3, 
'INTRA_CELL_RIPUP_FREQ': 100}
Best score: 10.011
score = (1 + delta_rate/100) * temp_avg_disp * (1 + of_cnt/100);
"""

"""
best config for mgc_pci_bridge32_1, considering loose of:
4.5, 0.9, 7.2, 9.0, 3, 100
score = (1 + delta_rate/100) * temp_avg_disp * (1 + of_cnt/1000);
Score: 18.0497, delta_rate: 40.4748%, temp_avg_disp: 12.8491, of_cnt: 860
"""

"""
after modifying, best config for fft_2_md2:
4.5, 1.0, 4.8, 9.0, 1, 100
score = (1 + delta_rate/100) * temp_avg_disp * (1 + of_cnt/1000);
Score: 13.8448 //wrong! it's double or triple height version
"""

"""
update: best config for mgc_pci_bridge32_1:
10.0, 1.0, 5.6, 9.0, 1, 200
score = (1 + delta_rate/100) * temp_avg_disp * (1 + of_cnt/1000);
avg_disp : 8.80869, of_cnt : 5, hpwl_gp : 276828 hpwl_lg : 339968 delta_rate : 22.8082%
"""

"""
mgc_des_perf_1 param:
12.0, 0.7, 8.8, 1.5, 1, 200
[ 860.184 ]   iteration : 3999 of_cnt : 4931; s_am : inf; m_max : 37.785; avg_disp : 19.4815; max_disp : 377.85
DVFA_cnt: 73083928
[ 860.184 ] main stage completed!
[ 860.184 ] ******************legalization completed. Total : 858.849 sec ******************
[ 860.187 ]   row aligned
[ 860.195 ]   fenceRegion matched
[ 860.393 ]   hpwl_gp : 1.18388e+06 hpwl_lg : 1.76063e+06 delta_rate : 48.7165%
"""

#temp member for strict score cal: naller.of_cnt_for_temp_output, ckt.temp_delta_rate, function nallsovler, cal_hpwl


GLOBAL_H_PATH = "/home/jhqiao/workspace/NBLG/Mixed-Cell-Height_legalizer/include/global.h"
BUILD_DIR = "/home/jhqiao/workspace/NBLG/Mixed-Cell-Height_legalizer/"
RESULT_FILE = "/home/jhqiao/workspace/NBLG/Mixed-Cell-Height_legalizer/temp_result"

WEIGHT_PARAMS = [
    'INTRA_MIA_ABUTT_WEIGHT',
    'MIA_WEIGHT',
    'FIXED_HARD_FILLER_WEIGHT',
    'ABS_DISPLACEMENT_WEIGHT'
]
OTHER_PARAMS = {
    'INTRA_CELL_RIPUP_TIMES': [0, 1, 2, 3],
    'INTRA_CELL_RIPUP_FREQ': [50, 100, 200]
}

DEFAULT_VALUES = {
    WEIGHT_PARAMS[0]: 13.0,
    WEIGHT_PARAMS[1]: 1.8,
    WEIGHT_PARAMS[2]: 6.4,
    WEIGHT_PARAMS[3]: 2.0,
    'INTRA_CELL_RIPUP_TIMES': 1,
    'INTRA_CELL_RIPUP_FREQ': 200
}

COARSE_RANGES = {
    'INTRA_MIA_ABUTT_WEIGHT': [12, 13, 14, 15, 16],    
    'MIA_WEIGHT': [1.2, 1.5, 1.8, 2.1, 2.4, 2.7],                  
    'FIXED_HARD_FILLER_WEIGHT': [5.6, 6.4, 7.2, 8.0, 8.8],
    'ABS_DISPLACEMENT_WEIGHT': [0.5, 1, 1.5, 2, 2.5]  
}

# COARSE_RANGES = {
#     'INTRA_MIA_ABUTT_WEIGHT': [0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 13, 13, 15],    
#     'MIA_WEIGHT': [0, 0.3, 0.6, 0.9, 1.2, 1.35, 1.5, 1.65, 1.8, 2.1, 2.4, 2.7, 3.0, 3.3, 3.6, 4.0, 4.5, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0],                  
#     'FIXED_HARD_FILLER_WEIGHT': [0, 0.8, 1.6, 2.4, 3.2, 4.0, 4.8, 5.6, 6.4, 7.2, 8.0, 9.0, 10.0],
#     'ABS_DISPLACEMENT_WEIGHT': [1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24]  
# }


FINE_CONFIG = {
    'INTRA_MIA_ABUTT_WEIGHT': {'step': 1, 'offset': 1},
    'MIA_WEIGHT': {'step': 0.5, 'offset': 0.5},
    'FIXED_HARD_FILLER_WEIGHT': {'step': 1, 'offset': 1},
    'ABS_DISPLACEMENT_WEIGHT': {'step': 0.5, 'offset': 0.5}
}

results = []
iter = 0

def update_global_file(params):
    with open(GLOBAL_H_PATH, "r") as f:
        content = f.read()
    
    for param, value in params.items():
        pattern = re.compile(rf"#define {param}\s+\(([\d.]+)\)")
        content = re.sub(pattern, f"#define {param} ({value})", content)
    
    with open(GLOBAL_H_PATH, "w") as f:
        f.write(content)

def run_experiment():
    try:
        subprocess.run(
            "./build.sh && ./run.sh",
            shell=True,
            cwd=BUILD_DIR,
            check=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )
        with open(RESULT_FILE, "r") as f:
            return float(f.read().split(": ")[1])
    except Exception as e:
        print(f"Error: {str(e)}")
        return float('inf')

def test_parameters(params):
    global iter
    complete_params = {**DEFAULT_VALUES, **params}
    original_content = open(GLOBAL_H_PATH).read()
    update_global_file(complete_params)
    score = run_experiment()
    results.append({**complete_params, 'score': score})
    
    # »Ö¸´Ô­Ê¼ÅäÖÃ
    with open(GLOBAL_H_PATH, "w") as f:
        f.write(original_content)
    print(f"Iteration {iter}: {params}, Score: {score}")
    iter += 1
    return score

def stage1_coarse_search():
    for param in WEIGHT_PARAMS:
        coarse_values = COARSE_RANGES.get(param, [DEFAULT_VALUES[param]])
        for value in coarse_values:
            params = {k: DEFAULT_VALUES[k] for k in WEIGHT_PARAMS}
            params[param] = float(value)
            test_parameters(params)

def stage2_fine_search():
    best_values = {}
    for param in WEIGHT_PARAMS:
        param_results = [
            r for r in results 
            if all(r[k] == DEFAULT_VALUES[k] for k in WEIGHT_PARAMS if k != param)
        ]
        if param_results:
            best = min(param_results, key=lambda x: x['score'])
            best_values[param] = best[param]
        else:
            best_values[param] = DEFAULT_VALUES[param]

    ranges = {}
    for param in WEIGHT_PARAMS:
        config = FINE_CONFIG.get(param, {'step': 1.0, 'offset': 1})
        best_val = best_values[param]
        
        start = max(0.0, best_val - config['offset'])
        end = min(15.0, best_val + config['offset'])
        
        current = start
        values = []
        while current <= end + 1e-9:  # Ìí¼ÓÎ¢Ð¡Á¿´¦Àí¸¡µã¾«¶È
            values.append(round(current, 2))
            current += config['step']
        
        ranges[param] = values

    # ²âÊÔËùÓÐ²ÎÊý×éºÏ
    for combination in product(*[ranges[p] for p in WEIGHT_PARAMS]):
        params = dict(zip(WEIGHT_PARAMS, combination))
        test_parameters(params)

def stage3_other_params():
    # µÚÈýÂÖ£º²âÊÔÆäËû²ÎÊý
    candidates = [
        r for r in results 
        if all(r.get(k) == DEFAULT_VALUES[k] for k in OTHER_PARAMS)
    ]
    
    if not candidates:
        raise ValueError("No candidate parameters found for stage3")

    best_weight = min(candidates, key=lambda x: x['score'])
    
    for times, freq in product(OTHER_PARAMS['INTRA_CELL_RIPUP_TIMES'], 
                             OTHER_PARAMS['INTRA_CELL_RIPUP_FREQ']):
        params = {
            'INTRA_CELL_RIPUP_TIMES': times,
            'INTRA_CELL_RIPUP_FREQ': freq
        }
        params.update({k: best_weight[k] for k in WEIGHT_PARAMS})
        test_parameters(params)

def main():
    # Ö´ÐÐ²âÊÔ½×¶Î
    print("Running coarse search...")
    stage1_coarse_search()
    
    print("\nRunning fine search...")
    stage2_fine_search()
    
    print("\nTesting other parameters...")
    stage3_other_params()

    # ±£´æ½á¹û
    with open("results.csv", "w") as f:
        writer = csv.DictWriter(f, fieldnames=list(results[0].keys()))
        writer.writeheader()
        writer.writerows(results)

    # ÕÒµ½×î¼Ñ½á¹û
    best = min(results, key=lambda x: x['score'])
    print(f"\nBest parameters: { {k: v for k, v in best.items() if k != 'score'} }")
    print(f"Best score: {best['score']}")

if __name__ == "__main__":
    main()