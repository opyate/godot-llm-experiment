extends Node2D

var EOS_TOKEN = "</s>"
var stop_sequence = [EOS_TOKEN, "\n", "\n\n"]

func _ready():
	var llm = GDLLM.new()
	llm.set_stop_sequence(stop_sequence)

	llm.connect("completion_generated", on_completion_generated)

	var completion_text = llm.run_completion("Standing on the shoulders of ", 32)
	print("Got completion via return value: " + completion_text)
	

func on_completion_generated(completion_text: String):
	print("Got completion via signal: " + completion_text)
