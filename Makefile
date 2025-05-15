CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lm

SRCS = main.c battlefield.c equipment.c simulation.c menu.c terrain.c noise.c terrain_generation.c terrain_renderer.c
OBJS = $(SRCS:.c=.o)
TARGET = battlefield_simulator

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-del *.o $(TARGET).exe 2>nul
	-rm -f *.o $(TARGET) 2>/dev/null 