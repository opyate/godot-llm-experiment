extends Node2D
# Constants
var BOS_TOKEN = "<s>"
var EOS_TOKEN = "</s>"
var EOS_TOKEN_JUNK = []
var INSTRUCTION_START_TOKEN = "[INST]"
var INSTRUCTION_END_TOKEN = "[/INST]"

var llm
var max_new_tokens = 40
var stop_sequence = [EOS_TOKEN, "\n", "\n\n"]

func _ready():
	llm = GDLLM.new()
	llm.set_stop_sequence(stop_sequence)
	
	# eos_token_junk = [eos_token[:i] for i in range(len(eos_token)) if eos_token[:i]]
	for i in range(len(EOS_TOKEN)):
		var part = EOS_TOKEN.left(i)
		if part != "":
			EOS_TOKEN_JUNK.append(part)

var DIALOGUE_SEED_STATE = [
	{"role": "user", "content": "You are going to simulate a real-life conversation..."},
	{"role": "assistant", "content": "Person 1: Ever get that feeling..."},
	{"role": "user", "content": "Continue."},
	{"role": "assistant", "content": "Person 2: Dude, the Panopticon is everywhere."}
]

func get_prompt(dialogue: Array) -> String:
	var prompt = BOS_TOKEN
	for i in range(dialogue.size()):
		var message = dialogue[i]
		if message["role"] == "user" and i % 2 == 1:
			push_error("Conversation roles must alternate...")
		elif message["role"] == "user":
			prompt += INSTRUCTION_START_TOKEN + " " + message["content"] + " " + INSTRUCTION_END_TOKEN
		elif message["role"] == "assistant":
			prompt += message["content"] + EOS_TOKEN + " "
		else:
			push_error("Only user and assistant roles are supported!")
	return prompt

func _get_prompt(dialogue_history: Array) -> Array:
	var prompt = get_prompt(dialogue_history)
	# var tokens = MODEL.tokenize(prompt, add_bos_token=true)
	# if tokens.size() >= 450:
	if len(dialogue_history) > 10:
		var truncated_dialogue_history = DIALOGUE_SEED_STATE + dialogue_history.slice(-7, -1)
		var shorter_prompt = get_prompt(truncated_dialogue_history)
		return [shorter_prompt, truncated_dialogue_history]
	else:
		return [prompt, dialogue_history]

var PUNC = ['.', '!', '?', '"']

func _clean_response(response: String) -> String:
	for junk in EOS_TOKEN_JUNK:
		response = response.replace(junk, "")
	response = response.strip_edges()
	if not PUNC.has(response[-1]):
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
	
