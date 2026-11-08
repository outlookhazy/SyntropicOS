# SyntropicOS Top-Level Convenience Makefile
# Forwards targets to tools/containers/Makefile

.PHONY: help clean test format lint misra stack size complexity san tsan qemu renode fuzz cov static dox integration examples install-hooks container-build container-test container-format container-lint container-misra container-stack container-size container-complexity container-san container-tsan container-qemu container-renode container-fuzz container-cov container-static container-dox container-integration container-examples

install-hooks:
	@git config core.hooksPath .githooks
	@chmod +x .githooks/pre-commit 2>/dev/null || true
	@echo "Git pre-commit hook installed successfully! (.githooks)"


JOBS ?= $(shell nproc 2>/dev/null || echo 4)


clean:
	@rm -rf build/ doxygen_output/ coverage_html/ coverage.info coverage_src.info doxygen_warnings.txt *.gcov
	@find . \( -name "*.gcda" -o -name "*.gcno" -o -name "*.o" -o -name "*.su" -o -name "*.gcov" \) -delete

help:
	@make -C tools/containers help

test:
	@make -C tools/containers test

format:
	@make -C tools/containers format

lint:
	@make -C tools/containers lint

misra:
	@make -C tools/containers misra

stack:
	@make -C tools/containers stack

size:
	@make -C tools/containers size

complexity:
	@make -C tools/containers complexity

san:
	@make -C tools/containers san

tsan:
	@make -C tools/containers tsan

qemu:
	@make -C tools/containers qemu

renode:
	@make -C tools/containers renode

fuzz:
	@make -C tools/containers fuzz

cov:
	@make -C tools/containers cov

static:
	@make -C tools/containers static

dox:
	@make -C tools/containers dox

valgrind:
	@make -C tools/containers valgrind

container-valgrind:
	@make -C tools/containers container-valgrind

analyzer:
	@make -C tools/containers analyzer

container-analyzer:
	@make -C tools/containers container-analyzer

flawfinder:
	@make -C tools/containers flawfinder

container-flawfinder:
	@make -C tools/containers container-flawfinder

integration:
	@make -C tools/containers integration

container-build:
	@make -C tools/containers container-build

container-test:
	@make -C tools/containers container-test

container-format:
	@make -C tools/containers container-format

container-lint:
	@make -C tools/containers container-lint

container-misra:
	@make -C tools/containers container-misra

container-stack:
	@make -C tools/containers container-stack

container-size:
	@make -C tools/containers container-size

container-complexity:
	@make -C tools/containers container-complexity

container-san:
	@make -C tools/containers container-san

container-tsan:
	@make -C tools/containers container-tsan

container-qemu:
	@make -C tools/containers container-qemu

container-renode:
	@make -C tools/containers container-renode

container-fuzz:
	@make -C tools/containers container-fuzz

container-cov:
	@make -C tools/containers container-cov

container-static:
	@make -C tools/containers container-static

container-dox:
	@make -C tools/containers container-dox

container-pio:
	@make -C tools/containers container-pio

container-integration:
	@make -C tools/containers container-integration

examples:
	@make -C tools/containers examples

container-examples:
	@make -C tools/containers container-examples
