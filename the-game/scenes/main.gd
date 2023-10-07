extends Node2D


func _ready():
	var llm = GDLLM.new()
	
	llm.connect("completion_generated", on_completion_generated)
	
	llm.run_completion("Hello, my name is ")
	

func on_completion_generated(_node: Node, completion_text: String):
	print("Got completion:")
	print(completion_text)
