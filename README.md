# iPhone Backup Viewer

A utility program that helps you view data from an iPhone backup in an easier manner.

## Compiling

```bash
meson setup build
meson compile -C build
```

## Usage

### View Contacts

You can view a list of all the contacts using the `contacts` subcommand.

```bash
iphone-backup-viewer -d /some/dir contacts
```
