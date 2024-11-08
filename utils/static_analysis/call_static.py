import json
import os, sys
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__))))
from static_analysis import static_analysis_in_Slither


def call_slither_static_analysis(sol_path):
    result = static_analysis_in_Slither(sol_path)
    analysis_report, cfg, cg, abi_output = result

    all_integrate = (
        "Analysis Report:\n" +
        (analysis_report if analysis_report else "No analysis report") +
        "\n**************************************************\n\n" +
        "Control Flow Graph (CFG):\n" +
        (json.dumps(cfg, indent=2) if cfg else "No CFG") +
        "\n**************************************************\n\n" +
        "Call Graph (CG):\n" +
        (json.dumps(cg, indent=2) if cg else "No CG") +
        "\n**************************************************\n\n" +
        "Application Binary Interface (ABI):" +
        (abi_output if abi_output else "No ABI") +
        "\n**************************************************\n\n"
    )

    return all_integrate


def call_solc_ar_abi(sol_path):
    result = static_analysis_in_Slither(sol_path)
    analysis_report, _, _, abi = result
    ar_abi_integrate = (
        "Analysis Report:\n" +
        (analysis_report if analysis_report else "No analysis report") +
        "Application Binary Interface (ABI):" +
        (abi if abi else "No ABI")
    )
    return ar_abi_integrate


def call_solc_abi(sol_path):
    result = static_analysis_in_Slither(sol_path)
    _, _, _, abi = result
    return abi


if __name__ == "__main__":
    path = "../../data_set_tmp/ERC20.sol"
    print(call_slither_static_analysis(path))