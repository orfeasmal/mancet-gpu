#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <glad/glad.h>
#include <SDL3/SDL.h>

#include "types.h"
#include "shader.h"

#define OPENGL_DEBUG

#define TITLE "ManCet GPU"
#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 600

#define DEFAULT_SCALE  200
#define DEFAULT_OFFSET_X 0
#define DEFAULT_OFFSET_Y 0
#define DEFAULT_ITERATIONS 100
#define DEFAULT_ITERATIONS_INCREASE 250
#define DEFAULT_ZOOM_FACTOR 1.5

static void glDebugOutput(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
    GLsizei length,
	const char *message,
	const void *userParam
);

int main(void)
{
	int32_t width = DEFAULT_WIDTH;
	int32_t height = DEFAULT_HEIGHT;

	SDL_Init(SDL_INIT_VIDEO);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, 1);
#ifdef OPENGL_DEBUG
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

	SDL_Window *window = SDL_CreateWindow(TITLE, width, height, SDL_WINDOW_OPENGL);
	if (!window) {
		fprintf(stderr, "SDL error: %s\n", SDL_GetError());
		return 1;
	}

	SDL_SetWindowResizable(window, true);

	SDL_GLContext context = SDL_GL_CreateContext(window);
	if (context == NULL) {
		fprintf(stderr, "SDL error: %s\n", SDL_GetError());
		return 1;
	}
	SDL_GL_MakeCurrent(window, context);

	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		fprintf(stderr, "failed to initialise GLAD\n");
		return 1;
	}

	SDL_GL_SetSwapInterval(1);

#ifdef OPENGL_DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
	glDebugMessageCallback(glDebugOutput, NULL);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
#endif

	glViewport(0, 0, width, height);

	float vertices[] = {
		-1.0f, -1.0f,
		 1.0f, -1.0f,
		 1.0f,  1.0f,
		-1.0f,  1.0f
	};

	uint32_t indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	uint32_t vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	uint32_t vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(*vertices), NULL);

	uint32_t ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	Shader shader = shader_create(
		(ShaderSource[]) {
			{
				.file_name = "assets/shaders/vertex.glsl",
				.shader_type = GL_VERTEX_SHADER
			},
			{
				.file_name = "assets/shaders/fragment.glsl",
				.shader_type = GL_FRAGMENT_SHADER
			}
		},
		2
	);

	//bool replot = true;
	vec2 size_half = { width / 2.0f, height / 2.0f };
	vec2 offset = { 0.0f, 0.0f };
	float scale = DEFAULT_SCALE;
	uint32_t iterations = DEFAULT_ITERATIONS;
	bool iterations_changed = true;

	int32_t mouse_last_x, mouse_last_y;
	int32_t mouse_scroll_amount = 0;
	//bool c_key_down = false;

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	bool quit = false;
	while (!quit) {
		// UPDATE

		mouse_scroll_amount = 0;

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_EVENT_QUIT:
					quit = true;
					break;
				case SDL_EVENT_WINDOW_RESIZED:
					SDL_GetWindowSize(window, &width, &height);
					size_half = (vec2) { width / 2.0, height / 2.0 };
					glViewport(0, 0, width, height);
					break;
				case SDL_EVENT_KEY_DOWN:
					switch (event.key.key) {
						case SDLK_ESCAPE:
							quit = true;
							break;
						case SDLK_UP:
							iterations += DEFAULT_ITERATIONS_INCREASE;
							iterations_changed = true;
							//replot = true;
							break;
						case SDLK_DOWN:
							if (iterations <= DEFAULT_ITERATIONS_INCREASE)
								break;

							iterations -= DEFAULT_ITERATIONS_INCREASE;
							iterations_changed = true;
							//replot = true;
							break;
					}
					break;
				case SDL_EVENT_MOUSE_BUTTON_DOWN:
					if (event.button.button == SDL_BUTTON_LEFT) {
						mouse_last_x = event.button.x;
						mouse_last_y = event.button.y;
					}
					break;
				case SDL_EVENT_MOUSE_BUTTON_UP:
					if (event.button.button == SDL_BUTTON_LEFT) {
						offset.x += (float)(event.button.x - mouse_last_x);// / pixel_width;
						offset.y += (float)(event.button.y - mouse_last_y);// / pixel_width;

						//replot = true;
					}
					break;
				case SDL_EVENT_MOUSE_WHEEL:
					mouse_scroll_amount = event.wheel.y;
					break;

			}
		}

		if (mouse_scroll_amount > 0) {
			scale *= DEFAULT_ZOOM_FACTOR;

			offset.x *= DEFAULT_ZOOM_FACTOR;
			offset.y *= DEFAULT_ZOOM_FACTOR;

			//replot = true;
		}
		else if (mouse_scroll_amount < 0) {
			scale /= DEFAULT_ZOOM_FACTOR;

			offset.x /= DEFAULT_ZOOM_FACTOR;
			offset.y /= DEFAULT_ZOOM_FACTOR;

			if (iterations == 0)
				iterations = 1;

			//replot = true;
		}


		if (iterations_changed) {
			iterations_changed = false;

			printf("\33[2K\r");
			printf("ITERATIONS: %u", iterations);
			fflush(stdout);
		}

		// RENDER
		glClear(GL_COLOR_BUFFER_BIT);

		shader_set_uniform_vec2(shader.id, "u_window_size_half", size_half);
		shader_set_uniform_vec2(shader.id, "u_offset", offset);
		shader_set_uniform_1f(shader.id, "u_scale", scale);
		shader_set_uniform_1ui(shader.id, "u_iterations", iterations);

		glBindVertexArray(vao);
		shader_bind(shader.id);
		glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(*indices), GL_UNSIGNED_INT, NULL);

		SDL_GL_SwapWindow(window);
	}

	shader_destroy(shader.id);
	glDeleteBuffers(1, &ibo);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);

	SDL_GL_DestroyContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

