# komsoftgw — KomSoft Gateway client for Godot

A Godot 3.x engine submodule that talks to the KomSoft Gateway **leaderboard** API.
Built on the shared `common/basic_http_request.h` HTTP helper; all calls are
asynchronous and deliver results via signals.

It is split in two (mirroring the SilentWolf design):

- **`KomsoftGw`** — a global singleton holding the config and every action
  (submit, rank, top, distribution, friends). Callable from any script. Each call
  is queued and only issued/polled when the singleton is pumped.
- **`KomsoftGwNode`** — a lightweight `Node` whose only job is to pump the
  singleton every frame. Add one instance to your scene once.

## Build

This submodule follows the standard `gdextensions` wiring:

1. It is registered in `modules/gdextensions/config.py` (`"komsoftgw"` in `modules`).
2. It is registered in `modules/gdextensions/register_types.cpp` (include +
   `ClassDB::register_class<KomsoftGwNode>()`).
3. All `.cpp` live at the submodule root, so the generic `SCsub` glob compiles
   them — no `SCsub` edit needed.

Build the engine as usual, e.g.:

```
scons platform=osx module_gdextensions_enabled=yes gdext_enable_submodules=all
```

(or list specific submodules in `gdext_enable_submodules`).

## Usage

```gdscript
func _ready():
    # Add the pump node once; it drives the singleton's request queue each frame.
    add_child(KomsoftGwNode.new())

    # user_id is the gateway's "<9charId> <DisplayName>" identity string.
    KomsoftGw.configure("https://api.example.com", "pingpal", api_key, user_id)
    KomsoftGw.connect("score_submitted", self, "_on_submitted")
    KomsoftGw.connect("request_failed", self, "_on_failed")
    KomsoftGw.submit_score("weekly_highscore", 4200, {"level": 7})

func _on_submitted(result):
    # result = { rank, score, numScore, total, periodKey }  (your own private-safe view)
    print("You are rank %d of %d" % [result.rank, result.total])
```

Actions are queued the moment you call them and issued on the next pump, so
`KomsoftGw` is safe to call from anywhere — just make sure one `KomsoftGwNode`
exists in the tree. The client automatically sends the headers the gateway
requires (`User-Agent`, `X-Client-Version`, `x-api-key`).

**Obtain a key first.** Like the chat API, every leaderboard call is authenticated
with a per-user API key. Request one from the gateway (`POST /v1/request-api-key`
with `{user_id, service}`) — typically the app already holds this key for chat —
and pass it to `configure(...)`. Calls without a valid key for the configured
`user_id`/`service` are rejected with `401`.

### Methods

| Method | Signal | Notes |
|--------|--------|-------|
| `list_leaderboards()` | `leaderboards_received(Array)` | public board metadata |
| `request_submit_token(board_key)` | `submit_token_received(token, expires_in)` | for boards that require a token |
| `submit_score(board_key, score, metadata:={}, context:={}, token:="")` | `score_submitted(Dictionary)` | returns your own rank only |
| `get_rank(board_key, around:=0)` | `rank_received(Dictionary)` | personal rank + `around` neighbors |
| `get_top(board_key, period_key:="", limit:=50, cursor:="")` | `top_received(entries, next_cursor)` | public boards only |
| `get_distribution(board_key)` | `distribution_received(Dictionary)` | percentile bands + histogram, no names |
| `get_friends(board_key, friend_ids)` | `friends_received(Array)` | board filtered to your id set |

Any failure emits `request_failed(endpoint, code, message)`.

`context` is a free-form dictionary stored server-side for later anti-cheat audit.
Scores outside a board's plausibility bounds are silently shadow-flagged by the
server (hidden from public reads) — the client still receives a normal response.

See `_test/leaderboard_test.gd` for a runnable example.
