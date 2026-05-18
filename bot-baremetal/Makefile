# BOT-BAREMETAL — Makefile Principal
#
# "Build simple. Déploiement propre. Zéro surprise."
#
# Cibles principales :
#   make all          - Build tout (C + Rust + Python check)
#   make core         - Build le Soma C (bot_dna, instinct, territory)
#   make immune       - Build le système immunitaire Rust
#   make test         - Lancer la suite de tests complète
#   make sim          - Lancer le simulateur d'attaques
#   make clean        - Nettoyage complet
#   make status       - Affiche l'état du projet

CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=c11 -O2 -g \
           -Icore -Iinstinct

PYTHON  = python
CARGO   = cargo

# ─── Sources C ──────────────────────────────────────────────────────────────

C_SOURCES = \
    core/bot_dna.c \
    instinct/threat_state.c \
    instinct/instinct_layer.c

C_OBJECTS = $(C_SOURCES:.c=.o)
C_LIB     = build/libbot_core.a

# ─── Cibles ──────────────────────────────────────────────────────────────────

.PHONY: all core immune test sim clean status check-deps

all: check-deps core immune
	@echo ""
	@echo "=== BOT-BAREMETAL BUILD COMPLETE ==="
	@echo "  Core (C)   : build/libbot_core.a"
	@echo "  Immune (Rust): immune/target/release/"
	@echo ""

# ── Core C ───────────────────────────────────────────────────────────────────

core: build/ $(C_LIB)
	@echo "[Core] Bot DNA + InstinctLayer built OK"

build/:
	mkdir -p build

$(C_LIB): $(C_OBJECTS)
	ar rcs $@ $^
	@echo "[Core] Library: $@"

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ── Immune System (Rust) ─────────────────────────────────────────────────────

immune:
	@echo "[Immune] Building SwarmMind + Watchers (Rust)..."
	cd immune && $(CARGO) build --release
	@echo "[Immune] Build complete"

immune-dev:
	@echo "[Immune] Building (dev mode)..."
	cd immune && $(CARGO) build

# ── Tests ────────────────────────────────────────────────────────────────────

test-python:
	@echo "[Test] Running Python pipeline simulation..."
	$(PYTHON) tests/attack_sim/attack_sim.py
	@echo "[Test] Python: OK"

test-rust:
	@echo "[Test] Running Rust unit tests..."
	cd immune && $(CARGO) test
	@echo "[Test] Rust: OK"

test-bridge:
	@echo "[Test] Running OOBridge test..."
	$(PYTHON) oo_bridge/bridge_agent.py
	@echo "[Test] Bridge: OK"

test-bench: build/ $(C_LIB)
	@echo "[Test] Building and running InstinctLayer benchmark..."
	$(CC) $(CFLAGS) tests/instinct_bench/instinct_bench.c $(C_LIB) -o build/instinct_bench
	./build/instinct_bench
	@echo "[Test] Bench: OK"

test: test-python test-rust test-bench
	@echo ""
	@echo "=== ALL TESTS COMPLETE ==="

# ── Simulateur ───────────────────────────────────────────────────────────────

sim:
	@echo "[Sim] Launching attack simulation..."
	$(PYTHON) tests/attack_sim/attack_sim.py

sim-swarm:
	@echo "[Sim] Launching SwarmMind simulation..."
	cd immune && $(CARGO) run --bin swarm_mind

# ── Nettoyage ─────────────────────────────────────────────────────────────────

clean:
	rm -f $(C_OBJECTS)
	rm -rf build/
	cd immune && $(CARGO) clean
	@echo "[Clean] Done"

# ── Statut ────────────────────────────────────────────────────────────────────

status:
	@echo ""
	@echo "=== BOT-BAREMETAL PROJECT STATUS ==="
	@echo ""
	@echo "  Core (C) sources:"
	@for f in $(C_SOURCES); do echo "    $$f"; done
	@echo ""
	@echo "  Immune (Rust) modules:"
	@ls immune/src/*.rs 2>/dev/null | sed 's/^/    /'
	@echo ""
	@echo "  Adaptive (Python):"
	@ls adaptive/*.py 2>/dev/null | sed 's/^/    /'
	@echo ""
	@echo "  OO Bridge:"
	@ls oo_bridge/*.py 2>/dev/null | sed 's/^/    /'
	@echo ""
	@echo "  Tests:"
	@ls tests/attack_sim/*.py 2>/dev/null | sed 's/^/    /'
	@echo ""

# ── Vérification des dépendances ──────────────────────────────────────────────

check-deps:
	@echo "[Check] Verifying build dependencies..."
	@which $(CC)     > /dev/null 2>&1 || (echo "ERROR: gcc not found" && exit 1)
	@which $(CARGO)  > /dev/null 2>&1 || (echo "ERROR: cargo not found" && exit 1)
	@which $(PYTHON) > /dev/null 2>&1 || (echo "ERROR: python not found" && exit 1)
	@echo "[Check] All dependencies found"
