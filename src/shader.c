#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "shader.h"

static int32_t get_uniform_location(uint32_t shader, const char *name);
static char *file_read(const char *file_name);

Shader shader_create(const ShaderSource *shader_sources, size_t count)
{
	Shader shader = {
		.sources_count = count
	};

	assert(shader.sources_count <= SHADER_MAX_SOURCES);
	memcpy(shader.sources, shader_sources, count * sizeof(*shader_sources));
	shader_reload(&shader);

	return shader;
}

void shader_reload(Shader *program)
{
	if (program->id != 0)
		glDeleteProgram(program->id);

	program->id = glCreateProgram();

	for (uint32_t i = 0; i < program->sources_count; ++i) {
		ShaderSource *source = &program->sources[i];
		char *str = file_read(source->file_name);
		if (!str) {
			fprintf(stderr, "error: %s: %s\n", source->file_name, strerror(errno));
			continue;
		}

		uint32_t shader;
		shader = glCreateShader(source->shader_type);
		glShaderSource(shader, 1, (const char *const *)&str, NULL);
		glCompileShader(shader);

		free(str);

		glAttachShader(program->id, shader);
		glDeleteShader(shader);
	}

	glLinkProgram(program->id);
	glValidateProgram(program->id);
}

void shader_destroy(uint32_t shader)
{
	glDeleteProgram(shader);
}

void shader_bind(uint32_t shader)
{
	glUseProgram(shader);
}

void shader_unbind(void)
{
	glUseProgram(0);
}

void shader_set_uniform_1i(uint32_t shader, const char *name, int32_t value)
{
	shader_bind(shader);
	glUniform1i(get_uniform_location(shader, name), value);
}

void shader_set_uniform_1ui(uint32_t shader, const char *name, uint32_t value)
{
	shader_bind(shader);
	glUniform1ui(get_uniform_location(shader, name), value);
}

void shader_set_uniform_1f(uint32_t shader, const char *name, float v)
{
	shader_bind(shader);
	glUniform1f(get_uniform_location(shader, name), v);
}

void shader_set_uniform_vec2(uint32_t shader, const char *name, vec2 v)
{
	shader_bind(shader);
	glUniform2f(get_uniform_location(shader, name), v.x, v.y);
}

static int32_t get_uniform_location(uint32_t shader, const char *name)
{
	// TODO: Cache the locations
	int32_t uniform_location = glGetUniformLocation(shader, name);

	if (uniform_location < 0)
		fprintf(stderr, "error: failed to get location of uniform %s\n", name);

	return uniform_location;
}

static char *file_read(const char *file_name)
{
	char *buffer = NULL;

	FILE *file = fopen(file_name, "rb");
	if (file == NULL)
		goto file_read_defer;

	fseek(file, 0, SEEK_END);
	int64_t file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (file_size < 0)
		goto file_read_defer;

	buffer = malloc((file_size + 1) * (sizeof *buffer));
	fread(buffer, (sizeof *buffer), file_size, file);
	buffer[file_size] = 0;

	if (ferror(file)) {
		free(buffer);
		buffer = NULL;
		goto file_read_defer;
	}

file_read_defer:
	if (file != NULL)
		fclose(file);

	return buffer;
}
