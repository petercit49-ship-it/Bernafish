/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2013 Marco Costalba, Joona Kiiski, Tord Romstad
  Copyright (C) 2026 Pedro Bernabé Moreno Maura (Bernafish modifications)

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef EXPERIENCE_H_INCLUDED
#define EXPERIENCE_H_INCLUDED

#include <vector>

#include "position.h"
#include "types.h"

/// Bernafish experience file. Stores, for every (position, move) the engine has
/// played in learning mode, how many games were won, drawn and lost afterwards.
///
/// File format (binary, little endian):
///   header : 8 bytes  "BEXP0001"
///   record : key u64, move u32, wins u32, draws u32, losses u32   (24 bytes each)

namespace Experience {

  bool use_enabled();   // Experience Use > 0 and a file name is set
  bool learn_enabled(); // Experience Learning is on and a file name is set

  // Looks the position up. Returns true if there is something to say. 'best'
  // is an experience move to play directly (MOVE_NONE if none), 'avoid' are
  // moves with a bad record that the search should skip.
  bool probe(const Position& pos, Move& best, std::vector<Move>& avoid);

  void new_game();                 // Forget the moves recorded for the current game
  void record(Key key, Move move); // Remember an own move played in the current game
  void learn(int result);          // Game ended: +1 win, 0 draw, -1 loss (engine's view)
  void info();                     // Prints a one line summary (command "expinfo")

}

#endif // #ifndef EXPERIENCE_H_INCLUDED
