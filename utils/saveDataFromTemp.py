import json
import os
import argparse

def read_temp_file(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()
        if len(lines) < 2:
            raise ValueError("Input file doesn't contain enough data.")
        coverage = float(lines[0].strip())
        num_vulnerability = int(lines[1].strip())
        return coverage, num_vulnerability

def update_json(file_path, coverage, num_vulnerability):
    if os.path.exists(file_path):
        with open(file_path, 'r') as json_file:
            data = json.load(json_file)
    else:
        data = []
        print(f"Creating new JSON file: [{file_path}] successfully!")

    new_entry = {
        "id": len(data) + 1,
        "coverage": coverage,
        "num_vulnerability": num_vulnerability
    }

    data.append(new_entry)

    with open(file_path, 'w') as json_file:
        json.dump(data, json_file, indent=4)


def saveDataFromTemp():
    """
    Single temp files, saved at the end of the fuzzing process, are consolidated
    --sol_name: main contract name, and `.sol` file name!
    --model_name: model name
    """
    parser = argparse.ArgumentParser(description="Save data from temp_[solName]_phase1_result.txt to [solName]_phase1_result.json")

    parser.add_argument("-n", "--sol_name", type=str, help="main contract name, and `.sol` file name!")
    parser.add_argument("-m", "--model_name", type=str, help="model name")

    args = parser.parse_args()

    saveResultDir = 'saveResult/' + args.model_name + '/'
    if not os.path.exists(saveResultDir):
        os.makedirs(saveResultDir)

    temp_file_path = 'temp_' + args.sol_name + '_phase1_result.txt'
    output_json_path = saveResultDir + args.sol_name + '_phase1_result.json'

    try:
        coverage, num_vulnerability = read_temp_file(temp_file_path)
        update_json(output_json_path, coverage, num_vulnerability)
        print(f"Data appended successfully to [{output_json_path}]!")
    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    saveDataFromTemp()
