#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Constants for limits
#define MAX_BOATS 120
#define NAME_LENGTH 128
#define MAX_BOAT_LENGTH 100
#define MAX_SLIP_NUM 85
#define MIN_SLIP_NUM 1
#define FIRST_BAY_LETTER 'A'
#define LAST_BAY_LETTER 'Z'
#define MAX_SPACE_NUM 50
#define MIN_SPACE_NUM 1

// Enum for different types of boat locations
typedef enum { SLIP, LAND, TRAILOR, STORAGE } LocationType;

// Structure to hold boat information
typedef struct {
    char name[NAME_LENGTH]; 
    int length;             
    LocationType location;
    
    // Union to store location-specific info
    union {
        int slipNumber;
        char bayLetter;
        char trailorTag[16];
        int storageNumber;
    } info;
    double amountOwed;
} Boat;

// Array to store pointers to Boat structures
Boat* boats[MAX_BOATS];
int boatCount = 0;

// Charge Rates for different locations
const double RATES[] = {12.5, 14.0, 25.0, 11.2};

// Convert string to lowercase
void toLowerCase(char* dest, const char* src) {
    while (*src) {
        *dest++ = tolower(*src++);
    }
    *dest = '\0';
}

//Finds the index of a boat by name
int findBoatIndex(const char* name) {
    char lowerName[NAME_LENGTH];
    toLowerCase(lowerName, name);
    for (int i = 0; i < boatCount; i++) {
        char boatName[NAME_LENGTH];
        toLowerCase(boatName, boats[i]->name);
        if (strcmp(boatName, lowerName) == 0) return i;
    }
    return -1;
}

// Case-insensitive comparison of two boats by name
int compareBoats(const Boat* a, const Boat* b) {
    return strcasecmp(a->name, b->name);
}

//qsort for boat names in alphabetical order
void quickSortBoats(int low, int high) {
    if (low >= high) return;

    Boat* pivot = boats[high];
    int i = low - 1;

    for (int j = low; j < high; j++) {
        if (compareBoats(boats[j], pivot) < 0) {
            i++;
            Boat* temp = boats[i];
            boats[i] = boats[j];
            boats[j] = temp;
        }
    }

    Boat* temp = boats[i + 1];
    boats[i + 1] = boats[high];
    boats[high] = temp;

    int pivotIndex = i + 1;
    quickSortBoats(low, pivotIndex - 1);
    quickSortBoats(pivotIndex + 1, high);
}

// Sorts all boats alphabetically
void sortBoats() {
    quickSortBoats(0, boatCount - 1);
}


// Loads boat data from a CSV file
void loadCSV(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (boatCount >= MAX_BOATS) break;
        Boat* b = malloc(sizeof(Boat));
        
        //Test malloc errors
        if (!b) {
        printf("Memory allocation failed\n");
        return;
        }
        char type[16];
        char extra[32];
        
        // Parse CSV line into boat fields
        sscanf(line, "%127[^,],%d,%15[^,],%31[^,],%lf", b->name, &b->length, type, extra, &b->amountOwed);
        
        // Set location type and corresponding data
        if (strcmp(type, "slip") == 0) {
            b->location = SLIP;
            b->info.slipNumber = atoi(extra);
        } else if (strcmp(type, "land") == 0) {
            b->location = LAND;
            b->info.bayLetter = extra[0];
        } else if (strcmp(type, "trailor") == 0) {
            b->location = TRAILOR;
            strcpy(b->info.trailorTag, extra);
        } else if (strcmp(type, "storage") == 0) {
            b->location = STORAGE;
            b->info.storageNumber = atoi(extra);
        }
        // Add boat to list
        boats[boatCount++] = b;
    }
    fclose(file);
    sortBoats(); // Sort boats after loading
}

