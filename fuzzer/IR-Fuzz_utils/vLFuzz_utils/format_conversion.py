import re
import json
import os

def process_content(content):
    # 移除开始和结尾的多余标记
    content = content.strip()
    content = re.sub(r'```.*?\n', '', content)
    content = re.sub(r'\n```', '', content)

    processed_data = {}
    abi_pattern = re.compile(r'(\w+):\s*\"(\[.*?\])\"', re.DOTALL)
    contracts = re.findall(abi_pattern, content)

    if not contracts:
        raise ValueError("No valid ABI content found")

    contract_counter = {}

    for contract in contracts:
        try:
            contract_name, abi_json = contract
            # 处理多余的引号
            abi_json = abi_json.replace('\\"', '"')
            abi_list = json.loads(abi_json)

            if contract_name in contract_counter:
                contract_counter[contract_name] += 1
                unique_contract_name = f"{contract_name}_{contract_counter[contract_name]}"
            else:
                contract_counter[contract_name] = 1
                unique_contract_name = contract_name

            processed_data[unique_contract_name] = abi_list
        except (ValueError, json.JSONDecodeError) as e:
            print(f"Error processing pair: {contract}. Error: {e}")

    return processed_data


def save_abi_to_file(contract_name, abi_info, output_dir):
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    file_path = os.path.join(output_dir, f"{contract_name}.json")
    content = {
        "contracts": {
            contract_name: {
                "abi": json.dumps(abi_info)
            }
        }
    }

    with open(file_path, 'w') as file:
        json.dump(content, file, indent=4)

    print(f"ABI for {contract_name} has been successfully saved to {file_path}.")
    return file_path
