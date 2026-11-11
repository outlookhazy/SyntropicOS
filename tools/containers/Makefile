# SyntropicOS Tools Container Makefile
# Run containerized sanitizers, QEMU ARM emulation, protocol fuzzing, coverage, static analysis, doxygen, and 3rd-party integration.

ifeq ($(OS),Windows_NT)
  CONTAINER_ENGINE ?= docker
  VOL_FLAG ?=
  ROOT_DIR_RAW := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/../..)
  ROOT_DIR ?= $(subst \,/,$(ROOT_DIR_RAW))
  JOBS ?= 16
else
  ROOT_DIR_RAW := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/../..)
  ROOT_DIR := $(shell if command -v wslpath >/dev/null 2>&1; then wslpath -m "$(ROOT_DIR_RAW)"; else echo "$(ROOT_DIR_RAW)"; fi)
  ifneq ($(GITHUB_ACTIONS),)
    CONTAINER_ENGINE ?= docker
  else
    CONTAINER_ENGINE ?= $(shell command -v podman 2>/dev/null || command -v docker.exe 2>/dev/null || command -v docker 2>/dev/null || echo docker)
  endif
  VOL_FLAG ?= $(shell echo "$(CONTAINER_ENGINE)" | grep -q "podman" && echo ":z" || true)
  JOBS ?= $(shell nproc 2>/dev/null || echo 4)
endif
export CONTAINER_ENGINE
CACHE_VOL ?= syntropicos-build-cache


.PHONY: help \
	test unit format lint misra stack size complexity san tsan qemu renode fuzz \
	cov static dox pio arduino valgrind analyzer flawfinder integration examples \
	container-build container-volume container-test container-unit container-format \
	container-lint container-misra container-stack container-size \
	container-complexity container-san container-tsan container-qemu \
	container-renode container-fuzz container-cov container-static \
	container-dox container-pio container-arduino container-valgrind container-analyzer \
	container-flawfinder container-integration container-examples

help:
	@echo "SyntropicOS Containerized Verification & Tooling"
	@echo "------------------------------------------------"
	@echo "  make container-build       Build verification container image (Podman/Docker)"
	@echo "  make container-unit        Run cached incremental Unity tests (optional FILTER=...)"
	@echo "  make container-test        Run Unity test suite & LCOV coverage in container"
	@echo "  make container-cov         Generate LCOV line/branch coverage report"
	@echo "  make container-format      Check code formatting via clang-format (FIX=1 to apply)"
	@echo "  make container-lint        Run AST linter (clang-tidy + protothread switch check)"
	@echo "  make container-misra       Run MISRA C:2023 compliance scan"
	@echo "  make container-static      Run Cppcheck & Clang scan-build static analysis"
	@echo "  make container-analyzer    Run GCC -fanalyzer deep interprocedural analysis"
	@echo "  make container-flawfinder  Run CWE security lexical scan"
	@echo "  make container-stack       Verify static stack frame bounds & zero VLAs"
	@echo "  make container-size        Report per-module Flash/RAM object footprint"
	@echo "  make container-complexity  Enforce cyclomatic complexity V(G) <= 15"
	@echo "  make container-san         Run test suite under ASan + UBSan"
	@echo "  make container-tsan        Run test suite under ThreadSanitizer (TSan)"
	@echo "  make container-valgrind    Run test suite under Valgrind Memcheck"
	@echo "  make container-fuzz        Run libFuzzer protocol fuzzing targets"
	@echo "  make container-qemu        Run QEMU ARM Cortex-M4 bare-metal emulation"
	@echo "  make container-renode      Run Renode STM32F4 board hardware emulation"
	@echo "  make container-examples    Cross-compile all examples & lean config profiles"
	@echo "  make container-pio         Cross-compile PlatformIO multi-MCU matrix"
	@echo "  make container-arduino     Compile AVR sketches & run HIL check on Arduino Uno"
	@echo "  make container-integration Run 3rd-party container integration suite"
	@echo "  make container-dox         Verify Doxygen API documentation (0 warnings)"
	@echo "  make clean                 Remove host build/coverage/doxygen artifacts"
	@echo "  make install-hooks         Configure Git .githooks pre-commit hook"
	@echo ""
	@echo "Note: Omit 'container-' prefix when inside the container or using a native toolchain."

test:
	@make -C $(ROOT_DIR) -f tests/Makefile.unity test

unit:
	@SYN_TEST_FILTER="$(FILTER)" $(MAKE) -j$(JOBS) -C $(ROOT_DIR) -f tests/Makefile.unity unit BUILD_DIR=build/unit

format:
	@bash $(ROOT_DIR)/tools/lint/run_format.sh

lint:
	@bash $(ROOT_DIR)/tools/lint/run_tidy.sh

misra:
	@bash $(ROOT_DIR)/tools/lint/run_misra.sh

stack:
	@bash $(ROOT_DIR)/tools/stack/run_stack_analysis.sh

size:
	@bash $(ROOT_DIR)/tools/size/run_size_analysis.sh

