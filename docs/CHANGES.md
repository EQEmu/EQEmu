# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org).

## [Unreleased]
- Added: Bazaar-and-Back AA travel with instance-aware return state persistence.
- Added: Enchanted and Legendary item-instance metadata with deterministic stat reconstruction from persisted `custom_data` through inventory, corpse, zoning, and reload paths.
- Fixed: Combat use is now blocked with a player-facing message.
- Fixed: Bazaar-and-Back now rejects dead, zoning, and casting use, validates stale return destinations, and falls back to a safe Bazaar location when the return state is bad.
- Changed: Bazaar-and-Back cooldown now uses the AA recast path with a rule-backed fallback.

### THJ-FND-004 Verification
- Persistence method: enchantment tier is stored on the item instance in `custom_data` under `thj_enchant_tier`, and both `inventory` and `character_corpse_items` already persist `custom_data`, so tier metadata survives save/load, corpse serialization, zoning, logout/login, and restart-driven item reloads without relying on transient in-memory stats.
- Reconstruction path: `EQ::ItemInstance` now rebuilds derived stats from the base item plus Monomyth rules whenever the instance is created from persisted metadata, preventing repeated reapplication on already-enchanted items.
- Debug helper: quest item bindings now expose `GetEnchantmentTier()` and `SetEnchantmentTier(tier)` for Perl and Lua, where `0 = normal`, `1 = Enchanted`, and `2 = Legendary`.
- Manual verification: create a test item instance, call `SetEnchantmentTier(1)` or `SetEnchantmentTier(2)`, save it in inventory, zone or relog, and confirm the same tier and reconstructed stats remain after reload.
- Manual verification: repeat the same flow with the item on a player corpse, then loot it and confirm the tiered stats match before and after corpse persistence.

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
