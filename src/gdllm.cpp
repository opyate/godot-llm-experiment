#include "common.h"
#include "llama.h"
#include "gdllm.h"

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void GDLLM::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_stop_sequence"), &GDLLM::get_stop_sequence);
    ClassDB::bind_method(D_METHOD("set_stop_sequence", "p_stop_sequence"), &GDLLM::set_stop_sequence);
    ClassDB::add_property("GDLLM", PropertyInfo(Variant::PACKED_STRING_ARRAY, "stopSeq"), "set_stop_sequence", "get_stop_sequence");

    ClassDB::bind_method(D_METHOD("get_debug"), &GDLLM::get_debug);
    ClassDB::bind_method(D_METHOD("set_debug", "p_debug"), &GDLLM::set_debug);
    ClassDB::add_property("GDLLM", PropertyInfo(Variant::BOOL, "debug"), "set_debug", "get_debug");

    ClassDB::bind_method(D_METHOD("get_random_seed"), &GDLLM::get_random_seed);
    ClassDB::bind_method(D_METHOD("set_random_seed", "p_random_seed"), &GDLLM::set_random_seed);
    ClassDB::add_property("GDLLM", PropertyInfo(Variant::INT, "random_seed"), "set_random_seed", "get_random_seed");

    // MethodInfo's first parameter will be the signal's name.
    // Its remaining parameters are PropertyInfo types which describe
    // the essentials of each of the method's parameters.
    // PropertyInfo parameters are defined with the data type of the parameter,
    // and then the name that the parameter will have by default.
    ADD_SIGNAL(MethodInfo("completion_generated", PropertyInfo(Variant::STRING, "completion_text")));

    ClassDB::bind_method(D_METHOD("run_completion", "prompt_from_godot"), &GDLLM::run_completion);
}

GDLLM::GDLLM() {
    // Initialize any variables here.
    longest_stop_sequence_string_length = 0;
    debug = false;
    random_seed = time(NULL);
}

GDLLM::~GDLLM() {
    // Add your cleanup here.
}

int getLongestStringLength(const PackedStringArray& p_stop_sequence) {
    int maxLength = 0;

    for (int i = 0; i < p_stop_sequence.size(); i++) {
        String currentString = p_stop_sequence[i];
        int currentLength = currentString.length();
        
        if (currentLength > maxLength) {
            maxLength = currentLength;
        }
    }

    return maxLength;
}

void GDLLM::set_stop_sequence(const PackedStringArray& p_stop_sequence) {
    stop_sequence = p_stop_sequence;
    longest_stop_sequence_string_length = getLongestStringLength(p_stop_sequence);
}

PackedStringArray GDLLM::get_stop_sequence() const {
    return stop_sequence;
}

void GDLLM::set_debug(const bool p_debug) {
    debug = p_debug;
}

bool GDLLM::get_debug() const {
    return debug;
}

void GDLLM::set_random_seed(const uint32_t p_random_seed) {
    random_seed = p_random_seed;
}

uint32_t GDLLM::get_random_seed() const {
    return random_seed;
}

