from pathlib import Path

from langchain.chains import LLMChain
from langchain.llms import CTransformers
from langchain.prompts import PromptTemplate

import time


cwd = Path(__file__).parent.absolute()
model_path = cwd / '..' / 'models' / 'mistral-7b-instruct-v0.1.Q5_K_M.gguf'

config = {'max_new_tokens': 64 }  # , 'gpu_layers': 256}

model = CTransformers(model=str(model_path), config=config)

prompt_template = "What does one say to a game developer holding a {thing}?"

chain = LLMChain(
    llm=model,
    prompt=PromptTemplate.from_template(prompt_template)
)

for thing in ['sketchbook', 'Xbox controller', 'pencil', 'banana']:
    start_time = time.time()
    print(chain(thing))
    print(f"Time taken: {time.time() - start_time} seconds")
