# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.0.7] - 2024-11-30

### Changed

- Updated submodule to track its last commit.

## [1.0.6] - 2024-11-28

### Added

- Activity 2.3 of lab3 implemented: Exploración del filesystem y búsqueda de configuraciones. Se hizo a través del
nuevo comando interno `explore_filesystem`, que necesita un argumento: el path a un dir a recorrer.

### Fixed

- Bug on `execute_external_cmd()`.

## [1.0.5] - 2024-11-23

### Fixed

- Those functions named starting with "_", as that kind of naming is reserved to Unix kernel functions.

## [1.0.4] - 2024-11-23

### Fixed

- Magic numbers on all *.c and *.h files.
- The dates on this file, which were wrong (pointed to month 10, when it should be 11).
- "build" GitHub action of the QA workflow, so it builds the executable in Debug mode, and not in Release mode.

## [1.0.3] - 2024-11-18

### Fixed

- `documentation` and `coverage_report` folders are now ignored from the repository.
- INSTALL.md file was modified to include documentation on how to run the tests, generating the coverage report; and how to generate the Doxygen API documentation for the project.

## [1.0.2] - 2024-11-16

### Fixed

- Potential flaws noticed by the warning flags.

## [1.0.1] - 2024-11-16

### Changed

- Testing submodule recognition on remote repository WF.

## [1.0] - 2024-11-16

### Added

- Submodule "metrics". Linked.

### Fixed

- Remote action WF to complete OK.

## [1.0-rc] - 2024-11-16

### Fixed

- Syntax (clang-format complying) fixes.

## [1.0-beta] - 2024-11-16

### Added

- "config.json" & "INSTALL.md" completed.

## [0.12-alpha] - 2024-11-16

### Added

- Activity 12 implemented: unit testing coverage of 5% minimum.
- Coverage report generated.
- Documentation (Doxygen) generated.
- "config.json" & "INSTALL.md" pre-generated.

### Fixed

- Minor grammatical errors.

## [0.10-alpha] - 2024-11-15

### Added

- Activity 10 implemented: usage of cJSON for "metrics" configuration.

## [0.9-alpha] - 2024-11-12

### Added

- Activity 9 implemented: integration with "metrics" app.

### Fixed

- Small fixes.

## [0.8-alpha] - 2024-11-10

### Added

- Activity 7 implemented: pipes.
- Activity 8 implemented: io redirection.

### Changed

- Code completely refactorized.

### Fixed

- Small fixes.

## [0.6-alpha] - 2024-11-08

### Added

- Activity 6 implemented: signals managment.

## [0.5-alpha] - 2024-11-08

### Added

- Activity 5 implemented: background execution.
- Include guards added to source headers.
- Job ID track.

### Fixed

- Small issues when passing length of certain arrays.

## [0.4-alpha] - 2024-11-07

### Added

- Activity 4 implemented: support for batch files.

### Changed

- start_shell() refactorized into start_shell_ml() and execute_command().

### Fixed

- Used macros instead of hardcoded numbers for exit status on chid process going wrong.
- Redundant code removed.

## [0.3-alpha] - 2024-11-07

### Added

- Activity 3 implemented: Invocation of external programs.

## [0.2-alpha] - 2024-11-06

### Added

- Activity 2 implemented: Internal commands. Remaining commands 'quit' and 'echo' were finished.

## [0.1-alpha.3] - 2024-11-06

### Added

- Activity 2 partially implemented: Internal commands: clr.

### Fixed

- Reference to certain macro wasn't linked.
- Macro not used were cleaned.

## [0.1-alpha.2] - 2024-11-06

### Added

- Activity 2 partially implemented: Internal commands: cd.

## [0.1-alpha] - 2024-11-05

### Added

- Activity 1 implemented: Command Line Prompt.
