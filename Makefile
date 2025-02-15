CC = clang
CFLAGS = -std=c11 -O2 -Wall -Wextra
OBJC_FLAGS = -fobjc-arc
INCLUDES = -I$(CURDIR)/libs/sokol -I$(CURDIR)/libs/nuklear -I$(CURDIR)/libs/linmath -I$(CURDIR)/libs/cJSON
FRAMEWORKS = -framework Metal -framework MetalKit -framework Cocoa -framework AudioToolbox -framework QuartzCore

BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

# Define source files
SRC_FILES = main.c ecs.c utils.c camera.c gui.c http_client.c scene.c character.c render.c audio.c
SOKOL_FILES = sokol.m
CJSON_FILES = cJSON.c

# Create full paths
SRC_PATHS = $(addprefix src/,$(SRC_FILES))
SOKOL_PATHS = $(addprefix libs/sokol/,$(SOKOL_FILES))
CJSON_PATHS = $(addprefix libs/cJSON/,$(CJSON_FILES))
SRCS = $(SRC_PATHS) $(SOKOL_PATHS) $(CJSON_PATHS)

# Create object file paths
OBJS = $(SRC_FILES:%.c=$(OBJ_DIR)/src/%.o)
OBJS += $(SOKOL_FILES:%.m=$(OBJ_DIR)/libs/sokol/%.o)
OBJS += $(CJSON_FILES:%.c=$(OBJ_DIR)/libs/cJSON/%.o)

LIBS = -lcurl -lfreetype

TARGET = $(BUILD_DIR)/demo

all: directories $(TARGET)

directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(OBJ_DIR)/src
	@mkdir -p $(OBJ_DIR)/libs/sokol
	@mkdir -p $(OBJ_DIR)/libs/cJSON

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(FRAMEWORKS) $(LIBS) -o $(TARGET)

$(OBJ_DIR)/src/main.o: src/main.c
	$(CC) $(CFLAGS) $(INCLUDES) -x objective-c $(OBJC_FLAGS) -c $< -o $@

$(OBJ_DIR)/src/%.o: src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/libs/sokol/%.o: libs/sokol/%.m
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJC_FLAGS) -c $< -o $@

$(OBJ_DIR)/libs/cJSON/%.o: libs/cJSON/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean directories
