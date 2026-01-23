PROJECT_NAME = mancet

NC     = \033[0m
BLUE   = \033[1;34m
CYAN   = \033[1;36m
GREEN  = \033[1;32m
YELLOW = \033[1;33m

SDL_DIR = SDL3-3.4.0

GLAD_DIR = glad
GLAD_OBJ = $(GLAD_DIR)/*.o

LA_DIR = la
# NOTE: this object file is useless,
# unless you change LADEF in la.h to something like static inline
# and remove LA_IMPLEMENTATION from main.c
LA_OBJ = $(LA_DIR)/*.o

CC = clang
LD = clang

CFLAGS =  -std=c11 -Wall -Wextra -Wpedantic -I.
CFLAGS += -Ideps/$(SDL_DIR)/include -Ideps/$(GLAD_DIR)/include -Ideps/$(LA_DIR)

CFLAGS_DEB = -O0 -g -gdwarf-4 -fsanitize=address
CFLAGS_REL = -O3

LDFLAGS = deps/build/$(GLAD_OBJ) deps/build/$(LA_OBJ) -Wl,-rpath,deps/build/$(SDL_DIR)/ -Ldeps/build/$(SDL_DIR) -lSDL3
LDFLAGS_DEB = -fsanitize=address

rwildcard = $(foreach d, $(wildcard $1*), $(call rwildcard, $d/, $2) $(filter $(subst *, %, $2), $d))

BUILD_DEB   = build/debug
BUILD_REL   = build/release
OBJ_DEB_DIR = $(BUILD_DEB)/obj
OBJ_REL_DIR = $(BUILD_REL)/obj

SRC_DIR     = src
SRC         = $(call rwildcard, $(SRC_DIR), *.c)
OBJ_DEB     = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DEB_DIR)/%.o, $(SRC))
OBJ_REL     = $(patsubst $(SRC_DIR)/%.c, $(OBJ_REL_DIR)/%.o, $(SRC))

EXE_DEB = $(BUILD_DEB)/$(PROJECT_NAME)
EXE_REL = $(BUILD_REL)/$(PROJECT_NAME)

.PHONY: run clean deps sdl glad la depsclean

debug: $(EXE_DEB)
release: $(EXE_REL)

$(EXE_DEB): $(OBJ_DEB)
	@ echo -e "$(GREEN)LINKING EXECUTABLE$(NC) $@"
	@ $(LD) $(OBJ_DEB) -o $@ $(LDFLAGS) $(LDFLAGS_DEB)

$(EXE_REL): $(OBJ_REL)
	@ echo -e "$(GREEN)LINKING EXECUTABLE$(NC) $@"
	@ $(LD) $(OBJ_REL) -o $@ $(LDFLAGS)

$(OBJ_REL_DIR)/%.o: $(SRC_DIR)/%.c
	@ mkdir -p $(@D)
	@ echo -e "$(GREEN)COMPILING OBJECT$(NC) $@"
	@ $(CC) $(CFLAGS) $(CFLAGS_REL) -c $< -o $@

$(OBJ_DEB_DIR)/%.o: $(SRC_DIR)/%.c
	@ mkdir -p $(@D)
	@ echo -e "$(GREEN)COMPILING OBJECT$(NC) $@"
	@ $(CC) $(CFLAGS) $(CFLAGS_DEB) -c $< -o $@

run: debug
	@ echo -e "$(CYAN)EXECUTING$(NC) $(EXE_DEB)"
	@ ./$(EXE_DEB)

clean:
	@ echo -e "$(YELLOW)CLEANING PROJECT$(NC)"
	@ rm -rf build

deps: sdl glad la

sdl:
	@ echo -e "$(BLUE)BUILDING DEPENDENCY $(SDL_DIR)$(NC)"
	@ mkdir -p deps/build/$(SDL_DIR) && cd deps/build/$(SDL_DIR) && cmake ../../$(SDL_DIR) && make -j16

glad:
	@ echo -e "$(BLUE)BUILDING DEPENDENCY $(GLAD_DIR)$(NC)"
	@ mkdir -p deps/build/$(GLAD_DIR) && cd deps/build/$(GLAD_DIR) && $(CC) -O3 -I../../$(GLAD_DIR)/include -c ../../$(GLAD_DIR)/src/*.c

la:
	@ echo -e "$(BLUE)BUILDING DEPENDENCY $(LA_DIR)$(NC)"
	@ mkdir -p deps/build/$(LA_DIR) && cd deps/build/$(LA_DIR) && $(CC) -O3 -I../../$(LA_DIR) -c ../../$(LA_DIR)/*.c

depsclean:
	@ echo -e "$(YELLOW)CLEANING DEPENDENCIES$(NC)"
	@ rm -rf deps/build/
