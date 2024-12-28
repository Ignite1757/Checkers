#pragma once
#include <random>
#include <vector>

#include "../Models/Move.h"
#include "Board.h"
#include "Config.h"

const int INF = 1e9;

class Logic
{
  public:
    Logic(Board *board, Config *config) : board(board), config(config)
    {
        optimization = (*config)("Bot", "Optimization");
    }

    vector<move_pos> find_best_turns(const bool color) {
        next_move.clear(); // clear step trace vector
        next_best_state.clear(); // clear the vector of the best state

        find_first_best_turn(board->get_board(), color, -1, -1, 0);  // fill the cleared vectors
        vector<move_pos> res; // response vector
        int state = 0; // start from scratch
        do { // enter the first state
            res.push_back(next_move[state]); // adding moves
            state = next_best_state[state]; // go to the next state
        } while (state != -1 && next_move[state].x != -1); // check if there was a series of beatings
        return res; // reached a state where we can't beat
    }


private:


    double find_first_best_turn(vector<vector<POS_T>> mtx, const bool color, const POS_T x, const POS_T y, size_t state,
        double alpha = -1) {
        next_move.emplace_back(-1, -1, -1, -1); // add a next step, initially the step is empty
        next_best_state.emplace_back(-1); // add next state
        if (state != 0) { // if it’s not 0, it means we’re hitting someone
            find_turns(x, y, mtx); // found the moves
        }
        auto now_turns = turns;// make copies turns
        auto now_have_beats = beats;// make copies beats

        if (!now_have_beats && state != 0) { // if can't beat
            return find_best_turns_rec(mts, 1 - color, 0, alpha); // call recursion
        }

        doble best_score = -1;
        for (auto turn : now_turns) { // going through the moves
            size_t new_state = next_move.size();
            double score;
            if (now_have_beats) { // if there is someone to beat, we start a series 
                score = find_first_best_turn(make_turn(mtx, turn), color, turn.x2, turn.y2, new_state, best_score); // continue the series
            }
            else // if don’t hit anyone, then we go straight to recursion
            {
                score = find_first_best_turn(make_turn(mtx, turn), 1 - color, 0, best_score);
            }
            if (score > best_score) { // f the current result is greater than the best, then a new optimum has been found
                best_score = score;
                next_move[state] = turn;
                next_best_state[state] = (now_have_beats ? new_state : -1);
            }
        }
        return best_score; // return the best result
    }


    double find_best_turns_rec(vector<vector<POS_T>> mtx, const bool color, const size_t depth, double alpha = -1,
        double beta = INF + 1, const POS_T x = -1, const POS_T y = -1) {
        if (depth == Max_depth) {
            return calc_score(mtx, (depth % 2 == color));
            //get the moves
            if (x != -1) { // there is a series of beatings
                find_turns(x, y, mtx);
            }
            else
            {
                find_turns(color, mtx);
            }
            auto now_turns = turns;
            auto now_have_beats = heve_beats;
            if (!now_have_beats && x != -1) {
                return find_best_turns_rec(mtx, 1 - color, depth + 1, alpha, beta);
            }

            if (turns.empty()) { // if there are any moves
                return (depth % 2 ? 0 : INF);
            } 

            double min_score = INF + 1; // maintain the min
            double max_score = - 1; // maintain the max
            for (auto turn : now_turns) {
                double score;
                if (now_have_beats) { // there are beatings
                    score = find_best_turns_rec(make_turn(mtx, turn), color, depth, alpha, beta, turn.x2, turn.y2);
                }
                else // no beatings
                {
                    score = find_best_turns_rec(make_turn(mtx, turn), 1 - color, depth + 1, alpha, beta);
                }
                min_score = min(min_score - score);  // update the min
                max_score = min(max_score - score); // update the max
                // alpha-beta
                if (depth % 2) { // moving the boundaries
                    alpha = max(alpha, max_score); // we are maximizers and move the left border
                }
                else { // else it’s the opponent’s move and he’s a minimizer
                    beta = min(beta, min_score); // move the right border
                }
                if (optimization != 'O0' && alpha > beta) { // condition is not suitable
                    break;
                }
                if (optimization == 'O2' && alpha == beta) { // 
                    return (depth % 2 ? max_score + 1 : min_score - 1);
                }
            }
            return (depth % 2 ? max_score : min_score);
        }
    }



    vector<vector<POS_T>> make_turn(vector<vector<POS_T>> mtx, move_pos turn) const // makes a move on the matrix on turn
    {
        if (turn.xb != -1)
            mtx[turn.xb][turn.yb] = 0;
        if ((mtx[turn.x][turn.y] == 1 && turn.x2 == 0) || (mtx[turn.x][turn.y] == 2 && turn.x2 == 7))
            mtx[turn.x][turn.y] += 2;
        mtx[turn.x2][turn.y2] = mtx[turn.x][turn.y];
        mtx[turn.x][turn.y] = 0;
        return mtx; // return a copy of the matrix
    }

