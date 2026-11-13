#!/usr/bin/env bash
# ==========================================================================
#  universal.sh  -  Alexander 9.2  -  binario UNIVERSAL x86-64 (Linux / GCC)
#  Da lanciare DENTRO la cartella "src" del sorgente.
#
#  ARCH=x86-64-universal compila 8 slice
#  (x86-64, sse41-popcnt, avx2, bmi2, avxvnni, avx512, vnni512, avx512icl)
#  e le linka in un unico eseguibile che sceglie a runtime quella giusta.
#
#  Alexander ha la valutazione classica: nessuna rete NNUE da incorporare
#  o da distribuire insieme all'eseguibile.
# ==========================================================================

set -e

fallita() {
    echo ""
    echo "*** BUILD UNIVERSAL FALLITA: $1 ***"
    exit 1
}

SDE_DIR="${SDE_DIR:-/opt/sde}"
NPROC=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

TARGET="build"
RUN_PREFIX=""

if [ -x "$SDE_DIR/sde" ]; then
    SDE_BIN="$SDE_DIR/sde"
elif command -v sde >/dev/null 2>&1; then
    SDE_BIN=$(command -v sde)
fi

# Il PGO esegue il bench di ogni slice: senza Intel SDE le slice AVX-512
# non girano su una CPU che non le supporta, quindi senza SDE niente PGO.
if [ -n "$SDE_BIN" ]; then
    RUN_PREFIX="$SDE_BIN -future --"
    TARGET="profile-build"
    echo "[universal] Intel SDE trovato ($SDE_BIN): build CON PGO (profile-build)"
else
    echo "[universal] Intel SDE non trovato: build SENZA PGO (build)"
fi

make clean

echo "[universal] Avvio compilazione ARCH=x86-64-universal con -j$NPROC..."

# Non passare CXXFLAGS da riga di comando: sostituisce -O3, -m... e -flto.
if [ -n "$RUN_PREFIX" ]; then
    make "$TARGET" ARCH=x86-64-universal COMP=gcc RUN_PREFIX="$RUN_PREFIX" -j"$NPROC"
else
    make "$TARGET" ARCH=x86-64-universal COMP=gcc -j"$NPROC"
fi

[ -f alexander ] || fallita "eseguibile non generato"

strip alexander
mv alexander Alexander9.2-universal

# --- verifica: il bench deve girare con la slice scelta per questa CPU ----
CHECKDIR=$(mktemp -d)
cp Alexander9.2-universal "$CHECKDIR/"
if ! (cd "$CHECKDIR" && ./Alexander9.2-universal bench 16 1 8 >/dev/null 2>&1); then
    rm -rf "$CHECKDIR"
    fallita "il bench dell'eseguibile universale non termina correttamente"
fi
rm -rf "$CHECKDIR"
echo "[universal] OK: bench eseguito"

# --- Verifica: stampa la slice selezionata sulla CPU corrente -------------
echo ""
echo "[universal] Test di esecuzione (compiler info):"
./Alexander9.2-universal compiler

make clean

echo ""
echo "[universal] OK: Alexander9.2-universal"
exit 0
