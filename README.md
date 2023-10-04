# Introduction

Trying to integrate an LLM into Godot, for spicing up NPC chatter.

# Work log

## Part 1

```
mkdir godot-llm-experiment
cd godot-llm-experiment
mkdir src
```

Start here: https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html

Conda environment (for scons):

```
ENVNAME=godot-llm-experiment-py311; conda deactivate; conda remove -y --name $ENVNAME --all ; conda create -y -n $ENVNAME python==3.11 ; [ -e .conda ] || echo "conda activate $ENVNAME" > .conda;  source .conda
```

In future, just `source .conda` to continue working.

Install scons:

```
pip install scons
```

Clone the godot-cpp repo:

```
git clone https://github.com/godotengine/godot-cpp.git
```

Build C++ bindings. 

```
cp godot-cpp/gdextension/extension_api.json the-game/extension_api.json
```

Or alternatively, it seems you can generate this file yourself:

```
cd the-game
godot --dump-extension-api extension_api.json
```

But I saw differences between the one I created, and the one in the repo. So, I'll use the one in the repo for now.

Continue...

```
cd godot-cpp
git checkout godot-4.1.1-stable
scons platform=linux -j32 custom_api_file=$(pwd)/../the-game/extension_api.json
cd ..
```

(takes about 35 seconds on my machine)

Now at this section: https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html#creating-a-simple-plugin

But I'm calling mine `the-game` instead of `demo`.

Add all the example code (.h and .cpp files).

Now at this section: https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html#compiling-the-plugin

Download the Scons file. It looks the same as the one here: `godot-cpp/test/SConstruct`
so ignore the warning
> This SConstruct file was written to be used with the latest godot-cpp master, you may need to make small changes using it with older versions or refer to the SConstruct file in the Godot 4.0 documentation.

Run 

```
scons platform=linux
```

Hmm, seems I ought to have used `-j31`` before :)

> Auto-detected 32 CPU cores available for build parallelism. Using 31 cores by default. You can override it with the -j argument.

Another 35 seconds later, I have: `the-game/bin/libgdexample.linux.template_debug.x86_64.so`

Later, I'll drop debug symbols with a prod build:

```
scons platform=linux target=template_release
```

## Part 2

Now at this section: https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html#adding-properties

Add new amplitude and speed properties.

Have this new property reflect in Godot editor with:

```
rm the-game/bin/libgdexample.linux.template_debug.x86_64.so
scons platform=linux
```

Then `Project -> Reload Current Project` in the editor.
