#define FUSE_USE_VERSION 35

#include <fuse3/fuse.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <thread>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <ctime>
#include "vfs.h"

using namespace std;

static string users_path = "/opt/users";

bool valid_shell(struct passwd* pwd) {
    if (!pwd || !pwd->pw_shell) return false;
    string shell = pwd->pw_shell;
    return shell.find("sh") != string::npos;
}

static int vfs_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    (void) fi;
    memset(stbuf, 0, sizeof(struct stat));
    
    time_t now = time(NULL);
    stbuf->st_atime = now;
    stbuf->st_mtime = now;
    stbuf->st_ctime = now;
    
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }
    
    char username[256];
    char filename[256];
    
    if (sscanf(path, "/%255[^/]/%255[^/]", username, filename) == 2) {
        struct passwd* pwd = getpwnam(username);
        if (!pwd) return -ENOENT;
        
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_uid = pwd->pw_uid;
        stbuf->st_gid = pwd->pw_gid;
        
        if (strcmp(filename, "id") == 0) {
            string content = to_string(pwd->pw_uid);
            stbuf->st_size = content.length();
            return 0;
        }
        if (strcmp(filename, "home") == 0) {
            stbuf->st_size = strlen(pwd->pw_dir);
            return 0;
        }
        if (strcmp(filename, "shell") == 0) {
            stbuf->st_size = strlen(pwd->pw_shell);
            return 0;
        }
        return -ENOENT;
    }
    
    if (sscanf(path, "/%255[^/]", username) == 1) {
        struct passwd* pwd = getpwnam(username);
        if (pwd && valid_shell(pwd)) {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;
            stbuf->st_uid = pwd->pw_uid;
            stbuf->st_gid = pwd->pw_gid;
            return 0;
        }
    }
    
    return -ENOENT;
}

static int vfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi,
                        enum fuse_readdir_flags flags) {
    (void) offset;
    (void) fi;
    (void) flags;
    
    filler(buf, ".", NULL, 0, FUSE_FILL_DIR_PLUS);
    filler(buf, "..", NULL, 0, FUSE_FILL_DIR_PLUS);
    
    if (strcmp(path, "/") == 0) {
        setpwent();
        struct passwd* pwd;
        while ((pwd = getpwent()) != NULL) {
            if (valid_shell(pwd)) {
                filler(buf, pwd->pw_name, NULL, 0, FUSE_FILL_DIR_PLUS);
            }
        }
        endpwent();
        return 0;
    }
    
    char username[256];
    if (sscanf(path, "/%255[^/]", username) == 1) {
        struct passwd* pwd = getpwnam(username);
        if (pwd) {
            filler(buf, "id", NULL, 0, FUSE_FILL_DIR_PLUS);
            filler(buf, "home", NULL, 0, FUSE_FILL_DIR_PLUS);
            filler(buf, "shell", NULL, 0, FUSE_FILL_DIR_PLUS);
        }
    }
    
    return 0;
}

static int vfs_open(const char *path, struct fuse_file_info *fi) {
    return 0;
}

static int vfs_read(const char *path, char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi) {
    (void) fi;
    
    char username[256];
    char filename[256];
    
    if (sscanf(path, "/%255[^/]/%255[^/]", username, filename) != 2) {
        return -ENOENT;
    }
    
    struct passwd* pwd = getpwnam(username);
    if (!pwd) return -ENOENT;
    
    string content;
    if (strcmp(filename, "id") == 0) {
        content = to_string(pwd->pw_uid);
    } else if (strcmp(filename, "home") == 0) {
        content = pwd->pw_dir;
    } else if (strcmp(filename, "shell") == 0) {
        content = pwd->pw_shell;
    } else {
        return -ENOENT;
    }
    
    if (offset >= (off_t)content.length()) {
        return 0;
    }
    
    if (offset + size > content.length()) {
        size = content.length() - offset;
    }
    
    memcpy(buf, content.c_str() + offset, size);
    return size;
}

static int vfs_mkdir(const char *path, mode_t mode) {
    (void) mode;
    
    char username[256];
    if (sscanf(path, "/%255[^/]", username) != 1) {
        return -ENOENT;
    }
    
    if (getpwnam(username) != NULL) {
        return -EEXIST;
    }
    
    string cmd = "useradd -m -s /bin/bash " + string(username) + " 2>/dev/null";
    int ret = system(cmd.c_str());
    
    return (ret == 0) ? 0 : -EIO;
}

static int vfs_rmdir(const char *path) {
    char username[256];
    if (sscanf(path, "/%255[^/]", username) != 1) {
        return -ENOENT;
    }
    
    if (getpwnam(username) == NULL) {
        return -ENOENT;
    }
    
    string cmd = "userdel -r " + string(username) + " 2>/dev/null";
    int ret = system(cmd.c_str());
    
    return (ret == 0) ? 0 : -EIO;
}

static struct fuse_operations vfs_ops;

void init_operations() {
    memset(&vfs_ops, 0, sizeof(vfs_ops));
    vfs_ops.getattr = vfs_getattr;
    vfs_ops.readdir = vfs_readdir;
    vfs_ops.open = vfs_open;
    vfs_ops.read = vfs_read;
    vfs_ops.mkdir = vfs_mkdir;
    vfs_ops.rmdir = vfs_rmdir;
}

void fuse_thread() {
    fprintf(stderr, "\n=== FUSE Debug ===\n");
    
    if (access("/dev/fuse", F_OK) == 0) {
        fprintf(stderr, "✓ /dev/fuse exists\n");
    } else {
        fprintf(stderr, "✗ /dev/fuse NOT found\n");
        return;
    }
    
    // НЕ СОЗДАЁМ ПАПКУ! Только проверяем
    if (access(users_path.c_str(), F_OK) != 0) {
        fprintf(stderr, "✗ Mount point %s does NOT exist. FUSE will create it.\n", users_path.c_str());
        // return; // НЕ ВЫХОДИМ!
    }
    
    init_operations();
    fprintf(stderr, "✓ Operations initialized\n");
    
    char* argv[] = {
        (char*)"kubsh",
        (char*)"-f",
        (char*)"-o",
        (char*)"default_permissions,allow_other",
        (char*)users_path.c_str(),
        NULL
    };
    
    fprintf(stderr, "Calling fuse_main with mount point: %s\n", users_path.c_str());
    int ret = fuse_main(5, argv, &vfs_ops, NULL);
    fprintf(stderr, "fuse_main returned: %d\n", ret);
}

void init_vfs() {
    fprintf(stderr, "Initializing VFS...\n");
    
    mkdir(users_path.c_str(), 0755);
    thread(fuse_thread).detach();
    sleep(2);
    
    fprintf(stderr, "VFS initialization complete\n");
}

void cleanup_vfs() {
    string cmd = "fusermount3 -u " + users_path + " 2>/dev/null";
    system(cmd.c_str());
}