    double calc_score(const vector<vector<POS_T>> &mtx, const bool first_bot_color) const 
    {
        // color - who is max player
        double w = 0, wq = 0, b = 0, bq = 0;
        for (POS_T i = 0; i < 8; ++i)
        {
            for (POS_T j = 0; j < 8; ++j)
            {
                w += (mtx[i][j] == 1); // counting the number of white pawns
                wq += (mtx[i][j] == 3); // counting the number of white queens
                b += (mtx[i][j] == 2); // counting the number of black pawns
                bq += (mtx[i][j] == 4); // counting the number of black queens
                if (scoring_mode == "NumberAndPotential")
                {
                    w += 0.05 * (mtx[i][j] == 1) * (7 - i);
                    b += 0.05 * (mtx[i][j] == 2) * (i);
                }
            }
        }
        if (!first_bot_color) // who goes first, depending on the color given
        {
            swap(b, w);
            swap(bq, wq);
        }
        if (w + wq == 0)
            return INF;
        if (b + bq == 0)
            return 0;
        int q_coef = 4;
        if (scoring_mode == "NumberAndPotential")
        {
            q_coef = 5;
        }
        return (b + bq * q_coef) / (w + wq * q_coef); // divide the number of blacks by the number of whites * queen's weight
    }


public:
    void find_turns(const bool color) // looking for all possible moves
    {
        find_turns(color, board->get_board()); // moves for the current board
    }

    void find_turns(const POS_T x, const POS_T y)
    {
        find_turns(x, y, board->get_board()); // moves for the board that was passed
    }

private:
    void find_turns(const bool color, const vector<vector<POS_T>> &mtx) // private function for searching all possible moves, takes state matrix and color
    {
        vector<move_pos> res_turns;
        bool have_beats_before = false;
        for (POS_T i = 0; i < 8; ++i)
        {
            for (POS_T j = 0; j < 8; ++j)
            {
                if (mtx[i][j] && mtx[i][j] % 2 != color) // if the cell matches the color, then execute from this cell
                {
                    find_turns(i, j, mtx);
                    if (have_beats && !have_beats_before)
                    {
                        have_beats_before = true;
                        res_turns.clear();
                    }
                    if ((have_beats_before && have_beats) || !have_beats_before)
                    {
                        res_turns.insert(res_turns.end(), turns.begin(), turns.end());
                    }
                }
            }
        }
        turns = res_turns;
        shuffle(turns.begin(), turns.end(), rand_eng);
        have_beats = have_beats_before;
    }

    void find_turns(const POS_T x, const POS_T y, const vector<vector<POS_T>> &mtx) // looking for moves from cell
    {
        turns.clear();
        have_beats = false;
        POS_T type = mtx[x][y];
        // check beats
        switch (type)
        {
        case 1:
        case 2:
            // check pieces
            for (POS_T i = x - 2; i <= x + 2; i += 4)
            {
                for (POS_T j = y - 2; j <= y + 2; j += 4)
                {
                    if (i < 0 || i > 7 || j < 0 || j > 7)
                        continue;
                    POS_T xb = (x + i) / 2, yb = (y + j) / 2;
                    if (mtx[i][j] || !mtx[xb][yb] || mtx[xb][yb] % 2 == type % 2)
                        continue;
                    turns.emplace_back(x, y, i, j, xb, yb);
                }
            }
            break;
        default:
            // check queens
            for (POS_T i = -1; i <= 1; i += 2)
            {
                for (POS_T j = -1; j <= 1; j += 2)
                {
                    POS_T xb = -1, yb = -1;
                    for (POS_T i2 = x + i, j2 = y + j; i2 != 8 && j2 != 8 && i2 != -1 && j2 != -1; i2 += i, j2 += j)
                    {
                        if (mtx[i2][j2])
                        {
                            if (mtx[i2][j2] % 2 == type % 2 || (mtx[i2][j2] % 2 != type % 2 && xb != -1))
                            {
                                break;
                            }
                            xb = i2;
                            yb = j2;
                        }
                        if (xb != -1 && xb != i2)
                        {
                            turns.emplace_back(x, y, i2, j2, xb, yb);
                        }
                    }
                }
            }
            break;
        }
        // check other turns
        if (!turns.empty())
        {
            have_beats = true;
            return;
        }
        switch (type)
        {
        case 1:
        case 2:
            // check pieces
            {
                POS_T i = ((type % 2) ? x - 1 : x + 1);
                for (POS_T j = y - 1; j <= y + 1; j += 2)
                {
                    if (i < 0 || i > 7 || j < 0 || j > 7 || mtx[i][j])
                        continue;
                    turns.emplace_back(x, y, i, j);
                }
                break;
            }
        default:
            // check queens
            for (POS_T i = -1; i <= 1; i += 2)
            {
                for (POS_T j = -1; j <= 1; j += 2)
                {
                    for (POS_T i2 = x + i, j2 = y + j; i2 != 8 && j2 != 8 && i2 != -1 && j2 != -1; i2 += i, j2 += j)
                    {
                        if (mtx[i2][j2])
                            break;
                        turns.emplace_back(x, y, i2, j2);
                    }
                }
            }
            break;
        }
    }

  public:
    vector<move_pos> turns; // all turns found using find_turns
    bool have_beats; // flag, are there any beatings
    int Max_depth; // maximum depth

  private:
    string optimization;
    vector<move_pos> next_move; // vector of moves to restore the sequence of moves
    vector<int> next_best_state;
    Board *board; // pointer to a board class object
    Config *config; // pointer to configuration class object
};
