# Build definitions
CXX=avr-g++
CFLAGS=-Os -Wpedantic --std=c++17
DEVICE_DEFS=-DF_CPU=8000000 -mmcu=attiny84

OBJHEX=avr-objcopy
AVRDUDE=avrdude
DUDE_ARGS=-c usbtiny -p t84 -b 115200 -u

# Source structure
MAIN=main

SRC=src
SRC_LIB=$(SRC)/lib

MAIN_CPP=$(SRC)/$(MAIN).cpp
MAIN_CPP_HDR=$(addprefix $(SRC_LIB)/, portlib.hpp pulsemanager.hpp)

BUILD=build

HEX=$(BUILD)/$(MAIN).hex
ELF=$(BUILD)/$(MAIN).elf

# Targets
.PHONY: all install clean toolchain

all: $(HEX)

install: $(HEX)
	sudo $(AVRDUDE) $(DUDE_ARGS) -U flash:w:$(HEX)

clean:
	rm -f $(ELF) $(HEX)

toolchain: .nvimrc

.nvimrc:
	@echo "Generating local .nvimrc..."
	@echo "let g:syntastic_cpp_compiler = '$(CXX)'" > .nvimrc
	@echo "let g:syntastic_cpp_compiler_options = ' $(DEVICE_DEFS) $(CFLAGS)'" >> .nvimrc

# Build
$(HEX): $(ELF)
	$(OBJHEX) -R .eeprom -O ihex $(ELF) $(HEX)

$(ELF): $(MAIN_CPP) $(MAIN_CPP_HDR)
	$(CXX) $(MAIN_CPP) -o $(ELF) $(DEVICE_DEFS) $(CFLAGS)
