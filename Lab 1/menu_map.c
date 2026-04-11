#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Struct definition representing a menu option */
struct fun_desc {
    char *name;
    char index;
    char (*fun)(char);
};

/* Map function from Task 2a */
char* map(char *array, int array_length, char (*f)(char)) {
    char* mapped_array = (char*)(malloc(array_length * sizeof(char)));
    for (int i = 0; i < array_length; i++) {
        mapped_array[i] = f(array[i]);
    }
    return mapped_array;
}

/* ---------------- Task 2b Functions ---------------- */

/* Reads and returns a character from stdin using fgetc */
char my_get(char c) {
    return (char)fgetc(stdin);
}

/* Prints the character and its hex value if printable, otherwise prints a dot and the hex value */
char cxprt(char c) {
    if (c >= 0x20 && c <= 0x7E) {
        printf("%c %02x\n", c, c);
    } else {
        printf(". %02x\n", c);
    }
    return c;
}

/* Encrypts the character by adding 1. No cyclic wrapping is applied. */
char encrypt(char c) {
    if (c >= 0x20 && c <= 0x7F) {
        return c + 1;
    }
    return c;
}

/* Decrypts the character by subtracting 1. No cyclic wrapping is applied. */
char decrypt(char c) {
    if (c >= 0x20 && c <= 0x7F) {
        return c - 1;
    }
    return c;
}

/* Prints the decimal value of the character */
char dprt(char c) {
    printf("%d\n", c);
    return c;
}

/* ---------------- Task 3b Main Menu ---------------- */

int main(int argc, char **argv) {
    char *carray = (char*)malloc(5 * sizeof(char));
    carray[0] = '\0';

    /* 2. Define array of fun_desc and initialize it */
    struct fun_desc menu[] = {
        {"Get String", '0', my_get},
        {"Print Hex", '1', cxprt},
        {"Encrypt", '2', encrypt},
        {"Decrypt", '3', decrypt},
        {"Print Dec", '4', dprt},
        {NULL, 0, NULL} /* Indicator of the end of the array */
    };

    char input[256];

    /* 7. Terminate upon EOF condition */
    while (1) {
        /* 3. Display the menu dynamically */
        printf("Select operation from the following menu:\n");
        int i = 0;
        while (menu[i].name != NULL) {
            printf("%c) %s\n", menu[i].index, menu[i].name);
            i++;
        }
        
        /* 4. Prompt the user for choice */
        printf("Option: ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break; /* EOF encountered (Ctrl+D) */
        }

        /* Extract the first letter of the input */
        char choice = input[0];
        if (choice == '\n') {
            continue;
        }

        int found = 0;
        int selected_index = -1;

        /* Search for the matching index in the menu array */
        for (int j = 0; menu[j].name != NULL; j++) {
            if (menu[j].index == choice) {
                found = 1;
                selected_index = j;
                break;
            }
        }

        if (found) {
            /* 5. Evaluate the appropriate function over carray using map 
               (Called strictly via the function pointer, without if/switch) */
            char *new_carray = map(carray, 5, menu[selected_index].fun);
            
            /* 6. Let carray point to the new array and free the old one to avoid memory leaks */
            free(carray);
            carray = new_carray;
            
        } else {
            printf("function not supported\n");
        }
    }

    free(carray);
    
    return 0;
}