from pathlib import Path

from ctransformers import AutoModelForCausalLM


from dataclasses import dataclass
import time


@dataclass
class Message:
    role: str
    content: str


cwd = Path(__file__).parent.absolute()
model_path = cwd / '..' / 'models' / 'mistral-7b-instruct-v0.1.Q5_K_M.gguf'

# see https://python.langchain.com/docs/integrations/providers/ctransformers
model = AutoModelForCausalLM.from_pretrained(model_path.as_posix())  # , lib='avx2')


dialogue_seed_state = [
    Message(role='user', content='You are a chatty friend of mine, and we are going to make some small talk. We will ask random questions and give random answers. Our answers will be short. Once you say you understand, I will start the conversation.'),
    Message(role='assistant', content="I understand."),
]

bos_token = "<s>"
eos_token = "</s>"

def get_prompt(dialogue: list[str]) -> str:
    """Renders a prompt using the official Mistral instruction template.
    
    From https://huggingface.co/mistralai/Mistral-7B-Instruct-v0.1/blob/main/tokenizer_config.json
    """
    prompt = f"{bos_token}"

    for idx, message in enumerate(dialogue):
        if message.role == 'user' and idx % 2 == 1:
            raise Exception('Conversation roles must alternate user/assistant/user/assistant/...')
        if message.role == 'user':
            prompt += f"[INST] {message.content} [/INST]"
        elif message.role == 'assistant':
            prompt += f"{message.content}{eos_token} "
        else:
            raise ValueError('Only user and assistant roles are supported!')
    
    return prompt


def _get_prompt(dialogue_history: list[str], this_role: str, message: str) -> tuple[str, list[str]]:
    dialogue = dialogue_history.copy()

    # Sometimes, it gets in a loop where it repeats itself over and over.
    # Get the last message from the dialogue history
    last_message = dialogue[-2].content
    if last_message == message:
        print("#"*30 + " (changing the subject) " + "#"*30)
        message = "Let's change the subject. Tell me something else."

    new_message = Message(role=this_role, content=message)
    dialogue.append(new_message)
    prompt = get_prompt(dialogue)

    tokens = model.tokenize(prompt, add_bos_token=True)
    if len(tokens) >= 512:
        
        # start over with the dialogue
        dialogue = dialogue_seed_state + [new_message,]
        prompt = get_prompt(dialogue)

        # tokens = model.tokenize(prompt, add_bos_token=True)
        # print(f"{len(tokens)}" + "~"*80)
    
    return prompt, dialogue


def talk(message: str, dialogue_history: list[str]) -> tuple[str, list[str]]:
    """Adds a message to the dialogue and returns the interlocutor's response with the updated dialogue history."""
    # get the last role who said something from dialogue_history
    last_role = dialogue_history[-1].role
    if last_role == 'user':
        this_role = 'assistant'
    else:
        this_role = 'user'
    
    prompt, dialogue = _get_prompt(dialogue_history, this_role, message)
    
    response = model(prompt, max_new_tokens=40)

    # remove all newlines from response
    response = response.replace('\n\n', ' ').replace('\n', ' ').replace('<br />', '').strip()
    dialogue.append(Message(role=last_role, content=response))
    return response, dialogue

response, new_dialogue_history = talk("Hey, buddy. Have you been up to anything exciting lately?", dialogue_seed_state)
print("\n"+response)

for i in range(40):
    response, new_dialogue_history = talk(response, new_dialogue_history)
    print("\n"+response.replace(eos_token, ''))
