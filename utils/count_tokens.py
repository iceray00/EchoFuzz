import requests
import json

model_url = "http://localhost:11434/api/generate"


def count_tokens(prompt, model_name="Qwen1.5-32B-Q4", max_tokens_length=1, max_context_length=2048):
    data = {
        "model": model_name,
        "prompt": prompt,
        "options": {
            "max_tokens": max_tokens_length,
            "num_ctx": max_context_length
        }
    }
    try:
        response = requests.post(model_url, json=data)
        response.raise_for_status()

        # more than one json object to parse
        response_text = response.text
        response_data_list = [json.loads(part) for part in response_text.strip().split("\n")]

        prompt_eval_count = None
        for response_data in response_data_list:
            prompt_eval_count = response_data.get("prompt_eval_count")
            if prompt_eval_count is not None:
                break

        return prompt_eval_count
    except requests.exceptions.RequestException as e:
        print(f"Request for token count failed: {e}")
        return None
