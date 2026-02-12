#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <thread>
#include <chrono>

using namespace std;

static string users_path;

void parse_passwd() {
    users_path = "/opt/users";
    
    mkdir(users_path.c_str(), 0755);
    
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
            
            if (shell.find("sh") != string::npos) {
                string user_path = users_path + "/" + username;
                mkdir(user_path.c_str(), 0755);
                
                ofstream id_file(user_path + "/id");
                id_file << uid;
                id_file.close();
                
                ofstream home_file(user_path + "/home");
                home_file << home_dir;
                home_file.close();
                
                ofstream shell_file(user_path + "/shell");
                shell_file << shell;
                shell_file.close();
            }
        }
    }
}

void init_vfs() {
    users_path = "/opt/users";
    
    system("rm -rf /opt/users 2>/dev/null");
    mkdir(users_path.c_str(), 0755);
    parse_passwd();
    
    thread([]() {
        while (true) {
            DIR *dir = opendir(users_path.c_str());
            if (dir) {
                struct dirent *entry;
                while ((entry = readdir(dir)) != NULL) {
                    string name = entry->d_name;
                    if (name != "." && name != ".." && entry->d_type == DT_DIR) {
                        string user_path = users_path + "/" + name;
                        
                        string id_path = user_path + "/id";
                        ifstream id_file(id_path);
                        if (!id_file.good()) {
                            ofstream new_id(id_path);
                            new_id << "1001";
                            new_id.close();
                            
                            ofstream home_file(user_path + "/home");
                            home_file << "/home/" + name;
                            home_file.close();
                            
                            ofstream shell_file(user_path + "/shell");
                            shell_file << "/bin/bash";
                            shell_file.close();
                        }
                    }
                }
                closedir(dir);
            }
            this_thread::sleep_for(chrono::milliseconds(500));
        }
    }).detach();
}

void update_vfs() {
    parse_passwd();
}

void cleanup_vfs() {}
