#!/bin/bash
# compilar_bernafish.sh
#   bash compilar_bernafish.sh                 -> compila las dos variantes y las verifica
#   bash compilar_bernafish.sh verificar EXE   -> solo verifica un ejecutable
cd "$(dirname "$0")" || exit 1
JOBS=${JOBS:-4}; LOG=compilacion.log

motor() {   # motor EXE ESPERA linea1 linea2 ...  (envia las lineas UCI, espera y cierra)
  local exe=$1 espera=$2; shift 2
  ( for l in "$@"; do printf '%s\n' "$l"; done; sleep "$espera"; printf 'quit\n' ) | "$exe" 2>&1
}

compilar() {   # compilar ARCH NOMBRE_FINAL
  echo ">>> Compilando $2 (ARCH=$1, enlace estatico)"
  make clean >>"$LOG" 2>&1
  if ! make build ARCH="$1" COMP=mingw EXTRALDFLAGS="-static" -j"$JOBS" >>"$LOG" 2>&1; then
    echo "    Fallo con -static; reintento sin el (necesitara libwinpthread-1.dll)"
    make clean >>"$LOG" 2>&1
    make build ARCH="$1" COMP=mingw -j"$JOBS" >>"$LOG" 2>&1 || { echo "    ERROR de compilacion. Ultimas lineas:"; tail -30 "$LOG"; return 1; }
  fi
  mv -f stockfish.exe "$2" && echo "    OK -> $2 ($(stat -c %s "$2") bytes)"
}

verificar() {   # verificar EXE
  local E=$1 fallos=0 i m n salida
  [ -x "$E" ] || { echo "No existe $E"; return 1; }
  echo "=== Verificando $E ==="
  echo "  DLL externas: $(objdump -p "$E" 2>/dev/null | awk '/DLL Name/{print $3}' | tr '\n' ' ')"

  salida=$(motor "$E" 1 uci isready)
  if echo "$salida" | grep -q '^uciok' && echo "$salida" | grep -q '^readyok'; then
    echo "  [OK]    UCI: uciok y readyok, $(echo "$salida" | grep -c '^option name') opciones"
  else echo "  [FALLO] UCI: no responde uciok/readyok"; fallos=$((fallos+1)); fi

  local FENMATE='r1bqkb1r/pppp1ppp/2n2n2/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR w KQkq - 4 4'
  n=0
  for i in 1 2 3 4 5; do
    m=$(motor "$E" 1 'setoption name Depth Limit value 8' 'setoption name Blunder Rate value 60' "position fen $FENMATE" 'go movetime 300' | awk '/^bestmove/{print $2}')
    [ "$m" = "h5f7" ] && n=$((n+1))
  done
  if [ "$n" = 5 ]; then echo "  [OK]    Mate en una protegido con Blunder Rate 60 (5/5)"
  else echo "  [FALLO] Mate en una: solo $n/5 veces h5f7"; fallos=$((fallos+1)); fi

  n=$(for i in 1 2 3 4 5 6 7 8; do
        motor "$E" 1 'setoption name Depth Limit value 6' 'setoption name Imprecision value 40' 'setoption name Blunder Rate value 30' 'position startpos' 'go movetime 300' | awk '/^bestmove/{print $2}'
      done | sort -u | wc -l)
  if [ "$n" -ge 2 ]; then echo "  [OK]    Humanizacion: $n jugadas distintas en 8 intentos"
  else echo "  [FALLO] Humanizacion: siempre la misma jugada (repite la prueba una vez)"; fallos=$((fallos+1)); fi

  n=0; local lineas=0
  for i in 1 2 3 4 5 6 7 8 9 10; do
    m=$(motor "$E" 1 'setoption name Depth Limit value 4' 'setoption name Imprecision value 40' 'setoption name Blunder Rate value 20' 'position startpos' 'go movetime 200' \
        | awk '/^info depth/{l++; p=0; for(i=1;i<=NF;i++){ if($i=="pv"){p=1;continue} if(p && length($i)>5) c++ }} END{print c+0, l+0}')
    set -- $m; n=$((n+${1:-0})); lineas=$((lineas+${2:-0}))
  done
  if [ "$lineas" = 0 ]; then echo "  [FALLO] PV: el motor no devolvio ninguna linea info"; fallos=$((fallos+1))
  elif [ "$n" = 0 ]; then echo "  [OK]    Formato del PV correcto ($lineas lineas revisadas)"
  else echo "  [FALLO] PV mal formado: $n jugadas con mas de 5 caracteres"; fallos=$((fallos+1)); fi

  local FENN='rnbqkb1r/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1' a b
  a=$(motor "$E" 1 'setoption name Knight Value MG value 100' 'setoption name Knight Value EG value 100' "position fen $FENN" eval | awk '/Total evaluation/{print $3}')
  b=$(motor "$E" 1 'setoption name Knight Value MG value 330' 'setoption name Knight Value EG value 330' "position fen $FENN" eval | awk '/Total evaluation/{print $3}')
  if awk -v a="$a" -v b="$b" 'BEGIN{exit !(b-a>3)}'; then echo "  [OK]    Valores de pieza efectivos (evaluacion $a -> $b)"
  else echo "  [FALLO] Valores de pieza sin efecto (evaluacion $a -> $b)"; fallos=$((fallos+1)); fi

  local N0 N1
  N1=$(timeout 300 "$E" bench 16 1 11 default depth 2>&1 | awk '/Nodes searched/{print $4}')
  echo "  [INFO]  bench (profundidad 11): $N1 nodos"
  if [ -x ./stockfish4_original.exe ]; then
    N0=$(timeout 300 ./stockfish4_original.exe bench 16 1 11 default depth 2>&1 | awk '/Nodes searched/{print $4}')
    if [ -n "$N1" ] && [ "$N0" = "$N1" ]; then echo "  [INFO]  Identico al Stockfish 4 original ($N0 nodos): la configuracion por defecto es neutra"
    else echo "  [INFO]  Distinto del Stockfish 4 original ($N0 vs $N1 nodos): los valores por defecto no son del todo neutros"; fi
  fi
  echo "  Resultado: $fallos fallo(s)"
  return $fallos
}

if [ "$1" = "verificar" ]; then verificar "$2"; exit $?; fi

: > "$LOG"; TOTAL=0
compilar x86-64-modern bernafish_1.0.0_x64_modern.exe || exit 1
verificar ./bernafish_1.0.0_x64_modern.exe; TOTAL=$((TOTAL+$?))
compilar x86-64 bernafish_1.0.0_x64.exe || exit 1
verificar ./bernafish_1.0.0_x64.exe; TOTAL=$((TOTAL+$?))
make clean >>"$LOG" 2>&1
echo; sha256sum bernafish_1.0.0_x64_modern.exe bernafish_1.0.0_x64.exe
echo; [ "$TOTAL" = 0 ] && echo "TODO CORRECTO" || echo "HAY $TOTAL FALLO(S): revisa arriba"