// Saves boat data to a CSV file
void saveCSV(const char* filename) {
    FILE* file = fopen(filename, "w");
    for (int i = 0; i < boatCount; i++) {
        Boat* b = boats[i];
        const char* type;
        char extra[32];
        
        // Set type and extra field based on location
        switch (b->location) {
            case SLIP: type = "slip"; sprintf(extra, "%d", b->info.slipNumber); break;
            case LAND: type = "land"; sprintf(extra, "%c", b->info.bayLetter); break;
            case TRAILOR: type = "trailor"; sprintf(extra, "%s", b->info.trailorTag); break;
            case STORAGE: type = "storage"; sprintf(extra, "%d", b->info.storageNumber); break;
        }
        // Write line to file
        fprintf(file, "%s,%d,%s,%s,%.2f\n", b->name, b->length, type, extra, b->amountOwed);
    }
    fclose(file);
}

// Prints a formatted list of all boats
void printInventory() {
    for (int i = 0; i < boatCount; i++) {
        Boat* b = boats[i];
        printf("%-20s %3d' ", b->name, b->length);
        switch (b->location) {
            case SLIP: printf("   slip   # %2d", b->info.slipNumber); break;
            case LAND: printf("   land      %c", toupper(b->info.bayLetter)); break;
            case TRAILOR: printf("trailor %s", b->info.trailorTag); break;
            case STORAGE: printf("storage   # %2d", b->info.storageNumber); break;
        }
        printf("   Owes $%7.2f\n", b->amountOwed);
    }
}

// Checks if a CSV line has exactly 4 commas (basic format check)
int checkCSV(const char* csvLine) {
    int comma_count = 0;
    for (int i = 0; i < strlen(csvLine); i++) {
        if (csvLine[i] == ',') {
            comma_count++;
        }
    }
   
    if (comma_count == 4) {
        return 1;     // valid csv
    } else {
        printf("Invalid CSV format. Try again.");
        return -1;    // Invalid csv
    }
}


// Adds a new boat to the system from a CSV line
void addBoat(const char* csvLine) {
    if (boatCount >= MAX_BOATS) return;
    if (checkCSV(csvLine) != 1) return;
   
    Boat* b = malloc(sizeof(Boat));
    char type[16], extra[32];
    sscanf(csvLine, "%127[^,],%d,%15[^,],%31[^,],%lf", b->name, &b->length, type, extra, &b->amountOwed);

    for (int i = 0; i < boatCount; i++) {
        if (strcasecmp(boats[i]->name, b->name) == 0) {
            free(b);
            printf("Invalid entry. Boat already exists.\n");
            return;
        }
    }

   
   
    // Check if the boat length exceeds 100
    if (b->length > MAX_BOAT_LENGTH) {
        free(b);
        printf("Boat length exceeds 100' feet.\n");
        return;
    }

    if (strcmp(type, "slip") == 0) {
        b->location = SLIP;
        int slipNumber = atoi(extra);
        if (slipNumber < MIN_SLIP_NUM || slipNumber > MAX_SLIP_NUM) {
            free(b);
            printf("Invalid slip number. Accepted range is 1-85.\n");
            return;
        }

        // Check if the slip number is already taken
        for (int i = 0; i < boatCount; i++) {
            if (boats[i]->location == SLIP && boats[i]->info.slipNumber == slipNumber) {
                free(b);
                printf("Slip number %d is already taken. Invalid entry, try again.\n", slipNumber);
                return;
            }
        }
        b->info.slipNumber = slipNumber;

    } else if (strcmp(type, "land") == 0) {
        b->location = LAND;
       
        char bay = toupper(extra[0]);
       
       
        if (bay < 'A' || bay > 'Z' || strlen(extra) > 1) {
            free(b);
            printf("Invalid bay letter. Accepted range is A-Z.\n");
            return;
        }

        // Check if the bay letter is already taken
        for (int i = 0; i < boatCount; i++) {
            if (boats[i]->location == LAND && toupper(boats[i]->info.bayLetter) == bay) {
                free(b);
                printf("Bay letter %c is already taken. Invalid entry, try again.\n", extra[0]);
                return;
            }
        }
        b->info.bayLetter = extra[0];

    } else if (strcmp(type, "trailor") == 0) {
        b->location = TRAILOR;
        if (strlen(extra) == 0 || strlen(extra) > 31) {
            free(b);
            printf("Invalid licence tag.\n");
            return;
        }

        // Check if the trailor tag is already taken
        for (int i = 0; i < boatCount; i++) {
            if (boats[i]->location == TRAILOR && strcmp(boats[i]->info.trailorTag, extra) == 0) {
                free(b);
                printf("Trailored tag '%s' is already taken. Invalid entry, try again.\n", extra);
                return;
            }
        }
        strcpy(b->info.trailorTag, extra);

    } else if (strcmp(type, "storage") == 0) {
        b->location = STORAGE;
        int storageNumber = atoi(extra);
        if (storageNumber < MIN_SPACE_NUM || storageNumber > MAX_SPACE_NUM) {
            free(b);
            printf("Invalid space number. Accepted range is 1-50.\n");
            return;
        }

        // Check if the storage number is already taken
        for (int i = 0; i < boatCount; i++) {
            if (boats[i]->location == STORAGE && boats[i]->info.storageNumber == storageNumber) {
                free(b);
                printf("Storage space %d is already taken. Invalid entry, try again.\n", storageNumber);
                return;
            }
        }
        b->info.storageNumber = storageNumber;

    } else {
        free(b);
        return;
    }

    boats[boatCount++] = b;
    sortBoats();
}

