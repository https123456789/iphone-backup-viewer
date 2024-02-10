#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <c-log/log.h>
#include "iphone-backup-viewer.h"
#include "util.h"

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

void iphone_contacts_list_print(struct iphone_contacts_list *contacts) {
    struct iphone_contact *c = contacts->contacts;

    if (c == NULL) {
        printf("No contacts");
        return;
    }

    while (c != NULL) {
        char *formatted_name = format_name(c->first_name, c->middle_name, c->last_name);
        printf("- %s\n", formatted_name);
        c = c->next;
    }
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
        contact->next = NULL;

        // Copy in the data
        if (first_name != NULL) {
            contact->first_name = malloc(strlen(first_name) + 1);
            strcpy((char *) contact->first_name, first_name);
        } else {
            contact->first_name = NULL;
        }

        if (middle_name != NULL) {
            contact->middle_name = malloc(strlen(middle_name) + 1);
            strcpy((char *) contact->middle_name, middle_name);
        } else {
            contact->middle_name = NULL;
        }

        if (last_name != NULL) {
            contact->last_name = malloc(strlen(last_name) + 1);
            strcpy((char *) contact->last_name, last_name);
        } else {
            contact->last_name = NULL;
        }

        iphone_contacts_add(ib->contacts, contact);
    }

    sqlite3_finalize(res);
    sqlite3_close(db);

    return 0;
}
