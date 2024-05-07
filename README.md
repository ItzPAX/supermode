# Supermode

Supermode is a program that allows your usermode process to read any arbitrary physical address aswell as virtual address of a process using page table manipulation

## How it works

For a detailed writeup and discussions regarding this project visit this thread:

https://www.unknowncheats.me/forum/anti-cheat-bypass/630419-using-pml4-manipulation.html#post4038289

## Issues

Supermode is unfortunately very unstable and physical memory reads are slow since you have to wait for a TLB flush between each read
