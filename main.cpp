#include <iostream>
#include <string>
#include <fstream>
#include <vector>

using namespace std;

// Функция для загрузки истории из файла
vector<string> loadHistory() {
    vector<string> history;
    ifstream file(getenv("HOME") + string("/.kubsh_history"));
    string line;
    
    while (getline(file, line)) {
        history.push_back(line);
    }
    return history;
}

// Функция для сохранения истории в файл
void saveHistory(const vector<string>& history) {
    ofstream file(getenv("HOME") + string("/.kubsh_history"));
    for (const auto& command : history) {
        file << command << endl;
    }
}

int main() {
    // Flush after every std::cout / std:cerr
    cout << unitbuf;
    cerr << unitbuf;
    
    string input;
    vector<string> history = loadHistory();
    
    while(true) {
        cout << "$ ";
        
        if (!getline(cin, input)) {
            cout << "\nCtrl+D" << endl;
            break;
        }
        
        if (input != "\\q") {
            history.push_back(input);
        }
        
        if (input == "\\q") {
            cout << "Выход из shell" << endl;
            break;
        }
        
        if (input.find("echo ") == 0) {
            cout << input.substr(5) << endl;  
        }

        else {
            cout << "Введённая строка: " << input << endl;
            /*while(true) {
	      cout << input << endl;
	      if (cin.eof()) {
		cout << "\nCtrl+D" << endl;
		saveHistory(history);
		return 0;
	      }
            }*/
        }
    }
    

    saveHistory(history);
    
    return 0;
}


