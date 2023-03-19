CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = pstree
SRCDIR =  .
OBJDIR = ./build
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@


$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR) 
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(OBJDIR):
	mkdir -p $@

-include $(OBJS:.o=.d)

clean:
	rm -rf $(TARGET) $(OBJDIR) 

.PHONY: all clean
