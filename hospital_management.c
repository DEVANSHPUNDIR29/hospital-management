#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PATIENTS 1000
#define NAME_LENGTH 80
#define GENDER_LENGTH 16
#define DIAGNOSIS_LENGTH 120
#define PHONE_LENGTH 24
#define DATA_FILE "patients.dat"

typedef struct {
    int id;
    char name[NAME_LENGTH];
    int age;
    char gender[GENDER_LENGTH];
    char diagnosis[DIAGNOSIS_LENGTH];
    char phone[PHONE_LENGTH];
} Patient;

static Patient patients[MAX_PATIENTS];
static int patient_count = 0;
static int next_id = 1;

static int read_line(const char *prompt, char *buffer, size_t size)
{
    size_t length;

    printf("%s", prompt);
    if (fgets(buffer, (int)size, stdin) == NULL) {
        return 0;
    }

    length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
    } else {
        int character;
        while ((character = getchar()) != '\n' && character != EOF) {
        }
    }
    return 1;
}

static int read_integer(const char *prompt, int minimum, int maximum, int *value)
{
    char input[64];
    char *end;
    long parsed;

    if (!read_line(prompt, input, sizeof(input))) {
        return 0;
    }

    parsed = strtol(input, &end, 10);
    if (input[0] == '\0' || *end != '\0' || parsed < minimum || parsed > maximum) {
        printf("Please enter a number from %d to %d.\n", minimum, maximum);
        return -1;
    }

    *value = (int)parsed;
    return 1;
}

static int save_patients(void)
{
    FILE *file = fopen(DATA_FILE, "wb");
    if (file == NULL) {
        perror("Could not save patient records");
        return 0;
    }

    if (fwrite(&patient_count, sizeof(patient_count), 1, file) != 1 ||
        (patient_count > 0 &&
         fwrite(patients, sizeof(Patient), (size_t)patient_count, file) !=
             (size_t)patient_count)) {
        perror("Could not write patient records");
        fclose(file);
        return 0;
    }

    if (fclose(file) != 0) {
        perror("Could not close patient records file");
        return 0;
    }
    return 1;
}

static void load_patients(void)
{
    FILE *file = fopen(DATA_FILE, "rb");
    int stored_count;
    int i;

    if (file == NULL) {
        return;
    }

    if (fread(&stored_count, sizeof(stored_count), 1, file) != 1 ||
        stored_count < 0 || stored_count > MAX_PATIENTS ||
        (stored_count > 0 &&
         fread(patients, sizeof(Patient), (size_t)stored_count, file) !=
             (size_t)stored_count)) {
        fprintf(stderr, "Warning: patient data file is invalid; starting with no records.\n");
        patient_count = 0;
        fclose(file);
        return;
    }

    patient_count = stored_count;
    for (i = 0; i < patient_count; i++) {
        if (patients[i].id >= next_id) {
            next_id = patients[i].id + 1;
        }
    }
    fclose(file);
}

static void register_patient(void)
{
    Patient patient = {0};
    int result;

    if (patient_count >= MAX_PATIENTS) {
        printf("Patient record limit reached.\n");
        return;
    }

    patient.id = next_id;
    if (!read_line("Patient name: ", patient.name, sizeof(patient.name)) ||
        patient.name[0] == '\0') {
        printf("Name is required.\n");
        return;
    }
    do {
        result = read_integer("Age (0-130): ", 0, 130, &patient.age);
        if (result == 0) {
            return;
        }
    } while (result < 0);

    if (!read_line("Gender: ", patient.gender, sizeof(patient.gender)) ||
        !read_line("Diagnosis: ", patient.diagnosis, sizeof(patient.diagnosis)) ||
        !read_line("Phone: ", patient.phone, sizeof(patient.phone))) {
        return;
    }

    patients[patient_count++] = patient;
    next_id++;
    if (save_patients()) {
        printf("Patient registered. ID: %d\n", patient.id);
    }
}

static void print_patient(const Patient *patient)
{
    printf("ID: %d | Name: %s | Age: %d | Gender: %s\n",
           patient->id, patient->name, patient->age, patient->gender);
    printf("Diagnosis: %s | Phone: %s\n", patient->diagnosis, patient->phone);
}

static void list_patients(void)
{
    int i;

    if (patient_count == 0) {
        printf("No patient records found.\n");
        return;
    }

    printf("\n--- Patient Records (%d) ---\n", patient_count);
    for (i = 0; i < patient_count; i++) {
        print_patient(&patients[i]);
    }
}

static void find_patient(void)
{
    int id;
    int result;
    int i;

    do {
        result = read_integer("Patient ID: ", 1, 2147483647, &id);
        if (result == 0) {
            return;
        }
    } while (result < 0);

    for (i = 0; i < patient_count; i++) {
        if (patients[i].id == id) {
            print_patient(&patients[i]);
            return;
        }
    }
    printf("Patient ID %d was not found.\n", id);
}

static void discharge_patient(void)
{
    int id;
    int result;
    int i;

    do {
        result = read_integer("Patient ID to discharge: ", 1, 2147483647, &id);
        if (result == 0) {
            return;
        }
    } while (result < 0);

    for (i = 0; i < patient_count; i++) {
        if (patients[i].id == id) {
            int j;
            for (j = i; j < patient_count - 1; j++) {
                patients[j] = patients[j + 1];
            }
            patient_count--;
            if (save_patients()) {
                printf("Patient ID %d has been discharged and removed from the records.\n", id);
            }
            return;
        }
    }
    printf("Patient ID %d was not found.\n", id);
}

int main(void)
{
    int choice;
    int result;

    load_patients();
    printf("=== Hospital Management System ===\n");

    for (;;) {
        printf("\n1. Register patient\n");
        printf("2. List patients\n");
        printf("3. Find patient by ID\n");
        printf("4. Discharge patient\n");
        printf("0. Exit\n");

        result = read_integer("Choose an option: ", 0, 4, &choice);
        if (result == 0) {
            printf("\nGoodbye.\n");
            break;
        }
        if (result < 0) {
            continue;
        }

        switch (choice) {
        case 1:
            register_patient();
            break;
        case 2:
            list_patients();
            break;
        case 3:
            find_patient();
            break;
        case 4:
            discharge_patient();
            break;
        case 0:
            printf("Goodbye.\n");
            return 0;
        }
    }
    return 0;
}
