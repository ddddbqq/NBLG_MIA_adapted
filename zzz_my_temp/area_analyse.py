import re
import sys
from collections import Counter

# --- Configuration ---
# Please specify the paths to your LEF and DEF files here.
LEF_FILE_PATH = "/data/jhqiao/workspace/NBLG/Mixed-Cell-Height_legalizer/benchmarks/fft_2_md2/cells_modified.lef"
DEF_FILE_PATH = "/data/jhqiao/workspace/NBLG/Mixed-Cell-Height_legalizer/benchmarks/fft_2_md2/placed.def"

# The scale factor for length units between LEF and DEF.
# Per the problem description, 1 LEF unit = 1000 DEF units.
LEF_TO_DEF_SCALE_FACTOR = 1000.0

def parse_lef_for_macro_sizes(file_path):
    """
    Parses a LEF file to extract the SIZE of each MACRO.

    Args:
        file_path (str): The path to the LEF file.

    Returns:
        dict: A dictionary mapping macro names to their (width, height) tuples.
              Returns an empty dictionary if the file cannot be read.
    """
    macro_sizes = {}
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            in_macro_definition = False
            current_macro_name = ""
            for line in f:
                stripped_line = line.strip()
                if stripped_line.startswith("MACRO"):
                    in_macro_definition = True
                    parts = stripped_line.split()
                    if len(parts) > 1:
                        current_macro_name = parts[1]
                elif in_macro_definition and stripped_line.startswith("END " + current_macro_name):
                    in_macro_definition = False
                    current_macro_name = ""
                elif in_macro_definition and stripped_line.startswith("SIZE"):
                    # Example: SIZE 2 BY 2 ;
                    parts = stripped_line.split()
                    if len(parts) >= 4 and parts[2].upper() == 'BY':
                        try:
                            width = float(parts[1])
                            height = float(parts[3])
                            macro_sizes[current_macro_name] = (width, height)
                        except ValueError:
                            print(f"Warning: Could not parse SIZE line for macro {current_macro_name}: {stripped_line}")
    except FileNotFoundError:
        print(f"Error: LEF file not found at {file_path}")
        return {}
    except Exception as e:
        print(f"An error occurred while reading the LEF file: {e}")
        return {}
    return macro_sizes

def parse_def_for_die_area_and_components(file_path):
    """
    Parses a DEF file to extract the DIEAREA and a count of each component.

    Args:
        file_path (str): The path to the DEF file.

    Returns:
        tuple: A tuple containing (die_area, component_counts).
               die_area is a float.
               component_counts is a Counter object mapping component model names to their counts.
               Returns (None, None) if the file cannot be read or essential info is missing.
    """
    die_area = 0.0
    component_counts = Counter()
    in_components_section = False
    
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            for line in f:
                stripped_line = line.strip()
                
                if not stripped_line:
                    continue

                if stripped_line.startswith("DIEAREA"):
                    # Example: DIEAREA ( 0 0 ) ( 342000 342000 ) ;
                    coords = re.findall(r'-?\d+', stripped_line)
                    if len(coords) == 4:
                        p1_x, p1_y, p2_x, p2_y = map(float, coords)
                        die_area = abs(p2_x - p1_x) * abs(p2_y - p1_y)
                    continue

                if stripped_line.startswith("COMPONENTS"):
                    in_components_section = True
                    continue
                if stripped_line.startswith("END COMPONENTS"):
                    in_components_section = False
                    break 

                if in_components_section and stripped_line.startswith("-"):
                    # Example: - x_out_9_reg_9_ ms00f80 + PLACED ...
                    parts = stripped_line.split()
                    if len(parts) >= 3:
                        component_model_name = parts[2]
                        component_counts[component_model_name] += 1
                        
    except FileNotFoundError:
        print(f"Error: DEF file not found at {file_path}")
        return None, None
    except Exception as e:
        print(f"An error occurred while reading the DEF file: {e}")
        return None, None

    if die_area == 0:
        print("Warning: DIEAREA not found or could not be parsed in DEF file.")
        return None, component_counts

    return die_area, component_counts

def main():
    """
    Main function to run the LEF/DEF area analysis.
    """
    print("--- LEF/DEF Cell Area Utilization Analysis ---")
    
    if "path/to/your/file.lef" in LEF_FILE_PATH or "path/to/your/file.def" in DEF_FILE_PATH:
        print("\nError: Please update the LEF_FILE_PATH and DEF_FILE_PATH variables in the script.")
        sys.exit(1)
    
    print(f"\nParsing LEF file: {LEF_FILE_PATH}")
    macro_sizes = parse_lef_for_macro_sizes(LEF_FILE_PATH)
    if not macro_sizes:
        print("Could not retrieve any macro sizes from the LEF file. Exiting.")
        sys.exit(1)
    print(f"Found definitions for {len(macro_sizes)} macros in the LEF file.")

    print(f"\nParsing DEF file: {DEF_FILE_PATH}")
    die_area, component_counts = parse_def_for_die_area_and_components(DEF_FILE_PATH)
    
    if die_area is None or not component_counts:
        print("Could not retrieve necessary information from the DEF file. Exiting.")
        sys.exit(1)

    total_components = sum(component_counts.values())
    print(f"Found DIEAREA: {die_area:,.2f} square units.")
    print(f"Found {total_components} component instances in the DEF file.")

    total_cell_area = 0.0
    unmatched_macros = set()

    for component_model, count in component_counts.items():
        if component_model in macro_sizes:
            width, height = macro_sizes[component_model]
            # Area in DEF units = (width * scale) * (height * scale)
            cell_area = (width * LEF_TO_DEF_SCALE_FACTOR) * (height * LEF_TO_DEF_SCALE_FACTOR)
            total_cell_area += count * cell_area
        else:
            unmatched_macros.add(component_model)
    
    if unmatched_macros:
        print("\nWarning: The following component models were found in the DEF file but not in the LEF file:")
        # Print only a few examples if the list is too long
        for i, macro in enumerate(sorted(list(unmatched_macros))):
            if i < 10:
                print(f"  - {macro}")
        if len(unmatched_macros) > 10:
            print(f"  ... and {len(unmatched_macros) - 10} more.")
        print("The area of these cells will not be included in the total.")

    if die_area > 0:
        utilization = (total_cell_area / die_area) * 100
    else:
        utilization = 0.0

    print("\n--- Analysis Results ---")
    print(f"Total Die Area: {die_area:,.2f}")
    print(f"Total Occupied Cell Area: {total_cell_area:,.2f}")
    print(f"Area Utilization: {utilization:.2f}%")
    print("------------------------\n")


if __name__ == "__main__":
    main()