#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h> /* Добавлено для инициализации генератора случайных чисел */

#define SIZE 10
#define TOTAL_SHIP_CELLS 20 /* Всего палуб во флоте (4 + 3*2 + 2*3 + 1*4 = 20) */

const char *COLS_UPPER[SIZE] = {"А", "Б", "В", "Г", "Д", "Е", "Ё", "Ж", "З", "И"};
const char *COLS_LOWER[SIZE] = {"а", "б", "в", "г", "д", "е", "ё", "ж", "з", "и"};

typedef struct {
    int length;
    int row;           /* 0..9 (соответствует строкам 1..10) */
    int col;           /* 0..9 (соответствует столбцам А..И) */
    int isHorizontal;  /* 1 - горизонтально, 0 - вертикально */
    int hits;
} Ship;

int getColIndex(const char *str) {
    int j;
    for (j = 0; j < SIZE; j++) {
        if (strcmp(str, COLS_UPPER[j]) == 0 || strcmp(str, COLS_LOWER[j]) == 0) {
            return j;
        }
    }
    return -1;
}

void initBoard(char board[SIZE][SIZE], char fillChar) {
    int i, j;
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            board[i][j] = fillChar;
        }
    }
}

void placeShips(char board[SIZE][SIZE], Ship ships[], int shipCount) {
    int s, k;
    for (s = 0; s < shipCount; s++) {
        ships[s].hits = 0;
        for (k = 0; k < ships[s].length; k++) {
            int r = ships[s].row;
            int c = ships[s].col;
            if (ships[s].isHorizontal) {
                if (r >= 0 && r < SIZE && c + k >= 0 && c + k < SIZE) {
                    board[r][c + k] = '#';
                }
            } else {
                if (r + k >= 0 && r + k < SIZE && c >= 0 && c < SIZE) {
                    board[r + k][c] = '#';
                }
            }
        }
    }
}

/* Проверка: можно ли поставить корабль в эти координаты с учетом отступа в 1 клетку */
int isPlacementValid(char board[SIZE][SIZE], int row, int col, int length, int isHorizontal) {
    int k, i, j;
    
    /* Проверка выхода за границы доски */
    if (isHorizontal) {
        if (col + length > SIZE) return 0;
    } else {
        if (row + length > SIZE) return 0;
    }

    /* Проверяем сам корабль и его окружение (на 1 клетку вокруг) */
    int startRow = row - 1 < 0 ? 0 : row - 1;
    int endRow = isHorizontal ? row + 1 : row + length;
    if (endRow >= SIZE) endRow = SIZE - 1;

    int startCol = col - 1 < 0 ? 0 : col - 1;
    int endCol = isHorizontal ? col + length : col + 1;
    if (endCol >= SIZE) endCol = SIZE - 1;

    for (i = startRow; i <= endRow; i++) {
        for (j = startCol; j <= endCol; j++) {
            if (board[i][j] == '#') {
                return 0; /* Рядом или на этом месте уже есть другой корабль */
            }
        }
    }
    return 1;
}

/* Функция случайной расстановки флота */
void generateRandomShips(Ship ships[], int shipCount) {
    char tempBoard[SIZE][SIZE];
    initBoard(tempBoard, '.');

    /* Набор длин кораблей по правилам «Морского боя» */
    int shipLengths[10] = {4, 3, 3, 2, 2, 2, 1, 1, 1, 1};
    int s;

    for (s = 0; s < shipCount; s++) {
        int length = shipLengths[s];
        int placed = 0;

        while (!placed) {
            int row = rand() % SIZE;
            int col = rand() % SIZE;
            int isHorizontal = rand() % 2;

            if (isPlacementValid(tempBoard, row, col, length, isHorizontal)) {
                ships[s].length = length;
                ships[s].row = row;
                ships[s].col = col;
                ships[s].isHorizontal = isHorizontal;
                ships[s].hits = 0;

                /* Временно фиксируем корабль на виртуальной доске, чтобы следующие с ним не пересекались */
                int k;
                for (k = 0; k < length; k++) {
                    if (isHorizontal) {
                        tempBoard[row][col + k] = '#';
                    } else {
                        tempBoard[row + k][col] = '#';
                    }
                }
                placed = 1;
            }
        }
    }
}

