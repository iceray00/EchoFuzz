import json
import csv
import os
import argparse


def calculate_averages(file_path):
    with open(file_path, 'r') as json_file:
        data = json.load(json_file)

    total_coverage = 0
    total_vulnerability = 0
    count = len(data)

    for each in data:
        total_coverage += each["coverage"]
        total_vulnerability += each["num_vulnerability"]

    avg_coverage = total_coverage / count if count != 0 else 0
    avg_vulnerability = total_vulnerability / count if count != 0 else 0

    return avg_coverage, avg_vulnerability


def generate_summary_csv(contract_name, model_name, time):
    json_file_path = f"saveResult/{model_name}/{contract_name}_phase1_result.json"
    model_performance = f"performance_{model_name}/"
    if not os.path.exists(model_performance):
        os.makedirs(model_performance)
    csv_file_path = f"{model_performance}/vLFuzz_sFuzz_{model_name}_dataset_result_{time}s.csv"

    avg_coverage, avg_vulnerability = calculate_averages(json_file_path)
    overall = avg_coverage*0.8 + avg_vulnerability*0.2

    # Check if the CSV file exists
    file_exists = os.path.isfile(csv_file_path)

    with open(csv_file_path, 'a', newline='') as csv_file:
        csv_writer = csv.writer(csv_file)

        # Write the header if the file does not exist
        if not file_exists:
            csv_writer.writerow(["id", "contractName", "coverage", "num_vulnerability", "model_name", "overall"])
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
            # contract_name + ".sol",
            avg_coverage,
            avg_vulnerability,
            model_name,
            overall
        ]
        csv_writer.writerow(row)

    print(f"CSV file '{csv_file_path}' updated successfully.")


def main():
    """
    Single temp files, saved at the end of the fuzzing process, are consolidated
    --sol_name: main contract name, and `.sol` file name!
    --model_name: model name
    --time: fuzzing running time
    """
    parser = argparse.ArgumentParser(description="Generate/Update summary CSV file")

    parser.add_argument("-n", "--sol_name", type=str, help="main contract name, and `.sol` file name!")
    parser.add_argument("-m", "--model_name", type=str, help="LLM name")
    parser.add_argument("-t", "--time", type=int, help="Fuzzing running time")
    # parser.add_argument("-g", "--gap_time", type=int, help="gap time between recordings")

    args = parser.parse_args()

    generate_summary_csv(args.sol_name, args.model_name, args.time)


if __name__ == "__main__":
    main()
