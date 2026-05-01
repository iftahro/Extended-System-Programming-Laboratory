#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int is_little_endian = 0;

typedef struct virus {
    unsigned short SigSize;
    unsigned char* VirusName;
    unsigned char* Sig;
} virus;

typedef struct link link;
struct link {
    link *nextVirus;
    virus *vir;
};

/* Prints buffer contents in hexadecimal format */
void PrintHex(unsigned char* buffer, int length, FILE* output) {
    for (int i = 0; i < length; i++) {
        fprintf(output, "%02X ", buffer[i]);
    }
    fprintf(output, "\n");
}

/* Reads a single virus from the file into dynamically allocated memory */
virus* readVirus(FILE* file) {
    virus* v = (virus*)malloc(sizeof(virus));
    if (!v) return NULL;

    if (fread(&(v->SigSize), 1, 2, file) != 2) {
        free(v);
        return NULL;
    }

    /* The operating system works with little endian, so we only swap if the file is not little endian */
    if (!is_little_endian) {
        v->SigSize = ((v->SigSize >> 8) & 0x00FF) | ((v->SigSize << 8) & 0xFF00);
    }

    v->VirusName = (unsigned char*)malloc(16);
    if (!v->VirusName) {
        free(v);
        return NULL;
    }
    fread(v->VirusName, 1, 16, file);

    v->Sig = (unsigned char*)malloc(v->SigSize);
    if (!v->Sig) {
        free(v->VirusName);
        free(v);
        return NULL;
    }
    fread(v->Sig, 1, v->SigSize, file);

    return v;
}

/* Prints virus details including name, size, and signature */
void printVirus(virus* v, FILE* output) {
    if (!v) return;
    
    fprintf(output, "Virus name: %.16s\n", v->VirusName);
    fprintf(output, "Virus size: %d\n", v->SigSize);
    fprintf(output, "signature:\n");
    PrintHex(v->Sig, v->SigSize, output);
    fprintf(output, "\n");
}

/* Iterates over the linked list and prints each virus */
void list_print(link *virus_list, FILE* output) {
    link* current = virus_list;
    while (current != NULL) {
        printVirus(current->vir, output);
        current = current->nextVirus;
    }
}

/* Appends a new virus node to the end of the linked list */
link* list_append(link* virus_list, virus* data) {
    link* new_link = (link*)malloc(sizeof(link));
    if (!new_link) return virus_list;
    new_link->vir = data;
    new_link->nextVirus = NULL;

    if (virus_list == NULL) {
        return new_link;
    }

    link* current = virus_list;
    while (current->nextVirus != NULL) {
        current = current->nextVirus;
    }
    current->nextVirus = new_link;
    return virus_list;
}

/* Frees all dynamically allocated memory for the list and its contents */
void list_free(link *virus_list) {
    link* current = virus_list;
    while (current != NULL) {
        link* next = current->nextVirus;
        if (current->vir) {
            if (current->vir->VirusName) free(current->vir->VirusName);
            if (current->vir->Sig) free(current->vir->Sig);
            free(current->vir);
        }
        free(current);
        current = next;
    }
}

/* Scans the buffer byte-by-byte to find matching virus signatures */
void detect_virus(char *buffer, unsigned int size, link *virus_list) {
    for (unsigned int i = 0; i < size; i++) {
        link* current = virus_list;
        
        while (current != NULL) {
            virus* v = current->vir;
            
            if (v->SigSize <= size - i) {
                if (memcmp(buffer + i, v->Sig, v->SigSize) == 0) {
                    printf("Starting byte location in the suspected file: %u\n", i);
                    printf("Virus name: %.16s\n", v->VirusName);
                    printf("Virus signature size: %d\n\n", v->SigSize);
                }
            }
            current = current->nextVirus;
        }
    }
}

/* Neutralizes a virus by overwriting its first byte with a RET instruction */
void neutralize_virus(char *fileName, int signatureOffset) {
    FILE *file = fopen(fileName, "r+b");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file to neutralize.\n");
        return;
    }
    
    unsigned char ret_instruction = 0xC3;
    fseek(file, signatureOffset, SEEK_SET);
    fwrite(&ret_instruction, 1, 1, file);
    
    fclose(file);
}

