#include <iostream>
#include <string>
#include <fstream>
#include <vector>

using namespace std;

vector<string> loadHistory() {
    vector<string> history;
    string home = getenv("HOME");
    ifstream file(home + "/.kubsh_history");
    string line;
    while (getline(file, line)) {
        history.push_back(line);
    }
    return history;
}

void saveHistory(const vector<string>& history) {
    string home = getenv("HOME");
    ofstream file(home + "/.kubsh_history");
    for (const auto& command : history) {
        file << command << endl;
    }
}

int main() {
    cout << unitbuf;
    cerr << unitbuf;
    
    string input;
    vector<string> history = loadHistory();
    
   while(true) {
        // НЕ выводим "$ " сразу
        
        if (!getline(cin, input)) {
            cout << "\nCtrl+D" << endl;
            break;
        }
        
        if (input.empty()) {
            continue;
        }
        
        if (input != "\\q") {
            history.push_back(input);
        }
        
        if (input == "\\q") {
            cout << "Выход из shell" << endl;
            break;
        }
        
        // DEBUG - МАКСИМАЛЬНО ПРОСТО
        if (input.rfind("debug ", 0) == 0) {
            string text = input.substr(6);
            
            // Убираем кавычки
            if (text.front() == '"' || text.front() == '\'') {
                text = text.substr(1, text.size() - 2);
            }
            
            cout << text << endl;
            cout << "$ ";  // <- приглашение ТОЛЬКО после вывода
            continue;
        }
               // Команда \e - вывод переменной окружения
        if (input.rfind("\\e ", 0) == 0) {
            string var = input.substr(3);
            
            // Убираем $ если есть
            if (var.front() == '$') {
                var = var.substr(1);
            }
            
            char* value = getenv(var.c_str());
            if (value != nullptr) {
                cout << value << endl;
            } else {
                cout << endl;  // пустая строка если нет переменной
            }
            continue;
        }

 
        // Command not found
        cout << input << ": command not found" << endl;
        cout << "$ ";
    }
    
    saveHistory(history);
    return 0;
}
