import subprocess
import json
import os
import tempfile


def run_command(command):
    try:
        result = subprocess.run(command, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        return result.stdout.strip(), result.stderr.strip()
    except subprocess.CalledProcessError as e:
        print(f"Error running command {command}: {e.stderr}")
        return None, e.stderr.strip()


def static_analysis_in_Slither(contract_file):
    """
    0. Use `Slither [contract name]` to generate static analysis report
    1. Generate Control Flow Graph
    2. Generate Call Graph
    3. Generate Application Binary Interface
    :param contract_file: Path to the Solidity contract file
    :return: [Analysis report, CFG, CG, ABI]
    """
    # Check if slither is installed
    version_output, version_error = run_command(['slither', '--version'])
    if version_output is None:
        raise EnvironmentError("Slither is not installed or not found in PATH")

    analysis_report, cfg, cg, abi_output = None, None, None, None

    # Create a temporary directory for the .dot files
    with tempfile.TemporaryDirectory() as temp_dir:
        # Generate analysis report
        analysis_report = subprocess.run(['slither', contract_file], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        slither_report = analysis_report.stdout + analysis_report.stderr
        if slither_report:
            analysis_report = slither_report
        else:
            print(f"Warning: No analysis report generated: {analysis_report.stderr}")

        # Generate Control Flow Graph (CFG)
        cfg_cmd = ['slither', contract_file, '--print', 'cfg', '--json', os.path.join(temp_dir, 'cfg.json')]
        cfg_output, cfg_error = run_command(cfg_cmd)
        cfg_path = os.path.join(temp_dir, 'cfg.json')
        if os.path.exists(cfg_path):
            with open(cfg_path, 'r') as f:
                cfg = json.load(f)
        else:
            print(f"Warning: No CFG generated: {cfg_error}")

        # Generate Call Graph (CG)
        cg_cmd = ['slither', contract_file, '--print', 'call-graph', '--json', os.path.join(temp_dir, 'cg.json')]
        cg_output, cg_error = run_command(cg_cmd)
        cg_path = os.path.join(temp_dir, 'cg.json')
        if os.path.exists(cg_path):
            with open(cg_path, 'r') as f:
                cg = json.load(f)
        else:
            print(f"Warning: No CG generated: {cg_error}")

    # Generate Application Binary Interface (ABI)
    abi_cmd = ['solc', '--abi', contract_file]
    abi_output, abi_error = run_command(abi_cmd)
    if abi_output is None:
        print(f"Warning: No ABI generated: {abi_error}")

    return [analysis_report, cfg, cg, abi_output]


if __name__ == "__main__":
    contract_file = "/root/fuzzing/project_1/try_static/data_set_tmp/FindThisHash.sol"
    result = static_analysis_in_Slither(contract_file)
    analysis_report, cfg, cg, abi_output = result

    print("\n**************************************************//Analysis Report//"
          "\nAnalysis Report:")
    print(analysis_report if analysis_report else "No analysis report")
    print("**************************************************//Analysis Report//\n")

    # print("\n**************************************************//CFG//"
    #       "\nControl Flow Graph (CFG):")
    # print(json.dumps(cfg, indent=2) if cfg else "No CFG")
    # print("**************************************************//CFG//\n")

    print("\n**************************************************//CG//"
          "\nCall Graph (CG):")
    print(json.dumps(cg, indent=2) if cg else "No CG")
    print("**************************************************//CG//\n")

    print("\n**************************************************//ABI//"
          "\nApplication Binary Interface (ABI):")
    print(abi_output if abi_output else "No ABI")
    print("**************************************************//ABI//\n")
