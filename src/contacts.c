#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <c-log/log.h>
#include "iphone-backup-viewer.h"

// Contacts are stored in an SQLite3 database
#define CONTACTS_PATH_EXT_LEN 43
#define CONTACTS_PATH_EXT "/31/31bb7ba8914766d4ba40d6dfb6113c8b614be442"

#define CONTACTS_SQL_QUERY "SELECT First, Middle, Last FROM ABPerson"

void iphone_contacts_init(struct iphone_contacts_list *contacts) {
    contacts->contacts = NULL;
}

void iphone_contacts_add(struct iphone_contacts_list *contacts, struct iphone_contact *contact) {
    struct iphone_contact *c = contacts->contacts;

    if (c == NULL) {
        contacts->contacts = contact;
        return;
    }

    while (c->next != NULL) {
        c = c->next;
    }

    c->next = contact;
}

int iphone_backup_contacts_scan(struct iphone_backup *ib) {
    char *database_path = malloc(strlen(ib->path) + CONTACTS_PATH_EXT_LEN);
    strcpy(database_path, ib->path);
    strcat(database_path, CONTACTS_PATH_EXT);

    log_debug("%s", database_path);
    log_debug("SQLite3 LibVersion: %s", sqlite3_libversion());

    sqlite3 *db;
    sqlite3_stmt *res;

    if (sqlite3_open(database_path, &db) != SQLITE_OK) {
        log_error("Cannot open database '%s': %s", database_path, sqlite3_errmsg(db));
        sqlite3_close(db);

        return -1;
    }

    if (sqlite3_prepare_v2(db, CONTACTS_SQL_QUERY, -1, &res, NULL) != SQLITE_OK) {
        log_error("Failed to prepare SQL query: %s", sqlite3_errmsg(db));
        sqlite3_close(db);

        return -1;
    }

    while (1) {
        int err = sqlite3_step(res);

        if (err == SQLITE_DONE) {
            break;
        } else if (err != SQLITE_ROW) {
            log_error("Failed to fetch data: %s", sqlite3_errmsg(db));
            sqlite3_finalize(res);
            sqlite3_close(db);

            return -1;
        }

        const char *first_name = (const char *) sqlite3_column_text(res, 0); // Only ASCII for now
        const char *middle_name = (const char *) sqlite3_column_text(res, 1);
        const char *last_name = (const char *) sqlite3_column_text(res, 2);

        struct iphone_contact *contact = malloc(sizeof(struct iphone_contact));
        contact->first_name = first_name;
        contact->middle_name = middle_name;
        contact->last_name = last_name;
        contact->next = NULL;

        iphone_contacts_add(ib->contacts, contact);

        log_debug("Contact: %s %s %s", contact->first_name, contact->middle_name, contact->last_name);
    }

    sqlite3_finalize(res);
    sqlite3_close(db);

    return 0;
}
