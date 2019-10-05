#version 450 core

in layout(location=0) vec2 col;
out layout(location=0) vec4 color;

void main() {
    color = vec4(col, 0, 1);
}