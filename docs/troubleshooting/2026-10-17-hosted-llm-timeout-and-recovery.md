# Hosted LLM timeout and recovery

**Date:** October 17, 2026. **Outcome:** the local demo recovered with NVIDIA-hosted GPT-OSS 20B. The cause of the original Nemotron failures remains unconfirmed.

## What happened

A previously working prerecorded question produced the correct ASR transcript, then stayed in `THINKING` for about 120 seconds and failed before TTS began:

```text
asr.final: "What is the capital of japan?"
state: THINKING
error: {"code":"turn_failure","message":"","recoverable":true}
```

The failure also occurred without Unreal connected. This isolated the failing stage to the hosted LLM request rather than the character or facial-animation pipeline. The adapter's 120-second read timeout was consistent with the observed delay; the exact lower-level exception was not captured for that turn.

A WebSocket keepalive timeout and observer cleanup error appeared in some attempts, but were not established as the cause of the LLM failure.

## What was tested

Using the same API key, the operator compared the original model with other hosted models in both streaming and non-streaming modes. Model discovery and an independent TTS request worked; restarting the workstation did not resolve the original inference failure.

The local three-round comparison recorded:

| Model / request profile | Mode | Successful requests | Median total time of successes |
|---|---|---:|---:|
| Nemotron 3.5 Lightning / instruct | Non-streaming | 0/3 | No successful result |
| Nemotron 3.5 Lightning / instruct | Streaming | 0/3 | No successful result |
| Nemotron 3 Nano Omni / default | Non-streaming | 2/3 | 1,380 ms |
| Nemotron 3 Nano Omni / default | Streaming | 2/3 | 1,816 ms |
| Nemotron 3 Nano Omni / instruct | Non-streaming | 1/3 | 442 ms |
| Nemotron 3 Nano Omni / instruct | Streaming | 3/3 | 500 ms |
| GPT-OSS 20B / default | Non-streaming | 3/3 | 1,473 ms |
| GPT-OSS 20B / default | Streaming | 3/3 | 1,455 ms |

These small samples describe that day's requests, not service reliability or full avatar response latency. Some Omni requests returned HTTP 503. The original Nemotron requests did not return response headers within the timeout, so they did not identify a specific authorization, capacity, or request-format error.

## Local recovery

The Windows deployment selected this model in its untracked `.env`:

```dotenv
ACE_NIM_LLM_MODEL=openai/gpt-oss-20b
```

The local recovery report also records model-specific request handling and reasoning-text filtering in `app/adapters/llm.py`, updated defaults, and additional tests. It reports **39 tests passed**, a real WAV answer of `Tokyo.`, and matching PCM/completion delivery to an observer. The same API key was retained.

**Repository difference:** those GPT-OSS adapter/default/test changes are not present in this GitHub snapshot. Its `llm.py` still sends the earlier request format, `settings.py` and `.env.example` still select Nemotron, and `tests/test_llm.py` is absent. Changing the model name alone has not been validated here as an equivalent recovery. The 39-test result belongs to the separate local deployment.

For that configured deployment, restart Uvicorn after changing `.env`, reconnect Unreal by restarting Play/Simulate, and follow the [demo guide](../DEMO_GUIDE.md). Check the generated text and TTS file first, then verify avatar playback separately. The local backend validation itself did not rerun Unreal; subsequent operator logs showed ACE playback and animation samples.

## Lessons for later runs

- Check the failing stage separately: ASR success does not establish LLM inference health.
- Test a real chat request; `/models` only confirms discovery/access.
- Check model-specific request fields and streaming output before selecting another model.
- Keep previous working configuration values for rollback; do not assume the old endpoint has recovered.
- Wait for Unreal's playback-completion message between questions. The separately observed `busy` rejection is a playback-pacing issue.

See [troubleshooting](README.md) for background animation, audio, and connection issues.
