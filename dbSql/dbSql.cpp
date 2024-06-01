#include <iostream>
#include <string>
#include <algorithm>
#include <fstream>
#include <locale>
#include <sqlite3.h>
#include <windows.h>

class DatabaseManager {
public:
    DatabaseManager(const std::string& dbName) : dbName(dbName), db(nullptr) {}

    ~DatabaseManager() {
        if (db) {
            sqlite3_close(db);
            std::cout << "База данных закрыта. Завершение программы.\n";
        }
    }

    bool openDatabase() {
        bool isNotNewDb = fileExists(dbName);

        if (isNotNewDb) {
            std::cout << "База данных найдена.\n";
        }
        else {
            std::cout << "База данных не найдена. Она будет создана.\n";
        }

        int rc = sqlite3_open(dbName.c_str(), &db);
        if (rc) {
            std::cerr << "Не удается открыть базу данных: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }
        else {
            std::cout << "База данных успешно открыта\n";
        }

        if (!isNotNewDb) {
            createTestTables();
            std::cout << "Добавлены таблицы 'employees' и 'departments'\n";
        }

        return true;
    }

    void executeSQL(const std::string& sql) const {
        char* errMsg = nullptr;
        int rc = sqlite3_exec(db, sql.c_str(), [](void*, int argc, char** argv, char** colName) -> int {
            for (int i = 0; i < argc; i++) {
                std::cout << colName[i] << ": " << (argv[i] ? argv[i] : "NULL") << "\n";
            }
            std::cout << std::endl;
            return 0;
            }, 0, &errMsg);

        if (rc != SQLITE_OK) {
            std::cerr << "Ошибка SQL: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
        else {
            std::cout << "Операция выполнена успешно\n";
        }
    }

private:
    std::string dbName;
    sqlite3* db;

    bool fileExists(const std::string& name) const {
        std::ifstream f(name.c_str());
        return f.is_open();
    }

    void createTestTables() const {
        const char* sqlCreateDepartmentsTable =
            "CREATE TABLE IF NOT EXISTS departments ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "department_name TEXT NOT NULL, "
            "location TEXT NOT NULL);";
        const char* sqlInsertDepartmentsTable =
            "INSERT INTO departments (department_name, location) VALUES "
            "('HR', 'New York'), "
            "('Engineering', 'San Francisco'), "
            "('Design', 'Los Angeles');";

        const char* sqlCreateEmployeesTable =
            "CREATE TABLE IF NOT EXISTS employees ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "name TEXT NOT NULL, "
            "position TEXT NOT NULL, "
            "salary INTEGER NOT NULL, "
            "department_id INTEGER NOT NULL, "
            "FOREIGN KEY(department_id) REFERENCES departments(id));";
        const char* sqlInsertEmployeesTable =
            "INSERT INTO employees (name, position, salary, department_id) VALUES "
            "('Alice', 'Manager', 5000, 1), "
            "('Bob', 'Developer', 4000, 2), "
            "('Charlie', 'Designer', 3500, 3);";

        executeSQL(sqlCreateDepartmentsTable);
        executeSQL(sqlInsertDepartmentsTable);
        executeSQL(sqlCreateEmployeesTable);
        executeSQL(sqlInsertEmployeesTable);
    }
};

int main() {
    std::string dbName;
    setlocale(LC_ALL, "ru");

    std::cout << "Введите имя базы данных: ";
    std::getline(std::cin, dbName);

    DatabaseManager dbManager(dbName);
    if (!dbManager.openDatabase()) {
        return 1;
    }

    std::string sql;
    while (true) {
        std::cout << "Введите SQL-команду (или введите 'close'/'exit' для завершения программы): ";
        std::getline(std::cin, sql);

        std::transform(sql.begin(), sql.end(), sql.begin(), ::tolower);
        if (sql == "close" || sql == "exit") {
            break;
        }

        dbManager.executeSQL(sql);
    }

    return 0;
}
