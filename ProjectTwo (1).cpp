// ProjectTwo.cpp
// CS 300 Project Two — ABC University Course Planner
// Loads course data from a CSV file into an unordered_map for fast lookup,
// prints an alphanumerically sorted course list, and prints details for a
// single course (including its prerequisites).
//
// Build:  g++ -std=c++17 -Wall -Wextra -o ProjectTwo ProjectTwo.cpp
// Run:    ./ProjectTwo

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cctype>

// ---------------------------------------------------------------------------
// Course data model
// ---------------------------------------------------------------------------

// A single course record. Stores the course identifier, the human readable
// title, and zero or more prerequisite course numbers.
struct Course {
    std::string courseNumber;                 // e.g. "CSCI300"
    std::string courseTitle;                  // e.g. "Introduction to Algorithms"
    std::vector<std::string> prerequisites;   // e.g. { "CSCI200", "MATH201" }
};

// ---------------------------------------------------------------------------
// Small string utilities
// ---------------------------------------------------------------------------

// Remove leading and trailing whitespace from a string. Handles spaces, tabs,
// and stray carriage returns that often appear in CSV files saved on Windows.
static std::string trim(const std::string& input) {
    const std::string whitespace = " \t\r\n";
    const auto start = input.find_first_not_of(whitespace);
    if (start == std::string::npos) {
        return "";
    }
    const auto end = input.find_last_not_of(whitespace);
    return input.substr(start, end - start + 1);
}

// Convert a string to uppercase so user input like "csci300" matches "CSCI300".
static std::string toUpper(const std::string& input) {
    std::string output = input;
    std::transform(output.begin(), output.end(), output.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return output;
}

// Split a single CSV line into trimmed tokens. We keep this simple on purpose:
// fields in this assignment do not contain embedded commas or quoted strings.
static std::vector<std::string> parseCsvLine(const std::string& line) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string field;
    while (std::getline(ss, field, ',')) {
        tokens.push_back(trim(field));
    }
    return tokens;
}

// ---------------------------------------------------------------------------
// File loading
// ---------------------------------------------------------------------------

// Read the CSV file at the given path and populate the courses map.
// Returns true on success, false if the file cannot be opened. Malformed lines
// (fewer than two fields or an empty course number) are skipped with a warning
// rather than aborting the load.
static bool loadCourses(const std::string& fileName,
                        std::unordered_map<std::string, Course>& courses) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        std::cout << "Error: could not open file \"" << fileName << "\"." << std::endl;
        return false;
    }

    courses.clear();

    std::string line;
    int lineNumber = 0;
    while (std::getline(file, line)) {
        ++lineNumber;

        // Skip completely blank lines silently.
        if (trim(line).empty()) {
            continue;
        }

        std::vector<std::string> tokens = parseCsvLine(line);

        // A valid line needs at least a course number and a title.
        if (tokens.size() < 2 || tokens[0].empty() || tokens[1].empty()) {
            std::cout << "Warning: skipping malformed line " << lineNumber
                      << ": \"" << line << "\"" << std::endl;
            continue;
        }

        Course course;
        course.courseNumber = toUpper(tokens[0]);
        course.courseTitle  = tokens[1];

        // Any remaining tokens are prerequisite course numbers.
        for (std::size_t i = 2; i < tokens.size(); ++i) {
            if (!tokens[i].empty()) {
                course.prerequisites.push_back(toUpper(tokens[i]));
            }
        }

        courses[course.courseNumber] = course;
    }

    file.close();
    std::cout << "Loaded " << courses.size() << " courses from \""
              << fileName << "\"." << std::endl;
    return true;
}

// ---------------------------------------------------------------------------
// Printing
// ---------------------------------------------------------------------------

