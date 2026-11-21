# 🎓 Student Result and Attendance Analyzer

A menu-driven **Student Result and Attendance Analyzer** built using the C programming language. This project allows users to manage student records, calculate academic results, track attendance, generate rank lists, and analyze class performance through a simple command-line interface.

The program also uses file handling to automatically save student records, ensuring that data remains available even after the program is closed.

## ✨ Features

- **Add Students:** Store student information, including roll number, name, subject marks, and attendance.
- **Display Records:** View all registered students in an organized table.
- **Search Students:** Search for students using their roll number or name.
- **Generate Report Cards:** Display individual academic performance, grades, and attendance.
- **Rank List:** Generate a merit list based on total marks using Bubble Sort.
- **Attendance Tracking:** Identify students whose attendance is below 75%.
- **Subject-wise Toppers:** Find the highest-scoring students in each subject.
- **Class Average:** Calculate subject-wise and overall class averages.
- **Failing List:** Identify students who score below the passing marks in any subject.
- **Update Records:** Modify student names, marks, and attendance.
- **Delete Records:** Remove student records with confirmation.
- **File Handling:** Automatically save and load student records using a text file.
- **Input Validation:** Handle invalid inputs and prevent duplicate roll numbers.

## 🛠️ Technologies Used

- **Programming Language:** C
- **Compiler:** GCC
- **Libraries:** 
  - `stdio.h`
  - `string.h`
  - `ctype.h`
- **Concepts Implemented:**
  - Structures
  - Arrays
  - Functions
  - Loops and Conditional Statements
  - Pointers and File Handling
  - String Manipulation
  - Bubble Sort
  - Input Validation

## 📚 Academic Rules

The project uses the following grading and attendance criteria:

| Criteria | Rule |
|---|---|
| Total Subjects | 5 |
| Marks per Subject | 100 |
| Passing Marks | 33 |
| Attendance Requirement | 75% |
| Maximum Students | 100 |

### Grading System

| Percentage | Grade |
|---|---|
| 90% and above | A+ |
| 80% – 89.99% | A |
| 70% – 79.99% | B |
| 60% – 69.99% | C |
| Below 60% | D |

**Note:** A student receives an F grade if they score below 33 marks in any subject, regardless of their overall percentage.

## 🚀 Getting Started

Follow these steps to run the project on your computer.

### 1. Clone the Repository

```bash
git clone https://github.com/your-username/Student-Result-and-Attendance-Analyzer.git
```

### 2. Navigate to the Project Directory

```bash
cd Student-Result-and-Attendance-Analyzer
```

### 3. Compile the Program

Make sure GCC is installed, then execute:

```bash
gcc -Wall -Wextra program.c -o program
```

### 4. Run the Program

**On Linux / macOS:**

```bash
./program
```

**On Windows:**

```bash
program.exe
```

## 📂 Project Structure

```text
Student-Result-and-Attendance-Analyzer/
│
├── program.c       # Main C source code
├── students.txt    # Automatically generated student database
└── README.md       # Project documentation
```

*Note: `students.txt` is automatically created when the program saves student records. It does not need to exist before the first run.*

## 💾 Data Storage

This project uses C file handling to maintain student records.

- Records are saved in `students.txt`.
- Existing records are loaded automatically when the program starts.
- Data is updated whenever a student is added, modified, or deleted.
- Data is saved again when the program exits.

This ensures that student information is preserved between program executions.

## 🎯 Learning Objectives

This project was developed to practice and understand fundamental C programming concepts, including:

- Organizing data using structures.
- Managing multiple records using arrays.
- Breaking a large program into smaller functions.
- Performing calculations using loops and conditions.
- Searching and sorting student records.
- Reading and writing data using file handling.
- Validating user input and handling errors.

## 👨‍💻 Author

**Priyanshu**

A student interested in programming, software development, and building practical projects.

---

⭐ If you find this project useful, consider giving the repository a star!
