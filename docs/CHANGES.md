# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org).

## [Unreleased]
- Added: Bazaar-and-Back AA travel with instance-aware return state persistence.
- Fixed: Combat use is now blocked with a player-facing message.
- Fixed: Bazaar-and-Back now rejects dead, zoning, and casting use, validates stale return destinations, and falls back to a safe Bazaar location when the return state is bad.
- Changed: Bazaar-and-Back cooldown now uses the AA recast path with a rule-backed fallback.

## - 2026-05-10
### Added
- Added OAuth2 support for third-party logins.
- New API endpoint to retrieve user statistics.

### Changed
- Increased maximum file upload size to 20MB.

### Fixed
- Fixed memory leak in the data processor.

### Removed
- Removed deprecated `/old-api` endpoint.

## - 2026-01-01
### Added
- Initial release of the project.
