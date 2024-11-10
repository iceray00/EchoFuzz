import json
import os
import csv
import sys


def process_json_file(current_time, contract_name):
    file_path = f"contracts/{contract_name}/{contract_name}_report.json"

    if not os.path.exists(file_path):
        print(f"文件 {file_path} 不存在.")
        return

    try:
        with open(file_path, 'r') as file:
            data = json.load(file)
    except json.JSONDecodeError:
        print(f"Unable to parse JSON file in {file_path}.")
        return

    coverage = data.get("coverage", 0)
    uniq_exceptions = data.get("uniqExceptions", 0)

    csv_file = "IR-FUZZZZ.csv"
    rows = []
    if os.path.exists(csv_file):
        try:
            with open(csv_file, 'r') as read_csvfile:
                reader = csv.DictReader(read_csvfile)
                for row in reader:
                    row['chain'] = json.loads(row['chain'])  # 将JSON字符串转换回Python对象
                    row['coverage'] = float(row['coverage']) if row['coverage'] else 0.0
                    row['num_vulnerability'] = int(row['num_vulnerability']) if row['num_vulnerability'] else 0
                    rows.append(row)
        except Exception as e:
            print(f"读取 {csv_file} 时发生错误: {e}")
            return

    # 查找是否存在当前 contractName 的行
    contract_row = next((row for row in rows if row['contractName'] == contract_name), None)

    # 如果不存在，则创建新的行
    if not contract_row:
        contract_row = {
            'id': len(rows) + 1,
            'contractName': contract_name,
            'chain': [],
            'coverage': 0,
            'num_vulnerability': 0
        }
        rows.append(contract_row)

    # 添加当前数据到链
    new_chain_node = {
        "id": len(contract_row['chain']) + 1,
        "current_time": current_time,
        "coverage": coverage,
        "num_vulnerability": uniq_exceptions
    }
    contract_row['chain'].append(new_chain_node)

    # 更新 coverage 和 num_vulnerability（累加或覆盖取决于需求）
    contract_row['coverage'] = coverage
    contract_row['num_vulnerability'] = uniq_exceptions

    # 更新 CSV 文件
    try:
        with open(csv_file, 'w', newline='') as csvfile:
            fieldnames = ['id', 'contractName', 'chain', 'coverage', 'num_vulnerability']
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)

            writer.writeheader()
            for row in rows:
                writer.writerow({
                    'id': row['id'],
                    'contractName': row['contractName'],
                    'chain': json.dumps(row['chain']),  # 将链数据转换为 JSON 字符串存储
                    'coverage': row['coverage'],
                    'num_vulnerability': row['num_vulnerability']
                })
    except Exception as e:
        print(f"写入 {csv_file} 时发生错误: {e}")
        return

    print(f"数据已成功处理并存入 {csv_file} 文件中.")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("用法: python json_to_csv.py <current_time> <contractName>")
        # python3 json_to_csv.py $DURATION $CONTRACT_NAME
    else:
        current_time = sys.argv[1]
        contract_name = sys.argv[2]
        process_json_file(current_time, contract_name)
