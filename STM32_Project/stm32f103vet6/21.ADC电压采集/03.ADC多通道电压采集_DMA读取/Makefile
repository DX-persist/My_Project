# 开启多核编译，极大加快编译速度
MAKEFLAGS += -j8 

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
# 生成依赖文件以支持秒级增量编译
CFLAGS += -MMD -MP 

# ==================== Include paths ====================
INCLUDES  = -ICMSIS
INCLUDES += -IUser
INCLUDES += -ISTM32F10x_StdPeriph_Driver/inc
INCLUDES += -IDrivers/UserLib/inc
INCLUDES += -IFatFs/inc

# ==================== Linker ====================
LDSCRIPT = stm32f103ve.ld
LDFLAGS  = $(MCU)
LDFLAGS += -T$(LDSCRIPT)
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(BUILD)/$(TARGET).map
LDFLAGS += --specs=nano.specs --specs=nosys.specs
#用于printf函数打印浮点型数据
LDFLAGS += -u _printf_float
LDFLAGS += -lc -lm -lnosys

# ==================== Sources ====================
SRCS += CMSIS/system_stm32f10x.c
SRCS += User/main.c
SRCS += User/syscalls.c
SRCS += User/stm32f10x_it.c
SRCS += STM32F10x_StdPeriph_Driver/src/misc.c
SRCS += STM32F10x_StdPeriph_Driver/src/stm32f10x_adc.c
SRCS += STM32F10x_StdPeriph_Driver/src/stm32f10x_dma.c
SRCS += STM32F10x_StdPeriph_Driver/src/stm32f10x_exti.c
SRCS += STM32F10x_StdPeriph_Driver/src/stm32f10x_flash.c
SRCS += STM32F10x_StdPeriph_Driver/src/stm32f10x_fsmc.c
SRCS += STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c
SRCS += STM32F10x_StdPeriph_Driver/src/stm32f10x_i2c.c
SRCS += STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c
SRCS += STM32F10x_StdPeriph_Driver/src/stm32f10x_spi.c
SRCS += STM32F10x_StdPeriph_Driver/src/stm32f10x_usart.c

SRCS += Drivers/UserLib/src/bsp_adc.c
SRCS += Drivers/UserLib/src/bsp_dma.c
SRCS += Drivers/UserLib/src/bsp_delay.c
#SRCS += Drivers/UserLib/src/bsp_eeprom_hi2c.c
SRCS += Drivers/UserLib/src/bsp_eeprom_si2c.c
SRCS += Drivers/UserLib/src/bsp_esp8266.c
SRCS += Drivers/UserLib/src/bsp_exti.c
SRCS += Drivers/UserLib/src/bsp_fatfs.c
SRCS += Drivers/UserLib/src/bsp_fsmc.c
SRCS += Drivers/UserLib/src/bsp_hi2c.c
SRCS += Drivers/UserLib/src/bsp_hspi.c
SRCS += Drivers/UserLib/src/bsp_key.c
SRCS += Drivers/UserLib/src/bsp_lcd.c
SRCS += Drivers/UserLib/src/bsp_led.c
SRCS += Drivers/UserLib/src/bsp_rccclk.c
SRCS += Drivers/UserLib/src/bsp_ringbuffer.c
SRCS += Drivers/UserLib/src/bsp_si2c.c
SRCS += Drivers/UserLib/src/bsp_sspi.c
SRCS += Drivers/UserLib/src/bsp_usart.c
SRCS += Drivers/UserLib/src/bsp_w25q64.c
SRCS += Drivers/UserLib/src/bsp_xpt2046.c

SRCS += FatFs/src/diskio.c
SRCS += FatFs/src/ff.c
SRCS += FatFs/src/ffsystem.c
SRCS += FatFs/src/ffunicode.c

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
	mkdir -p $(BUILD)/FatFs/src

# ==================== Targets ====================
all: firmware

firmware: $(ELF) $(BIN)

flashall: firmware
	@$(MAKE) flash

# 单独剥离出的 json 生成指令
json:
	@echo "Generating compile_commands.json..."
	@python3 gen_cc.py

# ==================== Link ELF ====================
$(ELF): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@
	$(SIZE) $@

# ==================== Generate BIN ====================
$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

# ==================== Compile C sources ====================
$(BUILD)/%.o: %.c | $(BUILD)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# ==================== Compile Startup .S file ====================
$(BUILD)/startup_stm32f10x_hd.o: $(STARTUP) | $(BUILD)
	$(CC) $(MCU) -x assembler-with-cpp -c $< -o $@

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

.PHONY: all firmware clean probe flash flashall disasm json

# 导入 GCC 自动生成的依赖 (.d) 文件，放在文件最末尾
-include $(OBJS:.o=.d)
