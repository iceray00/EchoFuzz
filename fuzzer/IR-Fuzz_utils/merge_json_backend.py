import os
import json
import time
from process_backend import main as get_overall_process  # 导入之前的 process_backend 脚本

# 文件路径配置
swap_backend_path = "swap_backend"
fuzz_state_path = os.path.join(swap_backend_path, "fuzz_state")

# 读取 phase_*.txt 内容生成 "llm output"
def generate_llm_output(overall_process):
    # 判断当前阶段是 "chain_guided" 还是 "iteration_process"
    phase_status = overall_process["overall process"]
    if any(phase_status[f"1_llm_{i}"] == 1 for i in range(1, 5)) or phase_status["1_fuzz"] == 1:
        current_phase = "chain_guided"
    elif phase_status["2_llm"] == 1 or phase_status["2_fuzz"] == 1:
        current_phase = "iteration_process"
    else:
        current_phase = "unknown"

    # 动态设置文件前缀
    if current_phase == "chain_guided":
        file_prefix = "phase_"
    elif current_phase == "iteration_process":
        file_prefix = "iteration_content"
    else:
        file_prefix = None

    latest_content = ""
    if file_prefix:  # 如果当前阶段明确
        # 查找对应前缀的文件
        files = [
            f for f in os.listdir(swap_backend_path)
            if f.startswith(file_prefix) and f.endswith(".txt")
        ]
        files.sort()  # 按文件名排序，确保按顺序处理

        # 读取最新的文件内容
        for file in files:
            file_path = os.path.join(swap_backend_path, file)
            try:
                with open(file_path, "r", encoding="utf-8") as f:
                    latest_content = f.read().strip()
            except Exception as e:
                print(f"Error reading {file}: {e}")

    return {
        "phase": current_phase,
        "content": latest_content
    }



def generate_fuzzing_state():
    state_keys = [
        "run_time",
        "last_new_path",
        "max_depth",
        "total_execs",
        "exec_speed",
        "cycles_done",
        "branches",
        "coverage"
    ]
    fuzzing_state = {}

    for key in state_keys:
        file_path = os.path.join(fuzz_state_path, f"{key}.txt")
        if os.path.exists(file_path):  # 检查文件是否存在
            try:
                with open(file_path, "r") as file:
                    content = file.read().strip()
                    if key in ["run_time", "last_new_path"]:  # 字符串内容
                        fuzzing_state[key] = content
                    elif key == "coverage":  # 去掉百分号
                        fuzzing_state[key] = int(content.replace("%", "").strip())
                    else:  # 整数内容
                        fuzzing_state[key] = int(content)
            except Exception as e:
                # print(f"Error reading {file_path}: {e}")
                fuzzing_state[key] = 0  # 解析失败时初始化为 0
        else:
            # print(f"File not found: {file_path}")
            fuzzing_state[key] = 0  # 文件不存在时初始化为 0

    return fuzzing_state


# 整合所有数据为 JSON
def generate_full_json():
    # 调用 process_backend 获取 "overall process"
    overall_process = get_overall_process()  # 此处直接返回字典
    if overall_process is None:
        raise ValueError("Failed to retrieve overall process data.")

    # 获取 "llm output"
    llm_output = generate_llm_output(overall_process)

    # 获取 "fuzzing state"
    fuzzing_state = generate_fuzzing_state()

    # 整合成完整的 JSON 数据
    full_data = {
        "llm output": llm_output,
        "overall process": overall_process["overall process"],
        "fuzzing state": fuzzing_state
    }
    return full_data

# 通过 SSE 发送到前端
def send_sse(data, event="message"):
    """
    模拟 SSE 数据流发送
    """
    # json_data = json.dumps(data)
    json_data = json.dumps(data, ensure_ascii=False)  # 确保中文不会转义
    return f"event: {event}\ndata: {json_data}\n\n"

# 主函数
def main():
    while True:  # 模拟 SSE 持续发送数据
        try:
            # 生成完整的 JSON 数据
            full_data = generate_full_json()
            # 打印 SSE 数据流格式（可替换为实际 SSE 服务发送逻辑）
            sse_message = send_sse(full_data)
            print(sse_message)  # 模拟发送
            time.sleep(5)  # 每秒发送一次
        except KeyboardInterrupt:
            print("SSE stream stopped.")
            break
        except Exception as e:
            print(f"Error in main loop: {e}")

# 执行主函数
if __name__ == "__main__":
    main()
