/*
 * Student Result and Attendance Analyzer
 * --------------------------------------
 * A menu-driven program that stores student marks and attendance,
 * calculates results, prints reports and saves everything to a text file.
 *
 * Compile:  gcc -Wall -Wextra program.c -o program
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* ---------- Constants (change them here, not all over the code) ---------- */
#define MAX_STUDENTS 100       /* maximum students we can store           */
#define NUM_SUBJECTS 5         /* subjects per student                    */
#define NAME_LENGTH 50         /* name buffer size (49 characters + '\0') */
#define LINE_SIZE 100          /* buffer size for reading one line        */
#define MAX_ROLL 999999        /* biggest roll number allowed             */
#define PASS_MARK 33           /* below this in any subject = fail        */
#define ATTENDANCE_LIMIT 75.0f /* below this = attendance warning         */
#define TABLE_WIDTH 80         /* width of the main table's divider line  */
#define DATA_FILE "students.txt"

/* ---------- Data structure ---------- */
struct Student
{
    int rollNumber;
    char name[NAME_LENGTH];
    int marks[NUM_SUBJECTS];
    float attendance; /* percentage, 0 to 100                       */
    int total;        /* sum of all subject marks (calculated)      */
    float percentage; /* total / number of subjects (calculated)    */
    char grade[3];    /* "A+", "A", "B", "C", "D" or "F" (calculated) */
};

/* ---------- Global data ---------- */
struct Student students[MAX_STUDENTS]; /* all students live here          */
int studentCount = 0;                  /* how many are currently stored   */
int inputEnded = 0;                    /* becomes 1 if the input closes   */

char subjectNames[NUM_SUBJECTS][12] = {"English", "Maths", "Science", "Social", "Computer"};
char shortNames[NUM_SUBJECTS][6] = {"Eng", "Math", "Sci", "Soc", "Comp"};

/* =====================================================================
 *  PART 1: SAFE INPUT HELPERS
 *  We read every input as a whole line with fgets and then convert it.
 *  This avoids the classic scanf problems (leftover newline, letters
 *  typed instead of numbers, infinite loops).
 * ===================================================================== */

/* Reads one line from the keyboard into buffer and removes the '\n'.
   Returns 1 on success, 0 if the input has ended (Ctrl+D / Ctrl+Z). */
int readLine(char buffer[], int size)
{
    int length;
    int ch;

    if (fgets(buffer, size, stdin) == NULL)
    {
        inputEnded = 1;
        buffer[0] = '\0';
        return 0;
    }

    length = (int)strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n')
    {
        buffer[length - 1] = '\0'; /* remove the newline */
    }
    else
    {
        /* The line was longer than the buffer: throw away the extra
           characters so they do not appear in the next question. */
        while ((ch = getchar()) != '\n' && ch != EOF)
        {
            /* just skipping characters */
        }
    }
    return 1;
}

/* Removes spaces from the start and end of a string (in place). */
void trimSpaces(char text[])
{
    int start = 0;
    int end = (int)strlen(text) - 1;
    int i;

    while (text[start] != '\0' && isspace((unsigned char)text[start]))
    {
        start++;
    }
    while (end >= start && isspace((unsigned char)text[end]))
    {
        end--;
    }
    for (i = start; i <= end; i++)
    {
        text[i - start] = text[i];
    }
    text[end - start + 1] = '\0';
}

/* Keeps asking until the user types a whole number between min and max.
   Letters, decimals, empty input or out-of-range numbers are rejected. */
int getInt(const char prompt[], int min, int max)
{
    char line[LINE_SIZE];
    int value;
    char extra;

    while (1)
    {
        printf("%s", prompt);
        if (!readLine(line, LINE_SIZE))
        {
            return min; /* input closed: stop asking */
        }
        /* "%d %c" gives 1 only when there is a number and NOTHING after it */
        if (sscanf(line, "%d %c", &value, &extra) == 1 && value >= min && value <= max)
        {
            return value;
        }
        printf("Invalid input. Please enter a whole number from %d to %d.\n", min, max);
    }
}

