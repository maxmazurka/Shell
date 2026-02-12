#ifndef VFS_H
#define VFS_H

// Инициализация VFS - вызывается при запуске шелла
void init_vfs();

// Очистка VFS - вызывается при выходе
void cleanup_vfs();

// Обновление списка пользователей
void update_vfs();

#endif
