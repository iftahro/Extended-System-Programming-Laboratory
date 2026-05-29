#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Global Variables ---
char debug_mode = 0;
char file_name[128] = "";
int unit_size = 1;
unsigned char mem_buf[10000];
size_t mem_count = 0;
int display_mode = 0; // 0 for Hexadecimal (default), 1 for Decimal

static char* hex_formats[] = {"%#hhx\n", "%#hx\n", "No such unit", "%#x\n"};
static char* dec_formats[] = {"%#hhd\n", "%#hd\n", "No such unit", "%#d\n"};

// --- Function Prototypes ---
void toggle_debug_mode();
void set_file_name();
void set_unit_size();
void load_into_memory();
void toggle_display_mode();
void memory_display();
void save_into_file();
void memory_modify();
void quit();

// --- Menu Structure ---
struct fun_desc {
    char *name;
    void (*fun)();
};

struct fun_desc menu[] = {
    {"Toggle <D>ebug Mode", toggle_debug_mode},
    {"Set <F>ile Name", set_file_name},
    {"Set <U>nit Size", set_unit_size},
    {"<L>oad Into Memory", load_into_memory},
    {"<T>oggle Display Mode", toggle_display_mode},
    {"<M>emory Display", memory_display},
    {"<S>ave Into File", save_into_file},
    {"Memory Modif<y>", memory_modify},
    {"<Q>uit", quit},
    {NULL, NULL}
};

// --- Implemented Functions ---

void toggle_debug_mode() {
    if (debug_mode == 0) {
        debug_mode = 1;
        fprintf(stderr, "Debug flag now on\n");
    } else {
        debug_mode = 0;
        fprintf(stderr, "Debug flag now off\n");
    }
}

void set_file_name() {
    printf("Enter file name: ");
    char input[128];
    if (fgets(input, sizeof(input), stdin) != NULL) {
        // Remove trailing newline if present
        input[strcspn(input, "\n")] = 0; 
        strncpy(file_name, input, sizeof(file_name) - 1);
        file_name[sizeof(file_name) - 1] = '\0'; // ensure null termination
        
        if (debug_mode) {
            fprintf(stderr, "Debug: file name set to '%s'\n", file_name);
        }
    }
}

void set_unit_size() {
    printf("Enter unit size (1, 2, or 4): ");
    int size;
    char input[128];
    if (fgets(input, sizeof(input), stdin) != NULL) {
        sscanf(input, "%d", &size);
        if (size == 1 || size == 2 || size == 4) {
            unit_size = size;
            if (debug_mode) {
                fprintf(stderr, "Debug: set size to %d\n", unit_size);
            }
        } else {
            fprintf(stderr, "Error: Invalid unit size. Size remains %d.\n", unit_size);
        }
    }
}

void quit() {
    if (debug_mode) {
        fprintf(stderr, "quitting\n");
    }
    exit(0);
}

// --- Stub Functions ---

void load_into_memory() {
    // Check if file name is empty
    if (strcmp(file_name, "") == 0) {
        fprintf(stderr, "Error: File name is empty.\n");
        return;
    }

    // Open file for reading in binary mode
    FILE *file = fopen(file_name, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: Failed to open file '%s'.\n", file_name);
        return;
    }

    // Prompt user for location and length
    printf("Please enter <location> <length>\n");
    char input[128];
    unsigned int location;
    int length;

    if (fgets(input, sizeof(input), stdin) != NULL) {
        // Read location as hex (%x) and length as decimal (%d)
        if (sscanf(input, "%x %d", &location, &length) != 2) {
            fprintf(stderr, "Error: Invalid input format.\n");
            fclose(file);
            return;
        }
    } else {
        fclose(file);
        return;
    }

    // Print debug information if debug mode is on
    if (debug_mode) {
        fprintf(stderr, "Debug: file_name=%s, location=%#x, length=%d\n", file_name, location, length);
    }

    // Move file pointer to the specified location
    fseek(file, location, SEEK_SET);

    // Calculate total bytes to read and read them into mem_buf
    size_t bytes_to_read = length * unit_size;
    size_t bytes_read = fread(mem_buf, 1, bytes_to_read, file);

    // Update the global mem_count (optional, but good practice based on task 0 globals)
    mem_count = bytes_read;

    printf("Loaded %d units into memory\n", length);

    // Close the file
    fclose(file);
}

void toggle_display_mode() {
    if (display_mode == 0) {
        // Currently off (Hex), turn it on (Dec)
        display_mode = 1;
        printf("Decimal display flag now on, decimal representation\n");
    } else {
        // Currently on (Dec), turn it off (Hex)
        display_mode = 0;
        printf("Decimal display flag now off, hexadecimal representation\n");
    }
}

void memory_display() {
    char input[128];
    unsigned int addr;
    int u; // number of units

    printf("Enter address and length\n> ");
    if (fgets(input, sizeof(input), stdin) != NULL) {
        // addr is read as hex (%x), u as decimal (%d)
        if (sscanf(input, "%x %d", &addr, &u) != 2) {
            fprintf(stderr, "Error: Invalid input format.\n");
            return;
        }
    } else {
        return;
    }

    // Print the header based on the display mode
    if (display_mode == 1) {
        printf("Decimal\n=======\n");
    } else {
        printf("Hexadecimal\n===========\n");
    }

    // Determine the starting pointer
    unsigned char* start_ptr;
    if (addr == 0) {
        start_ptr = mem_buf;
    } else {
        start_ptr = (unsigned char*)addr;
    }

    // Iterate and print u units
    for (int i = 0; i < u; i++) {
        // Calculate the exact address for the current unit
        unsigned char* curr_ptr = start_ptr + (i * unit_size);
        int val = 0;

        // Safely extract the value depending on unit_size
        if (unit_size == 1) {
            val = *((unsigned char*)curr_ptr);
        } else if (unit_size == 2) {
            val = *((unsigned short*)curr_ptr);
        } else if (unit_size == 4) {
            val = *((unsigned int*)curr_ptr);
        } else {
            fprintf(stderr, "Error: Invalid unit size.\n");
            return;
        }

        // Print using the format arrays. unit_size-1 gives the correct index (0, 1, or 3)
        if (display_mode == 1) {
            printf(dec_formats[unit_size - 1], val);
        } else {
            printf(hex_formats[unit_size - 1], val);
        }
    }
}

