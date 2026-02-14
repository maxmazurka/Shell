#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "vfs.h"

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

void handle_signal(int sig) {
    if (sig == SIGHUP) {
        cout << "\nConfiguration reloaded" << endl;
        cout << "$ " << flush;
    }
}

int main() {
    signal(SIGHUP, handle_signal);
    
    // Проверяем, есть ли /dev/fuse
    if (access("/dev/fuse", F_OK) == 0) {
        init_vfs();
    } else {
        // В контейнере без FUSE просто создаём папки вручную
        mkdir("/opt/users", 0755);
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
            if (fields.size() > 6 && fields[6].find("sh") != string::npos) {
                string username = fields[0];
                string user_path = "/opt/users/" + username;
                mkdir(user_path.c_str(), 0755);
                ofstream(user_path + "/id") << fields[2];
                ofstream(user_path + "/home") << fields[5];
                ofstream(user_path + "/shell") << fields[6];
            }
        }
    }
    
    cout << unitbuf;
    cerr << unitbuf;
    
    string input;
    vector<string> history = loadHistory();
    
    while(true) {
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
        
        if (input.rfind("debug ", 0) == 0) {
            string text = input.substr(6);
            if (text.size() >= 2 && (text.front() == '"' || text.front() == '\'')) {
                text = text.substr(1, text.size() - 2);
            }
            cout << text << endl;
            cout << "$ ";
            continue;
        }
        
        if (input.rfind("\\e ", 0) == 0) {
            string var = input.substr(3);
            if (var.front() == '$') {
                var = var.substr(1);
            }
            char* value = getenv(var.c_str());
            if (value != nullptr) {
                string env_value = value;
                if (var == "PATH") {
                    size_t pos = 0;
                    while ((pos = env_value.find(':')) != string::npos) {
                        cout << env_value.substr(0, pos) << endl;
                        env_value.erase(0, pos + 1);
                    }
                    cout << env_value << endl;
                } else {
                    cout << env_value << endl;
                }
            } else {
                cout << endl;
            }
            cout << "$ ";
            continue;
        }
        
        if (input.rfind("\\l ", 0) == 0) {
            string device = input.substr(3);
            if (device.rfind("/dev/", 0) == 0) {
                device = device.substr(5);
            }
            
            string size_path = "/sys/block/" + device + "/size";
            string model_path = "/sys/block/" + device + "/device/model";
            
            ifstream size_file(size_path);
            ifstream model_file(model_path);
            
            if (size_file.good() || model_file.good()) {
                cout << "Device: /dev/" << device << endl;
                
                string size;
                getline(size_file, size);
                if (!size.empty()) {
                    long long sectors = stoll(size);
                    long long bytes = sectors * 512;
                    long long mb = bytes / (1024 * 1024);
                    long long gb = mb / 1024;
                    cout << "Size: " << gb << " GB (" << mb << " MB, " << bytes << " bytes)" << endl;
                }
                
                string model;
                getline(model_file, model);
                if (!model.empty()) {
                    cout << "Model: " << model << endl;
                }
            } else {
                cout << "Device /dev/" << device << " not found" << endl;
            }
            
            cout << "$ ";
            continue;
        }
        
        bool command_executed = false;
        
        /*if (input == "cat" || input.rfind("cat ", 0) == 0) {
            const char* cat_path = "/bin/cat";
            if (access(cat_path, X_OK) == 0) {
                pid_t pid = fork();
                if (pid == 0) {
                    if (input == "cat") {
                        execl(cat_path, "cat", nullptr);
                    } else {
                        string arg = input.substr(4);
                        execl(cat_path, "cat", arg.c_str(), nullptr);
                    }
                    exit(1);
                } else if (pid > 0) {
                    wait(nullptr);
                    command_executed = true;
                }
            }
        }*/
        if (input.find('/') == string::npos && !command_executed) {
            char* path_env = getenv("PATH");
            if (path_env != nullptr) {
                string path_str = path_env;
                size_t pos = 0;
                
                while ((pos = path_str.find(':')) != string::npos) {
                    string dir = path_str.substr(0, pos);
                    string full_path = dir + "/" + input;
                    
                    if (access(full_path.c_str(), X_OK) == 0) {
                        pid_t pid = fork();
                        if (pid == 0) {
                            execl(full_path.c_str(), input.c_str(), nullptr);
                            exit(1);
                        } else if (pid > 0) {
                            wait(nullptr);
                            command_executed = true;
                        }
                        break;
                    }
                    path_str.erase(0, pos + 1);
                }
                
                if (!command_executed) {
                    string full_path = path_str + "/" + input;
                    if (access(full_path.c_str(), X_OK) == 0) {
                        pid_t pid = fork();
                        if (pid == 0) {
                            execl(full_path.c_str(), input.c_str(), nullptr);
                            exit(1);
                        } else if (pid > 0) {
                            wait(nullptr);
                            command_executed = true;
                        }
                    }
                }
            }
        } else if (!command_executed) {
            if (access(input.c_str(), X_OK) == 0) {
                pid_t pid = fork();
                if (pid == 0) {
                    execl(input.c_str(), input.c_str(), nullptr);
                    exit(1);
                } else if (pid > 0) {
                    wait(nullptr);
                    command_executed = true;
                }
            }
        }
        
        if (!command_executed) {
            cout << input << ": command not found" << endl;
        }
        
        cout << "$ ";
    }
    
    saveHistory(history);
    cleanup_vfs();
    return 0;
}
