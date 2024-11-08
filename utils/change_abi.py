import json
import sys

def change_abi_file(json_path):
    try:
        with open(json_path, 'r') as file:
            data = json.load(file)
    except FileNotFoundError:
        data = {"contracts": {}}

    contract_name = list(data['root']['contracts'].keys())[0]
    new_abi = data['root']['contracts'][contract_name]['abi']

    metadata_name = f"Metadata_{contract_name.replace(':', '_')}_1"
    data = {
        "root": {
            "contracts": {
                metadata_name: {
                    "abi": new_abi
                }
            }
        }
    }

    output_file_path = json_path
    with open(output_file_path, 'w') as file:
        json.dump(data, file, indent=4)

    print(f"ABI for {contract_name} has been successfully saved to {output_file_path}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python change_abi.py <json_path>")
        sys.exit(1)

    json_path = sys.argv[1]
    change_abi_file(json_path)
