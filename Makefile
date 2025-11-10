DEBUG = -g
CC    = qcc
LD    = qcc

TARGET = -Vgcc_ntoaarch64le

SRC_DIR = src
INC_DIR = $(SRC_DIR)/include
OBJDIR  = build
OUTDIR  = out

CFLAGS  += $(DEBUG) $(TARGET) -Wall -I$(INC_DIR) -MMD -MP
LDFLAGS += $(DEBUG) $(TARGET) -lscreen -lEGL -lGLESv2 -lm

SRCS = $(shell find $(SRC_DIR) -name '*.c')
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
BIN  = $(OUTDIR)/main

all: $(BIN)

$(BIN): $(OBJS)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(OBJDIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)

clean:
	rm -rf $(OBJDIR) $(OUTDIR)

