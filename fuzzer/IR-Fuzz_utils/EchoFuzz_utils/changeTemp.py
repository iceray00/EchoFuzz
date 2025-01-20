import argparse


def change_temp(data, gap_time):
    nodes = []
    for i in range(0, len(data), 4):
        node = {
            'id': data[i],
            'current_time': data[i + 1],
            'coverage': data[i + 2],
            'num_vulnerability': data[i + 3]
        }
        nodes.append(node)

    previous_end_time = 0  # current_time of the last node in the previous sequence
    for i in range(len(nodes)):
        if nodes[i]['id'] == 1 and i != 0:
            # New sequence starts, updates all current_times
            previous_end_time += nodes[i - 1]['current_time']

        nodes[i]['current_time'] += previous_end_time

    # return the corrected data
    corrected_data = []
    for node in nodes:
        corrected_data.append(node['id'])
        corrected_data.append(node['current_time'])
        corrected_data.append(node['coverage'])
        corrected_data.append(node['num_vulnerability'])

    return corrected_data

def changPro():
    parser = argparse.ArgumentParser(description="Generate VFCS")

    parser.add_argument("-i", "--input", type=str, help="input single one solidity file")
    parser.add_argument("-g", "--gap_time", type=str, help="input one file path for Contract source code")

    args = parser.parse_args()

    with open(args.input, 'r') as file:
        data = file.read().strip().split('\n')
        data = list(map(float, data))  # Convert to list of float numbers

    corrected_data = change_temp(data, args.gap_time)

    with open(args.input, 'w') as file:
        for i in range(0, len(corrected_data), 4):
            file.write(f"{int(corrected_data[i])}\n{int(corrected_data[i+1])}\n{corrected_data[i+2]}\n{int(corrected_data[i+3])}\n")

    print("Change TempFile Successfully!")



if __name__ == '__main__':
    changPro()

