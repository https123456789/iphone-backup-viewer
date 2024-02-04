#ifndef _IPHONE_BACKUP_VIEWER_H_
#define _IPHONE_BACKUP_VIEWER_H_

struct iphone_backup {
    const char *path;
};

int iphone_backup_init(struct iphone_backup *ib, const char *path);
int validate_backup_directory(const char *path);
int backup_is_complete(struct iphone_backup *ib);

#endif // _IPHONE_BACKUP_VIEWER_H_