/* Same idea as getInt, but for decimal numbers (used for attendance). */
float getFloat(const char prompt[], float min, float max)
{
    char line[LINE_SIZE];
    float value;
    char extra;

    while (1)
    {
        printf("%s", prompt);
        if (!readLine(line, LINE_SIZE))
        {
            return min;
        }
        if (sscanf(line, "%f %c", &value, &extra) == 1 && value >= min && value <= max)
        {
            return value;
        }
        printf("Invalid input. Please enter a number from %.0f to %.0f.\n", min, max);
    }
}

/* Reads a name (spaces allowed) and refuses an empty name. */
void getName(const char prompt[], char name[], int size)
{
    while (1)
    {
        printf("%s", prompt);
        if (!readLine(name, size))
        {
            name[0] = '\0';
            return;
        }
        trimSpaces(name);
        if (name[0] != '\0')
        {
            return;
        }
        printf("Name cannot be empty.\n");
    }
}

/* Asks a yes/no question. Returns 1 only if the answer starts with y or Y. */
int askYesNo(const char prompt[])
{
    char line[LINE_SIZE];

    printf("%s", prompt);
    if (!readLine(line, LINE_SIZE))
    {
        return 0;
    }
    trimSpaces(line);
    return (line[0] == 'y' || line[0] == 'Y');
}

/* =====================================================================
 *  PART 2: SMALL HELPER FUNCTIONS
 * ===================================================================== */

/* Returns the array position of the student with this roll number,
   or -1 if no such student exists. */
int findStudentByRoll(int roll)
{
    int i;
    for (i = 0; i < studentCount; i++)
    {
        if (students[i].rollNumber == roll)
        {
            return i;
        }
    }
    return -1;
}

/* Copies source into result, converting every letter to lowercase.
   Used so that name searching ignores capital letters. */
void makeLowercase(const char source[], char result[])
{
    int i;
    for (i = 0; source[i] != '\0'; i++)
    {
        result[i] = (char)tolower((unsigned char)source[i]);
    }
    result[i] = '\0';
}

/* Returns 1 if the student has marks below PASS_MARK in any subject. */
int hasFailedSubject(int index)
{
    int i;
    for (i = 0; i < NUM_SUBJECTS; i++)
    {
        if (students[index].marks[i] < PASS_MARK)
        {
            return 1;
        }
    }
    return 0;
}

/* Decides the grade from the percentage.
   Failing even one subject always gives grade F. */
void getGrade(float percentage, int failedAnySubject, char grade[])
{
    if (failedAnySubject)
    {
        strcpy(grade, "F");
    }
    else if (percentage >= 90)
    {
        strcpy(grade, "A+");
    }
    else if (percentage >= 80)
    {
        strcpy(grade, "A");
    }
    else if (percentage >= 70)
    {
        strcpy(grade, "B");
    }
    else if (percentage >= 60)
    {
        strcpy(grade, "C");
    }
    else
    {
        strcpy(grade, "D");
    }
}

/* Calculates total, percentage and grade for the student at this position. */
void calculateResult(int index)
{
    int i;
    int sum = 0;

    for (i = 0; i < NUM_SUBJECTS; i++)
    {
        sum += students[index].marks[i];
    }
    students[index].total = sum;
    students[index].percentage = (float)sum / NUM_SUBJECTS; /* each subject is out of 100 */
    getGrade(students[index].percentage, hasFailedSubject(index), students[index].grade);
}

/* Prints a line of dashes of the given length. */
void printLine(int length)
{
    int i;
    for (i = 0; i < length; i++)
    {
        putchar('-');
    }
    putchar('\n');
}

