#include <iostream>
#include <windows.h>
#include <thread>

using namespace std;

// Variables
// ------------------------------------------------
int _consoleWidth = 0;
int _consoleHeight = 0;
int _consoleSize = _consoleWidth * _consoleHeight;
int _centerX, _centerY;
wchar_t* screen = new wchar_t[_consoleSize];

int lScore = 0;
int rScore = 0;

bool _gameRunning = false;
bool _ballMoving = false;

// Functions
// ------------------------------------------------
void clearScreen() {
	for (int i = 0; i < _consoleSize; i++) screen[i] = L' ';
}

void setupScreen() {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);

	_consoleWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	_consoleHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	_consoleSize = _consoleWidth * _consoleHeight;
	_centerX = _consoleWidth / 2;
	_centerY = _consoleHeight / 2;
	screen = new wchar_t[_consoleSize];
	_gameRunning = true;

	clearScreen();
}

void print(int px, int py, wstring text) {
	int offset = (py * _consoleWidth) + px;

	for (unsigned int i = 0; i < text.size(); i++)
		screen[offset + i] = text[i];
}

void print(int px, int py, wchar_t text) {
	print(px, py, wstring(1, text));
}

void clearRow(int row) {
	if (row == 0) row = 1;
	for (int i = row * _consoleWidth; i < _consoleWidth * row + _consoleWidth; i++) screen[i] = L' ';
}

void clearPixel(int x, int y) {
	screen[y * _consoleWidth + x] = L' ';
}

// Game structures
// ------------------------------------------------
struct vec2d {
	int x, y;

	vec2d(int x, int y): x(x), y(y) {}

	vec2d operator+(const vec2d& other) const {
		return vec2d(this->x + other.x, this->y + other.y);
	}

	vec2d operator+=(const vec2d& other) {
		this->x += other.x;
		this->y += other.y;

		return *this;
	}

	vec2d operator-=(const vec2d& other) {
		this->x -= other.x;
		this->y -= other.y;

		return *this;
	}

	vec2d operator-(const vec2d& other) const {
		return vec2d(this->x - other.x, this->y - other.y);
	}
};

class Pallete {
public:
	vec2d pos{0, 0};
	int height;

	Pallete() {
		this->pos = vec2d(0, 0);
		this->height = 0;
	}

	Pallete(int height, vec2d pos): height(height), pos(pos) {}

	void draw() {
		for (int i = 0; i < height; i++)
			print(this->pos.x, this->pos.y + i, L'█');
	}
};

class Ball {
public:
	vec2d pos{_centerX, _centerY};

	Ball() {
		this->pos = vec2d(_centerX, _centerY);
	}

	Ball(vec2d pos): pos(pos) {};

	void draw() {
		print(this->pos.x, this->pos.y, L'⬤');
	}
};

// Game functions & create ball and palletes
// ------------------------------------------------
Pallete pallete1;
Pallete pallete2;
Ball ball;
vec2d velocity = vec2d(1, -1);

void initVars() {
	int palleteHeight = 6;
	int palleteY = _centerY - palleteHeight / 2;

	pallete1 = Pallete(palleteHeight, vec2d(1, palleteY));
	pallete2 = Pallete(palleteHeight, vec2d(_consoleWidth - 1, palleteY));
	ball = Ball(vec2d(_centerX, _centerY));
}

void handleInput() {
	bool _lKeys[2];
	bool _rKeys[2];

	// W key, S key controls
	for (int key = 0; key < 2; key++) // UP, DOWN
		_lKeys[key] = (0x8000 & GetAsyncKeyState((unsigned char)("\x57\x53"[key]))) != 0;

	// Arrow controls
	for (int key = 0; key < 2; key++) // UP, DOWN
		_rKeys[key] = (0x8000 & GetAsyncKeyState((unsigned char)("\x26\x28"[key]))) != 0;

	if (GetAsyncKeyState('\x45') != 0) _ballMoving = true;

	if (!_ballMoving) return;

	if (_lKeys[0]) {
		if (pallete1.pos.y > 0) {
			pallete1.pos.y -= 1;
			clearPixel(pallete1.pos.x, pallete1.pos.y + pallete1.height);
		}
	} else if (_lKeys[1]) {
		if (pallete1.pos.y < _consoleHeight - pallete1.height) {
			pallete1.pos.y += 1;
			clearPixel(pallete1.pos.x, pallete1.pos.y - 1);
		}
	}

	if (_rKeys[0]) {
		if (pallete2.pos.y > 0) {
			pallete2.pos.y -= 1;
			clearPixel(pallete2.pos.x, pallete2.pos.y + pallete2.height);
		}
	} else if (_rKeys[1]) {
		if (pallete2.pos.y < _consoleHeight - pallete2.height) {
			pallete2.pos.y += 1;
			clearPixel(pallete2.pos.x, pallete2.pos.y - 1);
		}
	}
}

void reset() {
	for (int i = pallete1.pos.y; i < pallete1.pos.y + pallete1.height; i++) clearPixel(1, i);
	for (int i = pallete2.pos.y; i < pallete2.pos.y + pallete2.height; i++) clearPixel(pallete2.pos.x, i);

	pallete1 = Pallete(pallete1.height, vec2d(1, _centerY - pallete1.height / 2));
	pallete2 = Pallete(pallete2.height, vec2d(_consoleWidth - 1,  _centerY - pallete2.height / 2));
	ball = Ball(vec2d(_centerX, _centerY));
	_ballMoving = false;
	velocity = vec2d(1, -1);
}

void update() {
	if (_ballMoving) {
		clearPixel(ball.pos.x, ball.pos.y);

		if (ball.pos.y <= 0 || ball.pos.y >= _consoleHeight) velocity.y *= -1;
		if (ball.pos.x < 1) {
			reset();
			rScore++;
			
		} else if (ball.pos.x > _consoleWidth - 2) {
			reset();
			lScore++;
		}

		if (ball.pos.x == pallete1.pos.x + 1 && ball.pos.y >= pallete1.pos.y && ball.pos.y <= pallete1.pos.y + pallete1.height) velocity.x *= -1;
		if (ball.pos.x == pallete2.pos.x - 1 && ball.pos.y >= pallete2.pos.y && ball.pos.y <= pallete2.pos.y + pallete2.height) velocity.x *= -1;

		ball.pos += velocity;
	}
}

void draw() {
	// Dotted line in center
	for (int i = 0; i < _consoleHeight; i++) {
		screen[i * _consoleWidth + _centerX] = L'▓';
		i++;
	}

	clearRow(5);

	if (!_ballMoving) {
		wstring text = L"Press 'E' to start game";
		print(_centerX - text.size() / 2, 5, text);
	}

	print(_centerX / 2, 5, to_wstring(lScore));
	print(_centerX * 1.5, 5, to_wstring(rScore));

	pallete1.draw();
	pallete2.draw();
	ball.draw();
}

// Init function
// ------------------------------------------------
int main() {
	// Initialize game
	setupScreen();
	initVars();

	// Setup console screen buffer
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	DWORD dwBytesWritten;
	SetConsoleActiveScreenBuffer(hConsole);

	// Game loop
	while (_gameRunning) {
		// Game timing
		this_thread::sleep_for(50ms);

		handleInput();
		update();
		draw();

		WriteConsoleOutputCharacterW(hConsole, screen, _consoleSize, { 0, 0 }, &dwBytesWritten);
	}

	return 0;
}