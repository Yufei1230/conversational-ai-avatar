# Python conversation backend

FastAPI backend for the NVIDIA ACE + MetaHuman demo. It receives speech audio, runs ASR, LLM, and TTS, and returns text events and PCM audio. Publish/observe routing forwards the same response to Unreal.

For the full operating sequence, see the [demo guide](../../docs/DEMO_GUIDE.md).

## Setup

Python **3.10 or newer** is required; the local Windows demo uses Python 3.11. From this directory:

```powershell
py -3.11 -m venv .venv
.\.venv\Scripts\python.exe -m pip install -e .
if (-not (Test-Path -LiteralPath .env)) {
    Copy-Item -LiteralPath .env.example -Destination .env
}
```

Edit `.env` with your provider credentials and configuration. Settings load it relative to this service directory. Keep an existing working `.env` when updating the repository. A complete clean-machine reproduction is not yet validated.

| Setting | Purpose |
|---|---|
| `ACE_NIM_API_KEY` | Hosted LLM/TTS credential; also used for ASR when its separate key is empty |
| `ACE_ASR_API_KEY` | Optional separate ASR credential |
| `ACE_ASR_SERVER`, `ACE_ASR_FUNCTION_ID`, `ACE_ASR_MODEL` | Recognition endpoint and model |
| `ACE_ASR_USE_HOSTED_API`, `ACE_ASR_USE_SSL` | Hosted gRPC authorization and transport |
| `ACE_NIM_LLM_BASE_URL`, `ACE_NIM_LLM_MODEL` | Chat-completions endpoint and model |
| `ACE_TTS_USE_HOSTED_API`, `ACE_TTS_API_URL`, `ACE_TTS_VOICE` | Speech synthesis endpoint and voice |
| `ASSISTANT_LANGUAGE`, `ASR_LANGUAGE`, `TTS_LANGUAGE` | Language configuration; English is the default |
| `ACE_SYSTEM_PROMPT` | Optional assistant prompt override |
| `ACE_MOCK_ASR`, `ACE_MOCK_LLM`, `ACE_MOCK_TTS` | Keep all false for a real demonstration |
| `ACE_SAVE_DEBUG_AUDIO`, `ACE_LOG_DIR`, `ACE_AUDIO_DIR` | Optional recordings and runtime logs |

## Model configuration and source status

The October 17 local demo switched to NVIDIA-hosted **GPT-OSS 20B**. Its `.env` selects:

```dotenv
ACE_NIM_LLM_BASE_URL=https://integrate.api.nvidia.com/v1
ACE_NIM_LLM_MODEL=openai/gpt-oss-20b
```

That local deployment also has model-specific adapter changes and extra tests which have **not been published here**. This checkout's `.env.example` and `app/settings.py` still default to `nvidia/nemotron-3.5-lightning-30b-a3b`; `app/adapters/llm.py` retains the earlier request format. Setting the model identifier alone is not a verified reproduction of the local GPT-OSS recovery.

For a model change, update the local `.env`, verify the adapter's request parameters and streaming response handling for that model, then restart Uvicorn. Confirm an actual answer and TTS output. Model discovery through `/models` does not prove inference works. The [October 17 incident record](../../docs/troubleshooting/2026-10-17-hosted-llm-timeout-and-recovery.md) documents the tested local recovery.

## Run

```powershell
.\.venv\Scripts\python.exe tools/init_storage.py
.\.venv\Scripts\python.exe -m uvicorn app.main:app --host 127.0.0.1 --port 8080 --workers 1
```

Use **one worker** because response routing is held in process memory. Explicit Uvicorn arguments control the bind address and port. The application has no authentication or TLS; use loopback or a protected private connection.

| Interface | Purpose |
|---|---|
| `GET /healthz` | Process liveness |
| `GET /status` | Adapter/configuration checks; not a complete inference test |
| `WS /ws/session` | Bidirectional conversation |
| `WS /ws/session?publish=metahuman-demo` | Question client whose response is mirrored |
| `WS /ws/session?observe=metahuman-demo` | Receive-only Unreal observer |

## Send a question

After the Unreal observer is ready, submit your prepared recording:

```powershell
.\.venv\Scripts\python.exe tools/demo_client.py --url 'ws://127.0.0.1:8080/ws/session?publish=metahuman-demo' --wav outputs/question1.wav --require-real --output outputs/answer1.wav
```

Input must be **16 kHz, mono, PCM16 WAV**. The client saves the generated answer and a JSON report; it does not play the question or answer aloud. Existing output files at the same path are replaced.

Each invocation handles one question with a new session. The LLM has no earlier conversation history. Wait for Unreal's `Response completed: ACE synchronized audio and animation playback ended.` before the next question. See the [two-question walkthrough](../../docs/DEMO_GUIDE.md#5-wait-then-submit-question-2).

`--realtime` paces upload only. `--locale` supplies metadata; server languages come from `.env`. `--generate-input` creates a fixed test question and can overwrite `demo-input.wav` beside the selected output.

## Protocol and microphone support

[ACE1 framing](app/protocol.py) carries a 32-byte header and little-endian PCM16 payload. Input frames are 20 ms at 16 kHz mono. Output uses the actual TTS sample rate, 44.1 kHz in the demonstrated English configuration.

ASR buffers the utterance, while Unreal buffers the entire generated answer before Audio2Face submission. See [audio and conversation flow](../../docs/AUDIO_AND_CONVERSATION.md) for state and playback semantics.

The optional [microphone client](tools/LIVE_MIC_CLIENT.md) supports one-turn capture and test-WAV input. Synthetic conversion/routing tests do not validate a complete physical microphone-to-MetaHuman session.

## Tests and generated files

```powershell
.\.venv\Scripts\python.exe -m pip install -r tools/live_mic_requirements.txt
.\.venv\Scripts\python.exe -m unittest discover -s tests -v
.\.venv\Scripts\python.exe -m pip check
```

Tests use unittest with mock providers and synthetic PCM. [Validation](../../docs/VALIDATION.md) distinguishes the repository's 30-test baseline from the later 39-test local GPT-OSS report.

`runtime/logs`, `runtime/audio`, and `outputs` are ignored. Debug artifacts may contain transcripts and personal recordings. Keep credentials in the untracked `.env`.
