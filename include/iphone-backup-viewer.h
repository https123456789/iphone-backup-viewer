#include <stddef.h>

#ifndef _IPHONE_BACKUP_VIEWER_H_
#define _IPHONE_BACKUP_VIEWER_H_

// Structures
struct iphone_backup {
    const char *path;
    struct iphone_contacts_list *contacts;
};

struct iphone_contacts_list {
    struct iphone_contact *contacts;
};

struct iphone_contact {
    const char *first_name;
    const char *middle_name;
    const char *last_name;

    // Internal
    struct iphone_contact *next;
};

// CLI Commands
int cmd_contacts(int argc, const char **argv, char *backup_dir_path);

int iphone_backup_init(struct iphone_backup *ib, const char *path);
int validate_backup_directory(const char *path);
int backup_is_complete(struct iphone_backup *ib);
// TODO: add free function for backup

// Contacts
void iphone_contacts_init(struct iphone_contacts_list *contacts);
void iphone_contacts_add(struct iphone_contacts_list *contacts, struct iphone_contact *contact);
void iphone_contacts_list_print(struct iphone_contacts_list *contacts);
int iphone_backup_contacts_scan(struct iphone_backup *ib);
// TODO: add free function for contacts

#endif // _IPHONE_BACKUP_VIEWER_H_
