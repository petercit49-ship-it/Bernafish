#!/bin/bash
MOTOR=${1:-./stockfish.exe}
medir() {
  ( printf 'uci\nsetoption name %s Value MG value %s\nsetoption name %s Value EG value %s\nposition fen %s\neval\nquit\n' "$1" "$3" "$1" "$3" "$2" ) \
  | "$MOTOR" | grep "Material, PST" | awk '{print $(NF-1), $NF}'
}
while IFS='|' read -r nombre fen; do
  read -r a_mg a_eg <<< "$(medir "$nombre" "$fen" 100)"
  read -r b_mg b_eg <<< "$(medir "$nombre" "$fen" 200)"
  awk -v n="$nombre" -v a="$a_mg" -v b="$b_mg" -v c="$a_eg" -v d="$b_eg" 'BEGIN{printf "%-7s MG %.2f   EG %.2f\n", n, b-a, d-c}'
done << 'FIN'
Pawn|rnbqkbnr/ppppppp1/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
Knight|rnbqkb1r/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
Bishop|rn1qkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
Rook|1nbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
Queen|rnb1kbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
FIN
