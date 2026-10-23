#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define DATA_FILE "students.dat"
#define NAME_LEN 60
#define COURSE_LEN 60

typedef struct {
    int id;
    char name[NAME_LEN];
    int age;
    char course[COURSE_LEN];
    float marks;
} Student;

static void discard_rest_of_line(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) { }
}

static int read_line(const char *prompt, char *buffer, size_t size) {
    size_t len;
    printf("%s", prompt);
    if (!fgets(buffer, (int)size, stdin)) return 0;
    len = strlen(buffer);
    if (len && buffer[len - 1] == '\n') buffer[len - 1] = '\0';
    else discard_rest_of_line();
    return 1;
}

static int read_int(const char *prompt, int *value) {
    char line[100], *end;
    long parsed;
    for (;;) {
        if (!read_line(prompt, line, sizeof line)) return 0;
        errno = 0;
        parsed = strtol(line, &end, 10);
        while (isspace((unsigned char)*end)) end++;
        if (!errno && end != line && *end == '\0' && parsed >= -2147483647L - 1 && parsed <= 2147483647L) {
            *value = (int)parsed;
            return 1;
        }
        puts("Please enter a valid whole number.");
    }
}

static int read_float(const char *prompt, float *value) {
    char line[100], *end;
    float parsed;
    for (;;) {
        if (!read_line(prompt, line, sizeof line)) return 0;
        errno = 0;
        parsed = strtof(line, &end);
        while (isspace((unsigned char)*end)) end++;
        if (!errno && end != line && *end == '\0') {
            *value = parsed;
            return 1;
        }
        puts("Please enter a valid number.");
    }
}

static int id_exists(int id) {
    FILE *file = fopen(DATA_FILE, "rb");
    Student student;
    if (!file) return 0;
    while (fread(&student, sizeof student, 1, file) == 1) {
        if (student.id == id) { fclose(file); return 1; }
    }
    fclose(file);
    return 0;
}

static void print_student(const Student *s) {
    printf("ID: %-8d Name: %-20s Age: %-3d Course: %-18s Marks: %.2f\n",
           s->id, s->name, s->age, s->course, s->marks);
}

static void add_student(void) {
    FILE *file;
    Student s;
    puts("\n--- Add Student ---");
    if (!read_int("ID: ", &s.id)) return;
    if (s.id <= 0) { puts("ID must be positive."); return; }
    if (id_exists(s.id)) { puts("That ID already exists."); return; }
    if (!read_line("Name: ", s.name, sizeof s.name) || s.name[0] == '\0') { puts("Name is required."); return; }
    if (!read_int("Age: ", &s.age) || s.age <= 0 || s.age > 150) { puts("Enter a valid age."); return; }
    if (!read_line("Course: ", s.course, sizeof s.course) || s.course[0] == '\0') { puts("Course is required."); return; }
    if (!read_float("Marks (0-100): ", &s.marks) || s.marks < 0 || s.marks > 100) { puts("Marks must be from 0 to 100."); return; }
    file = fopen(DATA_FILE, "ab");
    if (!file) { perror("Could not open data file"); return; }
    if (fwrite(&s, sizeof s, 1, file) != 1) perror("Could not save record");
    else puts("Student added successfully.");
    fclose(file);
}

static void display_students(void) {
    FILE *file = fopen(DATA_FILE, "rb");
    Student s;
    int count = 0;
    puts("\n--- Student Records ---");
    if (!file) { puts("No records found."); return; }
    while (fread(&s, sizeof s, 1, file) == 1) { print_student(&s); count++; }
    if (!count) puts("No records found.");
    fclose(file);
}

static void search_student(void) {
    FILE *file;
    Student s;
    int id;
    puts("\n--- Search Student ---");
    if (!read_int("Enter ID: ", &id)) return;
    file = fopen(DATA_FILE, "rb");
    if (!file) { puts("No records found."); return; }
    while (fread(&s, sizeof s, 1, file) == 1) {
        if (s.id == id) { print_student(&s); fclose(file); return; }
    }
    fclose(file);
    puts("Student not found.");
}

static void update_student(void) {
    FILE *file;
    Student s;
    int id;
    puts("\n--- Update Student ---");
    if (!read_int("Enter ID: ", &id)) return;
    file = fopen(DATA_FILE, "r+b");
    if (!file) { puts("No records found."); return; }
    while (fread(&s, sizeof s, 1, file) == 1) {
        if (s.id == id) {
            print_student(&s);
            if (!read_line("New name: ", s.name, sizeof s.name) || !s.name[0] ||
                !read_int("New age: ", &s.age) || s.age <= 0 || s.age > 150 ||
                !read_line("New course: ", s.course, sizeof s.course) || !s.course[0] ||
                !read_float("New marks (0-100): ", &s.marks) || s.marks < 0 || s.marks > 100) {
                puts("Invalid input; record was not changed."); fclose(file); return;
            }
            fseek(file, -(long)sizeof s, SEEK_CUR);
            if (fwrite(&s, sizeof s, 1, file) == 1) puts("Student updated successfully.");
            else perror("Could not update record");
            fclose(file); return;
        }
    }
    fclose(file);
    puts("Student not found.");
}

static void delete_student(void) {
    FILE *source, *temp;
    Student s;
    int id, found = 0;
    puts("\n--- Delete Student ---");
    if (!read_int("Enter ID: ", &id)) return;
    source = fopen(DATA_FILE, "rb");
    if (!source) { puts("No records found."); return; }
    temp = fopen("students.tmp", "wb");
    if (!temp) { perror("Could not create temporary file"); fclose(source); return; }
    while (fread(&s, sizeof s, 1, source) == 1) {
        if (s.id == id) found = 1;
        else if (fwrite(&s, sizeof s, 1, temp) != 1) { perror("Write error"); fclose(source); fclose(temp); return; }
    }
    fclose(source);
    fclose(temp);
    if (!found) { remove("students.tmp"); puts("Student not found."); return; }
    if (remove(DATA_FILE) != 0 || rename("students.tmp", DATA_FILE) != 0) perror("Could not replace data file");
    else puts("Student deleted successfully.");
}

int main(void) {
    int choice;
    puts("Student Management System");
    for (;;) {
        puts("\n1. Add student\n2. Display students\n3. Search student\n4. Update student\n5. Delete student\n6. Exit");
        if (!read_int("Choose an option: ", &choice)) break;
        switch (choice) {
            case 1: add_student(); break;
            case 2: display_students(); break;
            case 3: search_student(); break;
            case 4: update_student(); break;
            case 5: delete_student(); break;
            case 6: puts("Thank you for using the Student Management System."); return 0;
            default: puts("Choose a number from 1 to 6.");
        }
    }
    return 0;
}
