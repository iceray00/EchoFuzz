


## In AutoDL
* A widely used computing platform.
* https://www.autodl.com/

```bash
sh ./initial_llm_autodl.sh
```

After that, exec:
```bash
ollama run qwen2:7b
```
* If not specified, the default model downloaded will be Qwen2:7B.
  * In addition, if you do not want to use the default model, can download the another models in [ollama model library](https://ollama.com/library)!


## Deploy in local environment

```bash
export OLLAMA_MODELS=/path/you/want/to
echo 'export OLLAMA_MODELS=/path/you/want/to' >> ~/.bashrc
```

Restart your terminal or use the `source` statement to reactivate the environment.

After then, install the [ollama](https://ollama.com).
```bash
curl -fsSL https://ollama.com/install.sh | sh
```

Start __ollama__ with `ollama serve`
















