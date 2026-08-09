# Manual smoke test for the KomsoftGw leaderboard client.
#
# Usage: attach this script to a Node in a scene, fill in BASE_URL / API_KEY /
# USER_ID (issue a key via POST /v1/request-api-key), and run. Watch the output.
#
# KomsoftGw is a global singleton holding all actions; KomsoftGwNode drives the
# request queue each frame. You must add one KomsoftGwNode to the tree.
extends Node

const BASE_URL := "http://localhost:8080"
const SERVICE := "pingpal"
const API_KEY := "<user-api-key>"
const USER_ID := "abc123xyz TestPlayer"   # 9-char id + display name
const BOARD := "weekly_highscore"

func _ready() -> void:
	# Drive the singleton's request queue.
	add_child(KomsoftGwNode.new())

	KomsoftGw.set_use_ssl(false)   # local http
	KomsoftGw.configure(BASE_URL, SERVICE, API_KEY, USER_ID)

	KomsoftGw.connect("leaderboards_received", self, "_on_leaderboards")
	KomsoftGw.connect("score_submitted", self, "_on_submitted")
	KomsoftGw.connect("rank_received", self, "_on_rank")
	KomsoftGw.connect("top_received", self, "_on_top")
	KomsoftGw.connect("distribution_received", self, "_on_distribution")
	KomsoftGw.connect("friends_received", self, "_on_friends")
	KomsoftGw.connect("submit_token_received", self, "_on_token")
	KomsoftGw.connect("request_failed", self, "_on_failed")

	print("[komsoftgw] listing boards...")
	KomsoftGw.list_leaderboards()
	KomsoftGw.submit_score(BOARD, 4200, {"level": 7})

func _on_leaderboards(boards: Array) -> void:
	print("[komsoftgw] boards: ", boards)

func _on_submitted(result: Dictionary) -> void:
	print("[komsoftgw] submitted -> rank %s of %s (score %s)" % [result.get("rank"), result.get("total"), result.get("score")])
	KomsoftGw.get_rank(BOARD, 3)
	KomsoftGw.get_top(BOARD, "", 10, "")
	KomsoftGw.get_distribution(BOARD)
	KomsoftGw.get_friends(BOARD, ["def456uvw Friend"])

func _on_rank(result: Dictionary) -> void:
	print("[komsoftgw] my rank: ", result.get("rank"), " nearby: ", result.get("entries"))

func _on_top(entries: Array, next_cursor: String) -> void:
	print("[komsoftgw] top: ", entries, " nextCursor=", next_cursor)

func _on_distribution(result: Dictionary) -> void:
	print("[komsoftgw] distribution: total=", result.get("total"), " yourPercentile=", result.get("yourPercentile"), " thresholds=", result.get("thresholds"))

func _on_friends(entries: Array) -> void:
	print("[komsoftgw] friends: ", entries)

func _on_token(token: String, expires_in: int) -> void:
	print("[komsoftgw] token (expires in %ss): %s" % [expires_in, token])

func _on_failed(endpoint: String, code: int, message: String) -> void:
	push_error("[komsoftgw] %s failed (%d): %s" % [endpoint, code, message])
