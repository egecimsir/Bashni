# bashni

## Building

### Normal Build
```
$ make
gcc -Wall -Wextra -Werror -c sysprak-client.c
gcc -Wall -Wextra -Werror -c performConnection.c
gcc -Wall -Wextra -Werror -c sharedMemory.c
gcc -Wall -Wextra -Werror -o sysprak-client sysprak-client.o performConnection.o sharedMemory.o
```

### Debug Build
The debug build includes additional debug output and debug symbols.

```
$ make debug
rm -f *.o sysprak-client sysprak-client-debug
gcc -Wall -Wextra -Werror -DDEBUG -c sysprak-client.c
gcc -Wall -Wextra -Werror -DDEBUG -c performConnection.c
gcc -Wall -Wextra -Werror -DDEBUG -c sharedMemory.c
gcc -Wall -Wextra -Werror -DDEBUG -o sysprak-client sysprak-client.o performConnection.o sharedMemory.o
mv sysprak-client sysprak-client-debug
```

### Cleanup

```
$ make clean
rm -f *.o sysprak-client sysprak-client-debug
```

## Running the client

Either specify a valid game ID and optionally the desired player slot
and run the client with the following make target:
```
GAME_ID=1231231231231 make play
```

Or run the executable manually and pass the game ID and player slot
either via the environment variables or command-line parameters:
```
PLAYER=1 ./sysprak-client -g 1231231231231 -c custom-client.conf
```

## Release

Create the ZIP archive ready for submission:
```
$ make release
rm -f *.o sysprak-client sysprak-client-debug abgabe.zip
zip -r abgabe.zip . -x @.zip-excludes
  adding: sharedMemory.h (deflated 52%)
  .....
  adding: config.o (deflated 69%)
```

and then make sure, that it is small enough (lt 512KB):

```
$ du -h abgabe.zip
22K	abgabe.zip
```

## Testing

Run the official testscript with any build directory name and
file for the valgrind log and provide the path to the release archive (see Release above).

```
$ ./sysprak-abgabe.sh build_dir valgrind_log.txt abgabe.zip
```
