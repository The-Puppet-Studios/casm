echo -n "Do you want to compile it for linux or windows: "
read linuxorwindows

if [ $linuxorwindows == "linux" ]; then
    echo "Compiling..."
    gcc -o out/casm.out src/main.c
    echo "Compiled!"
elif [ $linuxorwindows == "windows" ]; then
    echo "Compiling..."
    x86_64-w64-mingw32-gcc -o out/casm.exe src/main.c
    echo "Compiled!"
else
    echo "Invalid option. Exiting..."
fi