// Print every course in the data structure sorted alphanumerically by course
// number. We pull the keys into a vector and sort them so the underlying
// unordered_map can stay optimized for fast lookup.
static void printCourseList(const std::unordered_map<std::string, Course>& courses) {
    if (courses.empty()) {
        std::cout << "No course data is loaded. Please choose option 1 first." << std::endl;
        return;
    }

    std::vector<std::string> keys;
    keys.reserve(courses.size());
    for (const auto& entry : courses) {
        keys.push_back(entry.first);
    }
    std::sort(keys.begin(), keys.end());

    std::cout << std::endl;
    std::cout << "Here is a sample schedule:" << std::endl << std::endl;
    for (const std::string& key : keys) {
        const Course& course = courses.at(key);
        std::cout << course.courseNumber << ", " << course.courseTitle << std::endl;
    }
    std::cout << std::endl;
}

// Print a single course's number, title, and prerequisites. If a prerequisite
// course number is not in the data structure we still display the number and
// note that the title is unavailable.
static void printCourse(const std::unordered_map<std::string, Course>& courses,
                        const std::string& rawCourseNumber) {
    if (courses.empty()) {
        std::cout << "No course data is loaded. Please choose option 1 first." << std::endl;
        return;
    }

    const std::string key = toUpper(trim(rawCourseNumber));
    auto it = courses.find(key);
    if (it == courses.end()) {
        std::cout << "Course \"" << key << "\" was not found." << std::endl;
        return;
    }

    const Course& course = it->second;
    std::cout << std::endl;
    std::cout << course.courseNumber << ", " << course.courseTitle << std::endl;

    if (course.prerequisites.empty()) {
        std::cout << "Prerequisites: none" << std::endl;
    } else {
        std::cout << "Prerequisites: ";
        for (std::size_t i = 0; i < course.prerequisites.size(); ++i) {
            const std::string& prereqKey = course.prerequisites[i];
            auto prereqIt = courses.find(prereqKey);
            if (prereqIt != courses.end()) {
                std::cout << prereqKey << " (" << prereqIt->second.courseTitle << ")";
            } else {
                std::cout << prereqKey << " (title unavailable)";
            }
            if (i + 1 < course.prerequisites.size()) {
                std::cout << ", ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

// ---------------------------------------------------------------------------
// Menu
// ---------------------------------------------------------------------------

// Show the main menu options exactly as required by the assignment.
static void displayMenu() {
    std::cout << std::endl;
    std::cout << "  1. Load Data Structure." << std::endl;
    std::cout << "  2. Print Course List."   << std::endl;
    std::cout << "  3. Print Course."        << std::endl;
    std::cout << "  9. Exit"                 << std::endl;
    std::cout << std::endl;
    std::cout << "What would you like to do? ";
}

// ---------------------------------------------------------------------------
// Program entry point
// ---------------------------------------------------------------------------

int main() {
    std::unordered_map<std::string, Course> courses;

    std::cout << "Welcome to the course planner." << std::endl;

    bool running = true;
    while (running) {
        displayMenu();

        // Read the menu choice as a string so non-numeric input doesn't put
        // cin into a failed state and lock up the program.
        std::string choiceInput;
        if (!std::getline(std::cin, choiceInput)) {
            // End of input stream — exit gracefully.
            break;
        }
        choiceInput = trim(choiceInput);

        int choice = -1;
        try {
            choice = std::stoi(choiceInput);
        } catch (...) {
            choice = -1;
        }

        switch (choice) {
            case 1: {
                std::cout << "Enter the course data file name: ";
                std::string fileName;
                std::getline(std::cin, fileName);
                fileName = trim(fileName);
                if (fileName.empty()) {
                    std::cout << "No file name provided." << std::endl;
                } else {
                    loadCourses(fileName, courses);
                }
                break;
            }
            case 2: {
                printCourseList(courses);
                break;
            }
            case 3: {
                std::cout << "What course do you want to know about? ";
                std::string courseNumber;
                std::getline(std::cin, courseNumber);
                printCourse(courses, courseNumber);
                break;
            }
            case 9: {
                running = false;
                break;
            }
            default: {
                // Echo the original input so the user sees what was rejected.
                std::cout << choiceInput << " is not a valid option." << std::endl;
                break;
            }
        }
    }

    std::cout << "Thank you for using the course planner!" << std::endl;
    return 0;
}
