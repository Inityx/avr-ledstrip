MAKEFLAGS+=--no-print-directory

# Source structure
MAIN=main
AVRSUPPORT=avrsupport

SRC=src
MAIN_CPP=$(SRC)/$(MAIN).cpp

BUILD=build

HEX=$(BUILD)/$(MAIN).hex
ELF=$(BUILD)/$(MAIN).elf

# Build definitions
CXX=avr-g++
CFLAGS=-Os -Wpedantic --std=c++17
DEVICE_DEFS=-DF_CPU=8000000 -mmcu=attiny84
AVRSUPPORT_PATH=-I $(AVRSUPPORT)/include

OBJHEX=avr-objcopy
AVRDUDE=avrdude
DUDE_ARGS=-c avrisp2 -p t84 -P /dev/ttyACM0 -u

# Targets
.PHONY: all build install spec fuse clean configure avrsupport

all: build

spec: CFLAGS+= -g
spec: $(ELF)
	avr-objdump --source --demangle $(ELF) | less

fuse:
	sudo $(AVRDUDE) $(DUDE_ARGS) -U lfuse:w:0xd2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m

install: build
	@echo " DUDE $(HEX)"
	@sudo $(AVRDUDE) $(DUDE_ARGS) -U flash:w:$(HEX)

build: $(HEX)

clean:
	@echo " RM $(ELF) $(HEX)"
	@rm -f $(ELF) $(HEX)

# Configure
configure: .nvimrc

.nvimrc: CXX_OPTS=$(DEVICE_DEFS) $(CFLAGS) $(AVRSUPPORT_PATH)
.nvimrc:
	@echo "Generating local .nvimrc..."
	@echo "let g:syntastic_cpp_compiler = '$(CXX)'" > .nvimrc
	@echo "let g:syntastic_cpp_compiler_options = ' $(CXX_OPTS)'" >> .nvimrc

# Build
$(HEX): $(ELF)
	@echo " HEX $(basename $(notdir $@))"
	@$(OBJHEX) -R .eeprom -O ihex $(ELF) $(HEX)

$(ELF): $(MAIN_CPP)
	@echo " CC $(basename $(notdir $@))"
	@$(CXX) $(MAIN_CPP) -o $(ELF) $(DEVICE_DEFS) $(CFLAGS) $(AVRSUPPORT_PATH)

avrsupport:
	@echo " MAKE $(AVRSUPPORT)"
	@$(MAKE) -C $(AVRSUPPORT)
