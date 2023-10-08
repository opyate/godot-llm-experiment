// from https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html#creating-a-simple-plugin
#ifndef GDLLM_H
#define GDLLM_H

#include "common.h"
#include "llama.h"
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {

class GDLLM : public Object {
    GDCLASS(GDLLM, Object)

private:
    // const PackedStringArray& stop_sequence;
    PackedStringArray stop_sequence;
    int longest_stop_sequence_string_length;
    bool debug;
    uint32_t random_seed;

// A static function that Godot will call to find out which methods can be called and which properties it exposes
protected:
    static void _bind_methods();

public:
    GDLLM();
    //GDLLM(const PackedStringArray& stopSeqs);
    ~GDLLM();

    godot::String run_completion(const String &prompt_from_godot, const int max_new_tokens);
    void set_stop_sequence(const PackedStringArray& p_stop_sequence);
    PackedStringArray get_stop_sequence() const;
    void set_debug(const bool p_debug);
    bool get_debug() const;
    void set_random_seed(const uint32_t p_random_seed);
    uint32_t get_random_seed() const;
};
}

#endif