godot::String GDLLM::run_completion(const String& prompt_from_godot, const int max_new_tokens) {
    
    if (debug) {
        godot::UtilityFunctions::print("[GDLLM] prompt: @@", prompt_from_godot, "@@");
    }

    gpt_params params;

    params.model = "bin/mistral-7b-instruct-v0.1.Q5_K_M.gguf";

    CharString temp = prompt_from_godot.utf8();
    const char* p_prompt = temp.get_data();
    params.prompt = p_prompt;

    if (params.prompt.empty()) {
        return "You didn't prompt me.";
    }

    // total length of the sequence including the prompt
    int prompt_length = prompt_from_godot.length();
    int n_len = prompt_length + max_new_tokens;

    // init LLM

    llama_backend_init(params.numa);

    // initialize the model

    llama_model_params model_params = llama_model_default_params();

    // model_params.n_gpu_layers = 99; // offload all layers to the GPU

    llama_model * model = llama_load_model_from_file(params.model.c_str(), model_params);

    if (model == NULL) {
        fprintf(stderr , "%s: error: unable to load model\n" , __func__);
        if (debug) {
            godot::UtilityFunctions::print("[GDLLM] unable to load model");
        }
        return "[GDLLM] unable to load model";
    }

    // initialize the context

    llama_context_params ctx_params = llama_context_default_params();

    ctx_params.seed = random_seed;
    params.seed = random_seed;
    std::mt19937 rng(params.seed);    
    ctx_params.n_ctx = 2048;
    ctx_params.n_threads = params.n_threads;
    ctx_params.n_threads_batch = params.n_threads_batch == -1 ? params.n_threads : params.n_threads_batch;

    llama_context * ctx = llama_new_context_with_model(model, ctx_params);
    llama_set_rng_seed(ctx, params.seed);

    if (ctx == NULL) {
        fprintf(stderr , "%s: error: failed to create the llama_context\n" , __func__);
        if (debug) {
            godot::UtilityFunctions::print("[GDLLM] failed to create llm context");
        }
        return "[GDLLM] failed to create llm context";
    }

    // tokenize the prompt

    std::vector<llama_token> tokens_list;
    tokens_list = ::llama_tokenize(ctx, params.prompt, true);

    const int n_ctx    = llama_n_ctx(ctx);
    const int n_kv_req = tokens_list.size() + (n_len - tokens_list.size());

    // make sure the KV cache is big enough to hold all the prompt and generated tokens
    if (n_kv_req > n_ctx) {
        if (debug) {
            godot::UtilityFunctions::print("[GDLLM] the required KV cache size is not big enough");
        }
        return "[GDLLM] the required KV cache size is not big enough";
    }

    // create a llama_batch with size 512
    // we use this object to submit token data for decoding

    llama_batch batch = llama_batch_init(512, 0);

    // evaluate the initial prompt
    batch.n_tokens = tokens_list.size();

    for (int32_t i = 0; i < batch.n_tokens; i++) {
        batch.token[i]  = tokens_list[i];
        batch.pos[i]    = i;
        batch.seq_id[i] = 0;
        batch.logits[i] = false;
    }

    // llama_decode will output logits only for the last token of the prompt
    batch.logits[batch.n_tokens - 1] = true;

    if (llama_decode(ctx, batch) != 0) {
        if (debug) {
            godot::UtilityFunctions::print("[GDLLM] llama_decode() failed");
        }
        return "[GDLLM] llama_decode() failed";
    }

    // main loop

    int n_cur    = batch.n_tokens;
    int n_decode = 0;

    const auto t_main_start = ggml_time_us();

    // a resizable array of tokens
    std::vector<llama_token> completions_list_tokens;

    while (n_cur <= n_len) {
        // sample the next token
        {
            auto   n_vocab = llama_n_vocab(model);
            auto * logits  = llama_get_logits_ith(ctx, batch.n_tokens - 1);

            std::vector<llama_token_data> candidates;
            candidates.reserve(n_vocab);

            for (llama_token token_id = 0; token_id < n_vocab; token_id++) {
                candidates.emplace_back(llama_token_data{ token_id, logits[token_id], 0.0f });
            }

            llama_token_data_array candidates_p = { candidates.data(), candidates.size(), false };

            // sample the most likely token
            // const llama_token new_token_id = llama_sample_token_greedy(ctx, &candidates_p);
            const llama_token new_token_id = llama_sample_token(ctx, &candidates_p);

            // is it an end of stream?
            if (new_token_id == llama_token_eos(ctx) || n_cur == n_len) {
                break;
            }

            // Add to the tokens list
            completions_list_tokens.push_back(new_token_id);

            // 1. Assemble the recent tokens into a string for checking stop sequences
            int window_size = std::min(static_cast<int>(completions_list_tokens.size()), longest_stop_sequence_string_length);
            std::string recent_tokens = "";
            for (int i = 0; i < window_size; i++) {
                recent_tokens = llama_token_to_piece(ctx, completions_list_tokens[completions_list_tokens.size() - 1 - i]).c_str() + recent_tokens;
            }

            // 2. Check against stop sequences
            for (int i = 0; i < stop_sequence.size(); i++) {
                std::string stop_seq = stop_sequence[i].utf8().get_data();
                if (recent_tokens.length() >= stop_seq.length() && 
                    recent_tokens.substr(recent_tokens.length() - stop_seq.length()) == stop_seq) {
                    if (debug) {
                        godot::UtilityFunctions::print("[GDLLM] stopping on stop sequence: ", stop_seq.c_str());
                    }
                    n_len = n_cur;  // set the target sequence length to current to stop further generation
                    break;
                }
            }

            // prepare the next batch
            batch.n_tokens = 0;

            // push this new token for next evaluation
            batch.token [batch.n_tokens] = new_token_id;
            batch.pos   [batch.n_tokens] = n_cur;
            batch.seq_id[batch.n_tokens] = 0;
            batch.logits[batch.n_tokens] = true;

            batch.n_tokens += 1;

            n_decode += 1;
        }

        n_cur += 1;

        // evaluate the current batch with the transformer model
        if (llama_decode(ctx, batch)) {
            fprintf(stderr, "%s : failed to eval, return code %d\n", __func__, 1);
            if (debug) {
                godot::UtilityFunctions::print("[GDLLM] failed to eval");
            }
            return "[GDLLM] failed to eval";
        }
    }

    const auto t_main_end = ggml_time_us();

    llama_batch_free(batch);

    llama_free(ctx);
    llama_free_model(model);

    llama_backend_free();

    // convert the strings in completion_list to a single string
    std::string completion_text = "";

    for (auto completion_token : completions_list_tokens) {
        std::string completion = llama_token_to_piece(ctx, completion_token).c_str();
        completion_text += completion;
    }

    // if completion_text ends with any of the strings in stop_sequence, remove it
    for (int i = 0; i < stop_sequence.size(); i++) {
        std::string stop_seq = stop_sequence[i].utf8().get_data();
        if (completion_text.length() >= stop_seq.length() && 
            completion_text.substr(completion_text.length() - stop_seq.length()) == stop_seq) {
            completion_text = completion_text.substr(0, completion_text.length() - stop_seq.length());
            break;
        }
    }

    // convert completion_text to Godot string
    // from https://ask.godotengine.org/110221/c-string-to-string
    const String godot_completion = completion_text.c_str();

    if (debug) {
        godot::UtilityFunctions::print("[GDLLM] completion: @@", godot_completion, "@@");
    }

    // emit a signal with the completion text
    emit_signal("completion_generated", godot_completion);

    return godot_completion;
}
