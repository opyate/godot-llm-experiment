extends Node2D


var llm
var max_new_tokens = 40

var by_len = func(a, b):
	return len(a) > len(b)
	

func _ready():
	llm = GDLLM.new()
	llm.set_stop_sequence(G.stop_sequence)
	llm.set_debug(false)
	# llm.set_random_seed(42)


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

const HISTORY_LENGTH = 11

func _get_prompt(dialogue_history: Array) -> Array:
	var prompt = get_prompt(dialogue_history)
	# var tokens = MODEL.tokenize(prompt, add_bos_token=true)
	# if tokens.size() >= 450:
	if len(dialogue_history) > HISTORY_LENGTH + 5:
		print("~~~~~~~~~~~~~~~~~~~~~~~~ truncating dialogue")
		# truncate the dialog, and just use the last few from the history.
		# we know the history will end with "user", and the seed end with "assistant",
		# so an odd numbered HISTORY_LENGTH ensures that the seed is followed by "user".
		var truncated_dialogue_history = G.DIALOGUE_SEED_STATE + dialogue_history.slice(-HISTORY_LENGTH)
		var shorter_prompt = get_prompt(truncated_dialogue_history)
		return [shorter_prompt, truncated_dialogue_history]
	else:
		return [prompt, dialogue_history]


func _clean_response(response: String) -> String:
	for junk in G.EOS_TOKEN_JUNK:
		response = response.replace(junk, "")
	response = response.strip_edges()
	if not G.PUNC.has(response.right(1)):
		var all_shorter_responses = []
		for p in G.PUNC:
			var maybe_shorter_response = response.substr(0, response.rfind(p) + 1)
			if maybe_shorter_response.length() == 0:
				continue
			all_shorter_responses.append(maybe_shorter_response)
		if all_shorter_responses.size() > 0:
			# use the longest shorter response
			all_shorter_responses.sort_custom(by_len)
			response = all_shorter_responses[0]
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

var running_dialogue = [] + G.DIALOGUE_SEED_STATE
func get_next_person_line():
	var result = assistant_get_next_line(running_dialogue)
	var assistant_response = result["assistant_response"]
	running_dialogue = result["possibly_truncated_dialogue_history"]
	return assistant_response
