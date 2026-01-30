#ifndef SHADER_H
#define SHADER_H

#include <stddef.h>
#include <stdint.h>

#include <glad/glad.h>

#include "types.h"

typedef struct {
	const char *file_name;
	GLenum shader_type;
} ShaderSource;

#define SHADER_MAX_SOURCES 3

typedef struct {
	ShaderSource sources[SHADER_MAX_SOURCES];
	uint32_t sources_count;
	uint32_t id;
} Shader;

Shader shader_create(const ShaderSource *shader_sources, size_t count);
void shader_reload(Shader *shader);
void shader_destroy(uint32_t shader);
void shader_bind(uint32_t id);
void shader_unbind(void);

void shader_set_uniform_1i(uint32_t shader, const char *name, int32_t value);
void shader_set_uniform_1ui(uint32_t shader, const char *name, uint32_t value);
void shader_set_uniform_1f(uint32_t shader, const char *name, float v);
void shader_set_uniform_vec2(uint32_t shader, const char *name, vec2 v);

#endif // SHADER_H
