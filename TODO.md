# TODO

- [ ] cleanup: consistent use of stdint.h
- [ ] cleanup: consistent linting in function declarations in .c
- [ ] nicetohave: stacktrace()

- [ ] bootloader: load kernel from ide hdd

## Long term

In a next iteration or re-write, things to consider:

- other/better bootloader? UEFI?
- x86_64
- network tcp/ip stack
- switch to gcc as? Would enable us to share defs between C and asm.
- SATA and DMA support
