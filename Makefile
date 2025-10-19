# Root Makefile for ge-patch-ultra
# Build & clean Vita (vitasdk/CMake) and PSP (pspsdk/Makefile) from repo root via Docker.

# ---- Config ----
VITASDK_IMAGE := vitasdk/vitasdk:latest
PSPDEV_IMAGE  := pspdev/pspdev:latest
PLATFORM_FLAG := --platform linux/amd64

ROOT := $(CURDIR)

# Number of parallel jobs to give to host-side steps (e.g. CMake --build)
JOBS ?= $(shell getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

.PHONY: help all vita psp clean clean-all clean-vita clean-psp docker-pull \
        vita-rebuild psp-rebuild

# ---- Help ----
help:
	@echo
	@echo "Targets:"
	@echo "  make all         -> build both vita_bridge (VPK) and pspemu_plugin (PRX)"
	@echo "  make vita        -> build vita_bridge VPK with vitasdk Docker"
	@echo "  make psp         -> build pspemu_plugin PRX with pspdev Docker"
	@echo "  make clean       -> clean vita & psp build outputs"
	@echo "  make clean-vita  -> clean only vita_bridge build artifacts"
	@echo "  make clean-psp   -> clean only pspemu_plugin artifacts (via Docker 'make clean')"
	@echo "  make docker-pull -> pull/update the Docker images"
	@echo "  make vita-rebuild-> clean-vita then vita"
	@echo "  make psp-rebuild -> clean-psp then psp"
	@echo

# ---- Meta ----
all: vita psp

docker-pull:
	@docker pull $(VITASDK_IMAGE)
	@docker pull $(PSPDEV_IMAGE)

# ---- Vita: build & clean ----
# Mirrors your command, but runnable from repo root.
vita:
	@echo "==> Building vita_bridge (CMake) in Docker..."
	docker run --rm -it $(PLATFORM_FLAG) \
	  -e VITASDK=/usr/local/vitasdk \
	  -v "$(ROOT)/vita_bridge":/work -w /work $(VITASDK_IMAGE) \
	  bash -lc 'cmake -S . -B build && cmake --build build --parallel $$(nproc)'

# Quick rebuild convenience
vita-rebuild: clean-vita vita

# Clean vita CMake build dir and common outputs
clean-vita:
	@echo "==> Cleaning vita_bridge artifacts..."
	@rm -rf "$(ROOT)/vita_bridge/build"
	@rm -f  "$(ROOT)/vita_bridge"/*.self "$(ROOT)/vita_bridge"/*.vpk

# ---- PSP: build & clean ----
# Wraps the pspsdk Makefile from the root, no cd needed.
psp:
	@echo "==> Building pspemu_plugin (PRX) in Docker..."
	docker run --rm -it $(PLATFORM_FLAG) \
	  -v "$(ROOT)":/work -w /work/pspemu_plugin $(PSPDEV_IMAGE) \
	  bash -lc 'export PSPDEV=/usr/local/pspdev; export PATH=$$PSPDEV/bin:$$PATH; \
	            echo "psp-config: $$(which psp-config)"; psp-config --pspsdk-path; \
	            echo "psp-gcc:    $$(which psp-gcc)"; \
	            make -j$$(nproc)'

psp-rebuild: clean-psp psp

# Use the toolchain's own 'make clean' so it knows what to remove
clean-psp:
	@echo "==> Cleaning pspemu_plugin artifacts (via Docker make clean)..."
	docker run --rm -it $(PLATFORM_FLAG) \
	  -v "$(ROOT)":/work -w /work/pspemu_plugin $(PSPDEV_IMAGE) \
	  bash -lc 'export PSPDEV=/usr/local/pspdev; export PATH=$$PSPDEV/bin:$$PATH; \
	            make clean || true'

# ---- Combined clean ----
clean: clean-vita clean-psp
clean-all: clean
