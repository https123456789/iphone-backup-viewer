#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "c-log/log.h"
#include <plist/plist.h>
#include "iphone-backup-viewer.h"

int validate_backup_directory(const char *path) {
    struct stat stat_res;
    char *check_path = malloc(strlen(path) + 13);

    // All valid backup directories have to contain a Manifest.plist file
    strcpy(check_path, path);
    strcat(check_path, "/Status.plist");

    log_debug("Validate path '%s'", check_path);

    if (stat(check_path, &stat_res) < 0) {
        log_error("%s", strerror(errno));
        return -1;
    }

    return 0;
}

int iphone_backup_init(struct iphone_backup *ib, const char *path) {
    log_debug("Initialize backup structure");

    if (validate_backup_directory(path) < 0) {
        log_fatal("Invalid backup directory '%s'!", path);
        return -1;
    }

    ib->path = path;

    return 0;
}

int backup_is_complete(struct iphone_backup *ib) {
    log_debug("Check backup completion status");

    char *status_path = malloc(strlen(ib->path) + 13);
    strcpy(status_path, ib->path);
    strcat(status_path, "/Status.plist");

    plist_t plist;
    plist_format_t format = PLIST_FORMAT_BINARY;
    plist_err_t err = plist_read_from_file(status_path, &plist, &format);

    if (err != PLIST_ERR_SUCCESS) {
        log_error("Error while reading status plist file: code %d", err);
        return -1;
    }

    plist_type type = plist_get_node_type(plist);

    if (type != PLIST_DICT) {
        log_error("Expected status file to contain a dict node");
        return -1;
    }

    plist_dict_iter iter;
    plist_dict_new_iter(plist, &iter);

    // The "SnapshotState" key is always last
    char *key;
    plist_t value;
    do {
        plist_dict_next_item(plist, iter, &key, &value);
    } while (value != NULL && strcmp(key, "SnapshotState") != 0);

    char *snapshot_state;
    plist_get_string_val(value, &snapshot_state);

    int result = (strcmp(snapshot_state, "finished") == 0) - 1;

    plist_free(plist);

    return result;
}
