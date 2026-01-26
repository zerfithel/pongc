#include <GL/glew.h>
#include <stdio.h>

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
