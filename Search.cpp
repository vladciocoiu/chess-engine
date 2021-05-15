#include <bits/stdc++.h>

#include "MoveGenerator.cpp"
#include "Evaluate.cpp"

using namespace std;

// considers captures first, sorting them by the difference between the captured and capturing piece
int moveScore(Move m) {
    int score = 0;

    if(m.capture) score += 1000000;

    int capturedPieceVal = pieceValues[m.capture ^ (Black & White)];
    int capturingPieceVal = pieceValues[board.squares[m.from] ^ (Black & White)];

    score += (capturedPieceVal - capturingPieceVal);

    return score;
}

// function that compares 2 moves in order to sort them
bool compareMoves(Move a, Move b) {
    return (moveScore(a) > moveScore(b));
}


int nrpos = 0;
int Search(int depth, int alpha, int beta) {
    if(depth == 0) {
        nrpos++;
        return Evaluate();
    }

    vector<Move> moves = board.GenerateLegalMoves();

    // game is over
    if(moves.size() == 0) {
        if(board.isInCheck())
            return (board.turn == White ? -1000000 : 1000000);
        return 0;
    }

    // sorting the moves by score in order to prune more branches by considering the potentially better moves first;
    sort(moves.begin(), moves.end(), compareMoves);

    if(board.turn == White) {
        int bestEval = -1000000;
        for(Move m: moves) {
            int ep = board.ep;
            bool castleRights[4] = {board.castleWK, board.castleWQ,
                                board.castleBK, board.castleBQ};

            board.makeMove(m);
            bestEval = max(bestEval, Search(depth-1, alpha, beta));
            board.unmakeMove(m, ep, castleRights);

            alpha = max(alpha, bestEval);
            if(alpha >= beta) break;
        }

        return bestEval;
    } else {
        int bestEval = 1000000;
        for(Move m: moves) {
            int ep = board.ep;
            bool castleRights[4] = {board.castleWK, board.castleWQ,
                                board.castleBK, board.castleBQ};

            board.makeMove(m);
            bestEval = min(bestEval, Search(depth-1, alpha, beta));
            board.unmakeMove(m, ep, castleRights);

            beta = min(beta, bestEval);
            if(alpha >= beta) break;
        }

        return bestEval;
    }
}

int main() {
    Init();
    string pos = "r1bq1rk1/1p2ppbp/p1np1np1/8/2BNP3/2N1B3/PPP2PPP/R2Q1RK1 w - - 0 1";
    board.LoadFenPos(pos);
    cout << Search(6, -1000000, 1000000) << '\n';
    cout << "Positions evaluated: " << nrpos << '\n';
}
