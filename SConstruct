#!/usr/bin/env python
import os
import sys

# Create a main environment for godot
godot_env = SConscript("godot-cpp/SConstruct")

llamacpp_path = ARGUMENTS.get('llamacpp_path', 'llama_dot_cpp')
# make it an absolute path
llamacpp_path = os.path.abspath(llamacpp_path)

# Create a main environment for llama
llama_env = SConscript( os.path.join(llamacpp_path, "SConstruct"))

# llama_env.Append(CPPPATH=["src/", llamacpp_path, os.path.join(llamacpp_path, 'common')])


headers_and_libraries_paths = [
    os.path.join(llamacpp_path, 'common'),
    llamacpp_path,
    "src/",
]

# Append paths to both environments
godot_env.Append(CPPPATH=headers_and_libraries_paths)
llama_env.Append(CPPPATH=headers_and_libraries_paths)

# Compile and link settings
sources = Glob("src/*.cpp")

# Add 'pthread' to libraries for both environments
# godot_env.Append(LIBS=['pthread'])
godot_env.Append(LIBS=['llama', 'pthread'])
godot_env.Append(LIBPATH=[llamacpp_path])
llama_env.Append(LIBS=['pthread'])

# Object files from the specified path for llama_env
# object_files = [
#     os.path.join(llamacpp_path, 'ggml-alloc.o'),
#     os.path.join(llamacpp_path, 'k_quants.o'),
#     os.path.join(llamacpp_path, 'ggml.o'),
#     os.path.join(llamacpp_path, 'common.o'),
#     os.path.join(llamacpp_path, 'llama.o')
# ]

# Create a static library from the object files using llama_env
# static_lib = llama_env.StaticLibrary('llama', object_files)



# Link the shared library against the static library using godot_env
if godot_env["platform"] == "macos":
    library = godot_env.SharedLibrary(
        "the-game/bin/libgdllm.{}.{}.framework/libgdllm.{}.{}".format(
            godot_env["platform"], godot_env["target"], godot_env["platform"], godot_env["target"]
        ),
        source=sources,
        # LIBS=[static_lib]
    )
else:
    library = godot_env.SharedLibrary(
        "the-game/bin/libgdllm{}{}".format(godot_env["suffix"], godot_env["SHLIBSUFFIX"]),
        source=sources,
        # LIBS=[static_lib]
    )

Default(library)
