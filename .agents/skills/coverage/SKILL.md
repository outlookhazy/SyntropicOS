---
name: coverage
description: >-
  Measure SyntropicOS line and branch code coverage, Doxygen API documentation coverage, and markdown
  module documentation completeness. Use when the user triggers /coverage or asks to audit test and doc coverage.
---

# SyntropicOS Code & Documentation Coverage

Measure line/branch code coverage, verify API documentation completeness, and confirm new or modified modules have matching documentation and examples.

## Step 0: Run Instrumented Code Coverage Analysis
Execute the containerized LCOV coverage build:
- **Containerized Coverage Report**: `make container-cov`

## Step 1: Inspect Coverage Summary & Uncovered Branches
Inspect the `lcov --summary` and `tools/coverage/analyze_coverage.py` output already printed inside `make container-cov` (note: `build/cov/` resides inside the `syntropicos-build-cache` container volume, not on the host filesystem) to identify untested functions, unreached branches, or edge cases.

## Step 2: Doxygen API Documentation Coverage
Verify API documentation completeness and check for missing symbol docs:
- **Doxygen Coverage Check**: `make container-dox`
- **Mandatory Reporting**: Report the exact Doxygen warning count (e.g., `Documentation Warning Count: X`) and list any undocumented functions/structs in the summary output. Never obscure or omit Doxygen warning metrics.

## Step 3: Module, Feature & Invariant Documentation Audit
Audit markdown documentation (`docs/`, `docs/modules/`, `README.md`, `mkdocs.yml`, and directory `AGENTS.md` files) for feature and contract accuracy:
- **New Module Coverage**: Verify every new or modified source file (`src/syntropic/*`, `src/port/*`) has matching documentation in `docs/modules/`.
- **API Reference Sync**: Confirm newly exposed public APIs are documented with signatures, parameters, return values, and usage notes.
- **Subsystem `AGENTS.md` Sync**: When changes alter cross-directory wiring or architectural invariants, verify affected `AGENTS.md` files (`src/syntropic/`, `src/port/`, `tests/unit/`) remain accurate in the same diff.
- **Example Coverage**: Verify new features or drivers have corresponding example sketches in `examples/`.
