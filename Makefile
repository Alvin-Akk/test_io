ifeq ($(TOOLS_PATH),)
    TOOLS_PATH = D:/dev/tools
endif

# Directories
MSPGCC_ROOT_DIR    = $(TOOLS_PATH)/msp430-gcc
MSPGCC_BIN_DIR     = $(MSPGCC_ROOT_DIR)/bin
MSPGCC_INCLUDE_DIR = $(MSPGCC_ROOT_DIR)/include
TI_CCS_DIR         = $(TOOLS_PATH)/ccs2041/ccs
DEBUG_BIN_DIR      = $(TI_CCS_DIR)/ccs_base/DebugServer/bin

BUILD_DIR = build
OBJ_DIR   = $(BUILD_DIR)/obj
BIN_DIR   = $(BUILD_DIR)/bin

SRC_DIR = src

LIB_DIRS     = $(MSPGCC_INCLUDE_DIR)
INCLUDE_DIRS = $(MSPGCC_INCLUDE_DIR) \
               $(TI_CCS_DIR)/ccs_base/msp430/include_gcc \
               $(SRC_DIR)

# Toolchain
ifneq ($(wildcard $(MSPGCC_BIN_DIR)/msp430-elf-gcc.exe),)
EXT = .exe
else
EXT =
endif

CC      = $(MSPGCC_BIN_DIR)/msp430-elf-gcc$(EXT)
OBJCOPY = $(MSPGCC_BIN_DIR)/msp430-elf-objcopy$(EXT)
DSLITE  = $(DEBUG_BIN_DIR)/DSLite$(EXT)
RM      = rm

# Files
TARGET = $(BIN_DIR)/test_io
CCXML  = MSP430G2553.ccxml

SOURCES = main.c \
          drivers/mcu_init.c \
          drivers/io.c

OBJECT_NAMES = $(SOURCES:.c=.o)
OBJECTS      = $(patsubst %,$(OBJ_DIR)/%,$(OBJECT_NAMES))

# Flags
MCU     = msp430g2553
WFLAGS  = -Wall -Wextra
CFLAGS  = -mmcu=$(MCU) $(WFLAGS) $(addprefix -I,$(INCLUDE_DIRS)) -Og -g
LDFLAGS = -mmcu=$(MCU) $(addprefix -L,$(LIB_DIRS))

# Build
## Linking
$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $^ -o $@

## Converting
$(TARGET).hex: $(TARGET)
	$(OBJCOPY) -O ihex $< $@

## Compiling
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

# Phonies
.PHONY: all clean flash

all: $(TARGET).hex

clean:
	@$(RM) -rf $(BUILD_DIR)

flash: $(TARGET).hex
	$(DSLITE) load -c $(CCXML) $<