import re
import sys

# --- Configuration ---
# Please specify the path to your custom DEF file.
DEF_FILE_PATH = "/data/jhqiao/workspace/NBLG/Mixed-Cell-Height_legalizer/temp_result.def"

# Please specify the total die dimensions.
DIE_WIDTH = 445000.0
DIE_HEIGHT = 445000.0


def parse_def_for_rectangles(file_path):
    """
    Parses the custom DEF file to extract the position and size of all
    cells and fillers, returning them as a list of rectangles.

    Args:
        file_path (str): The path to the custom DEF file.

    Returns:
        list: A list of tuples, where each tuple represents a rectangle
              in the format (x, y, width, height). Returns None if the
              file cannot be read.
    """
    rectangles = []
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            lines = f.readlines()
    except FileNotFoundError:
        print(f"Error: DEF file not found at {file_path}")
        return None
    except Exception as e:
        print(f"An error occurred while reading the DEF file: {e}")
        return None

    i = 0
    while i < len(lines):
        line = lines[i].strip()
        # A new component can be a cell or a filler
        is_cell = line.startswith('-')
        is_filler = "+ Filler" in line

        if is_cell or is_filler:
            try:
                # The next two lines should be PLACED and SIZE
                placed_line = lines[i + 1].strip()
                size_line = lines[i + 2].strip()

                if "+ PLACED" in placed_line and "+ SIZE" in size_line:
                    placed_coords = re.findall(r'-?\d+', placed_line)
                    size_coords = re.findall(r'-?\d+', size_line)

                    if len(placed_coords) == 2 and len(size_coords) == 2:
                        x, y = map(float, placed_coords)
                        width, height = map(float, size_coords)
                        rectangles.append((x, y, width, height))
                    
                    # Move the index past the processed lines
                    i += 3
                    continue
            except IndexError:
                # Reached end of file while processing a component
                break
            except Exception as e:
                print(f"Warning: Skipping malformed component entry at line {i+1}. Error: {e}")
        
        i += 1
        
    return rectangles

def calculate_simple_sum_area(rectangles):
    """
    Calculates the total area by simply summing the area of each rectangle,
    ignoring any overlaps.

    Args:
        rectangles (list): A list of (x, y, width, height) tuples.

    Returns:
        float: The total area.
    """
    total_area = 0.0
    for _, _, width, height in rectangles:
        total_area += width * height
    return total_area

def calculate_merged_union_area(rectangles):
    """
    Calculates the total area of the union of all rectangles, correctly
    handling overlaps using a sweep-line algorithm.

    Args:
        rectangles (list): A list of (x, y, width, height) tuples.

    Returns:
        float: The area of the union of all rectangles.
    """
    if not rectangles:
        return 0.0

    events = []
    for x, y, width, height in rectangles:
        x1, y1 = x, y
        x2, y2 = x + width, y + height
        # Event format: (x_coordinate, type, y1, y2)
        # type=+1 for rectangle start, type=-1 for rectangle end
        events.append((x1, 1, y1, y2))
        events.append((x2, -1, y1, y2))
    
    # Sort events by x-coordinate
    events.sort()

    total_area = 0.0
    active_y_intervals = []
    last_x = events[0][0]

    for i in range(len(events)):
        x, type, y1, y2 = events[i]
        
        # Calculate the area of the vertical strip before this event
        strip_width = x - last_x
        if strip_width > 0 and active_y_intervals:
            # Calculate the total length of the merged active y-intervals
            active_y_intervals.sort()
            merged_y_length = 0
            current_y_start, current_y_end = active_y_intervals[0]
            for next_y_start, next_y_end in active_y_intervals[1:]:
                if next_y_start < current_y_end:
                    current_y_end = max(current_y_end, next_y_end)
                else:
                    merged_y_length += (current_y_end - current_y_start)
                    current_y_start, current_y_end = next_y_start, next_y_end
            merged_y_length += (current_y_end - current_y_start)
            
            total_area += strip_width * merged_y_length

        # Update active intervals based on the event type
        interval = (y1, y2)
        if type == 1: # Enter event
            active_y_intervals.append(interval)
        else: # Exit event
            active_y_intervals.remove(interval)
        
        last_x = x
        
    return total_area


def main():
    """
    Main function to run the custom DEF area analysis.
    """
    print("--- Custom DEF Area Utilization Analysis ---")

    if "path/to/your/custom.def" in DEF_FILE_PATH:
        print("\nError: Please update the DEF_FILE_PATH variable in the script.")
        sys.exit(1)
    
    if DIE_WIDTH <= 0 or DIE_HEIGHT <= 0:
        print("\nError: Please set valid DIE_WIDTH and DIE_HEIGHT values.")
        sys.exit(1)

    die_area = DIE_WIDTH * DIE_HEIGHT
    print(f"\nConfigured Die Area: {die_area:,.2f} ({DIE_WIDTH} x {DIE_HEIGHT})")

    rectangles = parse_def_for_rectangles(DEF_FILE_PATH)
    if rectangles is None:
        print("Failed to parse DEF file. Exiting.")
        sys.exit(1)
    
    print(f"Found {len(rectangles)} total rectangular regions (cells and fillers).")

    # --- Method 1: Simple Summation (Ignoring Overlaps) ---
    simple_sum_area = calculate_simple_sum_area(rectangles)
    simple_utilization = (simple_sum_area / die_area) * 100 if die_area > 0 else 0

    print("\n--- Analysis Method 1: Simple Summation (Overlaps are double-counted) ---")
    print(f"Total Area by Simple Summation: {simple_sum_area:,.2f}")
    print(f"Area Utilization (Simple Sum): {simple_utilization:.2f}%")

    # --- Method 2: Merged Union (Correcting for Overlaps) ---
    merged_area = calculate_merged_union_area(rectangles)
    merged_utilization = (merged_area / die_area) * 100 if die_area > 0 else 0

    print("\n--- Analysis Method 2: Merged Union (Overlaps are corrected) ---")
    print(f"Total Area of Merged Shapes: {merged_area:,.2f}")
    print(f"Area Utilization (Merged): {merged_utilization:.2f}%")
    print("--------------------------------------------------------------------\n")


if __name__ == "__main__":
    main()