#include <stdio.h>
#include <stdlib.h>

FILE *in_file;
FILE *out_file;
unsigned char debug_pass[] = "my_password1";

char *encoding_string = "A"; 
int shift_direction = 1;   
int enc_idx = 0;   

int encode(int c) {
    int shift;
    int new_c = c;
    /* Process only alphabetic characters */
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
        shift = (encoding_string[enc_idx] - 'A') * shift_direction;
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
        enc_idx++;
        if (encoding_string[enc_idx] == '\0') {
            enc_idx = 0;
        }
    }
    return new_c;
}

int main(int argc, char **argv) {
    int i;
    int is_debug_on = 1; 
    int c;
    in_file = stdin;
    out_file = stdout;
    /* Handle program arguments */
    for (i = 0; i < argc; i++) {
        if (is_debug_on) {
            fprintf(stderr, "%s\n", argv[i]);
        }
        if (argv[i][0] == '-' && argv[i][1] == 'D' && argv[i][2] == '\0') {
            is_debug_on = 0;
        } 
        else if (argv[i][0] == '+' && argv[i][1] == 'D') {
            int j = 0;
            int match = 1;
            
            while (debug_pass[j] != '\0' || argv[i][j + 2] != '\0') {
                if (debug_pass[j] != argv[i][j + 2]) {
                    match = 0;
                    break;
                }
                j++;
            }
            if (match) {
                is_debug_on = 1
            };
        }
        else if ((argv[i][0] == '+' || argv[i][0] == '-') && argv[i][1] == 'V') {
            if (argv[i][0] == '+') {
                shift_direction = 1
            };
            else {
                shift_direction = -1
            };
            encoding_string = &argv[i][2];
        }
        else if (argv[i][0] == '-' && argv[i][1] == 'i') {
            in_file = fopen(&argv[i][2], "r");
            if (in_file == NULL) {
                fprintf(stderr, "Error: Cannot open input file %s\n", &argv[i][2]);
                return 1; 
            }
        }
        else if (argv[i][0] == '-' && argv[i][1] == 'o') {
            out_file = fopen(&argv[i][2], "w");
            if (out_file == NULL) {
                fprintf(stderr, "Error: Cannot open output file %s\n", &argv[i][2]);
                return 1; 
            }
        }
    }
    /* Read and encode input character by character */
    while (1) {
        c = fgetc(in_file);
        if (feof(in_file)) {
            break
        };
        c = encode(c);
        fputc(c, out_file);
    }
    if (out_file != stdout) {
        fclose(out_file)
    };
    if (in_file != stdin) {
        fclose(in_file)
    };
    return 0;
}