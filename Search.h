#pragma once

#ifndef SEARCH_H_
#define SEARCH_H_

#include <bits/stdc++.h>

#include "Board.h"
#include "Evaluate.h"

extern std::vector<int> pieceValues;
extern Board board;

pair<Move, int> Search();

#endif
