#include <algorithm>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Book {
    string id;
    string title;
    string author;
    string category;
    string edition;
    bool available;
    string borrowedBy;
    string description;
};

struct User {
    string role;
    string name;
    string schoolId;
};

string toLower(string text) {
    transform(text.begin(), text.end(), text.begin(),
              [](unsigned char c) { return static_cast<char>(tolower(c)); });
    return text;
}

string getLineInput(const string& prompt) {
    string value;
    cout << prompt;
    getline(cin, value);
    return value;
}

string currentTimestamp() {
    // Get current UTC time
    time_t now = time(nullptr);

    // Add Philippine Time offset (UTC+8)
    now += 8 * 60 * 60;

    tm philippineTime{};

#ifdef _WIN32
    gmtime_s(&philippineTime, &now);
#else
    gmtime_r(&now, &philippineTime);
#endif

    ostringstream out;
    out << put_time(&philippineTime, "%Y-%m-%d %H:%M:%S");
    return out.str();
}

class LibrarySystem {
private:
    vector<Book> catalog;
    vector<string> transactions;
    User currentUser;
    bool hasUser = false;

    const string dataFile = "catalog_data.txt";
    const string logFile = "library_logs.txt";

    vector<string> split(const string& line, char delimiter) {
        vector<string> parts;
        string item;
        stringstream ss(line);
        while (getline(ss, item, delimiter)) {
            parts.push_back(item);
        }
        return parts;
    }

    string makeSafeField(string value) {
        replace(value.begin(), value.end(), '|', '/');
        replace(value.begin(), value.end(), '\n', ' ');
        replace(value.begin(), value.end(), '\r', ' ');
        return value;
    }

public:
    LibrarySystem() {
        loadCatalog();
    }

    void loadCatalog() {
        ifstream file(dataFile);

        if (!file) {
            catalog = {
                {"B001", "Harry Potter and the Sorcerer's Stone", "J.K. Rowling", "Fantasy",
                 "-", true, "", "A young wizard discovers his magical destiny at Hogwarts."},
                {"B002", "Database Systems", "Jose Ramirez", "Technology",
                 "4th Edition", true, "", "Advanced SQL and NoSQL concepts."},
                {"B003", "World History Essentials", "Angela Cruz", "History",
                 "1st Edition", false, "Wenny", "Overview of major historical events."},
                {"B004", "Clean Code", "Robert C. Martin", "Programming",
                 "-", true, "", "Best practices for writing readable code."},
                {"B005", "Rich Dad Poor Dad", "Robert Kiyosaki", "Finance",
                 "-", true, "", "Lessons about money and financial education."},
                {"B006", "Introduction to Algorithms", "Thomas H. Cormen", "Computer Science",
                 "-", true, "", "Guide to algorithms and data structures."},
                {"B007", "Atomic Habits", "James Clear", "Personal Development",
                 "1st Edition", true, "", "Explains how small daily habits can lead to significant life improvements over time."},
                {"B008", "Clean Architecture", "Robert C. Martin", "Technology",
                 "1st Edition", true, "", "Teaches principles for designing flexible, scalable, and maintainable software systems."},
                {"B009", "Noli Me Tangere", "Dr. Jose Rizal", " History",
                 "-", true, "", "A novel exposing the abuses during Spanish colonization, important for understanding Philippine history and nationalism."},
                {"B010", "El Filibusterismo", "Dr. Jose Rizal", "History",
                 "-", true, "", "The sequel to Noli Me Tangere, focusing on revolution and reform against colonial rule."},
               
            };
 
            saveCatalog();
            return;
        }

        catalog.clear();
        string line;
        while (getline(file, line)) {
            vector<string> parts = split(line, '|');
            if (parts.size() < 8) {
                continue;
            }

            Book book;
            book.id = parts[0];
            book.title = parts[1];
            book.author = parts[2];
            book.category = parts[3];
            book.edition = parts[4];
            book.available = parts[5] == "1";
            book.borrowedBy = parts[6];
            book.description = parts[7];
            catalog.push_back(book);
        }
    }

    void saveCatalog() {
        ofstream file(dataFile);
        for (const Book& book : catalog) {
            file << makeSafeField(book.id) << '|'
                 << makeSafeField(book.title) << '|'
                 << makeSafeField(book.author) << '|'
                 << makeSafeField(book.category) << '|'
                 << makeSafeField(book.edition) << '|'
                 << (book.available ? "1" : "0") << '|'
                 << makeSafeField(book.borrowedBy) << '|'
                 << makeSafeField(book.description) << '\n';
        }
    }

    void logAction(const string& message) {
        string entry = currentTimestamp() + " | " + message;
        transactions.push_back(entry);

        ofstream file(logFile, ios::app);
        file << entry << '\n';
    }

    void showHeader() {
        cout << "\n=============================================\n";
        cout << "              BookNavi Library\n";
        cout << "=============================================\n";
        if (hasUser) {
            cout << "Welcome, " << currentUser.name << " (" << currentUser.role << ")\n";
        } else {
            cout << "Welcome, Guest\n";
        }
    }

