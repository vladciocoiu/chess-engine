#include <iostream>
#include <vector>
#include <unordered_map>

#include "Board.h"
#include "TranspositionTable.h"
#include "Search.h"
#include "MagicBitboards.h"
#include "MoveUtils.h"
#include "BoardUtils.h"
#include "Enums.h"

using namespace std;

typedef unsigned long long U64;

U64 TranspositionTable::TranspositionTable::pieceZobristNumbers[7][2][64];
U64 TranspositionTable::TranspositionTable::castleZobristNumbers[16];
U64 TranspositionTable::TranspositionTable::epZobristNumbers[8];
U64 TranspositionTable::TranspositionTable::blackTurnZobristNumber;

const int TranspositionTable::VAL_UNKNOWN = -1e9;
const int TranspositionTable::HASH_F_EXACT = 0;
const int TranspositionTable::HASH_F_ALPHA = 1;
const int TranspositionTable::HASH_F_BETA = 2;

struct TranspositionTable::hashElement {
    U64 key;
    short depth;
    int flags;
    int value;
    int best;
};

TranspositionTable::TranspositionTable(): SIZE(1 << 25), hashTable(new hashElement[SIZE]) {
    clear();
}


void TranspositionTable::generateZobristHashNumbers() {
    for(int pc = 0; pc < 7; pc++) {
        for(int c = 0; c < 2; c++) {
            for(int sq = 0; sq < 64; sq++) {
                TranspositionTable::pieceZobristNumbers[pc][c][sq] = randomULL();
            }
        }
    }
    for(int castle = 0; castle < 16; castle++) {
        TranspositionTable::castleZobristNumbers[castle] = randomULL();
    }
    for(int col = 0; col < 8; col++) {
        TranspositionTable::epZobristNumbers[col] = randomULL();
    }
    TranspositionTable::blackTurnZobristNumber = randomULL();
}

// get the best move from the tt
int TranspositionTable::retrieveBestMove() {
    int index = (board.hashKey & (SIZE-1));
    hashElement *h = &hashTable[index];

    if(h->key != board.hashKey) return MoveUtils::NO_MOVE;

    return h->best;
}

// check if the stored hash element corresponds to the current position and if it was searched at a good enough depth
int TranspositionTable::probeHash(short depth, int alpha, int beta) {
    int index = (board.hashKey & (SIZE-1));
    hashElement *h = &hashTable[index];

    if(h->key == board.hashKey) {
        if(h->depth >= depth) {
            if((h->flags == HASH_F_EXACT || h->flags == HASH_F_ALPHA) && h->value <= alpha) return alpha;
            if((h->flags == HASH_F_EXACT || h->flags == HASH_F_BETA) && h->value >= beta) return beta;
            if(h->flags == HASH_F_EXACT && h->value > alpha && h->value < beta) return h->value;
        }
    }
    return VAL_UNKNOWN;
}

// replace hashed element if replacement conditions are met
void TranspositionTable::recordHash(short depth, int val, int hashF, int best) {
    if(timeOver) return;

    int index = (board.hashKey & (SIZE-1));
    hashElement *h = &hashTable[index];

    bool skip = false;
    if((h->key == board.hashKey) && (h->depth > depth)) skip = true; 
    if(hashF != HASH_F_EXACT && h->flags == HASH_F_EXACT) skip = true; 
    if(hashF == HASH_F_EXACT && h->flags != HASH_F_EXACT) skip = false;
    if(skip) return;

    h->key = board.hashKey;
    h->best = best;
    h->value = val;
    h->flags = hashF;
    h->depth = depth;
}

TranspositionTable transpositionTable;


const int PAWN_TABLE_SIZE = (1 << 20);

struct pawnHashElement {
    U64 whitePawns, blackPawns;
    int eval;
};

pawnHashElement pawnHashTable[PAWN_TABLE_SIZE];

// get hashed value from map
int retrievePawnEval() {
    U64 key = board.pawnsBB;
    U64 whitePawns = (board.pawnsBB & board.whitePiecesBB);
    U64 blackPawns = (board.pawnsBB & board.blackPiecesBB);
    
    int idx = (key & (PAWN_TABLE_SIZE-1));

    if(pawnHashTable[idx].whitePawns == whitePawns && pawnHashTable[idx].blackPawns == blackPawns) {
        return pawnHashTable[idx].eval;
    }

    return TranspositionTable::VAL_UNKNOWN;
}

void recordPawnEval(int eval) {
    if(timeOver) return;

    U64 key = board.pawnsBB;
    U64 whitePawns = (board.pawnsBB & board.whitePiecesBB);
    U64 blackPawns = (board.pawnsBB & board.blackPiecesBB);

    int idx = (key & (PAWN_TABLE_SIZE-1));

    // always replace
    if(pawnHashTable[idx].whitePawns != whitePawns || pawnHashTable[idx].blackPawns != blackPawns) {
        pawnHashTable[idx] = {whitePawns, blackPawns, eval};
    }
}

void TranspositionTable::clear() {
    hashElement newElement = {0, 0, 0, 0, MoveUtils::NO_MOVE};
    pawnHashElement newPawnElement = {0, 0, 0};

    for(int i = 0; i < SIZE; i++) {
        hashTable[i] = newElement;
    }
    for(int i = 0; i < PAWN_TABLE_SIZE; i++) {
        pawnHashTable[i] = newPawnElement;
    }
}