void printBoard(const char board[SIZE][SIZE], const char *title) {
    int i, j;
    printf("\n=== %s ===\n", title);
    printf("  ");
    for (j = 0; j < SIZE; j++) {
        printf("%s ", COLS_UPPER[j]);
    }
    printf("\n");
    printf("  ");
    for (j = 0; j < SIZE * 3; j++) {
        printf("-");
    }
    printf("\n");
    for (i = 0; i < SIZE; i++) {
        int in = i + 1;
        if (in < 10) {
            printf(" %d |", in);
        } else {
            printf("%d |", in);
        }
        for (j = 0; j < SIZE; j++) {
            printf("%c ", board[i][j]);
        }
        printf("\n");
    }
}

int processShot(int r, int c, char targetFleet[SIZE][SIZE], char shooterTracking[SIZE][SIZE], Ship targetShips[], int shipCount) {
    if (targetFleet[r][c] == '#') {
        int s, k;
        targetFleet[r][c] = 'X';
        shooterTracking[r][c] = 'X';
        for (s = 0; s < shipCount; s++) {
            for (k = 0; k < targetShips[s].length; k++) {
                int sr = targetShips[s].isHorizontal ? targetShips[s].row : targetShips[s].row + k;
                int sc = targetShips[s].isHorizontal ? targetShips[s].col + k : targetShips[s].col;
                if (sr == r && sc == c) {
                    targetShips[s].hits++;
                    if (targetShips[s].hits >= targetShips[s].length) {
                        return 2;
                    }
                    return 1;
                }
            }
        }
        return 1;
    } else {
        if (targetFleet[r][c] == '.') {
            targetFleet[r][c] = '*';
        }
        shooterTracking[r][c] = '*';
        return 0;
    }
}

int allShipsSunk(const Ship ships[], int shipCount) {
    int s;
    for (s = 0; s < shipCount; s++) {
        if (ships[s].hits < ships[s].length) {
            return 0;
        }
    }
    return 1;
}

int countHitsOnBoard(const char board[SIZE][SIZE]) {
    int i, j, hits = 0;
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            if (board[i][j] == 'X') {
                hits++;
            }
        }
    }
    return hits;
}

void clearInputBuffer(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) { }
}

void renderScreen(const char p1Shots[SIZE][SIZE], const char p1Fleet[SIZE][SIZE], const char *status) {
    system("cls");
    printf("Статус: %s\n", status);
    printBoard(p1Shots, "Мины (Места где ты уже подорвал мину)");
    printBoard(p1Fleet, "Корабли (твои подлодки и места куда стрелял противник)");
}

void takeManualShotP1(char shooterTracking[SIZE][SIZE], char statusOut[256]) {
    int rInput = 0;
    char colStr[8];
    int r, c, res;
    while (1) {
        printf("\nПервый, введи координаты выстрела: ");
        fflush(stdout);
        if (scanf("%d %7s", &rInput, colStr) != 2) {
            clearInputBuffer();
            printf("Введи от 1 до 10 ярдов по вертикали и от А до И по горизонтали, например 3 А. Давай ещё раз..\n");
            continue;
        }
        clearInputBuffer();
        r = rInput - 1;
        c = getColIndex(colStr);
        if (r < 0 || r >= SIZE || c < 0 || c >= SIZE) {
            printf("В сторону гражданки подрывать мины запрещено! Головой своей подумай!\n");
            continue;
        }
        if (shooterTracking[r][c] != '.') {
            printf("Мина на (%d, %s) уже взорвана, подрывай другую!\n", rInput, COLS_UPPER[c]);
            continue;
        }
        while (1) {
            printf("Ты подорвал подлодку второго игрока? в (%d, %s)? (0 - мимо, 1 - ранил, 2 - потопил): ", rInput, COLS_UPPER[c]);
            fflush(stdout);
            if (scanf("%d", &res) == 1 && (res == 0 || res == 1 || res == 2)) {
                clearInputBuffer();
                break;
            }
            clearInputBuffer();
            printf("Чего..?\n");
        }
        if (res == 2) {
            shooterTracking[r][c] = 'X';
            snprintf(statusOut, 256, "Игрок 1 взорвал мину в (%d, %s): Потоплен (потопил)!", rInput, COLS_UPPER[c]);
        } else if (res == 1) {
            shooterTracking[r][c] = 'X';
            snprintf(statusOut, 256, "Игрок 1 взорвал мину в (%d, %s): Пробил каюту (ранил)!", rInput, COLS_UPPER[c]);
        } else {
            shooterTracking[r][c] = '*';
            snprintf(statusOut, 256, "Игрок 1 взорвал мину в (%d, %s): Мимо (мимо).", rInput, COLS_UPPER[c]);
        }
        return;
    }
}

