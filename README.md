# x86_vbrkit

`x86_vbrkit` is a small and lightweight FAT32 VBR (Volume Boot Record) bootkit for research purposes, with the intention to simplify the application of chained patches and the deployment of multiple stages. It is fully C99 compliant.

# Build

Requirements: `gcc`, `nasm` and `qemu-utils`

```bash
make clean && make all
```

# Usage

Tested with VMWare and a Windows 10 x64 VM (MBR).

To interact with the virtual machine through a serial port add a `Serial Port` device and configure it to use a socket (named pipe) from `Client` to `An Application`, with `Yield CPU on poll` checked in order to allow the guest OS to use the serial port in polled mode (as opposed to interrupt mode). Before starting the VM, execute the following command:

`socat -d -d unix-listen:/tmp/serial0 stdio`

# License

`x86_vbrkit` is licensed under the Apache License, Version 2.0

