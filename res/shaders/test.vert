#version 450 core

in layout(location=0) vec2 vpos;
out layout(location=0) vec2 col;

void main() {
	col = vpos * 0.5 + 0.5;
    gl_Position = vec4(vpos, 0, 1);
}