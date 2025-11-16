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


// ---- WINDOWS: Bat ANSI tren Windows ----
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

// ---- LINUX: tat che do canonical va dat non-blocking ----
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


// ---- Ham console tien ich ----
void hideCursor()  { cout << "\x1B[?25l"; }
void showCursor()  { cout << "\x1B[?25h"; }
void moveCursorHome(){ cout << "\x1B[H"; }
void clearScreenOnce() { cout << "\x1B[2J"; }


// ---- Doc phim khong chan ----
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

struct Game {
    deque<pii> snake;
    pii food;
    Direction dir;
    int width, height;
    bool over = false;

    Game(int w = 40, int h = 20): width(w), height(h) {
        reset();
    }
    void reset() {
        snake.clear();
        int sx = width/2, sy = height/2;

        snake.push_back({sx, sy});
        snake.push_back({sx+1, sy});
        snake.push_back({sx+2, sy});
        dir = LEFT;
        placeFood();
        score = 0;
        over = false;
    }
    void placeFood() {
        static mt19937 rng((unsigned)chrono::high_resolution_clock::now().time_since_epoch().count());
        uniform_int_distribution<int> dx(1, width-2), dy(1, height-2);
        while (true) {
            int fx = dx(rng), fy = dy(rng);
            bool on = false;
            for (auto &s: snake) if (s.first==fx && s.second==fy) { on = true; break; }
            if (!on) { food = {fx, fy}; break; }
        }
    }
    void changeDirFromKey(int key) {
    if (key==0) return;
    char c = (char)key;
    if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';

    Direction nd = dir;

    if (c == 'w') nd = UP;
    else if (c == 'a') nd = LEFT;
    else if (c == 's') nd = DOWN;
    else if (c == 'd') nd = RIGHT;

    if ((dir==LEFT && nd==RIGHT) || (dir==RIGHT && nd==LEFT) ||
        (dir==UP && nd==DOWN) || (dir==DOWN && nd==UP)) return;

    dir = nd;
}
    void update() {
        if (over) return;
        pii head = snake.front();
        pii nh = head;
        if (dir==LEFT) nh.first -= 1;
        else if (dir==RIGHT) nh.first += 1;
        else if (dir==UP) nh.second -= 1;
        else if (dir==DOWN) nh.second += 1;
        if (nh.first <= 0 || nh.first >= width-1 || nh.second <= 0 || nh.second >= height-1) {
            over = true; return;
        }

        for (auto &s: snake) if (s.first==nh.first && s.second==nh.second) { over = true; return; }

        snake.push_front(nh);
        if (nh == food) {
            score += 10;
            placeFood();
        } else snake.pop_back();
    }
};
void drawGameFrame(const Game &g) {
    stringstream ss;

    for (int x=0; x<g.width; ++x) ss << '#';
    ss << '\n';

    for (int y=1; y<g.height-1; ++y) {
        ss << '#';
        for (int x=1; x<g.width-1; ++x) {
            char ch = ' ';

            if (x == g.food.first && y == g.food.second)
                ch = 'O';
            else {
                int idx=0;
                for (auto &s: g.snake) {
                    if (s.first==x && s.second==y) {
                        ch = (idx == 0 ? '@' : 'o');
                        break;
                    }
                    ++idx;
                }
            }

            ss << ch;
        }
        ss << "#\n";
    }

    for (int x=0; x<g.width; ++x) ss << '#';

    ss << "\nScore: " << score 
       << "    Controls: W A S D    (q to quit)\n";

    moveCursorHome();
    cout << ss.str();
    cout.flush();
}
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

#ifdef _WIN32
    enableAnsiWindows();
#else
    termSaver.enable();
#endif

    hideCursor();
    clearScreenOnce();
    moveCursorHome();

    Game g(WIDTH, HEIGHT);

    const int TICK_MS = 100;
    drawGameFrame(g);

    while (!quitting) {
        int k = getCharNonBlocking();

        if (k) {
            if (k=='q' || k=='Q') { quitting = true; break; }
            g.changeDirFromKey(k);
        }

        g.update();
        drawGameFrame(g);

        if (g.over) break;

        this_thread::sleep_for(chrono::milliseconds(TICK_MS));
    }

    drawGameFrame(g);

    if (g.over) {
        cout << "\nGAME OVER! Final score: " << score << "\n";
        cout << "Press r to restart, any other key to exit.\n";

        while (true) {
            int k = getCharNonBlocking();
            if (k) {
                if (k=='r' || k=='R') {
                    g.reset();
                    while (!g.over) {
                        int k2 = getCharNonBlocking();
                        if (k2) {
                            if (k2=='q' || k2=='Q') { quitting = true; break; }
                            g.changeDirFromKey(k2);
                        }
                        g.update();
                        drawGameFrame(g);
                        this_thread::sleep_for(chrono::milliseconds(TICK_MS));
                    }
                    drawGameFrame(g);
                    break;
                } else break;
            }
            this_thread::sleep_for(chrono::milliseconds(50));
        }
    }

    showCursor();
#ifndef _WIN32
    termSaver.disable();
#endif

    return 0;
}
