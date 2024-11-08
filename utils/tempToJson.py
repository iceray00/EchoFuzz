import json
import os
import argparse
import glob

def generate_node_framework(num_files, duration, gap_time):
    data = []
    for _ in range(num_files + 1):
        row_data = []
        current_time = 0
        while current_time <= duration:
            row_data.append({
                "id": len(row_data) + 1,
                "current_time": current_time,
                "coverage": 0,
                "num_vulnerability": 0
            })
            current_time += gap_time
        data.append(row_data)
    return data


def fill_data_from_temp_files(data, file_pattern, gap_time):
    files = glob.glob(file_pattern)
    if not files:
        raise ValueError("No matching files found.")

    row_index = 0
    for file_path in files:
        with open(file_path, 'r') as file:
            lines = file.readlines()
            if len(lines) < 4:
                raise ValueError(f"File {file_path} doesn't contain enough data.")

            line_index = 0
            while line_index + 3 < len(lines):
                node_id = int(lines[line_index].strip())
                current_time = int(lines[line_index + 1].strip())
                coverage = float(lines[line_index + 2].strip())
                num_vulnerability = int(lines[line_index + 3].strip())

                node_index = current_time // gap_time
                if node_index < len(data[row_index]):
                    data[row_index][node_index]["coverage"] = coverage
                    data[row_index][node_index]["num_vulnerability"] = num_vulnerability

                line_index += 4
        row_index += 1

    return data

def fill_missing_values(data):
    for row in data[:-1]: # The chains are treated the same except for the last empty chain
        max_coverage = max(entry["coverage"] for entry in row)
        max_num_vulnerability = max(entry["num_vulnerability"] for entry in row)
        last_non_zero_coverage = max_coverage
        last_non_zero_num_vulnerability = max_num_vulnerability

        for entry in row:
            if entry["coverage"] == 0:
                entry["coverage"] = last_non_zero_coverage
                entry["num_vulnerability"] = last_non_zero_num_vulnerability
            else:
                last_non_zero_coverage = entry["coverage"]
                last_non_zero_num_vulnerability = entry["num_vulnerability"]

    return data


def merge_chains(data, merge_method):
    merged_chain = data[-1]  # Extract the last empty chain
    num_chains = len(data) - 1  # The number of chains excluding the last empty chain

    for i in range(len(merged_chain)):
        coverage_values = [chain[i]["coverage"] for chain in data[:-1]]  # Get the coverage value at the same position in all chains except the last empty chain
        num_vulnerability_values = [chain[i]["num_vulnerability"] for chain in data[:-1]]  # Get num_vulnerability at the same position in all chains except the last empty chain

        merged_chain[i]["coverage"] = max(coverage_values)
        merged_chain[i]["num_vulnerability"] = max(num_vulnerability_values)

    data[-1] = merged_chain  # Replace the last empty chain with the merged chain
    return data


def update_json(file_path, data):
    with open(file_path, 'w') as json_file:
        json.dump(data, json_file, indent=4)

def tempToJson():
    parser = argparse.ArgumentParser(description="Save data from temp_[solName]_phase1_result_*.txt to [solName]_phase1_result.json")

    parser.add_argument("-n", "--sol_name", type=str, required=True, help="main contract name, and `.sol` file name!")
    parser.add_argument("-m", "--model_name", type=str, required=True, help="model name")
    parser.add_argument("-d", "--duration", type=int, required=True, help="fuzzing duration")
    parser.add_argument("-g", "--gap_time", type=int, required=True, help="gap time between recordings")
    parser.add_argument("-a", "--merge_method", type=str, choices=["max", "min", "avg"], default="avg", help="Method to merge chains")

    args = parser.parse_args()

    saveResultDir = 'saveResult/' + args.model_name + '/'
    if not os.path.exists(saveResultDir):
        os.makedirs(saveResultDir)

    temp_file_pattern = 'temp_' + args.sol_name + '_phase1_result_*.txt'
    output_json_path = saveResultDir + args.sol_name + '_phase1_result.json'

    try:
        num_files = len(glob.glob(temp_file_pattern))
        data = generate_node_framework(num_files, args.duration, args.gap_time)
        data = fill_data_from_temp_files(data, temp_file_pattern, args.gap_time)
        data = fill_missing_values(data)
        data = merge_chains(data, args.merge_method)  # merge chains
        update_json(output_json_path, data)
        print(f"Data processed and saved successfully to [{output_json_path}]!")
    except Exception as e:
        print(f"An error occurred: {e}")


if __name__ == "__main__":
    tempToJson()
"""
Usage:
python3 tempToJson.py -n $contractName -m $model_name -d $duration -g $gap_time
"""

