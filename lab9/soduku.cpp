#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <stack>
#include <sys/socket.h>
#include <sys/un.h>
#include <tuple>
#include <unistd.h>
#include <vector>

class Board {
    static constexpr int s_allFullMask{ 0b111'111'111 };
    int m_row[9]{};
    int m_col[9]{};
    int m_cell[3][3]{};

    bool checkRow(int row, int number) {
        return (m_row[row] & (1 << (number - 1))) == 0;
    }

    bool checkCol(int col, int number) {
        return (m_col[col] & (1 << (number - 1))) == 0;
    }

    bool checkCell(int row, int col, int number) {
        return (m_cell[row / 3][col / 3] & (1 << (number - 1))) == 0;
    }

  public:
    int board[9][9]{};

    bool isFinish() {
        for (int i{ 0 }; i < 9; i++)
            if (m_row[i] != s_allFullMask)
                return false;
        return true;
    }

    void setValue(int row, int col, int number) {
        m_row[row] |= 1 << (number - 1);
        m_col[col] |= 1 << (number - 1);
        m_cell[row / 3][col / 3] |= 1 << (number - 1);
        board[row][col] = number;
    }

    bool checkValue(int row, int col, int number) {
        if (!checkRow(row, number))
            return false;
        if (!checkCol(col, number))
            return false;
        if (!checkCell(row, col, number))
            return false;
        return true;
    }
};

int main(int argc, char* argv[]) {
    int socketFd{ socket(AF_LOCAL, SOCK_STREAM, 0) };

    sockaddr_un address{};
    address.sun_family = AF_LOCAL;
    strncpy(address.sun_path, "/sudoku.sock", sizeof(address.sun_path) - 1);
    int n = connect(socketFd, (sockaddr*)&address, SUN_LEN(&address)); // SUN_LEN(): get length of sockaddr_un
    if (n < 0)
        fprintf(stderr, "connect error!\n");
    fprintf(stderr, "connect success!\n");

    write(socketFd, "S", 1);

    char boardChar[100]{};
    if (read(socketFd, boardChar, sizeof(boardChar)) < 0)
        fprintf(stderr, "read error!\n");

    fprintf(stderr, "board: %s\n", boardChar);

    Board initBoard{};

    for (int row{ 0 }; row < 9; row++) {
        for (int col{ 0 }; col < 9; col++) {
            int number{ boardChar[4 + row * 9 + col] - '0' };

            if (number > 0 && number <= 9)
                initBoard.setValue(row, col, number);
        }
    }

    for (int row{ 0 }; row < 9; row++) {
        for (int col{ 0 }; col < 9; col++) {
            fprintf(stderr, "%d", initBoard.board[row][col]);
        }
    }

    std::vector<Board> possibleSolution{};

    std::stack<std::tuple<Board, int, int>> temp{};
    int nextRow{ 0 };
    int nextCol{ 0 };
    while (initBoard.board[nextRow][nextCol] != 0) {
        if (nextCol == 8) {
            nextCol = 0;
            nextRow++;
        } else {
            nextCol++;
        }

        if (nextRow == 9)
            break;
    }
    fprintf(stderr, "start pos (%d %d)\n", nextRow, nextCol);
    temp.emplace(initBoard, nextRow, nextCol);

    fprintf(stderr, "solving...\n");
    Board currentBoard;
    while (!temp.empty()) {
        currentBoard = std::get<0>(temp.top());
        int currentRow{ std::get<1>(temp.top()) };
        int currentCol{ std::get<2>(temp.top()) };
        temp.pop();

        for (int i{ 1 }; i <= 9; i++) {
            if (currentBoard.checkValue(currentRow, currentCol, i)) {
                Board newBoard{ currentBoard };
                newBoard.setValue(currentRow, currentCol, i);

                int nextRow{ currentRow };
                int nextCol{ currentCol };
                do {
                    if (nextCol == 8) {
                        nextCol = 0;
                        nextRow++;
                    } else {
                        nextCol++;
                    }

                    if (nextRow == 9)
                        break;

                } while (newBoard.board[nextRow][nextCol] != 0);

                if (nextRow == 9) {
                    possibleSolution.emplace_back(newBoard);
                } else
                    temp.emplace(newBoard, nextRow, nextCol);
            }
        }
    }
    if (possibleSolution.empty())
        fprintf(stderr, "no solution!\n");
    fprintf(stderr, "solved!\n");

    char cmd[100];
    Board& solution{ possibleSolution[0] };
    for (int row{ 0 }; row < 9; row++) {
        for (int col{ 0 }; col < 9; col++) {
            if (initBoard.board[row][col] == 0) {
                sprintf(cmd, "V %d %d %d", row, col, solution.board[row][col]);
                write(socketFd, cmd, strlen(cmd));
                char input[1000];
                read(socketFd, input, sizeof(input));
            }
        }
    }

    write(socketFd, "C", 1);

    return 0;
}