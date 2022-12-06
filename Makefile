#
#   This file is part of x86_vbrkit.
#
#   Copyright 2017 / the`janitor / < email: base64dec(dGhlLmphbml0b3JAcHJvdG9ubWFpbC5jb20=) >
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

# To avoid "missing separator" errors when TABs get converted to spaces
ifeq ($(origin .RECIPEPREFIX), undefined)
  $(error Variable .RECIPEPREFIX is unsupported, upgrade to GNU Make 4.0 or later)
endif
.RECIPEPREFIX = >

CC = gcc
AS = nasm
LD = ld
DISABLED_WARNINGS = -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-function
CFLAGS = -Wall -Iinclude/ -Iinclude/fs/ -std=c99 -Os $(DISABLED_WARNINGS)
CFLAGS_LDR = -ffreestanding -fno-pie -fno-builtin -nostdlib -nostartfiles -nodefaultlibs -fshort-wchar \
	$(DISABLED_WARNINGS)
LFLAGS_LDR = -nostdlib -nostartfiles -nodefaultlibs
LFLAGS = -s -Llib/

SRC_DIR = src
LIB_DIR = lib
BIN_DIR = bin
ASM_DIR = asm
OBJ_DIR = obj

MOUNT_POINT = mp

FAT = 32
FAT_BS = bootsector
FAT_IMG = boot

all: $(BIN_DIR)/fixfs $(BIN_DIR)/$(FAT_BS).bin $(BIN_DIR)/ldr.bin

> @# create the image and format it (FAT32)
> @dd if="/dev/zero" of="$(BIN_DIR)/$(FAT_IMG).img" bs=1M count=64
> @sudo mkfs.fat -F $(FAT) "$(BIN_DIR)/$(FAT_IMG).img"

> @# update the boot sector
> $(BIN_DIR)/fixfs "$(BIN_DIR)/$(FAT_IMG).img" "$(BIN_DIR)/$(FAT_BS).bin"

> @# copy the 2nd stage bootloader
> @sudo mount -t vfat -o loop "$(BIN_DIR)/$(FAT_IMG).img" $(MOUNT_POINT)
> @sudo cp -fv $(BIN_DIR)/ldr.bin $(MOUNT_POINT)
> @sudo touch $(MOUNT_POINT)/payload
> @sudo umount $(MOUNT_POINT)

> @# convert the image to a vmware image
> qemu-img convert -pO vmdk "$(BIN_DIR)/$(FAT_IMG).img" "$(BIN_DIR)/$(FAT_IMG).vmdk"
> cp -f "$(BIN_DIR)/$(FAT_IMG).vmdk" vm/win10x64/

$(BIN_DIR)/fixfs: $(SRC_DIR)/fixfs.c

> $(CC) $(CFLAGS) $(LFLAGS) -o $(BIN_DIR)/fixfs $<

$(BIN_DIR)/$(FAT_BS).bin: $(ASM_DIR)/$(FAT_BS).asm

> $(AS) -f bin -o $@ $<

$(BIN_DIR)/ldr.bin: 		\
	$(OBJ_DIR)/ldr.asm.o 	\
	$(OBJ_DIR)/ldr.c.o 		\
	$(OBJ_DIR)/libc.c.o 	\
	$(OBJ_DIR)/disk.c.o 	\
	$(OBJ_DIR)/pe.c.o 		\
	$(OBJ_DIR)/console.c.o 	\
	$(OBJ_DIR)/video.c.o 	\
	$(OBJ_DIR)/mem.c.o		\
	$(OBJ_DIR)/pff.fs.c.o 	\
	$(OBJ_DIR)/serial.c.o 	\
	$(OBJ_DIR)/diskio.fs.c.o 	

> $(LD) $(LFLAGS_LDR) -m elf_i386 -T linker.ld -o $@ $^

$(OBJ_DIR)/ldr.asm.o: $(ASM_DIR)/ldr.asm

> $(AS) -f elf32 -o $@ $<

$(OBJ_DIR)/%.c.o: $(SRC_DIR)/%.c

> $(CC) $(CFLAGS) $(CFLAGS_LDR) -m32 -o $@ -c $<

$(OBJ_DIR)/%.fs.c.o: $(SRC_DIR)/fs/%.c

> $(CC) $(CFLAGS) $(CFLAGS_LDR) -m32 -o $@ -c $<
	
clean:
> @rm -rfv $(BIN_DIR)/*.bin $(BIN_DIR)/*.vmdk 
> @rm -rfv $(LIB_DIR)/* $(SRC_DIR)/*.o $(ASM_DIR)/*.bin
> @rm -rfv $(OBJ_DIR)/*.o $(ASM_DIR)/*.o

rebuild: clean all

.PHONY: clean
.SILENT: clean