CC = clang
CFLAGS = -std=c11 -O2 -Wall -Wextra
OBJC_FLAGS = -fobjc-arc
INCLUDES = -I$(CURDIR)/libs/sokol -I$(CURDIR)/libs/nuklear -I$(CURDIR)/libs/linmath
FRAMEWORKS = -framework Metal -framework MetalKit -framework Cocoa -framework AudioToolbox -framework QuartzCore

SRCS = src/main.c src/ecs.c src/camera.c src/gui.c src/http_client.c libs/sokol/sokol.m
OBJS = src/main.o src/ecs.o src/camera.o src/gui.o src/http_client.o libs/sokol/sokol.o
LIBS = -lcurl

TARGET = demo

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(FRAMEWORKS) $(LIBS) -o $(TARGET)

src/main.o: src/main.c
	$(CC) $(CFLAGS) $(INCLUDES) -x objective-c $(OBJC_FLAGS) -c $< -o $@

libs/sokol/sokol.o: libs/sokol/sokol.m
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJC_FLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