// Removes a boat by name
void removeBoat(const char* name) {
    int index = findBoatIndex(name);
    if (index == -1) {
        printf("No boat with that name\n");
        return;
    }
    free(boats[index]);
    for (int i = index; i < boatCount - 1; i++) boats[i] = boats[i + 1];
    boatCount--;
}

// Accept a payment for a boat
void acceptPayment(int index, double payment) {
    Boat* b = boats[index];
    if (payment > b->amountOwed) {
        printf("That is more than the amount owed, $%.2f\n", b->amountOwed);
        return;
    }
    b->amountOwed -= payment;
}

// Add monthly charge to each boat
void newMonth() {
    for (int i = 0; i < boatCount; i++) {
        Boat* b = boats[i];
        b->amountOwed += b->length * RATES[b->location];
    }
}

// Free all memory on exit
void freeAllBoats() {
    for (int i = 0; i < boatCount; i++) {
        free(boats[i]);
    }
}


// Main user interface loop
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <CSV file>\n", argv[0]);
        return 1;
    }
    
    // Load existing boats
    loadCSV(argv[1]);
    printf("Welcome to the Boat Management System\n-------------------------------------\n");
    char option[16];
    while (1) {
        printf("\n(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it : ");
        fgets(option, sizeof(option), stdin);
        switch (tolower(option[0])) {
            case 'i': printInventory(); break;
            case 'a': {
                char line[256];
                printf("Please enter the boat data in CSV format                 : ");
                fgets(line, sizeof(line), stdin);
                line[strcspn(line, "\n")] = 0;
                addBoat(line);
                break;
            }
            case 'r': {
                char name[NAME_LENGTH];
                printf("Please enter the boat name                               : ");
                fgets(name, sizeof(name), stdin);
                name[strcspn(name, "\n")] = 0;
                removeBoat(name);
                break;
            }
            case 'p': {
                char name[NAME_LENGTH];
                char amountStr[32];
                double amount;
                printf("Please enter the boat name                               : ");
                fgets(name, sizeof(name), stdin);
                name[strcspn(name, "\n")] = 0;
               
                // Find boat by name
                int index = findBoatIndex(name);
                if (index == -1) {
                    printf("No boat with that name\n");
                    break;
                }
                printf("Please enter the amount to be paid                       : ");
                fgets(amountStr, sizeof(amountStr), stdin);
                amount = atof(amountStr);
                acceptPayment(index, amount);
                break;
            }
            case 'm': newMonth(); break;   // Apply monthly charges
            case 'x': saveCSV(argv[1]); printf("\nExiting the Boat Management System\n"); freeAllBoats(); return 0;  // Exit program and save
            default: printf("Invalid option %c\n", option[0]); break;
        }
    }
}
