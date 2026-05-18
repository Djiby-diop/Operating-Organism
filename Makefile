# =============================================================================
# OO — Operating Organism — Root Build Orchestrator
# =============================================================================
# Builds all biological organ modules then llm-baremetal (the Cortex).
# Usage:
#   make            — build all organs + cortex
#   make organs     — build only the organ .o libs
#   make cortex     — build llm-baremetal/llama2.efi
#   make clean      — clean all organs + cortex build dirs

# Biological organ modules (order matters: bus first, then dependents)
ORGANS := \
    united-baremetal      \
    kernel-baremetal      \
    memory-baremetal      \
    network-baremetal     \
    identity-baremetal    \
    sense-baremetal       \
    vocal-baremetal       \
    reflex-baremetal      \
    evolution-baremetal   \
    dream-baremetal       \
    regen-baremetal       \
    swarm-baremetal       \
    shadow-baremetal      \
    bot-baremetal         \
    vital-baremetal       \
    proprioception-baremetal

CORTEX := llm-baremetal

.PHONY: all organs cortex clean $(ORGANS) $(CORTEX)

# ── Default: build everything ─────────────────────────────────────────
all: organs cortex

# ── Build all organ modules ───────────────────────────────────────────
organs: $(ORGANS)

$(ORGANS):
	@echo "  [OO] Building organ: $@"
	@mkdir -p $@/build
	@$(MAKE) -C $@ --no-print-directory 2>&1 | grep -v "^make\[" || true

# ── Build the Cortex (main EFI) ───────────────────────────────────────
cortex: $(CORTEX)

$(CORTEX): organs
	@echo "  [OO] Building cortex: $@"
	@$(MAKE) -C $@ --no-print-directory

# ── Clean all ─────────────────────────────────────────────────────────
clean:
	@for mod in $(ORGANS); do \
		echo "  [OO] Clean $$mod"; \
		$(MAKE) -C $$mod clean --no-print-directory 2>/dev/null || true; \
	done
	@$(MAKE) -C $(CORTEX) clean --no-print-directory 2>/dev/null || true

# ── Status: list built objects per organ ─────────────────────────────
status:
	@echo "=== OO Organ Build Status ==="
	@for mod in $(ORGANS); do \
		n=$$(ls $$mod/build/*.o 2>/dev/null | wc -l); \
		if [ "$$n" -gt 0 ]; then \
			echo "  ✅ $$mod ($$n objects)"; \
		else \
			echo "  ❌ $$mod (no objects)"; \
		fi; \
	done
	@if [ -f $(CORTEX)/llama2.efi ]; then \
		echo "  ✅ $(CORTEX) [llama2.efi]"; \
	else \
		echo "  ❌ $(CORTEX) [no efi]"; \
	fi
