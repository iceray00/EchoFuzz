import json
import csv
import os
import argparse


def generate_summary_csv(contract_name, model_name, duration, gap_time):
    json_file_path = f"saveResult/{model_name}/{contract_name}_phase1_result.json"
    model_performance = f"performance_{model_name}/"

    with open(json_file_path, 'r') as f:
        data = json.load(f)

    merged_chain = data[-1]

    if not os.path.exists(model_performance):
        os.makedirs(model_performance)
    csv_file_path = f"{model_performance}/vL+sFuzz-{model_name}-dataset_result_{duration}s.csv"

    # Check if the CSV file exists
    file_exists = os.path.isfile(csv_file_path)

    with open(csv_file_path, 'a', newline='') as csv_file:
        csv_writer = csv.writer(csv_file)

        # Write the header if the file does not exist
        if not file_exists:
            csv_writer.writerow(["id", "contractName", "chain", "coverage", "num_vulnerability", "model_name", "duration", "gap_time"])
            print(f"***CREATE CSV***\n CSV file '{csv_file_path}' created successfully!")

        # Calculate the next ID based on existing rows in the CSV file
        if file_exists:
            with open(csv_file_path, 'r') as read_csv_file:
                existing_data = list(csv.reader(read_csv_file))
                id_counter = len(existing_data)
        else:
            id_counter = 1

        row = [
            id_counter,
            contract_name,
            json.dumps(merged_chain),
            merged_chain[-1]['coverage'],
            merged_chain[-1]['num_vulnerability'],
            model_name,
            duration,
            gap_time,
        ]
        csv_writer.writerow(row)

    print(f"CSV file **{csv_file_path}** updated successfully.")



def main():
    """
    Single temp files, saved at the end of the fuzzing process, are consolidated
    --sol_name: main contract name, and `.sol` file name!
    --model_name: model name
    --duration: fuzzing running time
    --gap_time: gap time between recordings
    """
    parser = argparse.ArgumentParser(description="Generate/Update summary CSV file")

    parser.add_argument("-n", "--sol_name", type=str, required=True, help="main contract name, and `.sol` file name!")
    parser.add_argument("-m", "--model_name", type=str, required=True, help="LLM name")
    parser.add_argument("-d", "--duration", type=int, required=True, help="Fuzzing running time")
    parser.add_argument("-g", "--gap_time", type=int, required=True, help="gap time between recordings")

    args = parser.parse_args()

    generate_summary_csv(args.sol_name, args.model_name, args.duration, args.gap_time)

if __name__ == "__main__":
    main()
"""
Usage: 
python3 generate_summary_csv.py -n $contractName -m $model_name -d $duration -g $gap_time
"""

