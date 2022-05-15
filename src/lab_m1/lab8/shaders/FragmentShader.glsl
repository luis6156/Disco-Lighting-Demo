#version 330

// Input
in vec3 world_position;
in vec3 world_normal;

// Uniforms for light properties
uniform vec3 light_direction;
uniform vec3 light_position;
uniform vec3 eye_position;

uniform float material_kd;
uniform float material_ks;
uniform int material_shininess;

// TODO(student): Declare any other uniforms

uniform vec3 object_color;
uniform float cut_off;
uniform int type_light;

// Output
layout(location = 0) out vec4 out_color;


void main()
{   
    vec3 L = normalize(light_position - world_position);
    vec3 V = normalize(eye_position - world_position);
    vec3 H = normalize(L + V);
    vec3 R = reflect(-L, world_normal);

    float ambient_light = material_kd * 0.25;
    float diffuse_light = material_kd * max(dot(world_normal, L), 0);
    float specular_light = 0;

    if (diffuse_light > 0)
    {
         specular_light = material_ks * pow(max(dot(world_normal, H), 0), material_shininess);
    }

    float light;

    if (type_light == 1) {
        float d = distance(light_position, world_position);
        float att_factor = 1 / (d * d + 1);
        light = ambient_light + att_factor * (diffuse_light + specular_light);
    } else {
        float spot_light = dot(-L, light_direction);
        float spot_light_limit = cos(cut_off);

        if (spot_light > spot_light_limit) {
            float linear_att = (spot_light - spot_light_limit) / (1.0f - spot_light_limit);
            float att_factor = pow(linear_att, 2);
            light = ambient_light + att_factor * (diffuse_light + specular_light);
        } else {
            light = ambient_light;
        }
    }

    vec3 color = object_color * light;

    // TODO(student): Write pixel out color
    out_color = vec4(color, 1);
}
