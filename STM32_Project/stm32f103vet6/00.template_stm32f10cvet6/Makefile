# ==================== Toolchain ====================
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
SIZE    = arm-none-eabi-size

# ==================== Project ====================
TARGET = stm32f103ve
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
CFLAGS += -DSTM32F10X_HD
CFLAGS += -DUSE_STDPERIPH_DRIVER

# ==================== Include paths ====================
INCLUDES  = -ICMSIS
INCLUDES += -IUser
INCLUDES += -ISTM32F10x_StdPeriph_Driver/inc
INCLUDES += -IDrivers/UserLib/inc

# ==================== Linker ====================
LDSCRIPT = stm32f103ve.ld
LDFLAGS  = $(MCU)
LDFLAGS += -T$(LDSCRIPT)
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(BUILD)/$(TARGET).map
LDFLAGS += --specs=nano.specs --specs=nosys.specs
LDFLAGS += -lc -lm -lnosys

# ==================== Sources ====================
SRCS += CMSIS/system_stm32f10x.c
SRCS += User/main.c
SRCS += User/syscalls.c
SRCS += User/stm32f10x_it.c
SRCS += STM32F10x_StdPeriph_Driver/src/misc.c

SRCS += Drivers/UserLib/src/delay.c

# ==================== Startup ====================
STARTUP = CMSIS/startup_stm32f10x_hd.S

# ==================== Build Objects ====================
OBJS = $(addprefix $(BUILD)/, $(SRCS:.c=.o))
OBJS += $(BUILD)/startup_stm32f10x_hd.o

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
$(BUILD)/startup_stm32f10x_hd.o: $(STARTUP) | $(BUILD)
	$(CC) $(MCU) -x assembler-with-cpp -c $< -o $@

# ==================== Targets (Auto JSON Generation) ====================

# 1. 默认目标 all：自动执行编译+生成json
#    注意：它依赖 .json_trigger，这会触发 Python 脚本
#    Python 脚本内部会调用 make firmware 来执行真正的编译
all: .json_trigger

# 2. 实际编译目标：只负责生成二进制，不负责脚本 (被脚本调用)
firmware: $(ELF) $(BIN)

# 3. 烧录全家桶：执行 all (编译+json)，然后烧录
flashall: all
	@$(MAKE) flash

# 4. 触发器：调用 Python 脚本
.json_trigger:
	@python3 gen_cc.py

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
	rm -f compile_commands.json

# ==================== Disassembly ====================
disasm:
	$(OBJDUMP) -d $(ELF) > $(BUILD)/$(TARGET).dis

# ==================== ST-Link flash ====================
FLASH_ADDR = 0x08000000

probe:
	@echo "=== 探测 ST-Link 设备 ==="
	st-info --probe

flash:
	@echo "=== 烧写固件到 STM32 ==="
	st-flash --connect-under-reset write $(BIN) $(FLASH_ADDR)

.PHONY: all firmware clean probe flash flashall disasm .json_trigger