/* Prints the column titles of the main student table. */
void printTableHeader(void)
{
    int i;
    printf("%-6s %-20s", "Roll", "Name");
    for (i = 0; i < NUM_SUBJECTS; i++)
    {
        printf("%5s", shortNames[i]);
    }
    printf(" %6s %7s %6s %5s\n", "Total", "Perc(%)", "Att(%)", "Grade");
    printLine(TABLE_WIDTH);
}

/* Prints one student as one row of the main table. */
void printStudentRow(int index)
{
    int i;
    printf("%-6d %-20.20s", students[index].rollNumber, students[index].name);
    for (i = 0; i < NUM_SUBJECTS; i++)
    {
        printf("%5d", students[index].marks[i]);
    }
    printf(" %6d %7.2f %6.2f %5s\n", students[index].total, students[index].percentage,
           students[index].attendance, students[index].grade);
}

/* Prints the full report card of one student. */
void printReportCard(int index)
{
    int i;

    printf("\n==============================\n");
    printf("         REPORT CARD\n");
    printf("==============================\n");
    printf("Roll Number : %d\n", students[index].rollNumber);
    printf("Name        : %s\n", students[index].name);
    printf("------------------------------\n");
    printf("%-10s %-8s %s\n", "Subject", "Marks", "Status");
    for (i = 0; i < NUM_SUBJECTS; i++)
    {
        printf("%-10s %3d/100  %s\n", subjectNames[i], students[index].marks[i],
               students[index].marks[i] < PASS_MARK ? "FAIL" : "Pass");
    }
    printf("------------------------------\n");
    printf("Total       : %d / %d\n", students[index].total, NUM_SUBJECTS * 100);
    printf("Percentage  : %.2f%%\n", students[index].percentage);
    printf("Grade       : %s\n", students[index].grade);
    printf("Attendance  : %.2f%%", students[index].attendance);
    if (students[index].attendance < ATTENDANCE_LIMIT)
    {
        printf("  (WARNING: below %.0f%%)", ATTENDANCE_LIMIT);
    }
    printf("\nFinal Result: %s\n", hasFailedSubject(index) ? "FAIL" : "PASS");
    printf("==============================\n");
}

/* =====================================================================
 *  PART 3: FILE HANDLING (save and load)
 *  File format - three lines per student:
 *      roll number
 *      name (may contain spaces)
 *      mark1 mark2 mark3 mark4 mark5 attendance
 * ===================================================================== */

/* Writes every student to the data file. Called after every change. */
void saveData(void)
{
    FILE *file;
    int i;
    int j;

    file = fopen(DATA_FILE, "w");
    if (file == NULL)
    {
        printf("Error: could not open %s for saving.\n", DATA_FILE);
        return;
    }

    for (i = 0; i < studentCount; i++)
    {
        fprintf(file, "%d\n%s\n", students[i].rollNumber, students[i].name);
        for (j = 0; j < NUM_SUBJECTS; j++)
        {
            fprintf(file, "%d ", students[i].marks[j]);
        }
        fprintf(file, "%f\n", students[i].attendance);
    }
    fclose(file);
}

/* Reads the data file when the program starts.
   If the file does not exist (first run) we simply start empty. */
