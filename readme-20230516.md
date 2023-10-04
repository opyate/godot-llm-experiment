# godot-llm-poc

A free game, made with Godot, using an LLM to drive NPC dialogue, and player dialogue options

# Introduction

Something like [Coffee Talk](https://store.steampowered.com/app/914800/Coffee_Talk/):

> Coffee Talk is a coffee brewing and heart-to-heart talking simulator about listening to fantasy-inspired modern peoples’ problems, and helping them by serving up a warm drink or two.

I'm focusing on the "talking simulator" bit. A fantasy person could enter the coffee shop, and be a - say - plumber, with aspirations to become a writer. Then you'll shoot the breeze, and NPC dialogue will be LLM-generated, and the 2 or 3 player dialogue options will also be LLM-generated (to save them from typing much).

We can run sentiment analysis on the dialogue, which can drive the NPC's facial expressions. We can maybe start with a small set of expressions for these emotions:
- angry
- confused
- happy
- sad
- surprised

And maybe some extras:
- bored
- embarrassed
- excited
- frustrated
- lonely
- nervous
- proud
- scared
- shy
- silly

# Notes

(I'm busy, and will work on this every sometimes, but for now, I'll add thoughts directly to this README)

- NPC dialogue could be [role-prompted](https://learnprompting.org/docs/basics/roles), e.g. "You are a plumber who aspire to be a writer. \<remainder of prompt>"
- Something like Mosaic's StoryWriter has a generous context window, but I'd have to test and see what minimum GPU is required, and if the model needs to be quantised or somehow further reduced to fit regular/common consumer GPUs: https://huggingface.co/mosaicml/mpt-7b-storywriter
