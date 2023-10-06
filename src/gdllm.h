// from https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html#creating-a-simple-plugin
#ifndef GDLLM_H
#define GDLLM_H

#include "common.h"
#include "llama.h"
#include <godot_cpp/classes/object.hpp>

namespace godot {

class GDLLM : public Object {
    GDCLASS(GDLLM, Object)

private:

// A static function that Godot will call to find out which methods can be called and which properties it exposes
protected:
    static void _bind_methods();

public:
    GDLLM();
    ~GDLLM();

    virtual void run_completion(const String &prompt_from_godot);
};
}

#endif