#include <stdio.h>
#include <stdlib.h>

/* Global variables as recommended */
FILE *infile;
FILE *outfile;
unsigned char password[] = "my_password1";

/* Encode function for Part 1: returns the character unchanged */
int encode(int c) {
    return c;
}

int main(int argc, char **argv) {
    int i;
    int debug_mode = 1; /* Debug mode is ON by default */
    int c;

    /* Initialize file pointers to standard streams */
    infile = stdin;
    outfile = stdout;

    /* Parse command-line arguments */
    for (i = 1; i < argc; i++) {
        /* Print the argument to stderr if debug mode is currently ON */
        if (debug_mode) {
            fprintf(stderr, "%s\n", argv[i]);
        }

        /* Check if the argument is "-D" to turn debug mode OFF */
        if (argv[i][0] == '-' && argv[i][1] == 'D' && argv[i][2] == '\0') {
            debug_mode = 0;
        } 
        /* Check if the argument starts with "+D" to turn debug mode ON */
        else if (argv[i][0] == '+' && argv[i][1] == 'D') {
            int j = 0;
            int match = 1;
            
            /* Compare the rest of the argument with the password character by character */
            while (password[j] != '\0' || argv[i][j + 2] != '\0') {
                if (password[j] != argv[i][j + 2]) {
                    match = 0;
                    break;
                }
                j++;
            }
            
            if (match) {
                debug_mode = 1;
            }
        }
    }

    /* Input echoing loop */
    while (1) {
        c = fgetc(infile);
        
        /* Break the loop if EOF is detected */
        if (feof(infile)) {
            break;
        }
        
        c = encode(c);
        fputc(c, outfile);
    }

    /* Close the output stream if it's not stdout */
    if (outfile != stdout) {
        fclose(outfile);
    }
    
    /* Close the input stream if it's not stdin (preparation for Part 3) */
    if (infile != stdin) {
        fclose(infile);
    }

    return 0;
}