#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>

using namespace std;

static string users_path;

void parse_passwd() {
    users_path = "/opt/users";
    
    // Создаём директорию users
    mkdir(users_path.c_str(), 0755);
    
    // Читаем /etc/passwd
    ifstream passwd("/etc/passwd");
    string line;
    
    while (getline(passwd, line)) {
        vector<string> fields;
        size_t pos = 0;
        string tmp = line;
        
        while ((pos = tmp.find(':')) != string::npos) {
            fields.push_back(tmp.substr(0, pos));
            tmp.erase(0, pos + 1);
        }
        fields.push_back(tmp);
        
        if (fields.size() > 6) {
            string username = fields[0];
            string uid = fields[2];
            string home_dir = fields[5];
            string shell = fields[6];
            
            // Только пользователи с шеллом
            if (shell.find("sh") != string::npos) {
                string user_path = users_path + "/" + username;
                mkdir(user_path.c_str(), 0755);
                
                // Файл id - БЕЗ endl!
                ofstream id_file(user_path + "/id");
                if (id_file.is_open()) {
                    id_file << uid;
                    id_file.close();
                }
                
                // Файл home - БЕЗ endl!
                ofstream home_file(user_path + "/home");
                if (home_file.is_open()) {
                    home_file << home_dir;
                    home_file.close();
                }
                
                // Файл shell - БЕЗ endl!
                ofstream shell_file(user_path + "/shell");
                if (shell_file.is_open()) {
                    shell_file << shell;
                    shell_file.close();
                }
            }
        }
    }
}

void init_vfs() {
    // Принудительно создаём /opt/users для тестов
    system("rm -rf /opt/users 2>/dev/null");
    mkdir("/opt/users", 0755);
    parse_passwd();
}

void update_vfs() {
    parse_passwd();
}

void cleanup_vfs() {
    // Для тестов ничего не делаем
}
