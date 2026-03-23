#include <stdio.h>
#include <stdlib.h>

FILE *infile;
FILE *outfile;
unsigned char password[] = "my_password1";

char *key = "A"; 
int key_dir = 1;   
int key_idx = 0;   

int encode(int c) {
    int shift;
    int new_c = c;

    /* Process only alphabetic characters */
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
        
        shift = (key[key_idx] - 'A') * key_dir;

        if (c >= 'a' && c <= 'z') {
            new_c = c + shift;
            while (new_c > 'z') new_c -= 26;
            while (new_c < 'a') new_c += 26;
        } 
        else if (c >= 'A' && c <= 'Z') {
            new_c = c + shift;
            while (new_c > 'Z') new_c -= 26;
            while (new_c < 'A') new_c += 26;
        }

        key_idx++;
        
        if (key[key_idx] == '\0') {
            key_idx = 0;
        }
    }
    
    return new_c;
}

int main(int argc, char **argv) {
    int i;
    int debug_mode = 1; 
    int c;

    infile = stdin;
    outfile = stdout;

    for (i = 0; i < argc; i++) {
        if (debug_mode) {
            fprintf(stderr, "%s\n", argv[i]);
        }

        if (argv[i][0] == '-' && argv[i][1] == 'D' && argv[i][2] == '\0') {
            debug_mode = 0;
        } 
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
            
            if (match) debug_mode = 1;
        }
        else if ((argv[i][0] == '+' || argv[i][0] == '-') && argv[i][1] == 'V') {
            if (argv[i][0] == '+') key_dir = 1;
            else key_dir = -1;
            
            key = &argv[i][2];
        }
        else if (argv[i][0] == '-' && argv[i][1] == 'i') {
            infile = fopen(&argv[i][2], "r");
            if (infile == NULL) {
                fprintf(stderr, "Error: Cannot open input file %s\n", &argv[i][2]);
                return 1; 
            }
        }
        else if (argv[i][0] == '-' && argv[i][1] == 'o') {
            outfile = fopen(&argv[i][2], "w");
            if (outfile == NULL) {
                fprintf(stderr, "Error: Cannot open output file %s\n", &argv[i][2]);
                return 1; 
            }
        }
    }

    /* Read and encode input character by character */
    while (1) {
        c = fgetc(infile);
        
        if (feof(infile)) break;
        
        c = encode(c);
        fputc(c, outfile);
    }

    if (outfile != stdout) fclose(outfile);
    if (infile != stdin) fclose(infile);

    return 0;
}