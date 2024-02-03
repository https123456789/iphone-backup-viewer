#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "c-log/log.h"
#include "argparse.h"

static const char* const usage[] = {
    "iphone-backup-viewer [options]",
    NULL
};

int validate_backup_directory(const char *path) {
    struct stat stat_res;
    char *manifest_path = malloc(strlen(path) + 15);

    // All valid backup directories have to contain a Manifest.plist file
    strcpy(manifest_path, path);
    strcat(manifest_path, "/Manifest.plist");

    if (stat(manifest_path, &stat_res) < 0) {
        log_error("%s", strerror(errno));
        return -1;
    }

    return 0;
}

int main(int argc, const char **argv) {
    // Args
    char *backup_dir_path = ".";

    // Arg parsing
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_STRING('d', "backup-dir", &backup_dir_path, "Path to the backup directory"),
        OPT_END()
    };
    struct argparse argparse;
    argparse_init(&argparse, options, usage, 0);
    argc = argparse_parse(&argparse, argc, argv);

    log_info("iPhone Backup Viewer - Ben Landon 2024");
    log_info("Backup directory: %s", backup_dir_path);

    if (validate_backup_directory(backup_dir_path) < 0) {
        log_fatal("Invalid backup directory '%s'!", backup_dir_path);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
