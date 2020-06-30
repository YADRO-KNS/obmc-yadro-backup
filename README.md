# Configuration backup tool for OpenBMC
The project provides console utility that can be used to backup and restore
OpenBMC configuration files.

## Backup and restore
The backup/restore procedures are based on saving and writing configuration
files of the OpenBMC system.
A list of configuration files is defined as a static list of paths. These files
are added to the resulting archive during backup.
The restore operation extracts the files and lays them out to the root FS.

### User accounts
User accounts, groups and passwords are backed up as a diff between RO partition
(build-in accounts data) and RW partition (user defined accounts data).
The restore procedure handles these diffs and makes changes for allowed data
only.
This way allows to prevent modification of some critical configuration, such as
a root password or an end user memberships.

## Build with OpenBMC SDK
OpenBMC SDK contains a toolchain and all the dependencies needed for building
the project.
See [official documentation](https://github.com/openbmc/docs/blob/master/development/dev-environment.md#download-and-install-sdk) for details.

Build steps:
```sh
$ source /path/to/sdk/environment-setup-arm1176jzs-openbmc-linux-gnueabi
$ mkdir build_dir
$ meson build_dir
$ ninja -C build_dir
```
If build process succeeded, the directory `build_dir` contains the executable
file.

## Testing
Unit tests can be built and run with OpenBMC SDK.

Run tests:
```sh
$ source /path/to/sdk/environment-setup-arm1176jzs-openbmc-linux-gnueabi
$ # ... build the project (see above)
$ qemu-arm -L ${SDKTARGETSYSROOT} /path/to/testfile
```