complexity:
	@bash $(ROOT_DIR)/tools/complexity/run_complexity_analysis.sh

san:
	@bash $(ROOT_DIR)/tools/containers/run_sanitizers.sh

tsan:
	@bash $(ROOT_DIR)/tools/tsan/run_tsan.sh

qemu:
	@bash $(ROOT_DIR)/tests/emulation/run_qemu_arm.sh

renode:
	@bash $(ROOT_DIR)/tests/emulation/renode/run_renode.sh

fuzz:
	@bash $(ROOT_DIR)/tests/fuzz/run_fuzz.sh

cov:
	@bash $(ROOT_DIR)/tools/coverage/run_coverage.sh

static:
	@bash $(ROOT_DIR)/tools/static_analysis/run_static_analysis.sh

dox:
	@bash $(ROOT_DIR)/tools/docs/run_doxygen.sh

pio:
	@bash $(ROOT_DIR)/tools/platformio/run_platformio.sh

arduino:
	@SKETCH="$(SKETCH)" UPLOAD="$(UPLOAD)" ARDUINO_PORT="$(ARDUINO_PORT)" FQBN="$(FQBN)" bash $(ROOT_DIR)/tools/arduino/run_arduino.sh

integration:
	@bash $(ROOT_DIR)/tests/integration/run_integration.sh

ifeq ($(OS),Windows_NT)
  CONTAINER_VOLUME_CMD := @"$(CONTAINER_ENGINE)" volume inspect $(CACHE_VOL) >NUL 2>&1 || "$(CONTAINER_ENGINE)" volume create $(CACHE_VOL) >NUL 2>&1 || (exit 0)
  BUILD_FLAGS :=
  BASH_CMD ?= "C:/Program Files/Git/bin/bash.exe"
else
  CONTAINER_VOLUME_CMD := @"$(CONTAINER_ENGINE)" volume inspect $(CACHE_VOL) >/dev/null 2>&1 || "$(CONTAINER_ENGINE)" volume create $(CACHE_VOL) >/dev/null 2>&1 || true
  BUILD_FLAGS := $(shell "$(CONTAINER_ENGINE)" build --help 2>&1 | grep -q -- "--format" && echo "--format docker" || true)
  BASH_CMD ?= bash
endif

container-volume:
	$(CONTAINER_VOLUME_CMD)

CONTAINER_RUN := $(BASH_CMD) $(ROOT_DIR)/tools/containers/run_container.sh

container-build:
	@"$(CONTAINER_ENGINE)" build $(BUILD_FLAGS) -t syntropicos-test -f $(ROOT_DIR)/tools/containers/Containerfile $(ROOT_DIR)/tools/containers

container-format: container-build
	@$(CONTAINER_RUN) -v $(ROOT_DIR):/workspace$(VOL_FLAG) syntropicos-test make -C tools/containers format FIX=$(FIX) JOBS=$(JOBS)

container-lint: container-build
	@$(CONTAINER_RUN) -v $(ROOT_DIR):/workspace$(VOL_FLAG) syntropicos-test make -C tools/containers lint JOBS=$(JOBS)

container-misra: container-build
	@$(CONTAINER_RUN) -v $(ROOT_DIR):/workspace$(VOL_FLAG) syntropicos-test make -C tools/containers misra JOBS=$(JOBS)

container-stack: container-build container-volume
	@$(CONTAINER_RUN) -v $(CACHE_VOL):/workspace/build -v $(ROOT_DIR):/workspace$(VOL_FLAG) syntropicos-test make -C tools/containers stack JOBS=$(JOBS)

container-size: container-build container-volume
	@$(CONTAINER_RUN) -v $(CACHE_VOL):/workspace/build -v $(ROOT_DIR):/workspace$(VOL_FLAG) syntropicos-test make -C tools/containers size JOBS=$(JOBS)

container-complexity: container-build
	@$(CONTAINER_RUN) -v $(ROOT_DIR):/workspace$(VOL_FLAG) syntropicos-test make -C tools/containers complexity JOBS=$(JOBS)

container-san: container-build container-volume
	@$(CONTAINER_RUN) -v $(CACHE_VOL):/workspace/build -v $(ROOT_DIR):/workspace$(VOL_FLAG) syntropicos-test make -C tools/containers san JOBS=$(JOBS)

container-tsan: container-build container-volume
	@$(CONTAINER_RUN) -v $(CACHE_VOL):/workspace/build -v $(ROOT_DIR):/workspace$(VOL_FLAG) syntropicos-test make -C tools/containers tsan JOBS=$(JOBS)

container-qemu: container-build container-volume
	@$(CONTAINER_RUN) -v $(CACHE_VOL):/workspace/build -v $(ROOT_DIR):/workspace$(VOL_FLAG) syntropicos-test make -C tools/containers qemu JOBS=$(JOBS)

container-renode: container-build container-volume
	@$(CONTAINER_RUN) -v $(CACHE_VOL):/workspace/build -v $(ROOT_DIR):/workspace$(VOL_FLAG) syntropicos-test make -C tools/containers renode JOBS=$(JOBS)

