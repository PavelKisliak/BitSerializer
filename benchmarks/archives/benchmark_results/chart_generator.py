"""
********************************************************************************
* Copyright (C) 2018-2025 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
********************************************************************************
"""
import json
import matplotlib.pyplot as plt
import numpy as np
import sys

def generate_horizontal_grouped_bar_chart(json_file, output_file):
    """
    Generates a horizontal grouped bar chart from a JSON file and saves it as a PNG file.
    Groups are sorted by the largest value across all tests in ascending order.
    The chart dynamically handles any number of tests and assigns colors from a predefined list.
    Empty bars for missing or zero-value metrics are not rendered.
    Tests, bars, and legend entries are rendered in the same order as they appear in the source JSON.

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

    # Step 2: Extract group names and metrics
    # Get all unique test names dynamically in the order they appear in the JSON
    test_names = []
    for group_data in data.values():
        for test_name in group_data.keys():
            if test_name not in test_names:
                test_names.append(test_name)

    # Prepare data for each test, filtering out missing or zero values
    sorted_groups = sorted(
        data.keys(),
        key=lambda group: max(data[group].get(test, 0) for test in test_names),  # Sort by the largest value across all tests
        reverse=False  # Ascending order
    )
    test_values = {test: [data[group].get(test, 0) for group in sorted_groups] for test in test_names}

    # Step 3: Dynamically adjust the figure height based on the number of groups
    num_groups = len(sorted_groups)
    base_height_per_group = 0.6  # Base height per group in inches
    min_height = 8  # Minimum height of the figure
    fig_height = max(min_height, num_groups * base_height_per_group)  # Ensure a minimum height

    plt.rcParams['font.family'] = 'DejaVu Sans'
    plt.rcParams['font.size'] = 10

    # Set up the horizontal bar chart
    y = np.arange(num_groups)  # Y-axis positions for the groups
    num_tests = len(test_names)
    bar_height = 0.6 / num_tests  # Height of the bars (adjusted for multiple tests)

    # Create the figure and axis with a dark theme
    fig, ax = plt.subplots(figsize=(12, fig_height))
    fig.patch.set_facecolor('#0d1117')
    ax.set_facecolor('#151b23')

    # Define a list of colors for the bars
    colors = ['LightSkyBlue', 'DodgerBlue', '#65c778', 'ForestGreen' ]

    # Create the horizontal bars for each test, skipping zero or missing values
    bars = []
    for i, test in enumerate(test_names):
        # Filter out zero or missing values for this test
        filtered_y = [y_pos for y_pos, value in enumerate(test_values[test]) if value > 0]
        filtered_values = [value for value in test_values[test] if value > 0]

        if filtered_values:  # Only plot if there are non-zero values
            # Adjust the vertical position of the bars to preserve the order
            bar = ax.barh(
                [y_pos - (i * bar_height) for y_pos in filtered_y],  # Stack bars in the correct order
                filtered_values, height=bar_height,
                label=test, color=colors[i % len(colors)], edgecolor='#3d444d', linewidth=0.5
            )
            bars.append(bar)

    # Step 4: Adjust spine colors for visibility
    for spine in ax.spines.values():
        spine.set_color('grey')  # Set spine color to grey

    # Add labels, title, and legend
    ax.set_xlabel('Fields per millisecond (more is better)', fontsize=10, color='white')

    # Display group names in normal font
    ax.set_yticks(y)
    ax.set_yticklabels(
        sorted_groups, fontsize=11, color='white', fontweight='normal'
    )
    ax.tick_params(axis='x', colors='grey')

    # Configure the legend at the bottom-right corner
    handles, labels = ax.get_legend_handles_labels()
    legend = ax.legend(
        handles, labels,  # Use the original order of handles and labels
        loc='lower right', bbox_to_anchor=(1, 0), fontsize=10, frameon=True,
        facecolor='#010409', edgecolor='grey'
    )
    for text in legend.get_texts():
        text.set_color('white')

    # Dynamically adjust X-axis limits to prevent overlap
    max_value = max(max(values) for values in test_values.values() if values)  # Find the largest value in the data
    x_limit = max_value * 1.07  # Add 7% padding to the maximum value
    ax.set_xlim(0, x_limit)

    # Add value labels on the right side of each bar with reduced font size and normal font
    def add_labels(bar_patches, values):
        for bar, value in zip(bar_patches, values):
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

    for i, test in enumerate(test_names):
        # Filter out zero or missing values for this test
        filtered_y = [y_pos for y_pos, value in enumerate(test_values[test]) if value > 0]
        filtered_values = [value for value in test_values[test] if value > 0]

        if filtered_values:  # Only annotate if there are non-zero values
            # Get the corresponding bar patches for this test
            bar_patches = [bars[i].patches[j] for j in range(len(filtered_y))]
            add_labels(bar_patches, filtered_values)

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
        generate_horizontal_grouped_bar_chart(input_json, output_png)