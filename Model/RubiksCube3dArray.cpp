#include "RubiksCube3dArray.h"
#include <string>

void RubiksCube3dArray::rotateFace(int ind) {
    char temp_arr[3][3] = {};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            temp_arr[i][j] = cube[ind][i][j];
        }
    }
    for (int i = 0; i < 3; i++) cube[ind][0][i] = temp_arr[2 - i][0];
    for (int i = 0; i < 3; i++) cube[ind][i][2] = temp_arr[0][i];
    for (int i = 0; i < 3; i++) cube[ind][2][2 - i] = temp_arr[i][2];
    for (int i = 0; i < 3; i++) cube[ind][2 - i][0] = temp_arr[2][2 - i];
}

RubiksCube3dArray::RubiksCube3dArray() {
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++)
                cube[i][j][k] = getColorLetter(COLOR(i));
        }
    }
}

RubiksCube::COLOR RubiksCube3dArray::getColor(FACE face, unsigned row, unsigned col) const {
    char color = cube[int(face)][row][col];
    switch (color) {
        case 'B':
            return COLOR::BLUE;
        case 'R':
            return COLOR::RED;
        case 'G':
            return COLOR::GREEN;
        case 'O':
            return COLOR::ORANGE;
        case 'Y':
            return COLOR::YELLOW;
        default:
            return COLOR::WHITE;
    }
}

bool RubiksCube3dArray::isSolved() const {
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                if (this->cube[i][j][k] == getColorLetter(COLOR(i))) continue;
                return false;
            }
        }
    }
    return true;
}

RubiksCube &RubiksCube3dArray::u() {
    this->rotateFace(0);

    char temp_arr[3] = {};
    for (int i = 0; i < 3; i++) temp_arr[i] = cube[4][0][2 - i];
    for (int i = 0; i < 3; i++) cube[4][0][2 - i] = cube[1][0][2 - i];
    for (int i = 0; i < 3; i++) cube[1][0][2 - i] = cube[2][0][2 - i];
    for (int i = 0; i < 3; i++) cube[2][0][2 - i] = cube[3][0][2 - i];
    for (int i = 0; i < 3; i++) cube[3][0][2 - i] = temp_arr[i];

    return *this;
}

RubiksCube &RubiksCube3dArray::uPrime() {
    this->u();
    this->u();
    this->u();

    return *this;
}

RubiksCube &RubiksCube3dArray::u2() {
    this->u();
    this->u();

    return *this;
}

RubiksCube &RubiksCube3dArray::l() {
    this->rotateFace(1);

    char temp_arr[3] = {};
    for (int i = 0; i < 3; i++) temp_arr[i] = cube[0][i][0];
    for (int i = 0; i < 3; i++) cube[0][i][0] = cube[4][2 - i][2];
    for (int i = 0; i < 3; i++) cube[4][2 - i][2] = cube[5][i][0];
    for (int i = 0; i < 3; i++) cube[5][i][0] = cube[2][i][0];
    for (int i = 0; i < 3; i++) cube[2][i][0] = temp_arr[i];

    return *this;
}

RubiksCube &RubiksCube3dArray::lPrime() {
    this->l();
    this->l();
    this->l();

    return *this;
}

RubiksCube &RubiksCube3dArray::l2() {
    this->l();
    this->l();

    return *this;
}

RubiksCube &RubiksCube3dArray::f() {
    this->rotateFace(2);

    char temp_arr[3] = {};
    for (int i = 0; i < 3; i++) temp_arr[i] = cube[0][2][i];
    for (int i = 0; i < 3; i++) cube[0][2][i] = cube[1][2 - i][2];
    for (int i = 0; i < 3; i++) cube[1][2 - i][2] = cube[5][0][2 - i];
    for (int i = 0; i < 3; i++) cube[5][0][2 - i] = cube[3][i][0];
    for (int i = 0; i < 3; i++) cube[3][i][0] = temp_arr[i];

    return *this;
}

RubiksCube &RubiksCube3dArray::fPrime() {
    this->f();
    this->f();
    this->f();

    return *this;
}

RubiksCube &RubiksCube3dArray::f2() {
    this->f();
    this->f();

    return *this;
}

RubiksCube &RubiksCube3dArray::r() {
    this->rotateFace(3);

    char temp_arr[3] = {};
    for (int i = 0; i < 3; i++) temp_arr[i] = cube[0][2 - i][2];
    for (int i = 0; i < 3; i++) cube[0][2 - i][2] = cube[2][2 - i][2];
    for (int i = 0; i < 3; i++) cube[2][2 - i][2] = cube[5][2 - i][2];
    for (int i = 0; i < 3; i++) cube[5][2 - i][2] = cube[4][i][0];
    for (int i = 0; i < 3; i++) cube[4][i][0] = temp_arr[i];

    return *this;
}

RubiksCube &RubiksCube3dArray::rPrime() {
    this->r();
    this->r();
    this->r();

    return *this;
}

RubiksCube &RubiksCube3dArray::r2() {
    this->r();
    this->r();

    return *this;
}

RubiksCube &RubiksCube3dArray::b() {
    this->rotateFace(4);

    char temp_arr[3] = {};
    for (int i = 0; i < 3; i++) temp_arr[i] = cube[0][0][2 - i];
    for (int i = 0; i < 3; i++) cube[0][0][2 - i] = cube[3][2 - i][2];
    for (int i = 0; i < 3; i++) cube[3][2 - i][2] = cube[5][2][i];
    for (int i = 0; i < 3; i++) cube[5][2][i] = cube[1][i][0];
    for (int i = 0; i < 3; i++) cube[1][i][0] = temp_arr[i];

    return *this;
}

RubiksCube &RubiksCube3dArray::bPrime() {
    this->b();
    this->b();
    this->b();

    return *this;
}

RubiksCube &RubiksCube3dArray::b2() {
    this->b();
    this->b();

    return *this;
}

RubiksCube &RubiksCube3dArray::d() {
    this->rotateFace(5);

    char temp_arr[3] = {};
    for (int i = 0; i < 3; i++) temp_arr[i] = cube[2][2][i];
    for (int i = 0; i < 3; i++) cube[2][2][i] = cube[1][2][i];
    for (int i = 0; i < 3; i++) cube[1][2][i] = cube[4][2][i];
    for (int i = 0; i < 3; i++) cube[4][2][i] = cube[3][2][i];
    for (int i = 0; i < 3; i++) cube[3][2][i] = temp_arr[i];

    return *this;
}

RubiksCube &RubiksCube3dArray::dPrime() {
    this->d();
    this->d();
    this->d();

    return *this;
}

RubiksCube &RubiksCube3dArray::d2() {
    this->d();
    this->d();

    return *this;
}

bool RubiksCube3dArray::operator==(const RubiksCube3dArray &r1) const {
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                if (r1.cube[i][j][k] != cube[i][j][k]) return false;
            }
        }
    }
    return true;
}

RubiksCube3dArray &RubiksCube3dArray::operator=(const RubiksCube3dArray &r1) {
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                cube[i][j][k] = r1.cube[i][j][k];
            }
        }
    }
    return *this;
}

size_t Hash3d::operator()(const RubiksCube3dArray &r1) const {
    string str = "";
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                str += r1.cube[i][j][k];
            }
        }
    }
    return hash<string>()(str);
}