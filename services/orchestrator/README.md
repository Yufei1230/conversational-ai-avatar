# Python conversation orchestrator

FastAPI backend for the NVIDIA ACE + MetaHuman demo. It accepts framed microphone/WAV audio, runs ASR ? LLM ? TTS, and returns text events and PCM audio. An optional publish/observe channel mirrors the response to Unreal.

For the configured MetaHuman scene, start with the [demo guide](../../docs/DEMO_GUIDE.md). For implementation details, see [audio and conversation flow](../../docs/AUDIO_AND_CONVERSATION.md).

## Setup

The existing Windows environment uses Python 3.11; the package declares Python ?3.10. From this directory, a **fresh-environment recipe** is:

```powershell
py -3.11 -m venv .venv
.\.venv\Scripts\python.exe -m pip install -e .
if (-not (Test-Path -LiteralPath .env)) {
    Copy-Item -LiteralPath .env.example -Destination .env
}
```

This installation recipe was not rerun on a clean machine during the documentation review. Keep the working environment and credentials on the configured demo machine.

Edit the local `.env` with your own provider access. [The template](.env.example) contains no keys. Settings load this file relative to the service directory, not an arbitrary shell working directory.

| Settings | Purpose |
|---|---|
| `ACE_NIM_API_KEY` | Credential used by the hosted LLM/TTS adapters |
| `ACE_ASR_API_KEY` | Optional separate ASR credential; falls back to `ACE_NIM_API_KEY` when empty |
| `ACE_ASR_SERVER`, `ACE_ASR_FUNCTION_ID`, `ACE_ASR_MODEL` | Recognition service/model selection |
| `ACE_ASR_USE_HOSTED_API`, `ACE_ASR_USE_SSL` | Hosted gRPC authorization and transport selection |
| `ACE_NIM_LLM_BASE_URL`, `ACE_NIM_LLM_MODEL` | OpenAI-compatible chat-completions endpoint and configured model |
| `ACE_TTS_USE_HOSTED_API`, `ACE_TTS_API_URL`, `ACE_TTS_VOICE` | Speech synthesis route and voice |
| `ASSISTANT_LANGUAGE`, `ASR_LANGUAGE`, `TTS_LANGUAGE` | Server-side language configuration; English is the default |
| `ACE_SYSTEM_PROMPT` | Optional prompt override; blank selects the configured language's default |
| `ACE_MOCK_ASR`, `ACE_MOCK_LLM`, `ACE_MOCK_TTS` | Keep false for real-provider demonstrations |
| `ACE_LOG_DIR`, `ACE_AUDIO_DIR`, `ACE_SAVE_DEBUG_AUDIO` | Transcript/timing logs and optional input/output audio storage |
| `ACE_VALIDATE_EXTERNALS_ON_STARTUP` | Optional startup service checks; false in the template |
| `ACE_SKIP_LLM_MODEL_VALIDATION` | Optional skip of model discovery; not proof a model works |

Endpoint/model names in the template reflect the implementation's configuration. Provider availability and account access must be checked with a real turn. A separate Japanese/Tokkio configuration elsewhere in this repository is not automatically part of the Windows demo.

```powershell
.\.venv\Scripts\python.exe tools/init_storage.py
.\.venv\Scripts\python.exe -m uvicorn app.main:app --host 127.0.0.1 --port 8080 --workers 1
```

This launch pattern has been used on the configured machine. Keep **one worker** for publish/observe routing. Uvicorn's explicit `--host`/`--port` arguments control this command; do not assume `ACE_HOST` overrides them.

## Interfaces

| Interface | Meaning |
|---|---|
| `GET /healthz` | Process liveness only |
| `GET /status` | Adapter/configuration checks; hosted ASR connectivity still requires an audio turn |
| `WS /ws/session` | One bidirectional conversation connection |
| `WS /ws/session?publish=metahuman-demo` | Conversation publisher whose responses are mirrored |
| `WS /ws/session?observe=metahuman-demo` | Receive-only observer; connect/register before a publisher |

The application does not provide authentication or TLS. Keep the demonstration on loopback or an appropriately protected private connection. Channel names route messages; they are not access controls or conversation histories.

Audio uses the repository's [ACE1 binary envelope](app/protocol.py): a 32-byte header and PCM16 little-endian payload. The WAV client sends 16 kHz mono frames at 20 ms per frame. The validated English TTS configuration returns 44.1 kHz mono PCM16. JSON messages include `session.start`, `mic.end`, ASR/LLM events, `tts.start`, `tts.end`, and state changes.

The Riva ASR adapter currently buffers input and invokes offline recognition. Hosted TTS returns each sentence's WAV before the adapter chunks its PCM. The Unreal MetaHuman receiver buffers the complete response before Audio2Face submission. Backend `turn_complete` is **not** a playback-finished acknowledgement.

## Prerecorded input

From this directory, after the Unreal observer is ready:

```powershell
$question = Join-Path $env:USERPROFILE 'Documents\VoiceQuestions\question1.wav'
$answer = Join-Path 'outputs' ('answer-' + [guid]::NewGuid().ToString('N') + '.wav')
.\.venv\Scripts\python.exe tools/demo_client.py `
    --url 'ws://127.0.0.1:8080/ws/session?publish=metahuman-demo' `
    --wav $question --require-real --output $answer
```

This custom-input recipe uses existing arguments; a personal recording is not supplied or validated by the repository. Input must already be PCM16, 16 kHz, mono. The client writes the answer WAV and JSON report, but does not play audio. `--realtime` paces upload only. `--locale` sets metadata and does not override server languages.

Each CLI run processes one WAV with a new session and closes after backend completion. The server state machine supports returning to listening, but the current LLM adapter sends no previous messages. For repeated questions, follow the [manual playback wait](../../docs/DEMO_GUIDE.md#6-wait-for-unreal-before-the-next-question).

`--generate-input` synthesizes the fixed introduction question and writes `demo-input.wav` beside the selected output. Avoid it when preserving the existing demo audio. `--mock-audio` is a synthetic protocol diagnostic, not microphone input or a real ASR quality test.

## Optional microphone client

[tools/LIVE_MIC_CLIENT.md](tools/LIVE_MIC_CLIENT.md) documents `live_mic_client.py`, its dependencies, device conversion, optional Unreal channel, and test-WAV mode. It is a separate one-turn capture client. Synthetic capture and mirrored PCM tests do not establish physical Mac microphone or long-running conversation validation.

## Tests and artifacts

For the existing environment (rerun 2026-10-10: **30 passed**, dependency check clean):

```powershell
.\.venv\Scripts\python.exe -m unittest discover -s tests -v
.\.venv\Scripts\python.exe -m pip check
```

A fresh test environment also needs the microphone conversion dependencies:

```powershell
.\.venv\Scripts\python.exe -m pip install -r tools/live_mic_requirements.txt
```

Tests use unittest with mock adapters and synthetic PCM. They do not consume hosted inference for an end-to-end quality check. The [validation record](../../docs/VALIDATION.md) separates unit tests from saved real-provider runs and Unreal rendering checks.

Ignored `runtime/logs` can contain transcripts and timing data; `runtime/audio` can contain personal voice recordings when debug audio is enabled. CLI output goes to `--output`; use an ignored `outputs` path and unique filenames. These artifacts are not needed for a public source release.

[WINDOWS_SETUP_REPORT.md](WINDOWS_SETUP_REPORT.md) and [COMMAND_LOG.md](COMMAND_LOG.md) retain historical setup evidence. Existing Linux/Riva/Tokkio experiments are documented under [infra](../../infra/compose/README.md); they are not required to operate the configured hosted-service Windows path.
