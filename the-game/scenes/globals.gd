extends Node

# Constants
const BOS_TOKEN = "<s>"
const EOS_TOKEN = "</s>"
var EOS_TOKEN_JUNK = []
const INSTRUCTION_START_TOKEN = "[INST]"
const INSTRUCTION_END_TOKEN = "[/INST]"

const DIALOGUE_SEED_STATE = [
	{"role": "user", "content": "You are going to simulate a real-life conversation between two people. Their exchanges will be brief, but whacky and weird, as if they live in a strange world. Give me one response at a time, and be sure to end a sentence with a punctuation mark."},
	{"role": "assistant", "content": "Person 1: Ever get that feeling that we're being watched?"},
	{"role": "user", "content": "Continue."},
	# {"role": "assistant", "content": "Person 2: Dude, the Panopticon is everywhere."}
	{"role": "assistant", "content": "Person 2: All the time."}
]

const stop_sequence = [EOS_TOKEN, "\n", "\n\n"]

func _ready():
		# eos_token_junk = [eos_token[:i] for i in range(len(eos_token)) if eos_token[:i]]
	for i in range(len(G.EOS_TOKEN)):
		var part = G.EOS_TOKEN.left(i)
		if part != "":
			EOS_TOKEN_JUNK.append(part)
