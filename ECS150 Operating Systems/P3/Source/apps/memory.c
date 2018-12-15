#include "VirtualMachine.h"
#include <fcntl.h>

void VMMain(int argc, char *argv[]){
    int FileDescriptor, Length, Offset;
    char Buffer[121800];

    VMPrint("VMMain opening alice.txt\n");    
    VMFileOpen("alice.txt", O_RDONLY, 0644, &FileDescriptor);
    VMPrint("VMMain VMFileOpen returned %d\n", FileDescriptor);
    
    VMPrint("VMMain reading file\n");
    Length = sizeof(Buffer);
    VMFileRead(FileDescriptor,Buffer,&Length);
    VMPrint("VMMain VMFileRead returned %d\n", Length);
    if(0 <= Length){
        Buffer[Length] = '\0';
        VMPrint("VMMain read in \"%s\"\n", Buffer);
    }
    VMPrint("VMMain closing file\n");
    VMFileClose(FileDescriptor);
    VMPrint("Goodbye\n");    
}

