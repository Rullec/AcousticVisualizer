#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragCameraPos;
layout(location = 3) in vec3 fragVertexWorldPos;
layout(location = 4) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

// layout(push_constant) uniform constants { int enable_texture; }
// PushConstants;
layout(push_constant) uniform constants
{
    vec4 Ka, Kd, Ks;
    float Ns;
    int enable_texture;
    int enable_phongmodel;
    int enable_basic_color;
}
PushConstants;

float calc_specular(vec3 light_pos, vec3 pixel_world_pos, vec3 normal,
                    vec3 eye_pos)
{
    // input light L
    vec3 L = normalize(light_pos - pixel_world_pos);
    // output light R
    vec3 R = normalize(2 * (dot(normal, L)) * normal - L);
    vec3 V = normalize(eye_pos - pixel_world_pos);
    return pow(abs(dot(V, R)), 512);
}

vec3 calc_phong(vec3 Ka, vec3 Kd, vec3 Ks, vec3 light_pos)
{
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * Ka;

    // diffuse
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(light_pos - fragVertexWorldPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * Kd;

    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(fragCameraPos - fragVertexWorldPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    vec3 specular = specularStrength * spec * Ks;

    vec3 tmp_sum = ambient + diffuse + specular;
    return tmp_sum;
}

void main()
{
    vec3 light_pos = vec3(10.0, 10.0, 10.0);
    vec3 light_color = vec3(1.0, 1.0, 1.0);

    int enable_texture = PushConstants.enable_texture;
    int enable_phong = PushConstants.enable_phongmodel;
    int enable_basic_color = PushConstants.enable_basic_color;
    if (enable_texture != 0)
    {
        // enable texture
        vec3 tmp_sum;
        if (enable_phong != 0)
        {
            tmp_sum =
                calc_phong(light_color, light_color, light_color, light_pos);
        }
        else
        {
            tmp_sum = vec3(1, 1, 1);
        }

        vec4 tex_color4 = texture(texSampler, fragTexCoord);
        outColor.xyz = tmp_sum * tex_color4.xyz;
    }
    else
    {
        // 1. basic color + light phong
        // 3. basic color
        vec3 tmp_sum = vec3(1, 1, 1);
        if (enable_basic_color == 1)
        {
            if (enable_phong == 1)
            {
                tmp_sum = calc_phong(light_color, light_color, light_color,
                                     light_pos);
            }
            outColor.xyz = fragColor.xyz;
        }
        else
        {
            // do not use basic color
            //  = fragNormal;

            if (enable_phong == 1)
            {
                outColor.xyz =
                    calc_phong(PushConstants.Ka.xyz, PushConstants.Kd.xyz,
                               PushConstants.Ks.xyz, light_pos);
            }
            else
            {
                // outColor.xyz = vec3(0.32, 0.269804, 0.254118);
                outColor.xyz = PushConstants.Kd.xyz;
            }
        }
    }

    outColor[3] = fragColor[3];
}