void loadData(void)
{
    FILE *file;
    int roll;
    int marks[NUM_SUBJECTS];
    char name[NAME_LENGTH];
    float attendance;
    int i;
    int valid;

    file = fopen(DATA_FILE, "r");
    if (file == NULL)
    {
        printf("No saved data found (first run). Starting with an empty list.\n");
        return;
    }

    while (studentCount < MAX_STUDENTS)
    {
        /* Read the three parts of one record; stop at the end of the file. */
        if (fscanf(file, "%d", &roll) != 1)
        {
            break;
        }
        if (fscanf(file, " %49[^\n]", name) != 1)
        { /* 49 = NAME_LENGTH - 1 */
            break;
        }
        if (fscanf(file, "%d %d %d %d %d %f", &marks[0], &marks[1], &marks[2],
                   &marks[3], &marks[4], &attendance) != 6)
        {
            break;
        }

        /* Check the record is sensible before accepting it. */
        valid = (roll >= 1 && roll <= MAX_ROLL && findStudentByRoll(roll) == -1);
        for (i = 0; i < NUM_SUBJECTS; i++)
        {
            if (marks[i] < 0 || marks[i] > 100)
            {
                valid = 0;
            }
        }
        if (!(attendance >= 0 && attendance <= 100))
        {
            valid = 0;
        }
        if (!valid)
        {
            printf("Warning: skipped an invalid record (roll %d) in %s.\n", roll, DATA_FILE);
            continue;
        }

        students[studentCount].rollNumber = roll;
        strcpy(students[studentCount].name, name);
        for (i = 0; i < NUM_SUBJECTS; i++)
        {
            students[studentCount].marks[i] = marks[i];
        }
        students[studentCount].attendance = attendance;
        calculateResult(studentCount);
        studentCount++;
    }

    /* If the list became full, check whether the file still had more data. */
    if (studentCount == MAX_STUDENTS && fscanf(file, "%d", &roll) == 1)
    {
        printf("Warning: file has more than %d students; extra records were ignored.\n", MAX_STUDENTS);
    }

    fclose(file);
    printf("Loaded %d student(s) from %s.\n", studentCount, DATA_FILE);
}

/* =====================================================================
 *  PART 4: PROGRAM FEATURES (one function per menu option)
 * ===================================================================== */

/* Feature 2: add a new student. */
void addStudent(void)
{
    int roll;
    int i;
    int marks[NUM_SUBJECTS];
    float attendance;
    char name[NAME_LENGTH];
    char prompt[LINE_SIZE];

    /* Prevent array overflow: refuse when the list is full. */
    if (studentCount >= MAX_STUDENTS)
    {
        printf("Cannot add more students. The list is full (maximum %d).\n", MAX_STUDENTS);
        return;
    }

    printf("\n--- Add New Student ---\n");

    /* Roll numbers must be unique, so keep asking until we get a new one. */
    while (1)
    {
        roll = getInt("Enter roll number (1-999999): ", 1, MAX_ROLL);
        if (inputEnded)
        {
            return;
        }
        if (findStudentByRoll(roll) == -1)
        {
            break;
        }
        printf("Roll number %d already exists. Please enter a different one.\n", roll);
    }

    getName("Enter student name: ", name, NAME_LENGTH);

    for (i = 0; i < NUM_SUBJECTS; i++)
    {
        snprintf(prompt, sizeof(prompt), "Enter marks in %s (0-100): ", subjectNames[i]);
        marks[i] = getInt(prompt, 0, 100);
    }

    attendance = getFloat("Enter attendance percentage (0-100): ", 0.0f, 100.0f);

    if (inputEnded)
    {
        return; /* input closed in the middle: save nothing */
    }

    /* Everything is valid, so store the student in the next free slot. */
    students[studentCount].rollNumber = roll;
    strcpy(students[studentCount].name, name);
    for (i = 0; i < NUM_SUBJECTS; i++)
    {
        students[studentCount].marks[i] = marks[i];
    }
    students[studentCount].attendance = attendance;
    calculateResult(studentCount);
    studentCount++;

    saveData();
    printf("Student added successfully.\n");
}

/* Feature 4: show every student in a table. */
void displayAllStudents(void)
{
    int i;

    if (studentCount == 0)
    {
        printf("\nNo students stored yet.\n");
        return;
    }

    printf("\n--- All Students (%d) ---\n", studentCount);
    printTableHeader();
    for (i = 0; i < studentCount; i++)
    {
        printStudentRow(i);
    }
}

