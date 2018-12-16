#pragma once
#include <iostream>
#include <algorithm>
#include <cmath>
#include "board.h"
#include <numeric>
#include <vector>
#include <cstring>

class state_type {
public:
	enum type : char {
		before  = 'b',
		after   = 'a',
		illegal = 'i'
	};

public:
	state_type() : t(illegal) {}
	state_type(const state_type& st) = default;
	state_type(state_type::type code) : t(code) {}

	friend std::istream& operator >>(std::istream& in, state_type& type) {
		std::string s;
		if (in >> s) type.t = static_cast<state_type::type>((s + " ").front());
		return in;
	}

	friend std::ostream& operator <<(std::ostream& out, const state_type& type) {
		return out << char(type.t);
	}

	bool is_before()  const { return t == before; }
	bool is_after()   const { return t == after; }
	bool is_illegal() const { return t == illegal; }

//private:
	type t;
};

class state_hint {
public:
	state_hint(const board& state) : state(const_cast<board&>(state)) {}

	char type() const { return state.info() ? state.info() + '0' : 'x'; }
	operator board::cell() const { return state.info(); }

public:
	friend std::istream& operator >>(std::istream& in, state_hint& hint) {
		while (in.peek() != '+' && in.good()) in.ignore(1);
		char v; in.ignore(1) >> v;
		hint.state.info(v != 'x' ? v - '0' : 0);
		return in;
	}
	friend std::ostream& operator <<(std::ostream& out, const state_hint& hint) {
		return out << "+" << hint.type();
	}

private:
	board& state;
};

class solver {
public:
	typedef float value_t;

public:
    int lut(board state){
        int index = 0;
        for(int i = 0; i < 6; i++){
            int power = 1, n = i;
            while(n--) power *= 12;
            index += state.operator ()(i) * power;
        }
        return index;
    }

public:
	class answer{
	public:
		answer(value_t min = 0.0/0.0, value_t avg = 0.0/0.0, value_t max = 0.0/0.0) : min(min), avg(avg), max(max){}
	    friend std::ostream& operator <<(std::ostream& out, const answer& ans) {
	    	return !std::isnan(ans.avg) ? (out << ans.min << " " << ans.avg << " " << ans.max) : (out << "-1") << std::endl;
		}
	public:
		value_t min, avg, max;
	};

