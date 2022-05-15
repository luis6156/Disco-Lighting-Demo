#version 330

// Input
in vec3 world_position;
in vec3 world_normal;
in vec2 texcoord;

// Uniforms for light properties
uniform vec3 point_lights_position[9];
uniform int num_point_lights;
uniform vec3 point_lights_color[9];
uniform vec3 spot_lights_position[4];
uniform vec3 spot_lights_direction[4];
uniform int num_spot_lights;
uniform vec3 spot_lights_color[4];

uniform vec3 eye_position;
uniform vec3 disco_ball_position;
uniform float time;

uniform sampler2D texture;
uniform sampler2D last_texture;
uniform float mix_factor;

uniform int isPointLightOn;
uniform int isDiscoBallOn;
uniform int isSpotLightOn;
uniform int isPaused;

uniform float material_kd;
uniform float material_ks;
uniform int material_shininess;

// Uniform
uniform vec3 object_color;

// Output
layout(location = 0) out vec4 out_color;

vec3 compute_point_light(vec3 light_position, vec3 light_color) {
    vec3 L = normalize(light_position - world_position);
    vec3 V = normalize(eye_position - world_position);
    vec3 H = normalize(L + V);
    vec3 R = reflect(-L, world_normal);

    //float ambient_light = material_kd * 0.25;
    vec3 diffuse_light = material_kd * max(dot(world_normal, L), 0) * light_color;
    vec3 specular_light = vec3(0, 0, 0);

    if (diffuse_light.x > 0)
    {
         specular_light.x = material_ks * pow(max(dot(world_normal, H), 0), material_shininess) * light_color.x;
    }

    if (diffuse_light.y > 0)
    {
         specular_light.y = material_ks * pow(max(dot(world_normal, H), 0), material_shininess) * light_color.y;
    }

    if (diffuse_light.z > 0)
    {
         specular_light.z = material_ks * pow(max(dot(world_normal, H), 0), material_shininess) * light_color.z;
    }

    vec3 light;
    float d = distance(light_position, world_position);
    float att_factor = 0.f;
    float light_radius = 1.f;
    if (d < light_radius) {
        att_factor = pow(light_radius - d, 2);
    }

    light = att_factor * (diffuse_light + specular_light);

    return light;
}

vec3 compute_spot_light(vec3 light_position, vec3 light_direction, vec3 light_color) {
    vec3 L = normalize(light_position - world_position);
    vec3 V = normalize(eye_position - world_position);
    vec3 H = normalize(L + V);
    vec3 R = reflect(-L, world_normal);
    float cut_off = radians(20.f);
    float light_intensity = 1.5f;

    //float ambient_light = material_kd * 0.25;
    vec3 diffuse_light = material_kd * max(dot(world_normal, L), 0) * light_color;
    vec3 specular_light = vec3(0.f);

    if (diffuse_light.x > 0)
    {
         specular_light.x = material_ks * pow(max(dot(world_normal, H), 0), material_shininess) * light_color.x;
    }

    if (diffuse_light.y > 0)
    {
         specular_light.y = material_ks * pow(max(dot(world_normal, H), 0), material_shininess) * light_color.y;
    }

    if (diffuse_light.z > 0)
    {
         specular_light.z = material_ks * pow(max(dot(world_normal, H), 0), material_shininess) * light_color.z;
    }

    vec3 light;
    float spot_light = dot(-L, light_direction);
    float spot_light_limit = cos(cut_off);

    if (spot_light > spot_light_limit) {
        float linear_att = (spot_light - spot_light_limit) / (1.0f - spot_light_limit);
        float att_factor = pow(linear_att, 2);
        light = att_factor * (diffuse_light + specular_light);
    } else {
        light = vec3(0.f);
    }

    return light;
}

vec3 compute_discoball(vec3 light_position, vec3 light_color) {
    vec3 L = normalize(light_position - world_position);
    vec3 V = normalize(eye_position - world_position);
    vec3 H = normalize(L + V);
    vec3 R = reflect(-L, world_normal);

    vec3 diffuse_light = material_kd * max(dot(world_normal, L), 0) * light_color;
    vec3 specular_light = vec3(0, 0, 0);

    if (diffuse_light.x > 0) {
         specular_light.x = material_ks * pow(max(dot(world_normal, H), 0), material_shininess) * light_color.x;
    }

    if (diffuse_light.y > 0) {
         specular_light.y = material_ks * pow(max(dot(world_normal, H), 0), material_shininess) * light_color.y;
    }

    if (diffuse_light.z > 0) {
         specular_light.z = material_ks * pow(max(dot(world_normal, H), 0), material_shininess) * light_color.z;
    }

    vec3 light;
    float d = distance(light_position, world_position);
    float att_factor = 3 / (d * d + 1);

    light = att_factor * (diffuse_light + specular_light);
   
    return light;
}

vec3 discoball_color() {
    vec3 light_dir = world_position - disco_ball_position;

    vec2 texcoord;
    texcoord.x = (1.0 / (2 * 3.14159)) * atan (light_dir.x, light_dir.z);
    if (isPaused == 0) {
        texcoord.x -= time / 50.f;
    }
    texcoord.y = (1.0 / 3.14159) * acos (light_dir.y / length (light_dir));
 
    vec4 color = texture2D(texture, texcoord);
    vec4 lastColor = texture2D(last_texture, texcoord);

    return mix(lastColor.xyz, color.xyz, mix_factor);
}

void main()
{   
    vec3 color = vec3(0.f);

    if (isPointLightOn == 1) {
        for (int i = 0; i < num_point_lights; ++i) {
            color += compute_point_light(point_lights_position[i], point_lights_color[i]);
        }
    }

    if (isSpotLightOn == 1) {
        for (int i = 0; i < num_spot_lights; ++i) {
            color += compute_spot_light(spot_lights_position[i], spot_lights_direction[i], spot_lights_color[i]);
        }
    }

    if (isDiscoBallOn == 1) {
        color += compute_discoball(disco_ball_position, discoball_color());
    }

    color += object_color;

    // TODO(student): Write pixel out color
    out_color = vec4(color, 1);
}
