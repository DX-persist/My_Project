# ==================== Toolchain ====================
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
SIZE    = arm-none-eabi-size

# ==================== Project ====================
TARGET = stm32f103c8
BUILD  = build
BIN    = $(BUILD)/$(TARGET).bin
ELF    = $(BUILD)/$(TARGET).elf

# ==================== MCU ====================
CPU     = -mcpu=cortex-m3
MCU     = $(CPU) -mthumb

# ==================== Compiler flags ====================
CFLAGS  = $(MCU)
CFLAGS += -Wall -Wextra -O0 -g
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -std=gnu11
CFLAGS += -DSTM32F10X_MD
CFLAGS += -DUSE_STDPERIPH_DRIVER

# ==================== Include paths ====================
INCLUDES  = -ICMSIS
INCLUDES += -IUser
INCLUDES += -ISTM32F10x_StdPeriph_Driver/inc
INCLUDES += -IDrivers/UserLib/inc

# ==================== Linker ====================
LDSCRIPT = stm32f103c8.ld
LDFLAGS  = $(MCU)
LDFLAGS += -T$(LDSCRIPT)
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(BUILD)/$(TARGET).map
LDFLAGS += -nostdlib -nostartfiles -lgcc

# ==================== Sources ====================
SRCS += CMSIS/system_stm32f10x.c
SRCS += User/main.c
SRCS += User/stm32f10x_it.c
SRCS += STM32F10x_StdPeriph_Driver/src/misc.c
SRCS += STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c
SRCS += STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c
SRCS += Drivers/UserLib/src/delay.c
SRCS += Drivers/UserLib/src/bsp_led.c
SRCS += Drivers/UserLib/src/bsp_key.c

# ==================== Startup ====================
STARTUP = CMSIS/startup_stm32f10x_md.S

# ==================== Build Objects ====================
OBJS = $(addprefix $(BUILD)/, $(SRCS:.c=.o))
OBJS += $(BUILD)/startup_stm32f10x_md.o

# ==================== Build folders ====================
$(BUILD):
	mkdir -p $(BUILD)/CMSIS
	mkdir -p $(BUILD)/User
	mkdir -p $(BUILD)/STM32F10x_StdPeriph_Driver/src
	mkdir -p $(BUILD)/Drivers/UserLib/src

# ==================== Compile C sources ====================
$(BUILD)/%.o: %.c | $(BUILD)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# ==================== Compile Startup .S file ====================
$(BUILD)/startup_stm32f10x_md.o: $(STARTUP) | $(BUILD)
	$(CC) $(MCU) -x assembler-with-cpp -c $< -o $@

# ==================== Main build target ====================
all: $(ELF) $(BIN)

# ==================== Link ELF ====================
$(ELF): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@
	$(SIZE) $@

# ==================== Generate BIN ====================
$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

# ==================== Clean ====================
clean:
	rm -rf $(BUILD)

# ==================== Disassembly ====================
disasm:
	$(OBJDUMP) -d $(ELF) > $(BUILD)/$(TARGET).dis

# ==================== ST-Link flash ====================
FLASH_ADDR = 0x08000000

probe:
	@echo "=== 探测 ST-Link 设备 ==="
	st-info --probe

flash: $(BIN)
	@echo "=== 烧写固件到 STM32 ==="
	st-flash --connect-under-reset write $(BIN) $(FLASH_ADDR)

flashall: all flash
	@echo "=== 编译 + 烧写 完成 ==="
