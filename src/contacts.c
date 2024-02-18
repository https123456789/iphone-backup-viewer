#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <regex.h>
#include <c-log/log.h>
#include "iphone-backup-viewer.h"
#include "util.h"

// Contacts are stored in an SQLite3 database
#define CONTACTS_PATH_EXT_LEN 43
#define CONTACTS_PATH_EXT "/31/31bb7ba8914766d4ba40d6dfb6113c8b614be442"
#define CONTACTS_QUERY \
    "SELECT " \
	    "ABPerson.first, ABperson.middle, ABPerson.last, ABPerson.ROWID," \
	    "( SELECT value FROM ABMultiValue WHERE " \
			"record_id = ABPerson.ROWID AND " \
			"value REGEXP '^(\\+)?([0-9]{0,2})?(\\s)?(\\()?[0-9]{1,3}(\\)\\s)?[0-9]{1,3}(-)?[0-9]{1,4}$'" \
	    ") as phone " \
    "FROM ABPerson, ABMultiValue " \
    "WHERE ABMultiValue.record_id = ABPerson.ROWID " \
    "GROUP BY ABPerson.ROWID"

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
    int max_name_size = 0;

    if (c == NULL) {
        printf("No contacts");
        return;
    }

    // Iterate once over the list to get the max item sizes
    while (c != NULL) {
        char *formatted_name = format_name(c->first_name, c->middle_name, c->last_name);
        int len = strlen(formatted_name);
        if (len > max_name_size) {
            max_name_size = len;
        }
        c = c->next;
    }
    c = contacts->contacts;

    // Column Headers
    printf("ID\tName");
    for (int i = 0; i <= max_name_size; i++) {
        printf(" ");
    }
    printf("Phone\n");

    while (c != NULL) {
        char *formatted_name = format_name(c->first_name, c->middle_name, c->last_name);
        printf("%s\t%s", c->id, formatted_name);

        for (int i = strlen(formatted_name); i <= max_name_size; i++) {
            printf(" ");
        }

        if (c->phone == NULL) {
            printf("No phone\n");
        } else {
            printf("%s\n", c->phone);
        }

        c = c->next;
    }
}

// Implementations must provide the regexp function for SQLite
void sqlite_regexp(sqlite3_context *context, int argc, sqlite3_value **values) {
    int ret;
    regex_t regex;
    char *reg = (char*) sqlite3_value_text(values[0]);
    char *text = (char*) sqlite3_value_text(values[1]);

    if (
        argc != 2 ||
        sqlite3_value_type(values[0]) != SQLITE_TEXT ||
        (sqlite3_value_type(values[1]) != SQLITE_TEXT &&
         sqlite3_value_type(values[1]) != SQLITE_NULL)
    ) {
        sqlite3_result_error(context, "SQL function regexp() called with invalid arguments.\n", -1);
        return;
    }

    if (sqlite3_value_type(values[1]) == SQLITE_NULL) {
        return;
    }

    ret = regcomp(&regex, reg, REG_EXTENDED | REG_NOSUB);
    if (ret != 0) {
        sqlite3_result_error(context, "error compiling regular expression", -1);
        return;
    }

    ret = regexec(&regex, text , 0, NULL, 0);
    regfree(&regex);

    sqlite3_result_int(context, (ret != REG_NOMATCH));
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

    // For regex support
    sqlite3_create_function(db, "regexp", 2, SQLITE_ANY, 0, &sqlite_regexp, 0, 0);

    if (sqlite3_prepare_v2(db, CONTACTS_QUERY, -1, &res, NULL) != SQLITE_OK) {
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

        const char *first_name = (const char*) sqlite3_column_text(res, 0); // Only ASCII for now
        const char *middle_name = (const char*) sqlite3_column_text(res, 1);
        const char *last_name = (const char*) sqlite3_column_text(res, 2);
        const char *id = (const char*) sqlite3_column_text(res, 3);
        const char *phone = (const char*) sqlite3_column_text(res, 4);

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

        if (id != NULL) {
            contact->id = malloc(strlen(id) + 1);
            strcpy((char *) contact->id, id);
        } else {
            contact->id = NULL;
        }

        if (phone != NULL) {
            contact->phone = malloc(strlen(phone) + 1);
            strcpy((char*) contact->phone, phone);
        } else {
            contact->phone = NULL;
        }

        iphone_contacts_add(ib->contacts, contact);
    }

    sqlite3_finalize(res);
    sqlite3_close(db);

    return 0;
}
