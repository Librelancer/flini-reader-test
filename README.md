Test application for common.dll's INI_Reader

mingw compile instructions:

```
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../mingw-w64-i686.cmake ..
make
```

ini_reader_test.exe requires the following files from Freelancer/EXE:
- common.dll
- dacom.dll
- dalib.dll
- zlib.dll

On my machine (Fedora 39), it requires the following files from `/usr/i686-w64-mingw32/sys-root/mingw/bin`:

- libstdc++-6.dll
- libwinpthread-1.dll
- libgcc_s_dw2-1.dll

Run with `wine ./ini_reader_test.exe test.ini`

Example output:

```
HEADER: Header
ENTRY: e1
ENTRY: e2, values = 2
ENTRY: e3, values = 3
HEADER: Header2
ENTRY: name, values = 1
ENTRY: name_quote, values = "Quoted"
ENTRY: e2, values = 1.5
```

Tested ini (unclosed header deliberate):

```
[Header
e1
e2 = 2
e3 = 3

[Header2]
name = 1
name_quote = "Quoted"
e2 = 1.5
```

