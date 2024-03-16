## Simple DBUS Interface too Systemd for controlling service-units

Currently supported actions on service-units:

- enable / disable
- mask / unmask
- start / stop

See also example_app for usage.

# Dependencies
Only cpp stl and libsystemd.

# Build
mkdir build & cd ./build
cmake ..
make

# Note from the author

Its not pretty (yet) but it does the job. Work in progress.

Also consider that even if the code in the repo is under MIT Licesence it will link to
libsystemd which is under LGPLv2. So under certain conditions that are described in LGPLv2
the resulting Objectcode/Executable might be under LGPLv2.
