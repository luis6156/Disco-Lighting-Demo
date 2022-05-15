#version 330

// Uniform
uniform vec3 prev_object_color;
uniform vec3 object_color;
uniform float transparency;
uniform float mix_factor;

// Output
layout(location = 0) out vec4 out_color;

void main()
{   
    vec3 color = object_color;

    out_color = vec4(color, transparency);
}
