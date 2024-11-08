import json
import os, sys
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__))))
from static_analysis import static_analysis_in_Slither


def call_print_static(sol_path):
    result = static_analysis_in_Slither(sol_path)
    analysis_report, cfg, cg, abi_output = result

    print("\n**************************************************//Analysis Report//"
          "\nAnalysis Report:")
    print(analysis_report if analysis_report else "No analysis report")
    print("**************************************************//Analysis Report//\n")

    print("\n**************************************************//CFG//"
          "\nControl Flow Graph (CFG):")
    print(json.dumps(cfg, indent=2) if cfg else "No CFG")
    print("**************************************************//CFG//\n")

    print("\n**************************************************//CG//"
          "\nCall Graph (CG):")
    print(json.dumps(cg, indent=2) if cg else "No CG")
    print("**************************************************//CG//\n")

    print("\n**************************************************//ABI//"
          "\nApplication Binary Interface (ABI):")
    print(abi_output if abi_output else "No ABI")
    print("**************************************************//ABI//\n")


if __name__ == "__main__":
    sol_path = "../../data_set_tmp/FindThisHash.sol"
    call_print_static(sol_path)