/* Feature 5: search by roll number or by name and print report card(s). */
void searchStudent(void)
{
    int choice;
    int roll;
    int index;
    int i;
    int found = 0;
    char searchName[NAME_LENGTH];
    char lowerSearch[NAME_LENGTH];
    char lowerName[NAME_LENGTH];

    if (studentCount == 0)
    {
        printf("\nNo students stored yet.\n");
        return;
    }

    printf("\n--- Search Student ---\n");
    printf("1. Search by roll number\n");
    printf("2. Search by name\n");
    printf("0. Cancel\n");
    choice = getInt("Enter choice (0-2): ", 0, 2);

    switch (choice)
    {
    case 1:
        roll = getInt("Enter roll number: ", 1, MAX_ROLL);
        if (inputEnded)
        {
            return;
        }
        index = findStudentByRoll(roll);
        if (index == -1)
        {
            printf("No student found with roll number %d.\n", roll);
        }
        else
        {
            printReportCard(index);
        }
        break;

    case 2:
        getName("Enter name (or part of a name): ", searchName, NAME_LENGTH);
        if (inputEnded)
        {
            return;
        }
        makeLowercase(searchName, lowerSearch);
        /* Check every student; a match is any name that CONTAINS the text. */
        for (i = 0; i < studentCount; i++)
        {
            makeLowercase(students[i].name, lowerName);
            if (strstr(lowerName, lowerSearch) != NULL)
            {
                printReportCard(i);
                found++;
            }
        }
        if (found == 0)
        {
            printf("No student found with a name containing \"%s\".\n", searchName);
        }
        break;

    default:
        printf("Search cancelled.\n");
        break;
    }
}

/* Feature 6: rank list using bubble sort on total marks (highest first).
   We sort a list of positions (order[]) so the real data stays unchanged. */
void showRankList(void)
{
    int order[MAX_STUDENTS];
    int i;
    int j;
    int temp;
    int rank;

    if (studentCount == 0)
    {
        printf("\nNo students stored yet.\n");
        return;
    }

    for (i = 0; i < studentCount; i++)
    {
        order[i] = i;
    }

    /* Bubble sort: after each pass the smallest total "sinks" to the end. */
    for (i = 0; i < studentCount - 1; i++)
    {
        for (j = 0; j < studentCount - 1 - i; j++)
        {
            if (students[order[j]].total < students[order[j + 1]].total)
            {
                temp = order[j];
                order[j] = order[j + 1];
                order[j + 1] = temp;
            }
        }
    }

    printf("\n--- Rank List (by total marks) ---\n");
    printf("%-6s %-6s %-20s %6s %9s %6s\n", "Rank", "Roll", "Name", "Total", "Percent", "Grade");
    printLine(58);

    rank = 1;
    for (i = 0; i < studentCount; i++)
    {
        /* Students with the same total share the same rank. */
        if (i > 0 && students[order[i]].total != students[order[i - 1]].total)
        {
            rank = i + 1;
        }
        printf("%-6d %-6d %-20.20s %6d %8.2f%% %6s\n", rank, students[order[i]].rollNumber,
               students[order[i]].name, students[order[i]].total,
               students[order[i]].percentage, students[order[i]].grade);
    }
}

/* Feature 7: list students whose attendance is below the limit (75%). */
void showAttendanceWarnings(void)
{
    int i;
    int count = 0;

    if (studentCount == 0)
    {
        printf("\nNo students stored yet.\n");
        return;
    }

    printf("\n--- Attendance Warning (below %.0f%%) ---\n", ATTENDANCE_LIMIT);
    printf("%-6s %-20s %10s\n", "Roll", "Name", "Attendance");
    printLine(38);

    for (i = 0; i < studentCount; i++)
    {
        if (students[i].attendance < ATTENDANCE_LIMIT)
        {
            printf("%-6d %-20.20s %9.2f%%\n", students[i].rollNumber, students[i].name,
                   students[i].attendance);
            count++;
        }
    }

    if (count == 0)
    {
        printf("Good news: every student has attendance of %.0f%% or more.\n", ATTENDANCE_LIMIT);
    }
}

