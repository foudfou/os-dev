# Writing a Simple Operating System — from Scratch

Solutions to exercises in the [original book from Nick Blundell](http://www.cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf)

Look into git tag `before-kernel-reorg` for exercises before kernel code organization.

## Writing a Simple Operating System — from Scratch *Continued*

*Writing a Simple Operating System — from Scratch* (WASOS) by Nick Blundell is
incomplete 😭. This is an attempt at completing it 🎉.

We mostly follow the excellent
[hux-kernel](https://github.com/josehu07/hux-kernel) from Guanzhou Hu and
[xv6-x86](https://github.com/mit-pdos/xv6-public), with additions or
modifications from other sources.

Table of contents:

- [6.3 Handling Interrupts](doc/6.3.Handling_Interrupts.md)
- [6.4.1 Keyboard Driver](doc/6.4.1.Keyboard_Driver.md)
- [6.4.2 Timer Driver](doc/6.4.2.Timer_Driver.md)
- [7 Memory Management](doc/7.Memory_Management.md)
- [7.1 Interlude: Building and Utilities](doc/7.1.Building_and_Utilities.md)
- [7.2 Physical Memory Management](doc/7.2.Physical_Memory_Management.md)
- [7.3 Virtual Memory](doc/7.3.Virtual_Memory.md)
- [7.4 Heap Allocator](doc/7.4.Heap_Allocator.md)
- [7.5 Higher half kernel](doc/7.5.Higher_Half_Kernel.md)
- [7.6 Interlude — Spin Locks](doc/7.6.Spin_Locks.md)
- [8 Processes](doc/8.Processes.md)
- [8.1 Process Creation](doc/8.1.Process_Creation.md)
- [8.2 Interlude — Serial Console](doc/8.2.Interlude_Serial_Console.md)
- [8.3 Process Scheduling](doc/8.3.Process_Scheduling.md)

Note the original remaining chapters from WASOS were outlined as:

- 6.3 Handling Interrupts
- 6.4 Keyboard Driver
- 6.5 Hard-disk Driver
- 6.6 File System
- 7 Implementing Processes
- 7.1 Single Processing
- 7.2 Multi-processing
- 8 Memory management

Hux-kernel somewthat follows a different order inspired by the excellent
[*Operating Systems: Three Easy
Pieces*](http://pages.cs.wisc.edu/~remzi/OSTEP/) (OSTEP):

1. Memory
2. Processes
3. Filesystem

## Links

- [Examples' source code](https://github.com/tcharding/os-from-scratch/blob/master/examples/asm/)

A few inspirational sources:

- [ghaiklor-os-gcc](https://github.com/ghaiklor/ghaiklor-os-gcc)
- [hux-kernel](https://github.com/josehu07/hux-kernel)
- [linux 0.01](https://kernel.org/pub/linux/kernel/Historic/)
- [Osdev-Notes](https://github.com/dreamos82/Osdev-Notes)
- [SimpleOS](https://github.com/zzhiyi/SimpleOS)
- [skylight](https://github.com/austanss/skylight)
- [SOS](https://sos.enix.org/)
- [xv6 x86](https://github.com/mit-pdos/xv6-public)
- [brokenthorn](http://www.brokenthorn.com/Resources/OSDevIndex.html)
- [blogos-discuss](https://os.phil-opp.com/)
- [xv6-annotated](https://github.com/palladian1/xv6-annotated)
- [mittos64](https://github.com/thomasloven/mittos64)
