#include <stdio.h>
#include <stdlib.h>

/* Global variables */
FILE *infile;
FILE *outfile;
unsigned char password[] = "my_password1";

/* Vigenere Cipher globals */
char *key = "A"; /* Default key: 'A' means shift by 0 */
int key_dir = 1;   /* 1 for addition (+V), -1 for subtraction (-V) */
int key_idx = 0;   /* Current position in the key */

/* Encode function for Part 2: applies Vigenere cipher */
int encode(int c) {
    int shift;
    int new_c = c;

    /* Check if character is a letter using manual comparison */
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
        
        /* Calculate shift value (0 to 25) and apply direction (+ or -) */
        shift = (key[key_idx] - 'A') * key_dir;

        /* Apply shift for lowercase letters */
        if (c >= 'a' && c <= 'z') {
            new_c = c + shift;
            /* Handle wrap-around */
            while (new_c > 'z') {
                new_c -= 26;
            }
            while (new_c < 'a') {
                new_c += 26;
            }
        } 
        /* Apply shift for uppercase letters */
        else if (c >= 'A' && c <= 'Z') {
            new_c = c + shift;
            /* Handle wrap-around */
            while (new_c > 'Z') {
                new_c -= 26;
            }
            while (new_c < 'A') {
                new_c += 26;
            }
        }

        /* Advance key index only when a letter is encoded */
        key_idx++;
        
        /* Reset key index if we reached the end of the key string */
        if (key[key_idx] == '\0') {
            key_idx = 0;
        }
    }
    
    /* Non-letter characters skip the if-block and return unchanged */
    return new_c;
}

int main(int argc, char **argv) {
    int i;
    int debug_mode = 1; /* Debug mode is ON by default */
    int c;

    /* Initialize file pointers to standard streams */
    infile = stdin;
    outfile = stdout;

    /* Parse command-line arguments starting from 0 to include program name */
    for (i = 0; i < argc; i++) {
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
        /* Check if the argument is an encoding key (+V... or -V...) */
        else if ((argv[i][0] == '+' || argv[i][0] == '-') && argv[i][1] == 'V') {
            if (argv[i][0] == '+') {
                key_dir = 1;
            } else {
                key_dir = -1;
            }
            /* Set the key pointer to start exactly after the 'V' */
            key = &argv[i][2];
        }
    }

    /* Input processing loop */
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
    
    /* Close the input stream if it's not stdin */
    if (infile != stdin) {
        fclose(infile);
    }

    return 0;
}