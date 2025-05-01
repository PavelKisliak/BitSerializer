import json
import matplotlib.pyplot as plt
import sys

def generate_horizontal_bar_chart(json_file, output_file):
    """
    Generates a horizontal bar chart from a JSON file and saves it as a PNG file.
    Groups are sorted by the size of serialized data in descending order.

    Args:
        json_file (str): Path to the input JSON file.
        output_file (str): Path to save the output PNG file.
    """
    # Step 1: Load the JSON data
    try:
        # Open the file in binary mode to detect and strip the BOM if present
        with open(json_file, 'rb') as file:
            raw_data = file.read()
        # Strip the UTF-8 BOM if present
        if raw_data.startswith(b'\xef\xbb\xbf'):
            raw_data = raw_data[3:]  # Remove the BOM
        # Decode the data as UTF-8 and parse it as JSON
        data = json.loads(raw_data.decode('utf-8'))
    except FileNotFoundError:
        print(f"Error: File '{json_file}' not found.")
        return
    except json.JSONDecodeError:
        print(f"Error: Invalid JSON format in '{json_file}'.")
        return

    # Step 2: Extract group names and sizes
    # Sort groups by the size of serialized data in descending order
    sorted_groups = sorted(
        data.keys(),
        key=lambda group: data[group],
        reverse=True  # Descending order
    )
    sizes = [data[group] for group in sorted_groups]

    # Step 3: Dynamically adjust the figure height based on the number of groups
    num_groups = len(sorted_groups)
    base_height_per_group = 0.4  # Base height per group in inches
    min_height = 4  # Minimum height of the figure
    fig_height = max(min_height, num_groups * base_height_per_group)  # Ensure a minimum height

    plt.rcParams['font.family'] = 'DejaVu Sans'
    plt.rcParams['font.size'] = 10

    # Set up the horizontal bar chart
    y = range(num_groups)  # Y-axis positions for the groups

    # Create the figure and axis with a dark theme
    fig, ax = plt.subplots(figsize=(12, fig_height))
    fig.patch.set_facecolor('#0d1117')
    ax.set_facecolor('#151b23')

    # Define a list of colors for the bars
    colors = [ 'DodgerBlue' ]

    # Create the horizontal bars with visible borders
    bars = ax.barh(
        y, sizes, height=0.3,
        color=[colors[i % len(colors)] for i in range(num_groups)],
        edgecolor='#3d444d', linewidth=0.5
    )

    # Step 4: Adjust spine colors for visibility
    for spine in ax.spines.values():
        spine.set_color('grey')

    # Add labels, title, and legend
    ax.set_xlabel('Size of serialized data (bytes)', fontsize=10, color='white')

    # Display group names
    ax.set_yticks(y)
    ax.set_yticklabels(
        sorted_groups, fontsize=11, color='white', fontweight='normal'
    )
    ax.tick_params(axis='x', colors='grey')  # Grey ticks on the X-axis

    # Dynamically adjust X-axis limits to prevent overlap
    max_value = max(sizes)  # Find the largest value in the data
    x_limit = max_value * 1.07  # Add 7% padding to the maximum value
    ax.set_xlim(0, x_limit)

    # Add value labels on the right side of each bar with reduced font size and normal font
    def add_labels(bars, values):
        for bar, value in zip(bars, values):
            width = bar.get_width()
            ax.annotate(
                f'{value}',  # Annotation text
                xy=(width, bar.get_y() + bar.get_height() / 2),  # Position of the annotation
                xytext=(3, 0),  # Offset for the label
                textcoords="offset points",
                ha='left', va='center',
                fontsize=9,
                color='white',
                fontweight='normal'
            )

    add_labels(bars, sizes)

    # Step 5: Save the chart as a PNG file
    plt.tight_layout()  # Adjust layout to prevent clipping
    plt.savefig(output_file, dpi=150, facecolor=fig.patch.get_facecolor())  # Save with dark background
    print(f"Chart saved to '{output_file}'.")

# Example usage
if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python script.py <input_json_file> <output_png_file>")
    else:
        input_json = sys.argv[1]
        output_png = sys.argv[2]
        generate_horizontal_bar_chart(input_json, output_png)
