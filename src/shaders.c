#include <GL/glew.h>
#include <stdio.h>

#include "shaders.h"

// Paddle vertex shader
const char *paddle_vertex_shader =
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
const char *paddle_frag_shader = "#version 330 core\n"
                                 "out vec4 FragColor;\n"
                                 "uniform vec3 uColor;\n"
                                 "void main() {\n"
                                 "  FragColor = vec4(uColor, 1.0);\n"
                                 "}\n";

// Ball vertex shader
const char *ball_vertex_shader =
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
const char *ball_frag_shader = "#version 330 core\n"
                               "in vec2 vUV;\n"
                               "out vec4 FragColor;\n"
                               "void main() {"
                               "  FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
                               "}";

// Load shaders and return program
GLuint load_shader(const char *vertex_src, const char *frag_src) {
  GLint ok;
  char log[512];

  // vertex shader
  GLuint vert = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vert, 1, &vertex_src, NULL);
  glCompileShader(vert);

  glGetShaderiv(vert, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    glGetShaderInfoLog(vert, sizeof(log), NULL, log);
    fprintf(stderr, "ERROR: Failed to create vertex shader:\n%s\n", log);
    glDeleteShader(vert);
    return 0;
  }

  // fragment shader
  GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag, 1, &frag_src, NULL);
  glCompileShader(frag);

  glGetShaderiv(frag, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    glGetShaderInfoLog(frag, sizeof(log), NULL, log);
    fprintf(stderr, "ERROR: Failed to create fragment shader:\n%s\n", log);
    glDeleteShader(vert);
    glDeleteShader(frag);
    return 0;
  }

  // program
  GLuint program = glCreateProgram();
  glAttachShader(program, vert);
  glAttachShader(program, frag);
  glLinkProgram(program);

  glGetProgramiv(program, GL_LINK_STATUS, &ok);
  if (!ok) {
    glGetProgramInfoLog(program, sizeof(log), NULL, log);
    fprintf(stderr, "ERROR: Failed to link program:\n%s\n", log);
    glDeleteProgram(program);
    glDeleteShader(vert);
    glDeleteShader(frag);
    return 0;
  }

  glDeleteShader(vert);
  glDeleteShader(frag);

  return program; // > 0 = OK
}
