#include <iostream>
#include <windows.h>
#include <conio.h>
using namespace std;

const int BOARD_LEFT = 17;
const int BOARD_TOP = 1;
const int SQUARE_WIDTH = 5;
const int SQUARE_HEIGHT = 3;

void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

bool isInBounds(int x, int y) {
    return x >= 1 && x <= 8 && y >= 1 && y <= 8;
}

struct dataHighlight {
    int x, y, piece;
    bool unhighlight_it;
};
dataHighlight data[9][9];

class CPiece {
public:
    bool is_empty;
    bool is_highlight;
    int which_piece;
    bool kill_him;

    CPiece() {
        is_empty = true;
        is_highlight = false;
        which_piece = 0;
        kill_him = false;
    }

    void setSquare(int x, int y, bool empty, bool highlight, int piece, bool kill) {
        is_empty = empty;
        is_highlight = highlight;
        which_piece = piece;
        kill_him = kill;
    }

    bool selectPiece() const { return !is_empty; }
    bool selectNewPosition() const { return is_highlight; }

    void printBlank(int col, int row) {
        int isGreen = (col + row) % 2;
        int x = BOARD_LEFT + (col - 1) * SQUARE_WIDTH + 1;
        int y = BOARD_TOP + (8 - row) * SQUARE_HEIGHT + 1;
        gotoxy(x, y);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), isGreen ? 2 : 8);
        char ch = isGreen ? '\xB0' : '\xDB';
        cout << ch << ch;
    }

    void printPiece(int col, int row, int color, int pieceType) {
        int x = BOARD_LEFT + (col - 1) * SQUARE_WIDTH + 1;
        int y = BOARD_TOP + (8 - row) * SQUARE_HEIGHT + 1;
        gotoxy(x, y);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
        cout << getSymbol(pieceType);
    }

    void pinkHighlightStep(int col, int row, int piece) {
        int x = BOARD_LEFT + (col - 1) * SQUARE_WIDTH + 1;
        int y = BOARD_TOP + (8 - row) * SQUARE_HEIGHT + 1;
        gotoxy(x, y);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 13);
        if (piece == 0) cout << "**";
        else cout << getSymbol(piece);
    }

    void unhighlightStep(int col, int row) {
        if (!is_empty) printPiece(col, row, kill_him ? 15 : 10, which_piece);
        else printBlank(col, row);
    }

    string getSymbol(int piece) const {
        switch (piece) {
            case 1: return "\xDB"; // Pawn
            case 2: return "\x15"; // Knight
            case 3: return "\x0F"; // Bishop
            case 4: return "[]";   // Rook
            case 5: return "\x03"; // Queen
            case 6: return "\x05"; // King
            default: return "  ";
        }
    }

    void highlightPiece(int x, int y, int type);
};

CPiece gameSquares[9][9];

void highlightMove(int x, int y, int piece) {
    gameSquares[x][y].pinkHighlightStep(x, y, piece);
    gameSquares[x][y].setSquare(x, y, piece == 0, true, piece, gameSquares[x][y].kill_him);
    data[x][y].x = x; data[x][y].y = y; data[x][y].piece = piece; data[x][y].unhighlight_it = true;
}

void highlightLinearMoves(int x, int y, int directions[][2], int dirCount, bool isKing) {
    for (int i = 0; i < dirCount; i++) {
        int dx = directions[i][0], dy = directions[i][1];
        int nx = x + dx, ny = y + dy;
        while (isInBounds(nx, ny)) {
            if (gameSquares[nx][ny].is_empty) highlightMove(nx, ny, 0);
            else {
                if (gameSquares[nx][ny].kill_him != gameSquares[x][y].kill_him)
                    highlightMove(nx, ny, gameSquares[nx][ny].which_piece);
                break;
            }
            if (isKing) break;
            nx += dx; ny += dy;
        }
    }
}

