# Student Management System

A menu-driven C program that stores student records permanently in `students.dat`.

Features: add, display, search, update, and delete student records. Each record includes an ID, name, age, course, and marks.

## Build and run

```powershell
gcc -std=c11 -Wall -Wextra -pedantic student_management.c -o student_management.exe
.\student_management.exe
```

The binary data file is created automatically after the first student is added.