    answer build(board before){
        board after;
        if(before.type == 'b'){
            int r, v = 0;
            int index = lut(before);
            int hint = before.bag.back();
            float avg = -99999;
            float max = 0;
            float min = 0;

            if(b_v[2985983*(hint-1)+index] == 1){
                min = b_min[2985983*(hint-1)+index];
                avg = b_avg[2985983*(hint-1)+index];
                max = b_max[2985983*(hint-1)+index];
                return {min, avg, max};
            }

            for(int i = 0; i < 4; i++){
                after = before;
                r = after.slide(i);
                //legal
                if(r != -1){
                    v = 1;
                    after.type = 'a';
                    answer current = build(after);
                    if(avg < current.avg){
                        avg = current.avg;
                        max = current.max;
                        min = current.min;
                    }

                }
            }
            //non-leaf
            if(v == 1){
                b_max[2985983*(hint-1)+index] = max;
                b_min[2985983*(hint-1)+index] = min;
                b_avg[2985983*(hint-1)+index] = avg;
                b_v[2985983*(hint-1)+index] = 1;
                return {min, avg, max};
            }
            //leaf
            else{
                avg = 0;
                for(int i = 0; i < 6; i++) {
                    if(before.operator ()(i) > 2) {
                        int power = 1, n = before.operator ()(i) - 2;
                        while(n--) power *= 3;
                        max += power;
                        min += power;
                        avg += power;
                    }
                }
                b_max[2985983*(hint-1)+index] = max;
                b_min[2985983*(hint-1)+index] = min;
                b_avg[2985983*(hint-1)+index] = avg;
                b_v[2985983*(hint-1)+index] = 1;
                return {min, avg, max};
            }
        }
        else if(before.type == 'a'){
            int hint = before.bag.back();
            before.bag.pop_back();
            if(before.bag.empty()) before.bag = {1,2,3};
            int last = before.get_direct();
            int index = lut(before);
            int num_child = 0;
            float max = -99999;
            float min = 99999;
            float avg = 0;

            if(a_v[index + 2985983 * ((hint-1) + 3 * last)] == 1){
                min = a_min[index + 2985983 * ((hint-1) + 3 * last)];
                avg = a_avg[index + 2985983 * ((hint-1) + 3 * last)];
                max = a_max[index + 2985983 * ((hint-1) + 3 * last)];
                return {min, avg, max};
            }

            if(last == 0){
                for(int pos : up){
                    if (before(pos) != 0) continue;
                    after = before;
                    after.place(pos,hint);
                    after.type = 'b';
                    std::vector<int> temp = after.bag;
                    std::vector<int> t = after.bag;
                    for(int new_hint : temp){
                        after.bag = t;
                        auto match = std::find(after.bag.begin(), after.bag.end(), new_hint);
                        after.bag.erase(match);
                        after.bag.push_back(new_hint);
                        num_child++;
                        answer current = build(after);
                        if(max < current.max) max = current.max;
                        if(min > current.min) min = current.min;
                        avg += current.avg;
                    }
                }
            }
            else if(last == 2){
                for(int pos : down){
                    if (before(pos) != 0) continue;
                    after = before;
                    after.place(pos,hint);
                    after.type = 'b';
                    std::vector<int> temp = after.bag;
                    std::vector<int> t = after.bag;
                    for(int new_hint : temp){
                        after.bag = t;
                        auto match = std::find(after.bag.begin(), after.bag.end(), new_hint);
                        after.bag.erase(match);
                        after.bag.push_back(new_hint);
                        num_child++;
                        answer current = build(after);
                        if(max < current.max) max = current.max;
                        if(min > current.min) min = current.min;
                        avg += current.avg;
                    }
                }
            }
            else if(last == 3){
                for(int pos : left){
                    if (before(pos) != 0) continue;
                    after = before;
                    after.place(pos,hint);
                    after.type = 'b';
                    std::vector<int> temp = after.bag;
                    std::vector<int> t = after.bag;
                    for(int new_hint : temp){
                        after.bag = t;
                        auto match = std::find(after.bag.begin(), after.bag.end(), new_hint);
                        after.bag.erase(match);
                        after.bag.push_back(new_hint);
                        num_child++;
                        answer current = build(after);
                        if(max < current.max) max = current.max;
                        if(min > current.min) min = current.min;
                        avg += current.avg;
                    }
                }
            }
            else if(last == 1){

                for(int pos : right){
                    if (before(pos) != 0) continue;
                    after = before;
                    after.place(pos,hint);
                    after.type = 'b';
                    std::vector<int> temp = after.bag;
                    std::vector<int> t = after.bag;
                    for(int new_hint : temp){
                        after.bag = t;
                        auto match = std::find(after.bag.begin(), after.bag.end(), new_hint);
                        after.bag.erase(match);
                        after.bag.push_back(new_hint);
                        num_child++;
                        answer current = build(after);
                        if(max < current.max) max = current.max;
                        if(min > current.min) min = current.min;
                        avg += current.avg;
                    }
                }
            }
            a_max[index + 2985983 * ((hint-1) + 3 * last)] = max;
            a_min[index + 2985983 * ((hint-1) + 3 * last)] = min;
            a_avg[index + 2985983 * ((hint-1) + 3 * last)] = avg/num_child;
            a_v[index + 2985983 * ((hint-1) + 3 * last)] = 1;
            return {min, avg/num_child, max};
        }
    }

public:
	solver(const std::string& args) {
		// TODO: explore the tree and save the result
        board init_board;
        board after;
        answer current;
        float max = -99999;
        float min = 99999;
        float avg = 0;

        for(int i = 1; i <= 3; i++){
            for(int j = 0; j <= 5; j++){
                after = init_board;
                after.place(j, i);
                after.type = 'b';
                auto match = std::find(after.bag.begin(), after.bag.end(), i);
                after.bag.erase(match);
                std::vector<int> temp = after.bag;
                std::vector<int> t = after.bag;
                for(int hint : temp){
                    after.bag = t;
                    auto match = std::find(after.bag.begin(), after.bag.end(), hint);
                    after.bag.erase(match);
                    after.bag.push_back(hint);
                    current = build(after);
                    if(max < current.max) max = current.max;
                    if(min > current.min) min = current.min;
                    avg += current.avg;
                }
            }
            a_max[0 + 2985983 * ((i - 1) + 3 * 0)] = max;
            a_min[0 + 2985983 * ((i - 1) + 3 * 0)] = min;
            a_avg[0 + 2985983 * ((i - 1) + 3 * 0)] = avg/12;
            a_v[0 + 2985983 * ((i - 1) + 3 * 0)] = 1;
        }
	}

	answer solve(const board& state, state_type type = state_type::before) {
		// TODO: find the answer in the lookup table and return it
		//       do NOT recalculate the tree at here
        int index = lut(state);
		// to fetch the hint (if type == state_type::after, hint will be 0)
		board::cell hint = state_hint(state);
		//check for legality

		// for a legal state, return its three values.
//		return { min, avg, max };
        if(type.t == 'b'){
            if(b_v[2985983*(hint-1)+index] == 1){
                float min = b_min[2985983*(hint-1)+index];
                float avg = b_avg[2985983*(hint-1)+index];
                float max = b_max[2985983*(hint-1)+index];
                return { min, avg, max };
            }
        }
        else if(type.t == 'a'){
            for(int i = 0; i < 4; i++){
                if(a_v[index + 2985983 * ((hint-1) + 3 * i)] == 1){
                    float min = a_min[index + 2985983 * ((hint-1) + 3 * i)];
                    float avg = a_avg[index + 2985983 * ((hint-1) + 3 * i)];
                    float max = a_max[index + 2985983 * ((hint-1) + 3 * i)];
                    return { min, avg, max };
                }
            }
        }
		// for an illegal state, simply return {}
		return {};
	}

public:
	const int up[3] = {3, 4, 5};
	const int down[3] = {0, 1, 2};
    const int left[2] = {2, 5};
    const int right[2] = {0, 3};

private:
	// TODO: place your transposition table here
	float *b_max = new float[3*2985983];
	float *b_min = new float[3*2985983];
	float *b_avg = new float[3*2985983];
	int *b_v = new int[3*2985983]();
	float *a_max = new float[3*2985983*4];
	float *a_min = new float[3*2985983*4];
	float *a_avg = new float[3*2985983*4];
	int *a_v = new int[3*2985983*4]();
};