void save_into_file() {
    char input[128];
    unsigned int source_addr;
    unsigned int target_loc;
    int length;

    // Check if file name is empty
    if (strcmp(file_name, "") == 0) {
        fprintf(stderr, "Error: File name is empty.\n");
        return;
    }

    // Prompt user
    printf("Please enter <source-address> <target-location> <length>\n> ");
    if (fgets(input, sizeof(input), stdin) != NULL) {
        if (sscanf(input, "%x %x %d", &source_addr, &target_loc, &length) != 3) {
            fprintf(stderr, "Error: Invalid input format.\n");
            return;
        }
    } else {
        return;
    }

    if (debug_mode) {
        fprintf(stderr, "Debug: source_addr=%#x, target_loc=%#x, length=%d\n", 
                source_addr, target_loc, length);
    }

    // Open file for read/write without truncating ("r+b")
    FILE *file = fopen(file_name, "r+b");
    if (file == NULL) {
        fprintf(stderr, "Error: Failed to open file '%s' for writing.\n", file_name);
        return;
    }

    // Check file size to ensure target_loc is valid
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    if (target_loc > file_size) {
        fprintf(stderr, "Error: target-location %#x is greater than file size %#lx.\n", target_loc, file_size);
        fclose(file);
        return;
    }

    // Determine the source pointer
    unsigned char* source_ptr;
    if (source_addr == 0) {
        source_ptr = mem_buf;
    } else {
        source_ptr = (unsigned char*)source_addr;
    }

    // Move file pointer to target location
    fseek(file, target_loc, SEEK_SET);

    // Write to file
    size_t bytes_to_write = length * unit_size;
    size_t bytes_written = fwrite(source_ptr, 1, bytes_to_write, file);

    if (bytes_written != bytes_to_write) {
        fprintf(stderr, "Error: Failed to write all bytes to file.\n");
    } else {
        printf("Saved %zu bytes to file\n", bytes_written);
    }

    fclose(file);
}

void memory_modify() {
    char input[128];
    unsigned int location;
    unsigned int val;

    // Prompt user for location and val
    printf("Please enter <location> <val>\n> ");
    if (fgets(input, sizeof(input), stdin) != NULL) {
        // Both location and val are read as hexadecimal (%x)
        if (sscanf(input, "%x %x", &location, &val) != 2) {
            fprintf(stderr, "Error: Invalid input format.\n");
            return;
        }
    } else {
        return;
    }

    // Print debug information
    if (debug_mode) {
        fprintf(stderr, "Debug: location=%#x, val=%#x\n", location, val);
    }

    // Check bounds: Ensure we don't write outside the mem_buf (size 10000)
    if (location + unit_size > 10000) {
        fprintf(stderr, "Error: Location is out of bounds.\n");
        return;
    }

    // Pointer to the exact location in our memory buffer
    unsigned char* ptr = mem_buf + location;

    // Write the value into the memory buffer according to the unit size
    if (unit_size == 1) {
        *((unsigned char*)ptr) = (unsigned char)val;
    } else if (unit_size == 2) {
        *((unsigned short*)ptr) = (unsigned short)val;
    } else if (unit_size == 4) {
        *((unsigned int*)ptr) = (unsigned int)val;
    } else {
        fprintf(stderr, "Error: Invalid unit size.\n");
    }
}

// --- Main Program ---
int main(int argc, char **argv) {
    char input[128];
    int bounds = 0;

    // Count menu items
    while (menu[bounds].name != NULL) {
        bounds++;
    }

    while (1) {
        // Print debug info if enabled
        if (debug_mode) {
            fprintf(stderr, "\n--- Debug Info ---\n");
            fprintf(stderr, "unit_size: %d\n", unit_size);
            fprintf(stderr, "file_name: %s\n", file_name);
            fprintf(stderr, "mem_count: %zu\n", mem_count);
            fprintf(stderr, "------------------\n\n");
        }

        // Print menu (without numeric prefixes)
        printf("Choose action:\n");
        for (int i = 0; i < bounds; i++) {
            printf("%s\n", menu[i].name);
        }
        printf("> ");

        // Get user choice as a character
        char choice;
        if (fgets(input, sizeof(input), stdin) != NULL) {
            if (sscanf(input, "%c", &choice) == 1) {
                
                int found = 0;
                // Search for the chosen character in the menu array
                for (int i = 0; i < bounds; i++) {
                    // Find the location of '<' in the current menu name
                    char *bracket = strchr(menu[i].name, '<');
                    
                    // If '<' exists and the next character matches the input
                    if (bracket != NULL && bracket[1] == choice) {
                        menu[i].fun();
                        found = 1;
                        break;
                    }
                }
                
                if (!found) {
                    printf("Invalid action\n");
                }
            }
        }
        printf("\n");
    }
    return 0;
}