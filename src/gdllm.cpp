#include "common.h"
#include "llama.h"
#include "gdllm.h"

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void GDLLM::_bind_methods() {
    // ClassDB::bind_method(D_METHOD("get_prompt"), &GDLLM::get_prompt);
    // ClassDB::bind_method(D_METHOD("set_prompt", "p_amplitude"), &GDLLM::set_prompt);
    // ClassDB::add_property("GDLLM", PropertyInfo(Variant::STRING, "prompt"), "set_prompt", "get_prompt");

    // MethodInfo's first parameter will be the signal's name.
    // Its remaining parameters are PropertyInfo types which describe
    // the essentials of each of the method's parameters.
    // PropertyInfo parameters are defined with the data type of the parameter,
    // and then the name that the parameter will have by default.
    ADD_SIGNAL(MethodInfo("completion_generated", PropertyInfo(Variant::OBJECT, "node"), PropertyInfo(Variant::STRING, "completion_text")));

    ClassDB::bind_method(D_METHOD("run_completion", "prompt_from_godot"), &GDLLM::run_completion);
}

GDLLM::GDLLM() {
    // Initialize any variables here.
}

GDLLM::~GDLLM() {
    // Add your cleanup here.
}


void GDLLM::run_completion(const String& prompt_from_godot) {
    godot::UtilityFunctions::print("GDLLM prompt:\n", prompt_from_godot);
    gpt_params params;

    params.model = "mistral-7b-instruct-v0.1.Q5_K_M.gguf";

    // trying random stuff off the Internet: 
    // https://www.reddit.com/r/godot/comments/12u2l5i/gdextension_strings_gettings_mangledgarbage_from/
    CharString temp = prompt_from_godot.utf8();
    const char* p_prompt = temp.get_data();  // needs .c_str()?
    params.prompt = p_prompt;

    // // if the above doesn't work, try:
    // // https://ask.godotengine.org/138972/convert-godots-string-to-c-s-std-string
    // std::string stdstring_prompt(prompt_from_godot.utf8().get_data());
    // const char* p_prompt = stdstring_prompt.c_str();

    if (params.prompt.empty()) {
        params.prompt = "Hello my name is";
    }

    // total length of the sequence including the prompt
    const int n_len = 32;

    // init LLM

    llama_backend_init(params.numa);

    // initialize the model

    llama_model_params model_params = llama_model_default_params();

    // model_params.n_gpu_layers = 99; // offload all layers to the GPU

    llama_model * model = llama_load_model_from_file(params.model.c_str(), model_params);

    if (model == NULL) {
        fprintf(stderr , "%s: error: unable to load model\n" , __func__);
        godot::UtilityFunctions::print("GDLLM unable to load model");
        return;
    }

    // initialize the context

    llama_context_params ctx_params = llama_context_default_params();

    ctx_params.seed  = 1234;
    ctx_params.n_ctx = 2048;
    ctx_params.n_threads = params.n_threads;
    ctx_params.n_threads_batch = params.n_threads_batch == -1 ? params.n_threads : params.n_threads_batch;

    llama_context * ctx = llama_new_context_with_model(model, ctx_params);

    if (ctx == NULL) {
        fprintf(stderr , "%s: error: failed to create the llama_context\n" , __func__);
        godot::UtilityFunctions::print("GDLLM failed to create llm context");
        return;
    }

    // tokenize the prompt

    std::vector<llama_token> tokens_list;
    tokens_list = ::llama_tokenize(ctx, params.prompt, true);

    const int n_ctx    = llama_n_ctx(ctx);
    const int n_kv_req = tokens_list.size() + (n_len - tokens_list.size());

    // LOG_TEE("\n%s: n_len = %d, n_ctx = %d, n_kv_req = %d\n", __func__, n_len, n_ctx, n_kv_req);

    // make sure the KV cache is big enough to hold all the prompt and generated tokens
    if (n_kv_req > n_ctx) {
        // LOG_TEE("%s: error: n_kv_req > n_ctx, the required KV cache size is not big enough\n", __func__);
        // LOG_TEE("%s:        either reduce n_parallel or increase n_ctx\n", __func__);
        return;
    }

    // print the prompt token-by-token

    // fprintf(stderr, "\n");

    // for (auto id : tokens_list) {
    //     fprintf(stderr, "%s", llama_token_to_piece(ctx, id).c_str());
    // }

    // fflush(stderr);

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
        // LOG_TEE("%s: llama_decode() failed\n", __func__);
        return;
    }

    // main loop

    int n_cur    = batch.n_tokens;
    int n_decode = 0;

    const auto t_main_start = ggml_time_us();

    // a resizable array of std::string objects
    std::vector<std::string> completion_list;

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
            const llama_token new_token_id = llama_sample_token_greedy(ctx, &candidates_p);

            // is it an end of stream?
            if (new_token_id == llama_token_eos(ctx) || n_cur == n_len) {
                // LOG_TEE("\n");

                break;
            }

            // LOG_TEE("%s", llama_token_to_piece(ctx, new_token_id).c_str());
            // fflush(stdout);

            // TODO add this to an array?
            // llama_token_to_piece(ctx, new_token_id).c_str();
            // append the output of llama_token_to_piece to completion_list
            completion_list.push_back(llama_token_to_piece(ctx, new_token_id).c_str());

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
            godot::UtilityFunctions::print("GDLLM failed to eval");
            return;
        }
    }

    // LOG_TEE("\n");

    const auto t_main_end = ggml_time_us();

    // LOG_TEE("%s: decoded %d tokens in %.2f s, speed: %.2f t/s\n",
    //         __func__, n_decode, (t_main_end - t_main_start) / 1000000.0f, n_decode / ((t_main_end - t_main_start) / 1000000.0f));

    // llama_print_timings(ctx);

    // fprintf(stderr, "\n");

    llama_batch_free(batch);

    llama_free(ctx);
    llama_free_model(model);

    llama_backend_free();

    // convert the strings in completion_list to a single string
    std::string completion_text = "";

    for (auto completion : completion_list) {
        completion_text += completion;
    }

    // convert completion_text to Godot string
    // from https://ask.godotengine.org/110221/c-string-to-string
    const String godot_completion = completion_text.c_str();

    godot::UtilityFunctions::print("GDLLM completion:\n", godot_completion);

    // emit a signal with the completion text
    emit_signal("completion_generated", this, godot_completion);

    // free memory
    completion_list.clear();
    p_prompt = NULL;

    return;
}
