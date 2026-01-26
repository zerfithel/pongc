#pragma once

// Paddle vertex shader 
static const char *paddle_vertex_shader =
"#version 330 core\n"
"layout(location = 0) in vec2 aPos;\n" 
"uniform mat4 uProj;\n"
"uniform vec2 uPos;\n"
"uniform vec2 uSize;\n"
"void main() {\n"
"  vec2 worldPos = aPos * uSize + uPos;\n"
"  gl_Position = uProj * vec4(worldPos, 0.0, 1.0);\n"
"}\n";


// Paddle fragment shader
static const char *paddle_frag_shader =
"#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec3 uColor;\n"
"void main() {\n"
"  FragColor = vec4(uColor, 1.0);\n"
"}\n";

// Ball vertex shader
static const char *ball_vertex_shader =
"#version 330 core\n"
"layout (location = 0) in vec2 aPos;\n"
"uniform mat4 uProj;\n"
"uniform vec2 uPos;\n"
"uniform vec2 uSize;\n"
"out vec2 vUV;\n"
"void main() {"
"  vUV = aPos;\n"
"  vec2 worldPos = aPos * uSize + uPos;\n"
"  gl_Position = uProj * vec4(worldPos, 0.0, 1.0);\n"
"}";

// Ball fragment shader
static const char *ball_frag_shader =
"#version 330 core\n"
"in vec2 vUV;\n"
"out vec4 FragColor;\n"
"void main() {"
"  FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
"}";

// Loads vertex and fragment shader and returns it
GLuint load_shader(const char *vertex_src, const char *frag_src);
