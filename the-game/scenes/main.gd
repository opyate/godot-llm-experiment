extends Node2D

@onready var dialogue = $dialogue


#func _ready():
#	var d = [] + G.DIALOGUE_SEED_STATE
#	for i in range(21):
#		if i % 2 == 0:
#			d.append("user %s" % [i])
#		else:
#			d.append("assistant %s" % [i])
#	print(d)
#	print("-------------------")
#
#	var l = G.DIALOGUE_SEED_STATE + d.slice(-7)
#	print(l)

func _ready():
	var running_dialogue = [] + G.DIALOGUE_SEED_STATE
	
	# start the conversation
	var result = dialogue.assistant_get_next_line(running_dialogue)
	var assistant_response = result["assistant_response"]
	running_dialogue = result["possibly_truncated_dialogue_history"]
	print(">> " + assistant_response)
	
	for i in range(5):
		
		result = dialogue.assistant_get_next_line(running_dialogue)
		assistant_response = result["assistant_response"]
		running_dialogue = result["possibly_truncated_dialogue_history"]
		print(">> " + assistant_response)
		
	

#func _ready():
#	var llm = GDLLM.new()
#	llm.set_stop_sequence(stop_sequence)
#
#	llm.connect("completion_generated", on_completion_generated)
#
#	var completion_text = llm.run_completion("Standing on the shoulders of ", 32)
#	print("Got completion via return value: " + completion_text)
#
#
#func on_completion_generated(completion_text: String):
#	print("Got completion via signal: " + completion_text)