/* Main entry point handling the interactive menu and user input */
int main(int argc, char **argv) {
    link* virus_list = NULL;
    char input[256];
    char suspected_file[256] = {0};
    char choice;

    while (1) {
        printf("<L>oad signatures\n");
        printf("<P>rint signatures\n");
        printf("<S>elect file to inspect\n");
        printf("<D>etect viruses\n");
        printf("<F>ix file\n");
        printf("<Q>uit\n");

        if (fgets(input, sizeof(input), stdin) == NULL) break;
        if (sscanf(input, " %c", &choice) != 1) continue;

        if (choice == 'L' || choice == 'l') {
            char fileName[256];
            printf("Enter signature file name:\n");
            if (fgets(input, sizeof(input), stdin) != NULL) {
                sscanf(input, "%s", fileName);
                
                FILE* file = fopen(fileName, "rb");
                if (!file) {
                    fprintf(stderr, "Error: Could not open file.\n");
                    continue;
                }

                unsigned char magic[4];
                if (fread(magic, 1, 4, file) != 4) {
                    fprintf(stderr, "Error: Could not read magic number.\n");
                    fclose(file);
                    continue;
                }

                if (memcmp(magic, "VIRL", 4) == 0) {
                    is_little_endian = 1;
                } else if (memcmp(magic, "VIRB", 4) == 0) {
                    is_little_endian = 0;
                } else {
                    fprintf(stderr, "Error: Incorrect magic number. File format not recognized.\n");
                    fclose(file);
                    continue;
                }

                if (virus_list != NULL) {
                    list_free(virus_list);
                    virus_list = NULL;
                }

                virus* v;
                while ((v = readVirus(file)) != NULL) {
                    virus_list = list_append(virus_list, v);
                }
                fclose(file);
            }
        }
        else if (choice == 'P' || choice == 'p') {
            if (virus_list != NULL) {
                list_print(virus_list, stdout);
            }
        }
        else if (choice == 'S' || choice == 's') {
            printf("Enter suspected file name:\n");
            if (fgets(input, sizeof(input), stdin) != NULL) {
                sscanf(input, "%s", suspected_file);
            }
        }
        else if (choice == 'D' || choice == 'd') {
            if (suspected_file[0] == '\0') {
                fprintf(stderr, "Error: No suspected file selected. Please use option 'S' first.\n");
                continue;
            }
            
            FILE* s_file = fopen(suspected_file, "rb");
            if (!s_file) {
                fprintf(stderr, "Error: Could not open suspected file: %s\n", suspected_file);
                continue;
            }
            
            char buffer[10000];
            unsigned int bytes_read = fread(buffer, 1, 10000, s_file);
            
            detect_virus(buffer, bytes_read, virus_list);
            
            fclose(s_file);
        }
        else if (choice == 'F' || choice == 'f') {
            if (suspected_file[0] == '\0') {
                fprintf(stderr, "Error: No suspected file selected. Please use option 'S' first.\n");
                continue;
            }
            
            FILE* s_file = fopen(suspected_file, "rb");
            if (!s_file) {
                fprintf(stderr, "Error: Could not open suspected file: %s\n", suspected_file);
                continue;
            }
            
            char buffer[10000];
            unsigned int bytes_read = fread(buffer, 1, 10000, s_file);
            fclose(s_file);
            
            for (unsigned int i = 0; i < bytes_read; i++) {
                link* current = virus_list;
                
                while (current != NULL) {
                    virus* v = current->vir;
                    
                    if (v->SigSize <= bytes_read - i) {
                        if (memcmp(buffer + i, v->Sig, v->SigSize) == 0) {
                            neutralize_virus(suspected_file, i);
                        }
                    }
                    current = current->nextVirus;
                }
            }
        }
        else if (choice == 'Q' || choice == 'q') {
            if (virus_list != NULL) {
                list_free(virus_list);
            }
            break;
        }
        else {
            printf("Invalid option\n");
        }
    }
    
    return 0;
}