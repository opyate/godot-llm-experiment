# Introduction

Trying to integrate an LLM into Godot.

First use-case: for spicing up NPC chatter.

# Work log

See [docs/worklog.md](docs/worklog.md).

# Quickstart

```
mkdir -p models
cd models
wget https://huggingface.co/TheBloke/Mistral-7B-Instruct-v0.1-GGUF/resolve/main/mistral-7b-instruct-v0.1.Q5_K_M.gguf
cd ..

git clone https://github.com/ggerganov/llama.cpp.git
git clone https://github.com/godotengine/godot-cpp.git

cd llama.cpp
make
cd ..

cd godot-cpp
git checkout godot-4.1.1-stable
cp gdextension/extension_api.json ../the-game/extension_api.json
scons platform=linux -j31 custom_api_file=$(pwd)/../the-game/extension_api.json
cd ..

# build my gdextension here:
scons platform=linux -j31
```

But it's not complete yet. Getting [undefined symbols](https://github.com/opyate/godot-llm-experiment/blob/main/docs/worklog.md#part-6-convert-simplecpp-to-gdextension-module)