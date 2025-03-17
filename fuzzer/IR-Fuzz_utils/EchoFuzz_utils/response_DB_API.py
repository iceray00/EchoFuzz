import os

from openai import OpenAI


def generate_prompt_response_DB_api(prompt, model_name="gpt-4o-mini", max_tokens=400):
    API_KEY = "Bearer " + os.getenv("DB_API_KEY")
    BASE_URL = "https://api.deepbricks.ai/v1/"

    client = OpenAI(api_key=API_KEY, base_url=BASE_URL)

    if client.api_key is None:
        raise ValueError("Please set DB_API_KEY in your environment variables.")
    try:
        response = client.chat.completions.create(
            model=model_name,
            messages=[{"role": "user", "content": prompt}],
            max_tokens=max_tokens,
            temperature=0.02,
            # stream=True,
        )

        # full_response = ""
        # for chunk in response:
        #     delta = chunk.choices[0].delta
        #     if delta and delta.content:
        #         full_response += delta.content
        #         print(delta.content, flush=True)
        #
        # return "\n".join(full_response)

        print(response.choices[0].message.content)
        print(f"\n*****INPUT: Prompt token count: {response.usage.prompt_tokens} ***")
        print(f"*****OUTPUT: Generated token count: {response.usage.completion_tokens} ***")
        print(f"*****Total token count (input + generated): {response.usage.total_tokens} ***")
        return response.choices[0].message.content

    except Exception as e:
        print(f"\nAPI request failed: {e}", flush=True)
        return ""


if __name__ == "__main__":
    prompt = "What is the meaning of life?"
    response = generate_prompt_response_DB_api(prompt, max_tokens=10)
