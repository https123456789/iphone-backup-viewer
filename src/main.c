#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "c-log/log.h"
#include "argparse.h"
#include "iphone-backup-viewer.h"

static const char* const usage[] = {
    "iphone-backup-viewer [options]",
    NULL
};

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

    log_set_level(LOG_TRACE);

    log_info("iPhone Backup Viewer - Ben Landon 2024");
    log_info("Backup directory: %s", backup_dir_path);

    struct iphone_backup backup;

    if (iphone_backup_init(&backup, backup_dir_path) < 0) {
        return EXIT_FAILURE;
    }

    if (backup_is_complete(&backup) < 0) {
        log_fatal("Incomplete backup!");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
