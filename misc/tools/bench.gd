extends Node

func _ready():
	var bench = SynthBenchmark.new()
	bench.benchmark()
	yield(bench, "benchmark_ready")
	print("")
	print(bench.get_benchmark_report())
	get_tree().quit()
