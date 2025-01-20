from flask import Flask, Response, stream_with_context, request
import time
import json

from merge_json_backend import generate_full_json

app = Flask(__name__)

# def generate_full_json():
#     """模拟生成 JSON 数据"""
#     return {
#         "llm_output": {"phase": "iteration_process", "content": "示例内容"},
#         "overall_process": {"1_llm_1": 2, "1_llm_2": 2},
#         "fuzzing_state": {"run_time": "0 days, 0 hrs, 0 min, 50 sec"}
#     }

def sse_stream():
    """持续发送 SSE 数据"""
    while True:
        try:
            full_data = generate_full_json()
            print(full_data)
            message = f"data: {json.dumps(full_data)}\n\n"
            yield message  # 发送数据流
            time.sleep(5)  # 每 5 秒发送一次
        except Exception as e:
            print(f"Error: {e}")
            break

@app.route('/api/sse-stream', methods=['POST'])
def stream():
    """定义 SSE 接口，处理POST请求"""
    post_data = request.get_json()
    print(post_data)

    return Response(stream_with_context(sse_stream()), content_type='text/event-stream')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080)