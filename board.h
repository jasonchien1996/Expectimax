#pragma once
#include <array>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
/**
 * array-based board for Threes
 *
 * index (1-d form):
 *  (0)  (1)  (2)
 *  (3)  (4)  (5)
 *
 */
class board {
public:
	typedef uint32_t cell;
	typedef std::array<cell, 3> row;
	typedef std::array<row, 2> grid;
	typedef uint64_t data;
	typedef int reward;

public:
	board() : tile(), attr(0), type('a'), direct(-1), bag({1, 2, 3}) {}
	board(const grid& b, data v = 0) : tile(b), attr(v) {}
	board(const board& b) = default;
	board& operator =(const board& b) = default;

	operator grid&() { return tile; }
	operator const grid&() const { return tile; }
	row& operator [](unsigned i) { return tile[i]; }
	const row& operator [](unsigned i) const { return tile[i]; }
	cell& operator ()(unsigned i) { return tile[i / 3][i % 3]; }
	const cell& operator ()(unsigned i) const { return tile[i / 3][i % 3]; }

	data info() const { return attr; }
	data info(data dat) { data old = attr; attr = dat; return old; }

public:
	bool operator ==(const board& b) const { return tile == b.tile; }
	bool operator < (const board& b) const { return tile <  b.tile; }
	bool operator !=(const board& b) const { return !(*this == b); }
	bool operator > (const board& b) const { return b < *this; }
	bool operator <=(const board& b) const { return !(b < *this); }
	bool operator >=(const board& b) const { return !(*this < b); }

public:
    int get_direct() const{ return direct; }

	/**
	 * place a tile (index value) to the specific position (1-d form index)
	 * return 0 if the action is valid, or -1 if not
	 */
	reward place(unsigned pos, cell tile) {
		if (pos >= 6) return -1;
		if (tile != 1 && tile != 2 && tile!= 3) return -1;
		operator()(pos) = tile;
		return 0;
	}

	/**
	 * apply an action to the board
	 * return the reward of the action, or -1 if the action is illegal
	 */
	reward slide(unsigned opcode) {
		switch (opcode & 0b11) {
		case 0: return slide_up();
		case 1: return slide_right();
		case 2: return slide_down();
		case 3: return slide_left();
		default: return -1;
		}
	}

	reward slide_left()	{
		board prev = *this;
		reward score = 0;
		for (int r = 0; r < 2; r++) {
			auto& row = tile[r];
			int hold = 0, blank = 0;
			for (int c = 0; c < 3; c++) {
				int tile = row[c];
				if (tile == 0){blank = 1;}
                else {
                    if (hold == tile && hold > 2) {
                        row[c-1] = ++tile;
                        blank = 1;
                    }
					else if (abs((hold - tile)) == 1 && (hold + tile) == 3) {
                        row[c-1] = 3;
                        blank = 1;
                    }
                    else
                        hold = tile;
                }
                if(blank == 1) {
                    for(int i = c; i < 2; i++)
                        row[i] = row[i+1];
                    row[2] = 0;
                    break;
                }
			}
		}
		direct = 3;
		if(*this != prev) {
		    int a = 0, b = 0;
            for(int i = 0; i < 6; i++) {
                if(operator ()(i) > 2) {
                    int power = 1, n = operator ()(i) - 2;
                    while(n--) power *= 3;
                    a += power;
                }
                if(prev.operator ()(i) > 2) {
                    int power = 1, n = prev.operator ()(i) - 2;
                    while(n--) power *= 3;
                    b += power;
                }
            }
            score = a - b;
            return score;
		}
		return -1;
	}
	reward slide_right() {
		reflect_horizontal();
		reward score = slide_left();
		reflect_horizontal();
		direct = 1;
		return score;
	}
	reward slide_up() {
		board prev = *this;
		reward score = 0;
		for(int c = 0; c <= 2; c++) {
			int hold = 0, blank = 0;
			for(int r = 0; r <= 1 ; r++) {
				int now = tile[r][c];
				if (now == 0){blank = 1;}
                else {
                    if (hold == now && hold > 2) {
                        tile[r-1][c] = ++now;
                        blank = 1;
                    }
					else if (abs((hold - now)) == 1 && (hold + now) == 3) {
                        tile[r-1][c] = 3;
                        blank = 1;
                    }
                    else
                        hold = now;
                }
                if(blank == 1) {
                    for(int i = r; i < 1; i++)
                        tile[i][c] = tile[i+1][c];
                     tile[1][c] = 0;
                    break;
                }
			}
		}
		direct = 0;
 		if(*this != prev) {
		    int a = 0, b = 0;
            for(int i = 0; i < 6; i++) {
                if(operator ()(i) > 2) {
                    int power = 1, n = operator ()(i) - 2;
                    while(n--) power *= 3;
                    a += power;
                }
                if(prev.operator ()(i) > 2) {
                    int power = 1, n = prev.operator ()(i) - 2;
                    while(n--) power *= 3;
                    b += power;
                }
            }
            score = a - b;
            return score;
 		}
		return -1;
	}
	reward slide_down() {
		reflect_vertical();
		reward score = slide_up();
		reflect_vertical();
		direct = 2;
		return score;
	}

	/*void transpose() {
		for (int r = 0; r < 4; r++) {
			for (int c = r + 1; c < 4; c++) {
				std::swap(tile[r][c], tile[c][r]);
			}
		}
	}*/

	void reflect_horizontal() {
		for (int r = 0; r < 2; r++) {
			std::swap(tile[r][0], tile[r][2]);
			//std::swap(tile[r][1], tile[r][2]);
		}
	}

	void reflect_vertical() {
		for (int c = 0; c < 3; c++) {
			std::swap(tile[0][c], tile[1][c]);
			//std::swap(tile[1][c], tile[2][c]);
		}
	}

	/**
	 * rotate the board clockwise by given times
	 */
	/*void rotate(int r = 1) {
		switch (((r % 4) + 4) % 4) {
		default:
		case 0: break;
		case 1: rotate_right(); break;
		case 2: reverse(); break;
		case 3: rotate_left(); break;
		}
	}

	void rotate_right() { transpose(); reflect_horizontal(); } // clockwise
	void rotate_left() { transpose(); reflect_vertical(); } // counterclockwise*/

public:
	friend std::ostream& operator <<(std::ostream& out, const board& b) {
		for (int i = 0; i < 6; i++) {
            int k = (b(i) > 3) ? (1 << (b(i)-3) & -2u)*3 : b(i);
			out << std::setw(std::min(i, 1)) << "" << k;
		}
		return out;
	}
	friend std::istream& operator >>(std::istream& in, board& b) {
		for (int i = 0; i < 6; i++) {
			while (!std::isdigit(in.peek()) && in.good()) in.ignore(1);
			in >> b(i);
			if(b(i) > 3) b(i) = std::log2(b(i)/3)+3;
		}
		return in;
	}

public:
    char type;
	std::vector<int> bag;

private:
    int direct;
	grid tile;
	data attr;
};
