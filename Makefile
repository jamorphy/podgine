CC = clang
CFLAGS = -std=c11 -O2 -Wall -Wextra
OBJC_FLAGS = -fobjc-arc
INCLUDES = -I$(CURDIR)/libs/sokol -I$(CURDIR)/libs/nuklear
FRAMEWORKS = -framework Metal -framework MetalKit -framework Cocoa -framework AudioToolbox -framework QuartzCore

SRCS = src/main.c src/ecs.c libs/sokol/sokol.m
OBJS = src/main.o src/ecs.o libs/sokol/sokol.o

TARGET = demo

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(FRAMEWORKS) -o $(TARGET)

src/main.o: src/main.c
	$(CC) $(CFLAGS) $(INCLUDES) -x objective-c $(OBJC_FLAGS) -c $< -o $@

libs/sokol/sokol.o: libs/sokol/sokol.m
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJC_FLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
