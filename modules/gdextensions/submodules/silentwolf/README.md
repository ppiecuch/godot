# SilentWolf - Backend-as-a-Service for Godot

C++/Godot 3.x port of the [SilentWolf](https://silentwolf.com) GDScript plugin (v0.6.20). Provides leaderboards, player authentication, player data storage, and WebSocket multiplayer through the SilentWolf cloud backend.

## What It Does

Drop-in backend integration for indie games: persistent leaderboards with score hashing, player registration/login with session management, key-value player data storage, and real-time multiplayer via WebSockets. All API calls are async with signal-based completion.

## Architecture

### Singleton + Subsystems

```
SilentWolf (Object singleton)
  +-- SW_Auth (Reference)         Player authentication & session management
  +-- SW_Scores (Reference)       Leaderboard & score management
  +-- SW_Player (Reference)       Player data CRUD
  +-- SW_Multiplayer (Reference)  WebSocket multiplayer sessions
        +-- SW_WSClient           WebSocket client wrapper
```

### Scene Integration

```
SilentWolfInstance (Node2D)
  - Editor: shows progress icons during active requests
  - Runtime: drives sw_process() polling via NOTIFICATION_INTERNAL_PROCESS
  - Exposes game_id, api_key, active as editor properties
```

### Request Lifecycle

```
1. API method called (e.g. Scores->get_high_scores())
2. BasicHTTPRequest created, signal connected for completion callback
3. "sw_data_requested" emitted -> SilentWolf starts polling (or thread)
4. sw_process() polls all active BasicHTTPRequests each frame
5. On completion: response parsed, result signal emitted
6. When no active requests remain: polling stops
```

### Threading Model

Two modes:
- **Polling** (default): `SilentWolfInstance::_notification(INTERNAL_PROCESS)` calls `sw_process()`
- **Threaded**: Dedicated thread polls requests via `_thread_func()`, auto-exits when idle

## API Surface

### SW_Auth

| Method | Description |
|---|---|
| `register_player_anon(name)` | Anonymous registration with device ID |
| `register_player(name, email, password, confirm)` | Email registration |
| `register_player_user_password(name, password, confirm)` | Username/password registration |
| `login_player(username, password, remember_me)` | Login with optional session persistence |
| `logout_player()` | Logout and clear stored session |
| `verify_email(name, code)` | Email verification code confirmation |
| `resend_conf_code(name)` | Resend email confirmation code |
| `request_player_password_reset(name)` | Request password reset email |
| `reset_player_password(name, code, new_pwd, confirm)` | Reset password with confirmation code |
| `get_player_details(name)` | Get player account details |
| `validate_player_session(lookup, validator)` | Validate stored session for auto-login |
| `auto_login_player()` | Auto-login from saved session file |
| `save_session(lookup, validator)` | Save session to `user://swsession.save` |
| `load_session()` | Load session from local storage |

### SW_Scores

| Method | Description |
|---|---|
| `get_high_scores(max, ldboard, period_offset)` | Fetch top scores |
| `get_scores_by_player(name, max, ldboard, period_offset)` | Scores by player |
| `get_top_score_by_player(name, max, ldboard, period_offset)` | Player's top score |
| `get_score_position(score, ldboard)` | Score ranking position |
| `get_scores_around(score, count, ldboard)` | Scores above/below |
| `persist_score(name, score, ldboard, metadata)` | Post new score with UUID + hash |
| `delete_score(score_id)` | Delete a score |
| `wipe_leaderboard(ldboard)` | Delete all scores in leaderboard |
| `add_to_local_scores(result, ld_name)` | Local score cache |

### SW_Player

| Method | Description |
|---|---|
| `get_player_data(name)` | Fetch player data from backend |
| `post_player_data(name, data, overwrite)` | Push player data |
| `delete_player_items(name, item)` | Delete specific item |
| `delete_player_data(name, data)` | Delete specific data keys |
| `delete_all_player_data(name)` | Delete all player data |
| `get_stats()` | Extract stats from player data |
| `get_inventory()` | Extract inventory from player data |

### SW_Multiplayer

| Method | Description |
|---|---|
| `init_mp_session(player_name)` | Initialize WebSocket session |
| `send(data)` | Send data to WebSocket server |

## File Structure

```
silentwolf/
  silent_wolf.h              Main header: all class declarations, enums, utility prototypes
  silent_wolf.cpp            Singleton, configuration, threading, SilentWolfInstance Node2D
  sw_auth.cpp                Authentication: register, login, verify, reset, session (~676 lines)
  sw_scores.cpp              Leaderboards: CRUD, local cache, hash integrity (~564 lines)
  sw_players.cpp             Player data: get/post/delete (~244 lines)
  sw_multiplayer.cpp         WebSocket multiplayer session management (~87 lines)
  sw_wsclient.cpp            WebSocket client: connect, send, receive (~154 lines)
  sw_common_errors.cpp       HTTP status code checking (~45 lines)
  sw_hashing.cpp             MD5 hash computation for score integrity (~42 lines)
  sw_local_file_storage.cpp  JSON file I/O for session persistence (~89 lines)
  sw_logger.cpp              Configurable log levels (error/warning/info/debug) (~54 lines)
  uuid.cpp                   UUID v4 generation (RFC 4122) (~82 lines)
  _source/                   Original GDScript plugin source (reference)
```

## Port Status

### Fully Ported

| Component | Source | Status |
|---|---|---|
| Authentication (register/login/logout) | Auth.gd | Complete |
| Email verification & password reset | Auth.gd | Complete |
| Session management (save/load/validate) | Auth.gd | Complete |
| Leaderboard scores (CRUD) | Scores.gd | Complete |
| Score position & scores around | Scores.gd | Complete |
| Score integrity hashing | Scores.gd | Complete |
| Local score caching | Scores.gd | Complete |
| Player data (get/post/delete) | Players.gd | Complete |
| WebSocket multiplayer | Multiplayer.gd, WSClient.gd | Complete |
| UUID v4 generation | utils/uuid.gd | Complete |
| Local file storage | utils/swlocaldata.gd | Complete |
| Logging system | utils/swlogger.gd | Complete |
| Error handling | utils/swcommonerrors.gd | Complete |
| Editor integration (progress icons) | N/A (C++ only) | Complete |
| Thread support | N/A (C++ only) | Complete |

### Known Bugs (Fixed)

1. **`delete_score()`** (`sw_scores.cpp:246`): Checked `HighScores->is_active_request()` instead of `DeleteScore->is_active_request()` (copy-paste error). **Fixed.**

2. **`_on_GetTopScoreByPlayer_request_completed()`** (`sw_scores.cpp:302-303`): Checked `response.has("top_score")` (singular) but read `response["top_scores"]` (plural). **Fixed** to use `"top_score"` consistently.

3. **`sw_wsclient.cpp:35`**: Platform-specific `#include <_types/_uint8_t.h>` (macOS-only header). **Fixed** by removing it (uint8_t available via Godot's core headers).

4. **`_bind_methods()`** (`sw_scores.cpp`): Missing GDScript binding for `get_top_score_by_player`. **Fixed.**

5. **Signal declarations** (`sw_scores.cpp`): Several signals missing parameter info in `ADD_SIGNAL` declarations. Compatibility signals (`scores_received`, `position_received`) undeclared. **Fixed.**

### GDScript Parity Notes

- The C++ port matches GDScript v0.6.20 API surface
- `get_stats()` and `get_inventory()` are convenience methods with hardcoded field names (strength, speed, reflexes, max_health, career, weapons, gold) matching the original GDScript plugin's demo
- The GDScript plugin's `HTTPRequest` node pattern is replaced by `BasicHTTPRequest` (a custom polling-based HTTP client)
- Configuration uses static const Dictionary instead of GDScript's exported vars

## Signals

### SW_Auth Signals
| Signal | Parameters | When |
|---|---|---|
| `sw_login_succeeded` | | Login successful |
| `sw_login_failed` | error: String | Login failed |
| `sw_logout_succeeded` | | Logout complete |
| `sw_registration_succeeded` | | Registration successful |
| `sw_registration_user_pwd_succeeded` | email_conf: bool | User/password registration done |
| `sw_registration_failed` | error: String | Registration failed |
| `sw_email_verif_succeeded` | | Email verified |
| `sw_email_verif_failed` | error: String | Verification failed |
| `sw_session_check_complete` | result: Variant | Auto-login result |
| `sw_get_player_details_succeeded` | details: Dictionary | Player details received |

### SW_Scores Signals
| Signal | Parameters | When |
|---|---|---|
| `sw_scores_received` | result: Array | High scores fetched |
| `sw_player_scores_received` | scores: Array | Player scores fetched |
| `sw_top_player_score_received` | score: Dictionary | Top score fetched |
| `sw_position_received` | position: int | Score position found |
| `sw_scores_around_received` | above, below, position | Surrounding scores |
| `sw_score_posted` | score_id: String | Score posted |
| `sw_score_deleted` | | Score deleted |
| `sw_leaderboard_wiped` | | Leaderboard cleared |

### SW_Player Signals
| Signal | Parameters | When |
|---|---|---|
| `sw_player_data_received` | name, data | Player data fetched |
| `sw_player_data_posted` | name: String | Data pushed |
| `sw_player_data_removed` | name, data | Data deleted |

## Configuration

```cpp
// Default config (override via configure* methods or SilentWolfInstance properties)
SilentWolf::config = {
    "api_key": "...",
    "game_id": "sdktest",
    "game_version": "2.0.0",
    "use_ssl": true,
    "log_level": 3  // debug in DEBUG builds, 0 in release
};

SilentWolf::auth_config = {
    "session_duration_seconds": 0,       // 0 = no timeout
    "saved_session_expiration_days": 30
};
```

## References

- [SilentWolf](https://silentwolf.com) - Backend service
- [SilentWolf GDScript Plugin](https://github.com/nicemicro/silentwolf-godot) - Original GDScript source
- [SilentWolf Documentation](https://silentwolf.com/leaderboard) - API documentation
