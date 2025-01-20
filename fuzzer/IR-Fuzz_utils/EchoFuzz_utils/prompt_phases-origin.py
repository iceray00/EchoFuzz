import re
import importlib
import os
import sys

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__))))


BASE_PROMPT_PATH = "prompt.prompt_English"

# Import modules dynamically
try:
    prompt_vfcs_abi = importlib.import_module(f"{BASE_PROMPT_PATH}.prompt_vfcs_abi")
    prompt_phase1_intro = importlib.import_module(f"{BASE_PROMPT_PATH}.prompt_phase1_intro")
    prompt_phase2_static = importlib.import_module(f"{BASE_PROMPT_PATH}.prompt_phase2_static")
    prompt_target_VFCS = importlib.import_module(f"{BASE_PROMPT_PATH}.prompt_target_VFCS")
except ImportError as e:
    raise ImportError(f"Error importing modules: {e}")

# get the property
def get_attr(module, attr):
    if hasattr(module, attr):
        return getattr(module, attr)
    else:
        raise AttributeError(f"Module {module} has no attribute {attr}")

VFCS_abi_1 = get_attr(prompt_vfcs_abi, 'VFCS_abi_1')
input_abi = get_attr(prompt_vfcs_abi, 'input_abi')
input_VFCS = get_attr(prompt_vfcs_abi, 'input_VFCS')
VFCS_abi_2 = get_attr(prompt_vfcs_abi, 'VFCS_abi_2')

prompt_1_introduction = get_attr(prompt_phase1_intro, 'prompt_1_introduction')
prompt_1_target = get_attr(prompt_phase1_intro, 'prompt_1_target')

prompt_2_static_analysis_1 = get_attr(prompt_phase2_static, 'prompt_2_static_analysis_1')
prompt_2_static_analysis_2 = get_attr(prompt_phase2_static, 'prompt_2_static_analysis_2')
prompt_2_static_analysis_3 = get_attr(prompt_phase2_static, 'prompt_2_static_analysis_3')
prompt_2_static_analysis_target = get_attr(prompt_phase2_static, 'prompt_2_static_analysis_target')

intro_VFCS = get_attr(prompt_target_VFCS, 'intro_VFCS')
LLLM_generate_VFCS_1 = get_attr(prompt_target_VFCS, 'LLM_generate_VFCS_1')
LLLM_generate_VFCS_2 = get_attr(prompt_target_VFCS, 'LLM_generate_VFCS_2')
staticAnalysis_report_abi = get_attr(prompt_target_VFCS, 'staticAnalysis_report_abi')
LLM_generate_VFCS_target = get_attr(prompt_target_VFCS, 'LLM_generate_VFCS_target')

from response_ollama import generate_prompt_response
from response_DB_API import generate_prompt_response_DB_api
from static_analysis.call_static import call_solc_abi, call_slither_static_analysis, call_solc_ar_abi


def generate_response(prompt, model_name, max_tokens_length, max_context_length, mode):
    if re.match(r"^(gpt|claude|llama-3.1-405b)", model_name, re.IGNORECASE):
        return generate_prompt_response_DB_api(prompt, model_name, max_tokens_length)
    else:
        return generate_prompt_response(prompt, model_name, max_tokens_length, max_context_length, mode)

def prompt_to_vfcs_phase1(source_code, model_name, ctx, mode):
    print("*******************************************************\n"
          "##### Testing: Sending prompt_phase1 Generating111...#####\n"
          "##### Introduction VFCS & preliminary analysis #####\n"
          "*******************************************************")
    prompt_1 = (
        prompt_1_introduction +
        f"\n```sol\n{source_code}```\n" +
        prompt_1_target
    )
    result_1 = generate_response(prompt_1, model_name=model_name, max_tokens_length=600, max_context_length=ctx, mode=mode)
    return result_1

def prompt_vfcs_phase2(result_1, sol_path, source_code, model_name, ctx, mode):
    print("*******************************************************\n"
          "##### Testing: Sending prompt_phase2 Generating222...#####\n"
          "##### Static Analysis Report Feedback Analysis #####\n"
          "*******************************************************")
    prompt_2 = (
        prompt_2_static_analysis_1 +
        f"```sol\n{source_code}```" +
        prompt_2_static_analysis_2.format(result_1) +
        prompt_2_static_analysis_3.format(call_slither_static_analysis(sol_path)) +
        prompt_2_static_analysis_target
    )
    result_2 = generate_response(prompt_2, model_name=model_name, max_tokens_length=400, max_context_length=ctx, mode=mode)
    return result_2

def prompt_generate_VFCS(result_1, result_2, sol_path, source_code, model_name, ctx, mode):
    print("*******************************************************\n"
          "##### Testing: Sending prompt_generate_VFCS Generating333...#####\n"
          "##### *Use Q&A and static analysis to generate VFCS* #####\n"
          "*******************************************************")
    prompt_3 = (
        intro_VFCS +
        f"```sol\n{source_code}```" +
        LLLM_generate_VFCS_1.format(result_1) +
        LLLM_generate_VFCS_2.format(result_2) +
        staticAnalysis_report_abi.format(call_solc_ar_abi(sol_path)) +
        LLM_generate_VFCS_target
    )
    result_3 = generate_response(prompt_3, model_name=model_name, max_tokens_length=500, max_context_length=ctx, mode=mode)
    return result_3

def prompt_vfcs_abi(input_vfcs, sol_path, model_name, ctx, mode):
    print("*******************************************************\n"
          "##### Testing: Sending prompt_vfcs_abi Generating444...#####\n"
          "##### According to the ABI,VFCS generate the vfcs_abi #####\n"
          "*******************************************************")
    prompt_vfcs_ABI = (
        VFCS_abi_1 +
        input_abi.format(call_solc_abi(sol_path)) +
        input_VFCS.format(input_vfcs) +
        VFCS_abi_2
    )
    result_vfcs_abi = generate_response(prompt_vfcs_ABI, model_name=model_name, max_tokens_length=500, max_context_length=ctx, mode=mode)
    return result_vfcs_abi
