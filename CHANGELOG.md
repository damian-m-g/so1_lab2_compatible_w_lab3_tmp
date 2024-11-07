# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.4-alpha] - 2024-10-07

### Added

- Activity 4 implemented: support for batch files.

### Changed

- start_shell() refactorized into start_shell_ml() and execute_command().

### Fixed

- Used macros instead of hardcoded numbers for exit status on chid process going wrong.
- Redundant code removed.

## [0.3-alpha] - 2024-10-07

### Added

- Activity 3 implemented: Invocation of external programs.

## [0.2-alpha] - 2024-10-06

### Added

- Activity 2 implemented: Internal commands. Remaining commands 'quit' and 'echo' were finished.

## [0.1-alpha.3] - 2024-10-06

### Added

- Activity 2 partially implemented: Internal commands: clr.

### Fixed

- Reference to certain macro wasn't linked.
- Macro not used were cleaned.

## [0.1-alpha.2] - 2024-10-06

### Added

- Activity 2 partially implemented: Internal commands: cd.

## [0.1-alpha] - 2024-10-05

### Added

- Activity 1 implemented: Command Line Prompt.
