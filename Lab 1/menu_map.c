#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct fun_desc {
    char *name;
    char index;
    char (*fun)(char);
};

char* map(char *array, int array_length, char (*f)(char)) {
    char* mapped_array = (char*)(malloc(array_length * sizeof(char)));
    for (int i = 0; i < array_length; i++) {
        mapped_array[i] = f(array[i]);
    }
    return mapped_array;
}

char my_get(char c) {
    return (char)fgetc(stdin);
}

char cxprt(char c) {
    if (c >= 0x20 && c <= 0x7E) {
        printf("%c %02x\n", c, c);
    } else {
        printf(". %02x\n", c);
    }
    return c;
}

char encrypt(char c) {
    if (c >= 0x20 && c <= 0x7F) {
        return c + 1;
    }
    return c;
}

char decrypt(char c) {
    if (c >= 0x20 && c <= 0x7F) {
        return c - 1;
    }
    return c;
}

char dprt(char c) {
    printf("%d\n", c);
    return c;
}


int main(int argc, char **argv) {
    char* carray = (char*)malloc(5 * sizeof(char));
    carray[0] = '\0'; // To indicate the string is empty.

    struct fun_desc menu[] = {
        {"Get String", '0', my_get},
        {"Print Hex", '1', cxprt},
        {"Encrypt", '2', encrypt},
        {"Decrypt", '3', decrypt},
        {"Print Dec", '4', dprt},
        {NULL, 0, NULL}
    };

    char input[256];

    while (1) {
        printf("Select operation from the following menu:\n");
        int i = 0;
        while (menu[i].name != NULL) {
            printf("%c) %s\n", menu[i].index, menu[i].name);
            i++;
        }
        
        printf("Option: ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break; // If EOF (Ctrl+D)
        }

        char choice = input[0];
        if (choice == '\n') {
            continue;
        }

        int found = 0;
        int selected_index = -1;
        for (int j = 0; menu[j].name != NULL; j++) {
            if (menu[j].index == choice) {
                found = 1;
                selected_index = j;
                break;
            }
        }
        if (found) {
            char *new_carray = map(carray, 5, menu[selected_index].fun);
            free(carray);
            carray = new_carray;
            
        } else {
            printf("function not supported\n");
        }
    }

    free(carray);
    
    return 0;
}