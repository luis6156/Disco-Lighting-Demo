#version 330

// Input
in vec3 world_position;

// Uniform properties
uniform sampler2D texture;
uniform sampler2D last_texture;

uniform vec3 disco_ball_position;
uniform float time;
uniform float mix_factor;
uniform int isPaused;

// Output
layout(location = 0) out vec4 out_color;

void main()
{
    vec3 light_dir = world_position - disco_ball_position;
 
    vec2 texcoord;
    texcoord.x = (1.0 / (2 * 3.14159)) * atan (light_dir.x, light_dir.z);
    if (isPaused == 0) {
         texcoord.x -= (time / 50.f);
    }
    texcoord.y = (1.0 / 3.14159) * acos (light_dir.y / length (light_dir));
 
    vec4 color = texture2D(texture, texcoord);
    vec4 lastColor = texture2D(last_texture, texcoord);

    vec3 newColor = mix(lastColor.xyz, color.xyz, mix_factor);

    out_color = vec4(newColor, 1);
}