void takeAutoShotP2(char targetFleet[SIZE][SIZE], char shooterTracking[SIZE][SIZE], Ship targetShips[], int shipCount, char statusOut[256]) {
    int rInput = 0;
    char colStr[8];
    int r, c, res;
    while (1) {
        printf("\nВторой, взрывай уже с 1 до 10 и с А до И! ");
        fflush(stdout);
        if (scanf("%d %7s", &rInput, colStr) != 2) {
            clearInputBuffer();
            printf("Введи от 1 до 10 ярдов по вертикали и от А до И по горизонтали например 3 А. Давай ещё раз..\n");
            continue;
        }
        clearInputBuffer();
        r = rInput - 1;
        c = getColIndex(colStr);
        if (r < 0 || r >= SIZE || c < 0 || c >= SIZE) {
            printf("В сторону гражданки подрывать мины запрещено! Головой своей подумай!\n");
            continue;
        }
        if (shooterTracking[r][c] != '.') {
            printf("Мина на (%d, %s) уже взорвана, давай другую!\n", rInput, COLS_UPPER[c]);
            continue;
        }
        res = processShot(r, c, targetFleet, shooterTracking, targetShips, shipCount);
        if (res == 2) {
            snprintf(statusOut, 256, "Игрок 2 взорвал мину в (%d, %s): Потоплен!", rInput, COLS_UPPER[c]);
        } else if (res == 1) {
            snprintf(statusOut, 256, "Игрок 2 взорвал мину в (%d, %s): Пробило каюту!", rInput, COLS_UPPER[c]);
        } else {
            snprintf(statusOut, 256, "Игрок 2 взорвал мину в (%d, %s): Мимо.", rInput, COLS_UPPER[c]);
        }
        return;
    }
}

int main(void) {
    /* Инициализируем генератор случайных чисел текущим временем */
srand((unsigned int)time(NULL));
char p1Fleet[SIZE][SIZE];
char p1Shots[SIZE][SIZE];
char p2Shots[SIZE][SIZE];
Ship p1Ships[10];
int p1ShipCount = 10;
char status[256] = "Субмарины и мины случайно расставлены.";
int currentPlayer = 0;
SetConsoleOutputCP(65001);
SetConsoleCP(65001);
initBoard(p1Fleet, '.');
initBoard(p1Shots, '.');
initBoard(p2Shots, '.');
/* Генерация случайного расположения и перенос кораблей на доску */
generateRandomShips(p1Ships, p1ShipCount);
placeShips(p1Fleet, p1Ships, p1ShipCount);
while (1) {
renderScreen(p1Shots, p1Fleet, status);
if (countHitsOnBoard(p1Shots) >= TOTAL_SHIP_CELLS) {
printf("\nВсе корабли 2 игрока потоплены! Победа 1 игрока!\n\n Игрок 1 \n+--------+ Игрок 2 \n| 1место | +--------+\n|        | | 2место |\n+--------+ +--------+\n");
break;
}
if (allShipsSunk(p1Ships, p1ShipCount)) {
printf("\nВсе корабли 1 игрока потоплены! Победа 2 игрока!\n\n Игрок 2 \n+--------+ Игрок 1 \n| 1место | +--------+\n|        | | 2место |\n+--------+ +--------+\n");
break;
}
while (1) {
printf("\nКто сейчас подрывает? (1 - Первый Юнга, 2 - Второй Юнга): ");
fflush(stdout);
if (scanf("%d", &currentPlayer) == 1 && (currentPlayer == 1 || currentPlayer == 2)) {
clearInputBuffer();
break;
}
clearInputBuffer();
printf("Запиши 1 или 2!\n");
}
if (currentPlayer == 1) {
takeManualShotP1(p1Shots, status);
} else {
takeAutoShotP2(p1Fleet, p2Shots, p1Ships, p1ShipCount, status);
}
}
return 0;
}