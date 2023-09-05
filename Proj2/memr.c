#include <stdio.h>
#include <stdlib.h>

int main() {
    char input[20]; // Assuming a reasonable input size
    unsigned char *mem = NULL;
    unsigned long long address;

    // Prompt user for input as a string
    printf("Enter a hexadecimal memory address: ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("Error reading input.\n");
        return 1;
    }

    // Convert the input string to a hexadecimal address
    if (sscanf(input, "%llx", &address) != 1) {
        printf("Invalid input. Please enter a valid hexadecimal address.\n");
        return 1;
    }

    // Access memory location and print content
    mem = (unsigned char *)address;
    printf("Contents at address 0x%llx: 0x%02x\n", address, *mem);

    return 0;
}
