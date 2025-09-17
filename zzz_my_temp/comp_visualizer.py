import re
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle

# ¶¨ÒåÑÕÉ«Ó³ÉäºÍÎ¥¹æ¼ì²éº¯Êý
color_map = {'LVT': 'red', 'HVT': 'blue', 'SVT': 'grey'}

def parse_def_file(filename):
    cells = []
    current_cell = None
    current_filler = None
    
    with open(filename, 'r') as f:
        for line in f:
            stripped_line = line.lstrip().strip()
            if not stripped_line:
                continue
            
            # ½âÎöCELL¶¨Òå
            if stripped_line.startswith('-'):
                match = re.match(r'^- (\S+)\s+(\S+)\s+VT_TYPE\s*=\s*(\S+)', stripped_line)
                if match:
                    current_cell = {
                        'name': match.group(1),
                        'template': match.group(2),
                        'vt_type': match.group(3),
                        'x': None, 'y': None,
                        'width': None, 'height': None,
                        'fillers': []
                    }
                    cells.append(current_cell)
            
            # ½âÎöPLACEDºÍSIZE
            elif stripped_line.startswith('+'):
                if 'PLACED' in stripped_line:
                    match = re.search(r'PLACED\s*\(\s*(\d+)\s+(\d+)\s*\)', stripped_line)
                    if match:
                        x, y = map(int, match.groups())
                        if current_cell and current_cell['x'] is None:
                            current_cell['x'], current_cell['y'] = x, y
                        elif current_filler is not None:
                            current_filler['x'], current_filler['y'] = x, y
                
                elif 'SIZE' in stripped_line:
                    match = re.search(r'SIZE\s*\(\s*(\d+)\s+(\d+)\s*\)', stripped_line)
                    if match:
                        w, h = map(int, match.groups())
                        if current_cell and current_cell['width'] is None:
                            current_cell['width'], current_cell['height'] = w, h
                        elif current_filler is not None:
                            current_filler['width'], current_filler['height'] = w, h
                            current_cell['fillers'].append(current_filler)
                            current_filler = None
                
                elif 'Filler' in stripped_line:
                    parts = stripped_line.split()
                    if len(parts) >= 3:
                        current_filler = {'type': parts[2], 'x': None, 'y': None, 'width': None, 'height': None}
    
    return cells

def check_violations(cells):
    violations = []
    
    for cell in cells:
        cx1, cy1 = cell['x'], cell['y']
        cx2, cy2 = cx1 + cell['width'], cy1 + cell['height']
        
        for filler in cell['fillers']:
            if filler['type'] != 'HARD':
                continue
            
            # ¼ì²éÌî³äÆ÷ÊÇ·ñÔÚCELLÍâ
            fx1, fy1 = filler['x'], filler['y']
            fx2, fy2 = fx1 + filler['width'], fy1 + filler['height']
            if not (fx1 >= cx1 and fx2 <= cx2 and fy1 >= cy1 and fy2 <= cy2):
                violations.append(f"HARD filler in cell '{cell['name']}' at ({fx1},{fy1}) is outside cell boundaries")
            
            # ¼ì²éÓëÆäËûVTÀàÐÍCELLµÄÖØµþ
            for other in cells:
                if other['vt_type'] == cell['vt_type']:
                    continue
                
                ocx1, ocy1 = other['x'], other['y']
                ocx2, ocy2 = ocx1 + other['width'], ocy1 + other['height']
                
                x_overlap = (fx1 < ocx2) and (fx2 > ocx1)
                y_overlap = (fy1 < ocy2) and (fy2 > ocy1)
                if x_overlap and y_overlap:
                    violations.append(f"HARD filler in cell '{cell['name']}' overlaps with cell '{other['name']}' ({other['vt_type']})")
    
    return violations

#def plot_layout(cells, violations):
def plot_layout(cells):
    fig, ax = plt.subplots(figsize=(15, 10))
    
    # ÊÕ¼¯ËùÓÐ×ø±êÈ·¶¨»æÍ¼·¶Î§
    all_coords = []
    for cell in cells:
        all_coords.extend([cell['x'], cell['x']+cell['width'], cell['y'], cell['y']+cell['height']])
        for f in cell['fillers']:
            all_coords.extend([f['x'], f['x']+f['width'], f['y'], f['y']+f['height']])
    min_val, max_val = min(all_coords)-1000, max(all_coords)+1000
    
    # »æÖÆËùÓÐÔªËØ
    for cell in cells:
        color = color_map[cell['vt_type']]
        
        # »æÖÆCELL
        cell_rect = Rectangle(
            (cell['x'], cell['y']), cell['width'], cell['height'],
            facecolor=color, edgecolor='black', alpha=0.7,
            label=f"{cell['vt_type']} Cell"
        )
        ax.add_patch(cell_rect)
        
        # Ìí¼ÓÎÄ×Ö±êÇ©
        # ax.text(
        #     cell['x'] + cell['width']/2,
        #     cell['y'] + cell['height']/2,
        #     cell['name'],
        #     ha='center', va='center',
        #     color='white', fontsize=8
        # )
        
        # »æÖÆÌî³äÆ÷
        for f in cell['fillers']:
            filler_rect = Rectangle(
                (f['x'], f['y']), f['width'], f['height'],
                facecolor=color, edgecolor=color,
                fill=False, hatch='////', linestyle=':',
                linewidth=1, alpha=0.7
            )
            ax.add_patch(filler_rect)
    
    # ÉèÖÃ»æÍ¼²ÎÊý
    ax.set_xlim(min_val, max_val)
    ax.set_ylim(min_val, max_val)
    ax.set_aspect('equal')
    ax.set_title('Chip Layout Visualization')
    ax.set_xlabel('X Position')
    ax.set_ylabel('Y Position')
    
    # ´´½¨Í¼Àý
    legend_elements = [
        Rectangle((0,0),1,1, facecolor='red', edgecolor='black', label='LVT'),
        Rectangle((0,0),1,1, facecolor='blue', edgecolor='black', label='HVT'),
        Rectangle((0,0),1,1, facecolor='grey', edgecolor='black', label='SVT'),
        Rectangle((0,0),1,1, facecolor='white', edgecolor='red', hatch='////', label='Filler')
    ]
    ax.legend(handles=legend_elements, loc='upper right')
    
    # ÏÔÊ¾Î¥¹æÐÅÏ¢
    #if violations:
    #    print("\nFound violations:")
    #    for v in violations:
    #        print(f"? {v}")
    #else:
    #    print("\nNo violations found.")
    
    plt.grid(False)
    plt.show()

# Ö÷³ÌÐò
if __name__ == "__main__":
    input_file = "/home/jhqiao/workspace/NBLG/Mixed-Cell-Height_legalizer/temp_result.def"  # ÐÞ¸ÄÎªÄãµÄÎÄ¼þÂ·¾¶
    
    cells = parse_def_file(input_file)
    #violations = check_violations(cells)
    #plot_layout(cells, violations)
    plot_layout(cells)