/* Feature 8: show the topper(s) of each subject. */
void showSubjectToppers(void)
{
    int subject;
    int i;
    int highest;

    if (studentCount == 0)
    {
        printf("\nNo students stored yet.\n");
        return;
    }

    printf("\n--- Subject-wise Toppers ---\n");
    for (subject = 0; subject < NUM_SUBJECTS; subject++)
    {
        /* Step 1: find the highest mark in this subject. */
        highest = students[0].marks[subject];
        for (i = 1; i < studentCount; i++)
        {
            if (students[i].marks[subject] > highest)
            {
                highest = students[i].marks[subject];
            }
        }
        /* Step 2: print everyone who scored that mark (handles ties). */
        printf("%s (highest: %d)\n", subjectNames[subject], highest);
        for (i = 0; i < studentCount; i++)
        {
            if (students[i].marks[subject] == highest)
            {
                printf("    Roll %d - %s\n", students[i].rollNumber, students[i].name);
            }
        }
    }
}

/* Feature 9: average marks of each subject and the overall average. */
void showClassAverage(void)
{
    int subject;
    int i;
    int subjectSum;
    int allSum = 0;
    float averageTotal;

    if (studentCount == 0)
    {
        printf("\nNo students stored yet.\n");
        return;
    }

    printf("\n--- Class Average (%d students) ---\n", studentCount);
    for (subject = 0; subject < NUM_SUBJECTS; subject++)
    {
        subjectSum = 0;
        for (i = 0; i < studentCount; i++)
        {
            subjectSum += students[i].marks[subject];
        }
        printf("%-10s : %6.2f\n", subjectNames[subject], (float)subjectSum / studentCount);
        allSum += subjectSum;
    }

    averageTotal = (float)allSum / studentCount;
    printf("--------------------------\n");
    printf("Average total      : %.2f / %d\n", averageTotal, NUM_SUBJECTS * 100);
    printf("Overall average    : %.2f%%\n", averageTotal / NUM_SUBJECTS);
}

/* Feature 10: list every subject where a student scored below PASS_MARK. */
void showFailingList(void)
{
    int i;
    int j;
    int failedStudents = 0;
    int studentFailed;

    if (studentCount == 0)
    {
        printf("\nNo students stored yet.\n");
        return;
    }

    printf("\n--- Failing List (below %d in a subject) ---\n", PASS_MARK);
    printf("%-6s %-20s %-10s %5s\n", "Roll", "Name", "Subject", "Marks");
    printLine(45);

    for (i = 0; i < studentCount; i++)
    {
        studentFailed = 0;
        for (j = 0; j < NUM_SUBJECTS; j++)
        {
            if (students[i].marks[j] < PASS_MARK)
            {
                printf("%-6d %-20.20s %-10s %5d\n", students[i].rollNumber, students[i].name,
                       subjectNames[j], students[i].marks[j]);
                studentFailed = 1;
            }
        }
        failedStudents += studentFailed;
    }

    if (failedStudents == 0)
    {
        printf("No student has failed any subject.\n");
    }
    else
    {
        printf("Total students with at least one failed subject: %d\n", failedStudents);
    }
}

