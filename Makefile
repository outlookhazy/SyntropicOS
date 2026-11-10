# SyntropicOS Top-Level Convenience Makefile
# Forwards targets to tools/containers/Makefile

.PHONY: help clean install-hooks \
	test unit format lint misra stack size complexity san tsan qemu renode fuzz \
	cov static dox pio valgrind analyzer flawfinder integration examples \
	container-build container-test container-unit container-format container-lint \
	container-misra container-stack container-size container-complexity \
	container-san container-tsan container-qemu container-renode \
	container-fuzz container-cov container-static container-dox \
	container-pio container-valgrind container-analyzer \
	container-flawfinder container-integration container-examples

install-hooks:
	@git config core.hooksPath .githooks
	@chmod +x .githooks/pre-commit 2>/dev/null || true
	@echo "Git pre-commit hook installed successfully! (.githooks)"


JOBS ?= $(shell nproc 2>/dev/null || echo 4)


clean:
	@rm -rf build/ doxygen_output/ coverage_html/ coverage.info coverage_src.info doxygen_warnings.txt *.gcov
	@find . \( -name "*.gcda" -o -name "*.gcno" -o -name "*.o" -o -name "*.su" -o -name "*.gcov" \) -delete

help:
	@$(MAKE) -C tools/containers help

test:
	@$(MAKE) -C tools/containers test

unit:
	@$(MAKE) -C tools/containers unit FILTER="$(FILTER)"

format:
	@$(MAKE) -C tools/containers format

lint:
	@$(MAKE) -C tools/containers lint

misra:
	@$(MAKE) -C tools/containers misra

stack:
	@$(MAKE) -C tools/containers stack

size:
	@$(MAKE) -C tools/containers size

complexity:
	@$(MAKE) -C tools/containers complexity

san:
	@$(MAKE) -C tools/containers san

tsan:
	@$(MAKE) -C tools/containers tsan

qemu:
	@$(MAKE) -C tools/containers qemu

renode:
	@$(MAKE) -C tools/containers renode

fuzz:
	@$(MAKE) -C tools/containers fuzz

cov:
	@$(MAKE) -C tools/containers cov

static:
	@$(MAKE) -C tools/containers static

dox:
	@$(MAKE) -C tools/containers dox

pio:
	@$(MAKE) -C tools/containers pio

valgrind:
	@$(MAKE) -C tools/containers valgrind

container-valgrind:
	@$(MAKE) -C tools/containers container-valgrind

analyzer:
	@$(MAKE) -C tools/containers analyzer

container-analyzer:
	@$(MAKE) -C tools/containers container-analyzer

flawfinder:
	@$(MAKE) -C tools/containers flawfinder

container-flawfinder:
	@$(MAKE) -C tools/containers container-flawfinder

integration:
	@$(MAKE) -C tools/containers integration

container-build:
	@$(MAKE) -C tools/containers container-build

container-test:
	@$(MAKE) -C tools/containers container-test

container-unit:
	@$(MAKE) -C tools/containers container-unit FILTER="$(FILTER)"

container-format:
	@$(MAKE) -C tools/containers container-format

container-lint:
	@$(MAKE) -C tools/containers container-lint

container-misra:
	@$(MAKE) -C tools/containers container-misra

container-stack:
	@$(MAKE) -C tools/containers container-stack

container-size:
	@$(MAKE) -C tools/containers container-size

container-complexity:
	@$(MAKE) -C tools/containers container-complexity

container-san:
	@$(MAKE) -C tools/containers container-san

container-tsan:
	@$(MAKE) -C tools/containers container-tsan

container-qemu:
	@$(MAKE) -C tools/containers container-qemu

container-renode:
	@$(MAKE) -C tools/containers container-renode

container-fuzz:
	@$(MAKE) -C tools/containers container-fuzz

container-cov:
	@$(MAKE) -C tools/containers container-cov

container-static:
	@$(MAKE) -C tools/containers container-static

container-dox:
	@$(MAKE) -C tools/containers container-dox

container-pio:
	@$(MAKE) -C tools/containers container-pio

container-integration:
	@$(MAKE) -C tools/containers container-integration

examples:
	@$(MAKE) -C tools/containers examples

container-examples:
	@$(MAKE) -C tools/containers container-examples