static void glDebugOutput(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
    GLsizei length,
	const char *message,
	const void *userParam
)
{
	// to silence the unused parameters
	(void)length;
	(void)userParam;

	// magic numbers idk just copy pasted
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
		return; 

    printf("---------------\n");
    printf("Debug message (%u): %s\n", id, message);

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:
			printf("Source: API");
			break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: 
			printf("Source: Window System");
			break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: 
			printf("Source: Shader Compiler");
			break;
        case GL_DEBUG_SOURCE_THIRD_PARTY: 
			printf("Source: Third Party");
			break;
        case GL_DEBUG_SOURCE_APPLICATION: 
			printf("Source: Application");
			break;
        case GL_DEBUG_SOURCE_OTHER: 
			printf("Source: Other");
			break;
    }
	printf("\n");

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:
			printf("Type: Error");
			break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			printf("Type: Deprecated Behaviour");
			break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: 
			printf("Type: Undefined Behaviour");
			break; 
        case GL_DEBUG_TYPE_PORTABILITY:
			printf("Type: Portability");
			break;
        case GL_DEBUG_TYPE_PERFORMANCE: 
			printf("Type: Performance");
			break;
        case GL_DEBUG_TYPE_MARKER: 
			printf("Type: Marker");
			break;
        case GL_DEBUG_TYPE_PUSH_GROUP: 
			printf("Type: Push Group");
			break;
        case GL_DEBUG_TYPE_POP_GROUP: 
			printf("Type: Pop Group");
			break;
        case GL_DEBUG_TYPE_OTHER: 
			printf("Type: Other");
			break;
    }
	printf("\n");
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:
			printf("Severity: High");
			break;
        case GL_DEBUG_SEVERITY_MEDIUM:
			printf("Severity: Medium");
			break;
        case GL_DEBUG_SEVERITY_LOW:
			printf("Severity: Low");
			break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
			printf("Severity: Notification");
			break;
    }

	printf("\n");
}
