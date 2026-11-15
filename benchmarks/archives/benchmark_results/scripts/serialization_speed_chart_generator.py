"""
********************************************************************************
* Copyright (C) 2018-2026 by Pavel Kisliak                                     *
* This file is part of BitSerializer library, licensed under the MIT license.  *
********************************************************************************
"""
import json
import matplotlib.pyplot as plt
import sys

def generate_horizontal_grouped_bar_chart(json_file, output_file):
    """
    Generates a horizontal grouped bar chart from a JSON file and saves it as a PNG file.
    Groups are sorted by the average value across available tests in ascending order.
    The largest value is used as a tie-breaker, so groups with the same average
    (for example JSON and JSONC) are not ordered by their JSON insertion order.
    The average is calculated only over the stages present in the source JSON,
    so libraries with fewer supported stages are not penalized by missing bars.
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
    # Sort by the average throughput across the available stages, not by the
    # single best stage: a group that is fast only in one stage should not
    # outrank a group that is consistently faster. Averaging only over present
    # stages keeps libraries with fewer supported stages comparable.
    def group_sort_key(group):
        values = [value for value in (data[group].get(test, 0) for test in test_names) if value > 0]
        if not values:
            return (0.0, 0)
        return (sum(values) / len(values), max(values))

    sorted_groups = sorted(
        data.keys(),
        key=group_sort_key,
        reverse=False  # Ascending order
    )
    test_values = {test: [data[group].get(test, 0) for group in sorted_groups] for test in test_names}

    # Step 3: Dynamically adjust the figure height based on the number of groups
    num_groups = len(sorted_groups)
    base_height_per_group = 0.6  # Base height per group in inches
    min_height = 10  # Minimum height of the figure
    fig_height = max(min_height, num_groups * base_height_per_group)  # Ensure a minimum height

    plt.rcParams['font.family'] = 'DejaVu Sans'
    plt.rcParams['font.size'] = 10

    # Set up the horizontal bar chart
    # Bars keep the same thickness for every group; a group with fewer supported
    # stages simply occupies less vertical space instead of leaving empty slots.
    bar_height = 0.6 / len(test_names)
    group_gap = 0.3

    # Create the figure and axis with a dark theme
    fig, ax = plt.subplots(figsize=(12, fig_height))
    fig.patch.set_facecolor('#0d1117')
    ax.set_facecolor('#151b23')

    # Define a list of colors for the bars
    colors = ['LightSkyBlue', 'DodgerBlue', '#65c778', 'ForestGreen' ]

    # For every group, only the tests present in the report get a bar.
    # The group height is proportional to the number of present tests,
    # so a library with fewer supported stages (for example, msgpack-cxx has
    # no stream stages) takes less vertical space and the next group is closer.
    group_centers = {}
    group_test_offsets = {}
    cursor = 0.0
    for group in sorted_groups:
        present_tests = [test for test in test_names if data[group].get(test, 0) > 0]
        if not present_tests:
            continue
        group_height = bar_height * len(present_tests)
        group_center = cursor + group_height / 2
        group_centers[group] = group_center
        group_test_offsets[group] = {
            test: group_center + group_height / 2 - bar_height * (index + 0.5)
            for index, test in enumerate(present_tests)
        }
        cursor += group_height + group_gap

    # Create the horizontal bars for each test, skipping zero or missing values
    bars = []
    for i, test in enumerate(test_names):
        groups_with_test = [group for group in sorted_groups if test in group_test_offsets.get(group, {})]
        if not groups_with_test:
            continue
        bar = ax.barh(
            [group_test_offsets[group][test] for group in groups_with_test],
            [data[group].get(test, 0) for group in groups_with_test],
            height=bar_height,
            label=test, color=colors[i % len(colors)], edgecolor='#3d444d', linewidth=0.5
        )
        bars.append((test, bar))

    # Step 4: Adjust spine colors for visibility
    for spine in ax.spines.values():
        spine.set_color('grey')  # Set spine color to grey

    # Add labels, title, and legend
    ax.set_xlabel('Fields per millisecond (more is better)', fontsize=10, color='white')

    # Display group names in normal font
    ax.set_yticks([group_centers[group] for group in sorted_groups])
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

    for test, bar in bars:
        values = [data[group].get(test, 0) for group in sorted_groups if test in group_test_offsets.get(group, {})]
        add_labels(bar.patches, values)

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