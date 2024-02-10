#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "c-log/log.h"
#include "argparse.h"
#include "iphone-backup-viewer.h"

static const char* const usage[] = {
    "iphone-backup-viewer [options] [cmd] [args]",
    NULL
};

struct cmd_struct {
    const char *cmd;
    int (*fn) (int, const char**, char*);
};

static struct cmd_struct commands[] = {
    {"contacts", cmd_contacts}
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
    argparse_init(&argparse, options, usage, ARGPARSE_STOP_AT_NON_OPTION);
    argc = argparse_parse(&argparse, argc, argv);

    if (argc < 1) {
        argparse_usage(&argparse);
        return EXIT_FAILURE;
    }

    log_set_level(LOG_TRACE);

    log_info("iPhone Backup Viewer - Ben Landon 2024");
    log_info("Backup directory: %s", backup_dir_path);

    struct cmd_struct *cmd = NULL;

    for (int i = 0; i < sizeof(commands) / sizeof(struct cmd_struct); i++) {
        if (!strcmp(commands[i].cmd, argv[0])) {
            cmd = &commands[i];
        }
    }

    if (cmd) {
        return cmd->fn(argc, argv, backup_dir_path);
    }

    return EXIT_FAILURE;
}

int cmd_contacts(int argc, const char **argv, char *backup_dir_path) {
    // Initialize the primary backup structure
    struct iphone_backup backup;

    if (iphone_backup_init(&backup, backup_dir_path) < 0) {
        return EXIT_FAILURE;
    }

    if (backup_is_complete(&backup) < 0) {
        log_fatal("Incomplete backup!");
        return EXIT_FAILURE;
    }

    if (iphone_backup_contacts_scan(&backup) < 0) {
        log_fatal("Failed to scan for contacts");
        return EXIT_FAILURE;
    }

    iphone_contacts_list_print(backup.contacts);

    return EXIT_SUCCESS;
}
