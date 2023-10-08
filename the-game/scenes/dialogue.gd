extends Node2D


var llm
var max_new_tokens = 40

func _ready():
	llm = GDLLM.new()
	llm.set_stop_sequence(G.stop_sequence)



func get_prompt(dialogue: Array) -> String:
	var prompt = G.BOS_TOKEN
	for i in range(dialogue.size()):
		var message = dialogue[i]
		if message["role"] == "user" and i % 2 == 1:
			push_error("Conversation roles must alternate...")
		elif message["role"] == "user":
			prompt += G.INSTRUCTION_START_TOKEN + " " + message["content"] + " " + G.INSTRUCTION_END_TOKEN
		elif message["role"] == "assistant":
			prompt += message["content"] + G.EOS_TOKEN + " "
		else:
			push_error("Only user and assistant roles are supported!")
	return prompt

func _get_prompt(dialogue_history: Array) -> Array:
	var prompt = get_prompt(dialogue_history)
	# var tokens = MODEL.tokenize(prompt, add_bos_token=true)
	# if tokens.size() >= 450:
	if len(dialogue_history) > 10:
		# truncate the dialog, and just use the last few from the history.
		# we know the history will end with "user", and the seed end with "assistant",
		# so -7 (an odd number) ensures that the seed is followed by "user".
		var truncated_dialogue_history = G.DIALOGUE_SEED_STATE + dialogue_history.slice(-7)
		var shorter_prompt = get_prompt(truncated_dialogue_history)
		return [shorter_prompt, truncated_dialogue_history]
	else:
		return [prompt, dialogue_history]

var PUNC = ['.', '!', '?', '"']

func _clean_response(response: String) -> String:
	for junk in G.EOS_TOKEN_JUNK:
		response = response.replace(junk, "")
	response = response.strip_edges()
	if not PUNC.has(response.right(1)):
		var all_shorter_responses = []
		for p in PUNC:
			var maybe_shorter_response = response.substr(0, response.rfind(p) + 1)
			if maybe_shorter_response.length() == 0:
				continue
			all_shorter_responses.append(maybe_shorter_response)
		if all_shorter_responses.size() > 0:
			# response = max(all_shorter_responses, key=lambda x: x.length())
			pass
	return response

func assistant_get_next_line(dialogue_history: Array):
	dialogue_history.append({"role": "user", "content": "Continue."})
	var results = _get_prompt(dialogue_history)
	var prompt = results[0]
	var possibly_truncated_dialogue_history = results[1]
	var assistant_response = llm.run_completion(prompt, max_new_tokens)
	assistant_response = _clean_response(assistant_response)
	possibly_truncated_dialogue_history.append({"role": "assistant", "content": assistant_response})
	return {
		"assistant_response": assistant_response,
		"possibly_truncated_dialogue_history": possibly_truncated_dialogue_history,
	}
	
