# imgui_host — native test harness for syn_imgui / syn_canvas
#
# Usage:
#   make               — build ./imgui_host
#   make run           — start HTTP server on :8080
#   make png PAGE=0    — render one page to /tmp/p<N>.png and open it
#   make clean

SYN := ../..

CC      := gcc
CFLAGS  := -std=c99 -Wall -Wextra -Wshadow -Wconversion \
           -I$(SYN) \
           -DSYN_CONFIG_ASSERT_ENABLED=1 \
           -O2 -g

SRCS := main.c \
        $(SYN)/syntropic/display/syn_canvas.c \
        $(SYN)/syntropic/ui/syn_imgui.c

TARGET := imgui_host
VT     := visual_test

VT_SRCS := visual_test.c \
           $(SYN)/syntropic/display/syn_canvas.c \
           $(SYN)/syntropic/ui/syn_imgui.c

$(TARGET): $(SRCS) stb_image_write.h
	$(CC) $(CFLAGS) $(SRCS) -o $@

$(VT): $(VT_SRCS) stb_image_write.h
	$(CC) $(CFLAGS) $(VT_SRCS) -o $@

run: $(TARGET)
	./$(TARGET) --port 8080

PAGE ?= 0
ENC  ?= 0
png: $(TARGET)
	./$(TARGET) --page $(PAGE) --enc $(ENC) --out /tmp/p$(PAGE).png
	@echo "Wrote /tmp/p$(PAGE).png"

visual: $(VT)
	./$(VT)
	@echo "Wrote /tmp/imgui_visual_test.png"

clean:
	rm -f $(TARGET) $(VT)

.PHONY: run png visual clean
