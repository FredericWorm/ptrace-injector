# ptrace-injector

Minimal injector using ptrace to attach and load a shared library into a target process.

## Build
```bash
make
```

## Usage
The command line content can be found in "/proc/pid/cmdline".
The path to the libary has to be absolute.
ptrace requires root.
```bash
sudo ./InjectorBin -p <process_cmdline_content> -l <library_path>
```

## Code Style
Project follows [this C code style](https://github.com/MaJerle/c-code-style).

## Test
After building with "make", the test binary is located at "out/test/TestBin"
and the shared library at "out/test/libtest.so".
Test the injector by running the test binary and then injecting as told above, if no error occurs and a log file gets created and printed to, whilst the binary also keeps printing, it works.

## Documenation
I don't know why one would need it for this small project, however I included doxygen documentation to the files, I didn't generate the doc files though.
