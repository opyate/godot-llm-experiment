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

Now at this section: https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html#signals

Add the `position_changed` signal, then reload using the steps above (remove the .so, re-run scons, and reload the project).

Now, if you run the game, you'll see something like this in the logs:

```
The position of GDExample is now (153.6926, 191.1001)
The position of GDExample is now (190.3038, 166.3663)
The position of GDExample is now (199.328, 130.0965)
The position of GDExample is now (178.0086, 88.59532)
The position of GDExample is now (132.8584, 49.07652)
```

## Part 3: the LLM

Try https://huggingface.co/mistralai/Mistral-7B-Instruct-v0.1

Mistral announcement: https://mistral.ai/news/announcing-mistral-7b/

At the time of writing, GGUF is the recommended format to use, and the Q5_K_M model is one of TheBoke's recommended models, because it's quality los is very low. (Not sure yet what level of quality we'll need for this use-case, but hey.)

From https://huggingface.co/TheBloke/Mistral-7B-Instruct-v0.1-GGUF/blob/main/mistral-7b-instruct-v0.1.Q5_K_M.gguf

``` 
mkdir -p models
cd models
wget https://huggingface.co/TheBloke/Mistral-7B-Instruct-v0.1-GGUF/resolve/main/mistral-7b-instruct-v0.1.Q5_K_M.gguf
sha256sum mistral-7b-instruct-v0.1.Q5_K_M.gguf
# c4b062ec7f0f160e848a0e34c4e291b9e39b3fc60df5b201c038e7064dbbdcdc
cd ..
```

A test for the model is in `fine-tune/app.py`:

```
cd fine-tune
python app.py
```

It generates this using CPU:

```
{'thing': 'sketchbook', 'text': '\n\n"Wow, you\'re quite the artist!"'}
Time taken: 1.5295381546020508 seconds
{'thing': 'Xbox controller', 'text': ' "Hey, are you ready for some Gears of War 5 action?"'}
Time taken: 1.4008283615112305 seconds
{'thing': 'pencil', 'text': ' “You’re drawing the line.'}
Time taken: 0.7739419937133789 seconds
{'thing': 'banana', 'text': '\n\n"You\'re not going to use that in your game, are you?"'}
Time taken: 1.456860065460205 seconds
```

("Gears of War 5"? Ooh, we shan't be drawing the attention of the lawyers, now, shall we?)

GPU (which is quicker):

```
ggml_cuda_set_main_device: using device 0 (NVIDIA RTX 6000 Ada Generation) as main device
{'thing': 'sketchbook', 'text': ' “You’re an artist!'}
Time taken: 0.26387453079223633 seconds
{'thing': 'Xbox controller', 'text': '\n\n"I see you\'re a fan of the green ring."'}
Time taken: 0.3325464725494385 seconds
{'thing': 'pencil', 'text': ' "Draw me a game!"'}
Time taken: 0.28811216354370117 seconds
{'thing': 'banana', 'text': '\n\n"What\'s the story behind this banana?"'}
Time taken: 0.29184842109680176 seconds
```

LLM stuff to look at:
- I'll experiment more with dialogue soon (for the "spicing up NPC chatter" goal)
- be mindful of not mentioning real product/people names, in case of defamation, etc
- detect GPU/AVX/AVX2 capabilities, and use them if available


# Part 4: It talks! To itself!

See the [chat transcript](docs/chat-transcript.txt) (which is funny, because I've jsut finished Left Hand Of Darkness last week). The `~~~` is when I refresh the dialogue history so the context doesn't blow up.

A bit rough and ready, but run with:

```
cd fine-tune
python dialogue.py
```

# Part 4: Dialogue cleanup

OK, so it was late last night, and I didn't properly use EOS tokens, so cleaned up a lot of bot responses by hand. But it's way better now, and I've seeded the dialogue so we end up with a pair of right prepper nutters.

See the [chat transcript](docs/chat-transcript2.txt).

It does 60 exchanges in ~50 seconds with GPU.

