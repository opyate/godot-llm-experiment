import time
from dataclasses import dataclass
from pathlib import Path

from ctransformers import AutoModelForCausalLM

bos_token = "<s>"
eos_token = "</s>"
eos_token_junk = [eos_token[:i] for i in range(len(eos_token)) if eos_token[:i]]
instruction_start_token = "[INST]"
instruction_end_token = "[/INST]"


@dataclass
class Message:
    role: str
    content: str


cwd = Path(__file__).parent.absolute()
model_path = cwd / '..' / 'models' / 'mistral-7b-instruct-v0.1.Q5_K_M.gguf'

# see https://python.langchain.com/docs/integrations/providers/ctransformers
stop_sequence = [eos_token, '\n', '\n\n']
model = AutoModelForCausalLM.from_pretrained(model_path.as_posix(), gpu_layers=250, stop=stop_sequence)  # , lib='avx2')


dialogue_seed_state = [
    Message(
        role='user',
        content='You are going to simulate a real-life conversation between two people. Their exchanges will be brief, but whacky and weird, as if they live in a strange world. Give me one response at a time, and be sure to end a sentence with a punctuation mark.'
    ),
    Message(
        role='assistant',
        content="Person 1: Ever get that feeling that we're being watched?"
    ),
    Message(
        role='user',
        content="Continue."
    ),
    Message(
        role='assistant',
        content="Person 2: Dude, the Panopticon is everywhere."
    ),
]


def get_prompt(dialogue: list[str]) -> str:
    """Renders a prompt using the official Mistral instruction template.

    From https://huggingface.co/mistralai/Mistral-7B-Instruct-v0.1/blob/main/tokenizer_config.json
    """
    prompt = f"{bos_token}"

    for idx, message in enumerate(dialogue):
        if message.role == 'user' and idx % 2 == 1:
            raise Exception(
                'Conversation roles must alternate user/assistant/user/assistant/...')
        if message.role == 'user':
            prompt += f"{instruction_start_token} {message.content} {instruction_end_token}"
        elif message.role == 'assistant':
            prompt += f"{message.content}{eos_token} "
        else:
            raise ValueError('Only user and assistant roles are supported!')

    return prompt


def _get_prompt(dialogue_history: list[str]) -> tuple[str, list[str]]:

    prompt = get_prompt(dialogue_history)

    tokens = model.tokenize(prompt, add_bos_token=True)
    # context is 512, but it includes the response
    if len(tokens) >= 450:
        # truncate the dialog, and just use the last few from the history
        truncated_dialogue_history = dialogue_seed_state + dialogue_history[-7:]
        shorter_prompt = get_prompt(truncated_dialogue_history)

        # tokens = model.tokenize(shorter_prompt, add_bos_token=True)
        # print(f"shorter prompt: {len(tokens)}" + "~"*80)

        return shorter_prompt, truncated_dialogue_history
    else:

        return prompt, dialogue_history

punc = ['.', '!', '?', '"']

def _clean_response(response: str) -> str:
    """Sometimes, the max_new_tokens limit cuts off the eos_token, so remove any partial eos_tokens
    """
    for junk in eos_token_junk:
        response = response.replace(junk, '')
    
    response = response.strip()

# if the response doesn't end with punctuation, then cut the response off after the last punctuation.
    if response[-1] not in punc:
        all_shorter_responses = []
        for p in punc:
            maybe_shorter_response = response[:response.rfind(p)+1]
            if len(maybe_shorter_response) == 0:
                # p wasn't in the response, and we cut everything off. Start over for next p
                continue
            elif len(maybe_shorter_response) <= len(response):
                all_shorter_responses.append(maybe_shorter_response)
            else:
                raise Exception("This shouldn't happen.")
        
        if len(all_shorter_responses) > 0:
            # use the longest shorter response
            response = max(all_shorter_responses, key=len)

    
    return response


def assistant_get_next_line(dialogue_history: list[str]) -> tuple[str, list[str]]:

    dialogue_history.append(
        Message(
            role='user',
            content='Continue.'
        )
    )

    prompt, possibly_truncated_dialogue_history = _get_prompt(dialogue_history)

    assistant_response = model(prompt, max_new_tokens=40)

    assistant_response = _clean_response(assistant_response)

    possibly_truncated_dialogue_history.append(
        Message(
            role='assistant',
            content=assistant_response
        )
    )

    return assistant_response, possibly_truncated_dialogue_history


start_time = time.time()
running_dialogue = dialogue_seed_state.copy()
for i in range(60):
    response, running_dialogue = assistant_get_next_line(running_dialogue)
    print("\n"+response)
print(f"Elapsed time: {time.time() - start_time}")