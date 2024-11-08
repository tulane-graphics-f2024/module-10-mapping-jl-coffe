#version 150

uniform vec4 ambient;              // Ambient lighting color
uniform vec4 LightPosition;        // Light position
uniform mat4 ModelViewLight;       // Model-View matrix for light

in vec4 pos;                       // Position of the fragment
in vec4 N;                         // Normal at the fragment
in vec2 texCoord;                  // Texture coordinates

uniform sampler2D textureEarth;     // Day texture
uniform sampler2D textureNight;     // Night texture
uniform sampler2D textureCloud;     // Cloud texture

uniform float animate_time;         // Animation time variable

out vec4 fragColor;

void main() {
    // (animation) set light position to simulate sun movement across the day
    float lightDistance = 10.0;
    float lightAngle = animate_time * 2.0 * 3.14159; // Full circle per day
    vec4 animatedLightPos = vec4(lightDistance * cos(lightAngle), 0.0, lightDistance * sin(lightAngle), 1.0);

    // Calculate diffuse lighting based on the animated light position
    vec3 L = normalize(vec3(ModelViewLight * animatedLightPos - pos));
    vec3 normalizedN = normalize(vec3(N));
    float diff = max(dot(normalizedN, L), 0.0);

    // 3 textures
    vec4 earthColor = texture(textureEarth, texCoord);
    vec4 nightColor = texture(textureNight, texCoord);
    vec4 cloudColor = texture(textureCloud, texCoord);

    // Blend day and night colors based on the amount of sunlight (diff)
    vec4 dayColor = earthColor * diff;  // Diffuse shading on day texture
    vec4 blendedColor = mix(nightColor, dayColor, diff); // Blend day and night based on light

    // Add clouds over the blended result
    float cloudOpacity = 0.5;
    vec4 finalColor = blendedColor + cloudColor * cloudOpacity;

    // Combine with ambient light and clamp the result
    fragColor = ambient + finalColor;
    fragColor = clamp(fragColor, 0.0, 1.0);
    fragColor.a = 1.0;
}
