#!/usr/bin/env python
import os
import sys

env = SConscript("godot-cpp/SConstruct")

# For reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

llamacpp_path = ARGUMENTS.get('llamacpp_path', 'llama.cpp')  # llama.cpp is a directory, not a cpp file :)

# Specify where to find headers and libraries
env.Append(CPPPATH=[
    "src/",
    llamacpp_path,
    os.path.join(llamacpp_path, 'common'),
    "godot-cpp/include/godot_cpp/classes/"  # Add this path for wrapped.hpp
])
sources = Glob("src/*.cpp")

# Include wrapped.cpp in your sources
sources.append("godot-cpp/src/classes/wrapped.cpp")


# Object files from the specified path
object_files = [
    os.path.join(llamacpp_path, 'ggml-alloc.o'),
    os.path.join(llamacpp_path, 'k_quants.o'),
    os.path.join(llamacpp_path, 'ggml.o'),
    os.path.join(llamacpp_path, 'common.o'),
    os.path.join(llamacpp_path, 'llama.o')
]

# add 'pthread' to libraries
libraries = ['pthread']
env.Append(LIBS=libraries)

# Create a static library from the object files
static_lib = env.StaticLibrary('llama', object_files)

# Link the shared library against the static library
if env["platform"] == "macos":
    library = env.SharedLibrary(
        "the-game/bin/libgdllm.{}.{}.framework/libgdllm.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
        LIBS=[static_lib]
    )
else:
    library = env.SharedLibrary(
        "the-game/bin/libgdllm{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
        LIBS=[static_lib]
    )

Default(library)
