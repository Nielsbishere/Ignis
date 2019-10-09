#version 450

in layout(location=0) vec2 vpos;
out layout(location=0) vec2 col;

uniform layout(binding=0, std140) Test {
	vec2 mask;
};

void main() {
	col = (mask + 0.2) * (vpos * 0.5 + 0.5);
    gl_Position = vec4(vpos, 0, 1);
}