container-fuzz: container-build container-volume
	@$(CONTAINER_RUN) -v $(CACHE_VOL):/workspace/build -v $(ROOT_DIR):/workspace$(VOL_FLAG) syntropicos-test make -C tools/containers fuzz JOBS=$(JOBS)

container-test: container-cov

container-unit: container-build container-volume
	@$(CONTAINER_RUN) -v $(CACHE_VOL):/workspace/build -v $(ROOT_DIR):/workspace$(VOL_FLAG) syntropicos-test make -C tools/containers unit JOBS=$(JOBS) FILTER="$(FILTER)"

container-cov: container-build container-volume
	@$(CONTAINER_RUN) -v $(CACHE_VOL):/workspace/build -v $(ROOT_DIR):/workspace$(VOL_FLAG) syntropicos-test make -C tools/containers cov JOBS=$(JOBS)

container-static: container-build container-volume
	@$(CONTAINER_RUN) -v $(CACHE_VOL):/workspace/build -v $(ROOT_DIR):/workspace$(VOL_FLAG) syntropicos-test make -C tools/containers static JOBS=$(JOBS)

container-dox: container-build
	@$(CONTAINER_RUN) -v $(ROOT_DIR):/workspace$(VOL_FLAG) syntropicos-test make -C tools/containers dox JOBS=$(JOBS)

ifeq ($(strip $(ARDUINO_PORT)),)
  override ARDUINO_PORT := /dev/ttyACM0
endif
ARDUINO_DEV_FLAGS := $(shell if [ -c "$(ARDUINO_PORT)" ]; then \
	if echo "$(CONTAINER_ENGINE)" | grep -q "podman"; then \
		echo "--device=$(ARDUINO_PORT) --group-add keep-groups --security-opt label=disable"; \
	else \
		echo "--device=$(ARDUINO_PORT) --security-opt label=disable"; \
	fi; \
fi)

container-pio: container-build container-volume
	@$(CONTAINER_RUN) -v $(CACHE_VOL):/workspace/build -v $(ROOT_DIR):/workspace$(VOL_FLAG) syntropicos-test make -C tools/containers pio JOBS=$(JOBS)

container-arduino: container-build container-volume
	@$(CONTAINER_RUN) $(ARDUINO_DEV_FLAGS) -v $(CACHE_VOL):/workspace/build -v $(ROOT_DIR):/workspace$(VOL_FLAG) syntropicos-test make -C tools/containers arduino JOBS=$(JOBS) SKETCH="$(SKETCH)" UPLOAD="$(UPLOAD)" ARDUINO_PORT="$(ARDUINO_PORT)" FQBN="$(FQBN)"

valgrind:
	@bash $(ROOT_DIR)/tools/valgrind/run_valgrind.sh

container-valgrind: container-build
	@$(CONTAINER_RUN) -v $(ROOT_DIR):/workspace$(VOL_FLAG) syntropicos-test make -C tools/containers valgrind

analyzer:
	@bash $(ROOT_DIR)/tools/analyzer/run_analyzer.sh

container-analyzer: container-build
	@$(CONTAINER_RUN) -v $(ROOT_DIR):/workspace$(VOL_FLAG) syntropicos-test make -C tools/containers analyzer

flawfinder:
	@python3 $(ROOT_DIR)/tools/flawfinder/run_flawfinder.py

container-flawfinder: container-build
	@$(CONTAINER_RUN) -v $(ROOT_DIR):/workspace$(VOL_FLAG) syntropicos-test make -C tools/containers flawfinder

ifeq ($(OS),Windows_NT)
  COMPOSE_CMD ?= docker compose
else
  COMPOSE_CMD ?= $(shell if command -v podman-compose >/dev/null 2>&1; then echo "podman-compose"; else echo "docker compose"; fi)
endif

container-integration: container-build
	@$(COMPOSE_CMD) -f $(ROOT_DIR)/tests/integration/docker-compose.yml down -v -t 1 --remove-orphans >/dev/null 2>&1 || true
	@$(COMPOSE_CMD) -f $(ROOT_DIR)/tests/integration/docker-compose.yml up -d --build mosquitto sntp http ws dns can wireguard modbus ethercat-soem ethercat-soes uds isotp doip
	@$(CONTAINER_RUN) --net=host -v $(ROOT_DIR):/workspace$(VOL_FLAG) syntropicos-test bash /workspace/tests/integration/run_integration.sh; \
	EXIT_CODE=$$?; \
	$(COMPOSE_CMD) -f $(ROOT_DIR)/tests/integration/docker-compose.yml down -v -t 1 --remove-orphans || true; \
	exit $$EXIT_CODE

examples:
	@bash $(ROOT_DIR)/tools/examples/run_examples_check.sh

container-examples: container-build container-volume
	@$(CONTAINER_RUN) -v $(CACHE_VOL):/workspace/build -v $(ROOT_DIR):/workspace$(VOL_FLAG) syntropicos-test make -C tools/containers examples JOBS=$(JOBS)


