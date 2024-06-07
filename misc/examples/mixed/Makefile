EXE = arena

WARN = -Werror -Wall -Wextra -Wno-unused-parameter -Wno-unused-function
CFLAGS += -g3 -fno-omit-frame-pointer -fno-pie -no-pie $(WARN)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: all
all: $(EXE)

.PHONY: clean
clean:
	rm -f $(EXE)

.PHONY: watch
watch:
	fd -ec -eh | entr -cc clang $(WARN) -fsyntax-only -ferror-limit=1 -fmacro-backtrace-limit=1 /_

