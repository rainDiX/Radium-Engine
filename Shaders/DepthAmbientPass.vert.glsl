layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTangent;
layout (location = 3) in vec3 inBitangent;
layout (location = 4) in vec3 inTexcoord;

struct Transform
{
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 worldNormal;
};

uniform Transform transform;

out VS_OUT
{
    vec3 position;
    vec3 normal;
    vec3 texcoord;
	vec3 eye;
} vs_out;

void main()
{
    mat4 mvp = transform.proj * transform.view * transform.model;
    gl_Position = mvp * vec4(inPosition, 1.0);

    vec4 pos = transform.model * vec4(inPosition, 1.0);
    vs_out.position = pos.xyz / pos.w;
    vs_out.normal = vec3(transform.worldNormal * vec4(inNormal, 0.0));

    vs_out.texcoord = inTexcoord;
}
