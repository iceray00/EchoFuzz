import argparse
import json
import os


def replace_abi_in_contract(vfcs_abi_file, target_contract_dir, id):
    try:
        full_vfcs_abi_file = os.path.join(vfcs_abi_file)

        # 读取VFCS ABI文件内容
        with open(full_vfcs_abi_file, 'r') as vfcs_file:
            vfcs_data = json.load(vfcs_file)
        print("**\nRead VFCS ABI file content successfully")

        # 提取输入文件中的合约名
        vfcs_filename = os.path.basename(full_vfcs_abi_file)
        print(f"**\nProcessing file: {vfcs_filename}")

        # 提取合约名（忽略 _VFCS_1.json 部分）
        contract_name = vfcs_filename.split('_VFCS_')[0]
        print(f"**\nContract name: {contract_name}")

        # 在目标目录中找到对应的合约文件
        target_contract_file = None
        for file in os.listdir(target_contract_dir):
            if file.startswith(contract_name) and file.endswith('.sol.json'):
                target_contract_file = os.path.join(target_contract_dir, file)
                break

        if not target_contract_file:
            raise FileNotFoundError(
                f"**\nContract file starting with {contract_name} and ending with .sol.json not found in {target_contract_dir}!!!!!!!!!!!!!!!!\n \n")

        # 读取目标合约文件
        with open(target_contract_file, 'r') as target_file:
            target_data = json.load(target_file)

        # 替换目标合约文件中的ABI内容
        vfcs_abi = vfcs_data['contracts'][f'{contract_name}_VFCS_{id}']['abi']
        for key in target_data['contracts'].keys():
            if key.endswith(contract_name):
                target_data['contracts'][key]['abi'] = vfcs_abi
                break

        print("**\nReplace ABI content")

        # 保存更新后的目标合约文件
        with open(target_contract_file, 'w') as target_file:
            json.dump(target_data, target_file, indent=4)

        print(f"ABI in {target_contract_file} has been successfully!!!!! replaced with ABI from {full_vfcs_abi_file}")

    except FileNotFoundError as e:
        print(f"File not found: {e}")
    except Exception as e:
        print(f"Error processing file: {e}")
        import traceback
        traceback.print_exc()


def main():
    """
    利用LLM生成的VFCS_abi，fuzzing/contracts下相同合约名的json文件中的abi依次替换掉，再进行fuzzing过程
    """

    print("^^^\n Current working directory: \n^^^", os.getcwd())
    parser = argparse.ArgumentParser(description="ABI replaced with VFCS_abi")
    parser.add_argument("-i", "--input", type=str, required=True,
                        help="input the VFCS_abi file you want to replace (relative to /root/vLFuzz/seed_pool)")
    parser.add_argument("-t", "--target_dir", type=str, default="/root/fuzzing/field_test/test_9_star/contracts",
                        help="target directory containing contract files")
    parser.add_argument("--id", type=int, default=1, help="the id of the VFCS_abi file")

    args = parser.parse_args()

    replace_abi_in_contract(args.input, args.target_dir, args.id)


if __name__ == "__main__":
    main()