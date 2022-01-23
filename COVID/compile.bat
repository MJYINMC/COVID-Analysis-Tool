del main.exe
windres icon.rc icon.o
gcc icon.o *.c ../libgraphics/*.c ../simpleGUI/*.c -o main -g -mwindows -m64 -lgdi32 -std=c99 -lwinmm -lcomdlg32
main