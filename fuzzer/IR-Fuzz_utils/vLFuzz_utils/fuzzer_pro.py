import argparse
import os
import sys

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__))))

from prompt.prompt_English.import_VFCS import introduction_VFCS
from prompt.prompt_pro.prompt_pro_EN import pro_1, pro_2_totalBranchSnippet, pro_3_sequence, pro_4_duration, \
    pro_5_coverage, pro_6_covered, pro_7_target
from prompt_phases import generate_response, prompt_vfcs_abi
from format_conversion import process_content, save_abi_to_file

def extract_contract_name(file_path):
    """
    从文件路径中提取合同名称
    """
    file_name = os.path.basename(file_path)
    contract_name, _ = os.path.splitext(file_name)
    return contract_name


def exactPro_file(contractName):
    totalBranchSnippet_file = "totalBranchSnippet_" + contractName + ".txt"
    sequence_file = "sequence_" + contractName + ".txt"
    coverage_file = "coverage_" + contractName + ".txt"
    covered_file = "covered_" + contractName + ".txt"
    return totalBranchSnippet_file, sequence_file, coverage_file, covered_file


def promptFuzzerPro(contractName, source_code, duration, model_name):
    print("*******************************************************")
    print("########** Fuzzer Pro generating next VFCS **########")
    print("*******************************************************")
    totalBranchSnippet_file, sequence_file, coverage_file, covered_file = exactPro_file(contractName)

    with open(totalBranchSnippet_file, 'r', encoding='utf-8') as file:
        totalBranchSnippet_content = file.read()

    with open(sequence_file, 'r', encoding='utf-8') as file:
        sequence_content = file.read()

    with open(coverage_file, 'r', encoding='utf-8') as file:
        coverage_content = file.read()

    with open(covered_file, 'r', encoding='utf-8') as file:
        covered_content = file.read()

    total_prompt = (
            introduction_VFCS +
            pro_1 +
            source_code +
            pro_2_totalBranchSnippet.format(totalBranchSnippet_content) +
            pro_3_sequence.format(sequence_content) +
            pro_4_duration.format(duration) +
            pro_5_coverage.format(float(coverage_content)*100) +
            pro_6_covered.format(covered_content) +
            pro_7_target
    )

    fuzzerPro = generate_response(total_prompt, model_name=model_name, mode="print", max_tokens_length=2000,
                                  max_context_length=20000)
    # print(total_prompt)
    return fuzzerPro

def fuzzer_pro():
    """
    params:
    -i --input: input single one solidity file
    -d --duration: duration for fuzzing
    -m --model_name: model name
    -c --ctx: The maximum length of the context
    --mode: 'silence' or 'print'. Default is 'print'

    """
    parser = argparse.ArgumentParser(description="Fuzzer Pro")

    parser.add_argument("-i", "--input", type=str, help="input single one solidity file",
                        default="./simple_dataset/FindThisHash.sol")
    # parser.add_argument("-f", "--input_file", type=str, help="input one file path for Contract source code")
    parser.add_argument("-d", "--duration", type=int, default="20", help="duration for fuzzing")
    parser.add_argument("-m", "--model_name", type=str, default="qwen2_7b", help="model name")
    parser.add_argument("-c", "--ctx", type=int, default=20000, help="The maximum length of the context")
    parser.add_argument("--mode", type=str, default="print", help="'silence' or 'print'. Default is 'print'")

    args = parser.parse_args()

    if args.input:
        try:
            print(f"\n \nReading input file: {args.input}\n \n ")
            with open(args.input, 'r') as file:
                source_code = file.read()

        except FileNotFoundError:
            print(f"Error: File {args.input} not found.  ----FuzzerPro")
            exit(1)

        contractName = extract_contract_name(args.input)

        fuzzerPro = promptFuzzerPro(contractName, source_code, args.duration, args.model_name)

        # print(fuzzerPro)
        fuzzerPro_vfcs_abi = prompt_vfcs_abi(fuzzerPro, args.input, model_name=args.model_name, ctx=args.ctx,
                                          mode=args.mode)
        print(f"\n \nGenerated VFCS ABI:(fuzzerPro_vfcs_abi)\n \n {fuzzerPro_vfcs_abi}")

        fuzzerPro_vfcs_abi_dict = process_content(fuzzerPro_vfcs_abi)
        if fuzzerPro_vfcs_abi_dict:
            print(f"\n\nConversion successful!: (fuzzerPro_vfcs_abi_dict)\n\n {fuzzerPro_vfcs_abi_dict}")
        else:
            print("No valid ABI content was processed.")

        new_fuzzerPro_vfcs_abi_dict = {}
        for index, (key, value) in enumerate(fuzzerPro_vfcs_abi_dict.items(), start=1):
            new_key = f"{contractName}_VFCS_{index}"
            new_fuzzerPro_vfcs_abi_dict[new_key] = value

        fuzzerProVFCS = "fuzzer_pro_vfcs"
        if not os.path.exists(fuzzerProVFCS):
            os.makedirs(fuzzerProVFCS)
            print(f"\n \nCreated folder: [{fuzzerProVFCS}]")

        # Save the entire result_vfcs_abi to a file for change_abi.py
        for main_contract_name, abi_info in new_fuzzerPro_vfcs_abi_dict.items():
            json_path = f"{main_contract_name}"
            output_file_path = save_abi_to_file(json_path, abi_info, fuzzerProVFCS)



if __name__ == '__main__':
    fuzzer_pro()


