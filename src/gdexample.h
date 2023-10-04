// from https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html#creating-a-simple-plugin
#ifndef GDEXAMPLE_H
#define GDEXAMPLE_H

#include <godot_cpp/classes/sprite2d.hpp>

namespace godot {

class GDExample : public Sprite2D {
    GDCLASS(GDExample, Sprite2D)

private:
    double time_passed;

// A static function that Godot will call to find out which methods can be called and which properties it exposes
protected:
    static void _bind_methods();

public:
    GDExample();
    ~GDExample();

    // works exactly the same as the _process function in GDScript
    void _process(double delta);
};

}

#endif