#include <stdio.h>
#include <string.h>

char * PrintCMD (char buffer[]);

int main (void) {

    char input[128];
    memset(input, 0, sizeof(input));
    /*
        Pretending the code has been entered to process a command
    */
    printf("Enter the print command, followed by the echo response\n\r");
    fgets(input, sizeof(input), stdin);

    printf("\n\rEcho: %s\n\r", PrintCMD(input));

    return 0;
}

/* Print command called */
char * PrintCMD (char buffer[]) {
    // Search for substring:
    char * result = strstr(buffer, "-print ");

    result = result != NULL ? result + strlen("-print "): NULL;

    return result;
}