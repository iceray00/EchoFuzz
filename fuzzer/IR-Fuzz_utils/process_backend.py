import os
import json

# 定义返回的 JSON 结构模板
process_status = {
    "1_llm_1": 0,
    "1_llm_2": 0,
    "1_llm_3": 0,
    "1_llm_4": 0,
    "1_fuzz": 0,
    "2_llm": 0,
    "2_fuzz": 0,
}

# 读取 signal_iteration.txt 的内容
def read_signal():
    try:
        with open("swap_backend/signal_iteration.txt", "r") as file:
            return int(file.read().strip())
    except (FileNotFoundError, ValueError):
        print("signal_iteration.txt not found or invalid content!")
        return None

# 检查当前存在的 phase_*.txt 文件
def get_existing_phases():
    phases = []
    for filename in os.listdir("swap_backend"):
        if filename.startswith("phase_") and filename.endswith(".txt"):
            try:
                phase_num = int(filename.split("_")[1].split(".")[0])
                phases.append(phase_num)
            except ValueError:
                continue
    return sorted(phases)

# 更新 process_status 的状态
def update_status(signal, phases):
    global process_status

    if signal == 0:  # Chain-guided LLM 阶段
        process_status[f"1_llm_{1}"] = 1  # 一旦开始，则阶段 1_llm_1 马上开始
        for i in range(1, 5):  # 检查 1_llm_1 到 1_llm_4
            if i in phases:
                process_status[f"1_llm_{i}"] = 2  # 已完成
                if i+1 < 5:
                    process_status[f"1_llm_{i+1}"] = 1  # 正在进行
            elif i == min(phases, default=5):  # 当前正在进行的阶段
                process_status[f"1_llm_{i}"] = 1  # 正在进行
                break

    elif signal == 5:  # Chain-guided fuzzing 阶段
        for i in range(1, 5):
            process_status[f"1_llm_{i}"] = 2  # 已完成
        process_status["1_fuzz"] = 1  # 正在进行

    elif signal in (1, 2):  # Iteration 阶段
        for i in range(1, 5):
            process_status[f"1_llm_{i}"] = 2  # Chain-guided 全部完成
        process_status["1_fuzz"] = 2  # Chain-guided fuzzing 已完成

        # 动态更新 2_llm 和 2_fuzz
        if signal == 1:
            process_status["2_llm"] = 1  # 正在进行 iteration_LLM
            process_status["2_fuzz"] = 0  # iteration_fuzzing 尚未开始
        elif signal == 2:
            process_status["2_llm"] = 0  # iteration_LLM 已完成
            process_status["2_fuzz"] = 1  # 正在进行 iteration_fuzzing

# 主函数
def main():
    # 读取 signal_iteration.txt 的值
    signal = read_signal()
    if signal is None:
        return

    # 获取现有的 phase 文件
    phases = get_existing_phases()

    # 更新状态
    update_status(signal, phases)

    # # 转换为 JSON 并输出
    result_json = json.dumps({"overall process": process_status}, indent=4)
    print(result_json)

    return {"overall process": process_status}


# 执行主函数
if __name__ == "__main__":
    main()