    void showDashboard() {
        int total = static_cast<int>(catalog.size());
        int available = 0;

        for (const Book& book : catalog) {
            if (book.available) {
                available++;
            }
        }

        cout << "\nDashboard\n";
        cout << "Total Books : " << total << '\n';
        cout << "Available   : " << available << '\n';
        cout << "Borrowed    : " << total - available << '\n';
    }

    void showMenu() {
        cout << "\nMenu\n";
        cout << "1. Register User\n";
        cout << "2. Admin Login\n";
        cout << "3. Logout\n";
        cout << "4. View All Books\n";
        cout << "5. Search Book\n";
        cout << "6. Add Book\n";
        cout << "7. Borrow Book\n";
        cout << "8. Return Book\n";
        cout << "9. Delete Book\n";
        cout << "10. Show Transactions\n";
        cout << "11. Exit\n";
    }

    void registerUser() {
        string roleChoice;
        string name;
        string schoolId;

        cout << "\nRegister User\n";
        cout << "1. Student\n";
        cout << "2. Faculty\n";
        roleChoice = getLineInput("Choose user status: ");
        name = getLineInput("Name: ");
        schoolId = getLineInput("School ID No.: ");

        if (name.empty() || schoolId.empty()) {
            cout << "Error: name and school ID are required.\n";
            return;
        }

        currentUser.role = (roleChoice == "2") ? "Faculty" : "Student";
        currentUser.name = name;
        currentUser.schoolId = schoolId;
        hasUser = true;

        logAction("LOGIN/REG | " + name + " (" + currentUser.role + ") - ID: " + schoolId);
        cout << "User registered successfully.\n";
    }

    void adminLogin() {
        string password = getLineInput("\nAdmin password: ");
        if (password == "admin123") {
            currentUser = {"Admin", "Administrator", ""};
            hasUser = true;
            cout << "Admin logged in successfully.\n";
            logAction("ADMIN LOGIN | Administrator logged in.");
        } else {
            cout << "Invalid password.\n";
        }
    }

    void logoutUser() {
        if (!hasUser) {
            cout << "No active user to logout.\n";
            return;
        }

        logAction("LOGOUT | " + currentUser.name + " logged out.");
        hasUser = false;
        currentUser = {};
        cout << "Logged out successfully.\n";
    }

    Book* findBookById(const string& id) {
        string target = toLower(id);
        for (Book& book : catalog) {
            if (toLower(book.id) == target) {
                return &book;
            }
        }
        return nullptr;
    }

    string generateBookId() {
        int highest = 0;
        for (const Book& book : catalog) {
            if (book.id.size() > 1 && book.id[0] == 'B') {
                try {
                    highest = max(highest, stoi(book.id.substr(1)));
                } catch (...) {
                }
            }
        }

        ostringstream id;
        id << 'B' << setw(3) << setfill('0') << highest + 1;
        return id.str();
    }

    void printBooks(const vector<Book>& books) {
        if (books.empty()) {
            cout << "No books to display.\n";
            return;
        }

        cout << left << setw(8) << "ID"
             << setw(28) << "Title"
             << setw(20) << "Author"
             << setw(16) << "Category"
             << setw(16) << "Edition"
             << setw(12) << "Status"
             << "Borrowed By\n";
        cout << string(115, '-') << '\n';

        for (const Book& book : books) {
            cout << left << setw(8) << book.id
                 << setw(28) << book.title.substr(0, 27)
                 << setw(20) << book.author.substr(0, 19)
                 << setw(16) << book.category.substr(0, 15)
                 << setw(16) << book.edition.substr(0, 15)
                 << setw(12) << (book.available ? "Available" : "Borrowed")
                 << (book.borrowedBy.empty() ? "-" : book.borrowedBy)
                 << '\n';
        }
    }

    void viewAllBooks() {
        cout << "\nBook Catalog\n";
        printBooks(catalog);
    }

    void searchBook() {
        string title = toLower(getLineInput("\nTitle keyword (leave blank for any): "));
        string author = toLower(getLineInput("Author keyword (leave blank for any): "));
        string category = toLower(getLineInput("Category keyword (leave blank for any): "));
        vector<Book> results;

        for (const Book& book : catalog) {
            bool titleMatch = title.empty() || toLower(book.title).find(title) != string::npos;
            bool authorMatch = author.empty() || toLower(book.author).find(author) != string::npos;
            bool categoryMatch = category.empty() || toLower(book.category).find(category) != string::npos;

            if (titleMatch && authorMatch && categoryMatch) {
                results.push_back(book);
            }
        }

        cout << "\nSearch Results: " << results.size() << " book(s) found.\n";
        printBooks(results);
    }
    
    void showDescription() {
        string id = getLineInput("\nBook ID to view description: ");
        Book* book = findBookById(id);

        if (!book) {
            cout << "Book ID does not exist.\n";
            return;
        }

        cout << "\nTitle: " << book->title << '\n';
        cout << "Description: "
             << (book->description.empty() ? "No description available." : book->description)
             << '\n';
    }