void CPiece::highlightPiece(int x, int y, int type) {
    printPiece(x, y, 14, type);
    if (type == 1) {
        int dir = kill_him ? -1 : 1;
        int startRow = kill_him ? 7 : 2;
        int fwd1 = y + dir;
        if (isInBounds(x, fwd1) && gameSquares[x][fwd1].is_empty) highlightMove(x, fwd1, 0);
        int fwd2 = y + 2 * dir;
        if (y == startRow && gameSquares[x][fwd1].is_empty && gameSquares[x][fwd2].is_empty)
            highlightMove(x, fwd2, 0);
        for (int dx = -1; dx <= 1; dx += 2) {
            int nx = x + dx, ny = y + dir;
            if (isInBounds(nx, ny) && !gameSquares[nx][ny].is_empty && gameSquares[nx][ny].kill_him != kill_him)
                highlightMove(nx, ny, gameSquares[nx][ny].which_piece);
        }
    } else if (type == 2) {
        const int moves[8][2] = {{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
        for (int i = 0; i < 8; ++i) {
            int nx = x + moves[i][0], ny = y + moves[i][1];
            if (!isInBounds(nx, ny)) continue;
            if (gameSquares[nx][ny].is_empty || gameSquares[nx][ny].kill_him != kill_him)
                highlightMove(nx, ny, gameSquares[nx][ny].which_piece);
        }
    } else if (type == 3 || type == 4 || type == 5 || type == 6) {
        int bishopDirs[4][2] = {{1,1},{-1,1},{-1,-1},{1,-1}};
        int rookDirs[4][2] = {{0,1},{0,-1},{1,0},{-1,0}};
        if (type == 3) highlightLinearMoves(x, y, bishopDirs, 4, false);
        else if (type == 4) highlightLinearMoves(x, y, rookDirs, 4, false);
        else if (type == 5) {
            highlightLinearMoves(x, y, bishopDirs, 4, false);
            highlightLinearMoves(x, y, rookDirs, 4, false);
        }
        else if (type == 6) {
            highlightLinearMoves(x, y, bishopDirs, 4, true);
            highlightLinearMoves(x, y, rookDirs, 4, true);
        }
    }
}

// Game utilities
void check_and_unhighlight() {
    for (int i = 1; i <= 8; ++i)
        for (int j = 1; j <= 8; ++j)
            if (data[i][j].unhighlight_it) {
                gameSquares[i][j].unhighlightStep(i, j);
                data[i][j].unhighlight_it = false;
            }
}

bool end_game_or_not() {
    int kingCount = 0;
    for (int i = 1; i <= 8; i++)
        for (int j = 1; j <= 8; j++)
            if (gameSquares[i][j].which_piece == 6)
                kingCount++;
    return kingCount < 2;
}

void makeMove(int fromX, int fromY, int toX, int toY, bool& turn) {
    CPiece& from = gameSquares[fromX][fromY];
    CPiece& to = gameSquares[toX][toY];
    int color = from.kill_him ? 15 : 10;

    if (!to.is_empty) to.printBlank(toX, toY);
    if (from.which_piece == 1 && ((from.kill_him == 0 && toY == 8) || (from.kill_him == 1 && toY == 1)))
        from.which_piece = 5; // Promote to Queen

    to.setSquare(toX, toY, false, false, from.which_piece, from.kill_him);
    to.printPiece(toX, toY, color, from.which_piece);
    from.setSquare(fromX, fromY, true, false, 0, false);
    from.printBlank(fromX, fromY);
    check_and_unhighlight();
    turn = !turn;
}

int main() {
    // Draw board
    for (int row = 1; row <= 8; row++) {
        for (int col = 1; col <= 8; col++) {
            gameSquares[col][row].printBlank(col, row);
        }
    }

    // Print labels
    for (int col = 1; col <= 8; col++) {
        int x = BOARD_LEFT + (col - 1) * SQUARE_WIDTH;
        gotoxy(x + 1, BOARD_TOP + 8 * SQUARE_HEIGHT);
        cout << col;
    }
    for (int row = 1; row <= 8; row++) {
        int y = BOARD_TOP + (8 - row) * SQUARE_HEIGHT + 1;
        gotoxy(BOARD_LEFT - 3, y);
        cout << row;
    }

    // Setup full board: pawns and kings only for demo
    for (int i = 1; i <= 8; i++) {
        gameSquares[i][2].setSquare(i, 2, false, false, 1, 0); // Green Pawns
        gameSquares[i][2].printPiece(i, 2, 10, 1);

        gameSquares[i][7].setSquare(i, 7, false, false, 1, 1); // White Pawns
        gameSquares[i][7].printPiece(i, 7, 15, 1);
    }
    gameSquares[5][1].setSquare(5, 1, false, false, 6, 0); // Green King
    gameSquares[5][1].printPiece(5, 1, 10, 6);

    gameSquares[5][8].setSquare(5, 8, false, false, 6, 1); // White King
    gameSquares[5][8].printPiece(5, 8, 15, 6);

    // Start loop
    bool turn = 0; // 0 = green, 1 = white
    while (true) {
        gotoxy(0, 25);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
        cout << (turn ? "White" : "Green") << " to move. Select piece (XY): ";

        char chx = getche();
        char chy = getche();
        int fx = chx - '0', fy = chy - '0';

        if (!isInBounds(fx, fy) || !gameSquares[fx][fy].selectPiece() || gameSquares[fx][fy].kill_him != turn) {
            gotoxy(0, 26); cout << "Invalid selection.";
            continue;
        }

        gameSquares[fx][fy].highlightPiece(fx, fy, gameSquares[fx][fy].which_piece);
        gotoxy(0, 26); cout << "Move to (XY or 00 to cancel): ";
        chx = getche(); chy = getche();
        int tx = chx - '0', ty = chy - '0';

        if (tx == 0 && ty == 0) {
            check_and_unhighlight();
            continue;
        }

        if (!isInBounds(tx, ty) || !gameSquares[tx][ty].is_highlight) {
            gotoxy(0, 27); cout << "Invalid move!";
            check_and_unhighlight();
            continue;
        }

        makeMove(fx, fy, tx, ty, turn);
        if (end_game_or_not()) break;
    }

    gotoxy(0, 28); cout << "Game Over!";
    system("pause");
    return 0;
}
