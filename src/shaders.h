#pragma once

/*
 * Shaders for OpenGL defined in shaders.c
 */
extern const char *paddle_vertex_shader;
extern const char *paddle_frag_shader;
extern const char *ball_vertex_shader;
extern const char *ball_frag_shader;

// Loads vertex and fragment shader and returns it
GLuint load_shader(const char *vertex_src, const char *frag_src);