    void addBook() {
        if (!hasUser || currentUser.role != "Admin") {
            cout << "Access denied. Only administrators can add books.\n";
            return;
        }

        Book book;
        book.id = generateBookId();
        book.title = getLineInput("\nTitle: ");
        book.author = getLineInput("Author: ");
        book.category = getLineInput("Category: ");
        book.edition = getLineInput("Volume/Edition: ");
        book.description = getLineInput("Description: ");
        book.available = true;
        book.borrowedBy = "";

        if (book.title.empty() || book.author.empty() || book.category.empty() || book.edition.empty()) {
            cout << "Error: title, author, category, and edition are required.\n";
            return;
        }

        catalog.push_back(book);
        saveCatalog();
        logAction("ADD BOOK | " + book.title + " (" + book.id + ")");
        cout << "Book added successfully with ID " << book.id << ".\n";
    }

    void borrowBook() {
        if (!hasUser) {
            cout << "Please register or login first.\n";
            return;
        }

        string id = getLineInput("\nBook ID to borrow: ");
        Book* book = findBookById(id);

        if (!book) {
            cout << "Book ID does not exist.\n";
        } else if (!book->available) {
            cout << "Book is already borrowed.\n";
        } else {
            book->available = false;
            book->borrowedBy = currentUser.name;
            saveCatalog();
            logAction("BORROW | " + currentUser.name + " borrowed " + book->title + " (" + book->id + ")");
            cout << "Book borrowed successfully.\n";
        }
    }

    void returnBook() {
        if (!hasUser) {
            cout << "Please register or login first.\n";
            return;
        }

        string id = getLineInput("\nBook ID to return: ");
        Book* book = findBookById(id);

        if (!book) {
            cout << "Book ID does not exist.\n";
        } else if (book->available) {
            cout << "Book is already available.\n";
        } else {
            string previousBorrower = book->borrowedBy;
            book->available = true;
            book->borrowedBy = "";
            saveCatalog();
            logAction("RETURN | " + currentUser.name + " returned " + book->title + " (" + book->id + ") (Prev: " + previousBorrower + ")");
            cout << "Book returned successfully.\n";
        }
    }

    void deleteBook() {
        if (!hasUser || currentUser.role != "Admin") {
            cout << "Access denied. Only administrators can delete books.\n";
            return;
        }

        string id = getLineInput("\nBook ID to delete: ");
        auto it = find_if(catalog.begin(), catalog.end(), [&](const Book& book) {
            return toLower(book.id) == toLower(id);
        });

        if (it == catalog.end()) {
            cout << "Book ID does not exist.\n";
            return;
        }

        string confirm = getLineInput("Delete \"" + it->title + "\" permanently? (y/n): ");
        if (toLower(confirm) != "y") {
            cout << "Delete cancelled.\n";
            return;
        }

        string title = it->title;
        string bookId = it->id;
        catalog.erase(it);
        saveCatalog();
        logAction("DELETE BOOK | Removed: " + title + " (" + bookId + ")");
        cout << "Book deleted successfully.\n";
    }


    void showTransactions() {
        if (!hasUser || currentUser.role != "Admin") {
            cout << "Access denied. Only administrators can view transaction history.\n";
            return;
        }

        ifstream existingLogs(logFile);
        string line;
        cout << "\nTransaction History\n";
        cout << string(60, '-') << '\n';

        bool hasLogs = false;
        while (getline(existingLogs, line)) {
            cout << line << '\n';
            hasLogs = true;
        }

        if (!hasLogs) {
            cout << "No transactions.\n";
        }
    }
};

int main() {
    LibrarySystem app;

    bool running = true;

    while (running) {
        app.showHeader();
        app.showDashboard();
        app.showMenu();

        string choice = getLineInput("\nChoose an option: ");

        if (choice == "1") {
            app.registerUser();
        } else if (choice == "2") {
            app.adminLogin();
        } else if (choice == "3") {
            app.logoutUser();
        } else if (choice == "4") {
            app.viewAllBooks();
            app.showDescription();
        } else if (choice == "5") {
            app.searchBook();
            app.showDescription();
        } else if (choice == "6") {
            app.addBook();
        } else if (choice == "7") {
            app.borrowBook();
        } else if (choice == "8") {
            app.returnBook();
        } else if (choice == "9") {
            app.deleteBook();
        }  else if (choice == "10") {
            app.showTransactions();
        } else if (choice == "11") {
            string confirm = getLineInput("Exit the system? (Yes/No): ");

            if (toLower(confirm) == "yes" || toLower(confirm) == "y") {
                running = false;
            }
        } else {
            cout << "Invalid option. Please try again.\n";
        }

        if (running) {
            cout << "\nPress Enter to continue...";
            string pause;
            getline(cin, pause);
        }
    }

    return 0;
}