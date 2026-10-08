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

#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <string>

#include "experience.h"
#include "misc.h"
#include "rkiss.h"
#include "ucioption.h"

using std::string;

namespace {

  struct Entry {
    Move move;
    uint32_t wins, draws, losses;
    uint32_t games() const { return wins + draws + losses; }
    double score() const { return (wins + 0.5 * draws) / double(games()); }
  };

  typedef std::map<Key, std::vector<Entry> > Table;

  Table table;
  string loadedPath;
  bool loaded = false;
  std::vector<std::pair<Key, Move> > gameMoves;

  const char Magic[8] = { 'B', 'E', 'X', 'P', '0', '0', '0', '1' };

  string path() { return Options["Experience File"]; }

  void put32(FILE* f, uint32_t v) {
    unsigned char b[4] = { (unsigned char)(v), (unsigned char)(v >> 8),
                           (unsigned char)(v >> 16), (unsigned char)(v >> 24) };
    fwrite(b, 1, 4, f);
  }

  bool get32(FILE* f, uint32_t& v) {
    unsigned char b[4];
    if (fread(b, 1, 4, f) != 4) return false;
    v = b[0] | (b[1] << 8) | (b[2] << 16) | ((uint32_t)b[3] << 24);
    return true;
  }

  void load() {

    table.clear();
    loadedPath = path();
    loaded = true;

    FILE* f = fopen(loadedPath.c_str(), "rb");
    if (!f)
        return; // New file, it will be created at the first learn

    char hdr[8];
    if (fread(hdr, 1, 8, f) != 8 || memcmp(hdr, Magic, 8))
    {
        fclose(f);
        sync_cout << "info string Experience: invalid file ignored: " << loadedPath << sync_endl;
        loadedPath.clear(); // Do not overwrite a file we do not understand
        return;
    }

    while (true)
    {
        uint32_t k1, k2, m, w, d, l;
        if (!get32(f, k1) || !get32(f, k2) || !get32(f, m) || !get32(f, w) || !get32(f, d) || !get32(f, l))
            break;

        Entry e = { Move(m), w, d, l };
        table[(Key(k2) << 32) | k1].push_back(e);
    }
    fclose(f);
  }

  void ensure_loaded() {
    if (!loaded || loadedPath != path())
        load();
  }

  void save() {

    if (loadedPath.empty())
        return;

    string tmp = loadedPath + ".tmp";
    FILE* f = fopen(tmp.c_str(), "wb");
    if (!f)
    {
        sync_cout << "info string Experience: cannot write " << tmp << sync_endl;
        return;
    }

    fwrite(Magic, 1, 8, f);
    for (Table::const_iterator it = table.begin(); it != table.end(); ++it)
        for (size_t i = 0; i < it->second.size(); i++)
        {
            const Entry& e = it->second[i];
            put32(f, uint32_t(it->first));
            put32(f, uint32_t(it->first >> 32));
            put32(f, uint32_t(e.move));
            put32(f, e.wins);
            put32(f, e.draws);
            put32(f, e.losses);
        }
    fclose(f);

    remove(loadedPath.c_str()); // rename() does not overwrite on Windows
    if (rename(tmp.c_str(), loadedPath.c_str()))
        sync_cout << "info string Experience: cannot replace " << loadedPath << sync_endl;
  }

} // namespace


namespace Experience {

bool use_enabled()   { return Options["Experience Use"] > 0 && !path().empty(); }
bool learn_enabled() { return Options["Experience Learning"] && !path().empty(); }

bool probe(const Position& pos, Move& best, std::vector<Move>& avoid) {

  static RKISS rk;
  for (int i = Time::now() % 50; i > 0; i--) // Not deterministic
      rk.rand<unsigned>();

  best = MOVE_NONE;
  avoid.clear();

  ensure_loaded();

  Table::const_iterator it = table.find(pos.key());
  if (it == table.end())
      return false;

  // With probability 'Experience Use' the experience is applied to this move
  if (int(rk.rand<unsigned>() % 100) >= int(Options["Experience Use"]))
      return false;

  const uint32_t minGames = Options["Experience Min Games"];
  double bestScore = -1;

  for (size_t i = 0; i < it->second.size(); i++)
  {
      const Entry& e = it->second[i];

      if (e.games() < minGames)
          continue;

      double s = e.score();

      if (s >= 0.55 && s > bestScore)
          best = e.move, bestScore = s;
      else if (s <= 0.25)
          avoid.push_back(e.move);
  }
  return best != MOVE_NONE || !avoid.empty();
}

void new_game() { gameMoves.clear(); }

void record(Key key, Move move) {
  if (move != MOVE_NONE && move != MOVE_NULL)
      gameMoves.push_back(std::make_pair(key, move));
}

void learn(int result) {

  if (!learn_enabled())
  {
      gameMoves.clear();
      return;
  }

  ensure_loaded();

  for (size_t i = 0; i < gameMoves.size(); i++)
  {
      std::vector<Entry>& v = table[gameMoves[i].first];
      size_t j = 0;
      while (j < v.size() && v[j].move != gameMoves[i].second)
          j++;

      if (j == v.size())
      {
          Entry e = { gameMoves[i].second, 0, 0, 0 };
          v.push_back(e);
      }

      if (result > 0)       v[j].wins++;
      else if (result < 0)  v[j].losses++;
      else                  v[j].draws++;
  }

  sync_cout << "info string Experience: learned " << gameMoves.size() << " moves" << sync_endl;
  gameMoves.clear();
  save();
}

void info() {

  ensure_loaded();

  size_t entries = 0;
  for (Table::const_iterator it = table.begin(); it != table.end(); ++it)
      entries += it->second.size();

  sync_cout << "info string Experience file: " << path()
            << " positions: " << table.size()
            << " moves: " << entries << sync_endl;
}

} // namespace Experience
