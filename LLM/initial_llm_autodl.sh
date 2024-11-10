#!/bin/bash

export OLLAMA_MODELS=/root/autodl-tmp/models
echo 'export OLLAMA_MODELS=/root/autodl-tmp/models' >> ~/.bashrc
source ~/.bashrc

curl -fsSL https://ollama.com/install.sh | sh

ollama serve