/* Feature 11a: update a student's name, one subject's marks, or attendance. */
void updateStudent(void)
{
    int roll;
    int index;
    int choice;
    int subject;
    int newMarks;
    int i;
    float newAttendance;
    char newName[NAME_LENGTH];

    if (studentCount == 0)
    {
        printf("\nNo students stored yet.\n");
        return;
    }

    printf("\n--- Update Student ---\n");
    roll = getInt("Enter roll number of the student to update: ", 1, MAX_ROLL);
    if (inputEnded)
    {
        return;
    }
    index = findStudentByRoll(roll);
    if (index == -1)
    {
        printf("No student found with roll number %d.\n", roll);
        return;
    }

    printReportCard(index);
    printf("\nWhat do you want to update?\n");
    printf("1. Name\n");
    printf("2. Marks of one subject\n");
    printf("3. Attendance\n");
    printf("0. Cancel\n");
    choice = getInt("Enter choice (0-3): ", 0, 3);

    switch (choice)
    {
    case 1:
        getName("Enter new name: ", newName, NAME_LENGTH);
        if (inputEnded)
        {
            return;
        }
        strcpy(students[index].name, newName);
        break;

    case 2:
        for (i = 0; i < NUM_SUBJECTS; i++)
        {
            printf("%d. %s\n", i + 1, subjectNames[i]);
        }
        subject = getInt("Choose subject number (1-5): ", 1, NUM_SUBJECTS);
        newMarks = getInt("Enter new marks (0-100): ", 0, 100);
        if (inputEnded)
        {
            return;
        }
        students[index].marks[subject - 1] = newMarks;
        calculateResult(index); /* total and grade may change */
        break;

    case 3:
        newAttendance = getFloat("Enter new attendance percentage (0-100): ", 0.0f, 100.0f);
        if (inputEnded)
        {
            return;
        }
        students[index].attendance = newAttendance;
        break;

    default:
        printf("Update cancelled.\n");
        return;
    }

    saveData();
    printf("Student updated successfully.\n");
}

/* Feature 11b: delete a student by roll number (after confirmation). */
void deleteStudent(void)
{
    int roll;
    int index;
    int i;

    if (studentCount == 0)
    {
        printf("\nNo students stored yet.\n");
        return;
    }

    printf("\n--- Delete Student ---\n");
    roll = getInt("Enter roll number of the student to delete: ", 1, MAX_ROLL);
    if (inputEnded)
    {
        return;
    }
    index = findStudentByRoll(roll);
    if (index == -1)
    {
        printf("No student found with roll number %d.\n", roll);
        return;
    }

    printf("Student found: %s\n", students[index].name);
    if (!askYesNo("Are you sure you want to delete this student? (y/n): "))
    {
        printf("Delete cancelled.\n");
        return;
    }

    /* Shift every student after the deleted one a single place to the left. */
    for (i = index; i < studentCount - 1; i++)
    {
        students[i] = students[i + 1];
    }
    studentCount--;

    saveData();
    printf("Student deleted successfully.\n");
}

/* =====================================================================
 *  PART 5: MENU AND MAIN
 * ===================================================================== */

/* Prints the main menu. */
void showMenu(void)
{
    printf("\n===== STUDENT RESULT AND ATTENDANCE ANALYZER =====\n");
    printf(" 1. Add a student\n");
    printf(" 2. Display all students\n");
    printf(" 3. Search a student (report card)\n");
    printf(" 4. Rank list\n");
    printf(" 5. Attendance warning list\n");
    printf(" 6. Subject-wise toppers\n");
    printf(" 7. Class average\n");
    printf(" 8. Failing list\n");
    printf(" 9. Update a student\n");
    printf("10. Delete a student\n");
    printf(" 0. Exit\n");
    printf("==================================================\n");
}

/* Program starts here: load data, then repeat the menu until Exit. */
int main(void)
{
    int choice;

    loadData();

    do
    {
        showMenu();
        choice = getInt("Enter your choice (0-10): ", 0, 10);

        switch (choice)
        {
        case 1:
            addStudent();
            break;
        case 2:
            displayAllStudents();
            break;
        case 3:
            searchStudent();
            break;
        case 4:
            showRankList();
            break;
        case 5:
            showAttendanceWarnings();
            break;
        case 6:
            showSubjectToppers();
            break;
        case 7:
            showClassAverage();
            break;
        case 8:
            showFailingList();
            break;
        case 9:
            updateStudent();
            break;
        case 10:
            deleteStudent();
            break;
        case 0:
            printf("Saving data... Goodbye!\n");
            break;
        default:
            printf("Invalid choice.\n");
            break;
        }
    } while (choice != 0);

    saveData(); /* final save, just to be safe */
    return 0;
}

/*
    link of the claude file:
    https://claude.ai/share/bd246b3a-806c-4e9d-b7a3-550644841d39
*/