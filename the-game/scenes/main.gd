extends Node2D


func _ready():
	var llm = GDLLM.new()

	llm.connect("completion_generated", on_completion_generated)

	llm.run_completion("Standing on the shoulders of ")
	

func on_completion_generated(completion_text: String):
	print("Got completion:")
	print(completion_text)
