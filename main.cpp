#include <bits/stdc++.h>
#include <thread>
#include <chrono>

#ifdef _WIN32
  #include <windows.h>
  #include <conio.h>
  #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
  #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
  #endif

#else
  #include <termios.h>
  #include <unistd.h>
  #include <fcntl.h>
#endif

using namespace std;
using pii = pair<int,int>;

int WIDTH = 40;
int HEIGHT = 20;
int score = 0;
bool quitting = false;

enum Direction { LEFT=0, UP=1, DOWN=2, RIGHT=3 };


// ---- WINDOWS: B?t ANSI mode ----
#ifdef _WIN32
void enableAnsiWindows() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}
#endif

// ---- LINUX: t?t ch? ?? canonical ?? ??c non-blocking ----
#ifndef _WIN32
struct TermSaver {
    struct termios orig{};
    bool enabled = false;

    void enable() {
        if (enabled) return;

        tcgetattr(STDIN_FILENO, &orig);
        struct termios raw = orig;

        raw.c_lflag &= ~(ICANON | ECHO);
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);

        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

        enabled = true;
    }

    void disable() {
        if (!enabled) return;
        tcsetattr(STDIN_FILENO, TCSANOW, &orig);
        enabled = false;
    }
} termSaver;
#endif


// ---- Hàm console ti?n ích ----
void hideCursor()  { cout << "\x1B[?25l"; }
void showCursor()  { cout << "\x1B[?25h"; }
void moveCursorHome(){ cout << "\x1B[H"; }
void clearScreenOnce() { cout << "\x1B[2J"; }


// ---- ??c phím không ch?n ----
int getCharNonBlocking() {
#ifdef _WIN32
    if (_kbhit()) {
        int c = _getch();
        if (c == 0 || c == 224) { 
            if (_kbhit()) _getch();
            return 0;
        }
        return c;
    }
    return 0;
#else
    unsigned char ch = 0;
    ssize_t r = read(STDIN_FILENO, &ch, 1);
    if (r == 1) return ch;
    return 0;
#endif
}


