CC=gcc
CFLAGS=-march=native -mtune=native -O3 -fomit-frame-pointer -fwrapv

FORRO_REF_SRC=src/forro/ref/forro.c
FORRO_DEBUG_SRC=src/forro/ref/forro-debug.c
FORRO_ARG_SRC=src/forro/forro-args.c
FORRO_DRA_SRC=src/forro/forro-args-dra.c

BUILD_DIR=build

.PHONY: ref ref-args ref-args-dra

ref: $(BUILD_DIR)/xote_ref.o $(BUILD_DIR)/forro_ref.o
ref-args: $(BUILD_DIR)/forro_debug.o $(BUILD_DIR)/forro-args
ref-args-dra: $(BUILD_DIR)/forro_debug.o $(BUILD_DIR)/forro-args-dra

$(BUILD_DIR)/forro_ref.o:
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(CFLAGS) -c $(FORRO_REF_SRC) -o $@

$(BUILD_DIR)/forro_debug.o:
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(CFLAGS) -c $(FORRO_DEBUG_SRC) -o $@

$(BUILD_DIR)/forro-args:
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(CFLAGS) -I. $(BUILD_DIR)/forro_debug.o $(FORRO_ARG_SRC) -o $@

$(BUILD_DIR)/forro-args-dra:
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(CFLAGS) -I. $(BUILD_DIR)/forro_debug.o $(FORRO_DRA_SRC) -o $@

.PHONY : clean
clean:
	@rm -r $(BUILD_DIR)
