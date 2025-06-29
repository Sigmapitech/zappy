#version 330 core
in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform sampler2D tex;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0); // white light

    vec4 texColor = texture(tex, TexCoord);
    vec3 ambient = 0.5 * texColor.rgb;
    vec3 finalColor = ambient + diffuse * texColor.rgb;

    FragColor = vec4(finalColor, texColor.a);  // use texture's alpha
}
