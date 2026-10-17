# Incident report: NVIDIA Hosted LLM timeouts and demo recovery (2026-10-17)

**Project:** NVIDIA ACE + Unreal Engine MetaHuman Conversational AI Avatar  
**Environment:** AWS EC2 Windows workstation, Unreal Engine 5.6.1, Python FastAPI orchestrator, NVIDIA-hosted ASR/LLM/TTS, local NVIDIA ACE Audio2Face (LocalA2F-Mark)  
**Status:** **LLM pipeline restored using NVIDIA-hosted GPT-OSS 20B streaming.** The underlying cause of the original Nemotron endpoint failures is **not confirmed**. Unreal playback was subsequently observed to work, but rapid consecutive responses exposed a separate playback-busy limitation.

## Summary

On October 17, prerecorded questions that had worked on October 10 stopped producing spoken answers. The backend accepted WAV input and returned the correct ASR transcript, then entered `THINKING` and waited around 120 seconds before emitting `turn_failure` with an empty message. An earlier attempt also reported WebSocket `1011 keepalive ping timeout`. The fault was isolated to the NVIDIA-hosted LLM inference request, before TTS began. The MetaHuman character and ACE playback were independently operational.

Investigation compared the existing `nvidia/nemotron-3.5-lightning-30b-a3b` model with other hosted models using the **same existing API key**. The original model repeatedly timed out in both streaming and non-streaming requests. A tested replacement, `openai/gpt-oss-20b`, succeeded in both modes and was selected as the active streaming LLM. No API-key rotation or Unreal Engine asset changes were needed.

## Symptoms and initial evidence

The real WAV client received:

```text
asr.final: "What is the capital of japan?"
state: THINKING
... approximately 120 seconds ...
error: {"code":"turn_failure","message":"","recoverable":true}
```

Another test produced a client `ConnectionClosedError: received 1011 ... keepalive ping timeout`. The backend separately logged a `WebSocketDisconnect` while attempting to close an already-disconnected observer in `app/response_mirror.py`. These connection/cleanup errors were not established as the root cause: the LLM failure was reproduced without Unreal attached and without a WebSocket disconnect.

A code review identified an **HTTP read timeout of 120 seconds** for the LLM adapter in `app/adapters/llm.py`. The error payload was empty because `app/session.py` forwarded `str(exc)`, while `httpx.ReadTimeout` can have an empty string representation. A direct traceback for the exact October timeout was unavailable, so the specific lower-level read exception remains an evidence-backed inference rather than a confirmed trace.

Historical comparison from the local diagnosis:

| Observation | Oct 10 successful runs | Oct 17 failing runs |
| --- | --- | --- |
| ASR final | 0.64–1.66 s | about 1.45–1.49 s |
| First LLM answer token | Present | Not received |
| TTS audio | Present | Not started |
| End-to-end | 1.84–3.59 s | failure after about 121.5 s |

## Diagnostic process

1. **Isolate from Unreal.** Repeat the WAV test without Unreal, confirming the problem occurs after ASR and before TTS.
2. **Inspect settings and exceptions.** Compare Git-tracked code and existing `.env` modification history; locate the 120-second LLM read timeout and empty error-string behavior. No relevant code/configuration drift was identified.
3. **Verify access without assuming inference works.** Model discovery succeeded, and an independent NVIDIA-hosted TTS synthesis call returned HTTP 200 and a valid 44.1 kHz mono WAV (approximately 1.81 seconds of audio). This did not prove that chat inference worked.
4. **Probe the chat endpoint from regular Windows PowerShell.** The diagnostic environment used by Codex could not make outbound connections (`WinError 10013`); therefore the real calls were executed in an unrestricted PowerShell session on the AWS machine.
5. **Compare exact backend and official-example Nemotron payloads.** Both stalled for 120 seconds while awaiting response headers, despite completed TCP/TLS handshakes and request transmission. This did not identify a specific invalid request parameter.
6. **Compare models and streaming modes.** Gemma non-streaming answered successfully (about 24.1 seconds on the first test), while Gemma streaming and both original Nemotron modes timed out. A second run after stopping and starting EC2 reproduced the same general pattern: Gemma non-streaming succeeded (about 48.7 seconds), and streaming remained unsuccessful. The EC2 restart did not remedy the incident.
7. **Test additional hosted models with repeated requests.** A three-round model comparison identified a reliable replacement in the observed sample.

### Three-round hosted-model comparison

The results below come from `tools/Test-HostedLlms.ps1` and `outputs/llm-probe-20261017T061544589713Z.json` on the Windows workstation. **Success rates reflect only three requests per configuration, not guaranteed service reliability.** Latency figures are medians across successful attempts.

| Profile | Mode | Successes | Median total | Median first answer text |
| --- | --- | ---: | ---: | ---: |
| Original Nemotron 3.5 Lightning (instruct profile) | Non-streaming | 0/3 | — | — |
| Original Nemotron 3.5 Lightning (instruct profile) | Streaming | 0/3 | — | — |
| Nemotron 3 Nano Omni (default) | Non-streaming | 2/3 | 1,380 ms | — |
| Nemotron 3 Nano Omni (default) | Streaming | 2/3 | 1,816 ms | 1,650 ms |
| Nemotron 3 Nano Omni (instruct) | Non-streaming | 1/3 | 442 ms | — |
| Nemotron 3 Nano Omni (instruct) | Streaming | 3/3 | 500 ms | 285 ms |
| GPT-OSS 20B (default) | Non-streaming | **3/3** | 1,473 ms | — |
| GPT-OSS 20B (default) | Streaming | **3/3** | 1,455 ms | 1,346 ms |

Omni intermittently returned HTTP 503 or unsuccessful streamed responses. GPT-OSS 20B passed all six observed requests (three streaming, three non-streaming). The original Nemotron failed all six. Playground inference for the original model also succeeded separately, demonstrating only that the model was reachable through that interface; it did **not** establish that the application's API key, endpoint route, and request path were healthy.

## Mitigation implemented

The backend was switched to **NVIDIA Hosted API / `openai/gpt-oss-20b` / streaming mode**, retaining the existing API key. Codex reported these modifications in the local orchestrator working tree:

| File | Change |
| --- | --- |
| `.env` | Active LLM model changed (local only; **do not commit secrets**) |
| `.env.example`, `app/settings.py` | Updated default model |
| `app/adapters/llm.py` | GPT-OSS request settings and reasoning-content exclusion from TTS text; original Nemotron configuration retained |
| `tests/test_llm.py`, `tests/test_settings.py` | Streaming, reasoning, default-selection, and rollback coverage |
| `README.md` | Configuration, rollback, and validation notes |

**Validation reported by Codex:** 39 unit tests passed. A real WAV input produced the answer `Tokyo.` and a valid TTS output file, `outputs/gpt-oss-e2e.wav`. Publish/observe testing delivered matching PCM audio and completion events. Unreal playback was not part of that Codex validation, but the operator later observed ACE audio playback and animation samples in Unreal.

**Important separate limitation:** On rapidly consecutive turns, Unreal logged `busy: Previous ACE response or standalone test still active; discarding this response.` The backend may return to `LISTENING` before the local ACE playback finishes. Wait for `Response completed: ACE synchronized audio and animation playback ended` before submitting the next turn, or implement a bounded playback queue and completion-aware scheduling. This is **not** evidence that GPT-OSS inference failed.

## Recovery and verification steps

Working directory on the tested Windows workstation:

```powershell
Set-Location 'C:\Users\Administrator\Documents\tokkio\services\orchestrator'
```

1. Configure the active model in the **local, untracked** `.env`:

   ```dotenv
   ACE_NIM_LLM_MODEL=openai/gpt-oss-20b
   ```

2. Restart the orchestrator. Ensure the previous process on port 8080 is stopped first. If the original terminal was lost, inspect its command line before terminating it:

   ```powershell
   Get-NetTCPConnection -LocalPort 8080 -State Listen |
       Select-Object LocalAddress, LocalPort, OwningProcess
   # Replace <PID> with the number from above:
   Get-CimInstance Win32_Process -Filter 'ProcessId=<PID>' |
       Select-Object ProcessId, Name, CommandLine
   # Only if confirmed as this orchestrator:
   Stop-Process -Id <PID>

   .\.venv\Scripts\python.exe -m uvicorn app.main:app --host 127.0.0.1 --port 8080 --workers 1
   ```

3. In Unreal, stop and re-enter **Play** after the backend restart. Wait for `Observer connected and session ready` on the `metahuman-demo` observer.
4. In a **second** PowerShell window, issue a real question:

   ```powershell
   Set-Location 'C:\Users\Administrator\Documents\tokkio\services\orchestrator'
   $answer = "outputs\demo-answer-$([guid]::NewGuid().ToString('N')).wav"

   .\.venv\Scripts\python.exe tools/demo_client.py `
       --url 'ws://127.0.0.1:8080/ws/session?publish=metahuman-demo' `
       --wav 'outputs/demo-question.wav' `
       --require-real `
       --output $answer
   ```

5. Confirm successful ASR transcription, LLM text, TTS WAV output, matching observer delivery, and Unreal ACE playback/lip sync. When testing multiple turns, wait for the Unreal playback-completion log before sending the next question.

### Rollback

To restore the prior configured model (once its hosted endpoint is healthy), change only the local `.env` and restart the orchestrator:

```dotenv
ACE_NIM_LLM_MODEL=nvidia/nemotron-3.5-lightning-30b-a3b
```

The original adapter-specific request settings were retained. For an unfamiliar hosted model, verify supported request parameters, reasoning fields, and streaming response parsing before changing the model identifier in `.env`.

## What remains uncertain

- **Unconfirmed underlying cause:** The original Nemotron timeouts could involve model-specific serving capacity, inference routing, entitlement, or network-path behavior. The captured requests did not return HTTP response headers within the timeout, so there was no `401`, `403`, `429`, or `5xx` response that conclusively identified a cause.
- **API key:** The existing key was active and capable of real inference with alternative models. No evidence justified rotating it as the fix.
- **AWS restart:** Stop/start did not resolve the symptoms. Avoid repeated restarts as a troubleshooting substitute, particularly where GPU instance restart capacity is constrained.
- **Streaming is not inherently broken for this machine or account:** GPT-OSS and Omni streaming calls subsequently succeeded. The issue depended on the tested model/path at the time.
- **MetaHuman playback scheduling:** Rapid sequential requests can be discarded while ACE is busy; this requires a separate fix and regression test.

## Security and repository hygiene

Do **not** commit `.env`, API tokens, private URLs with embedded credentials, session recordings with sensitive content, raw infrastructure credentials, or large generated audio files unless deliberately reviewed. Keep the production key outside Git. The probe JSON reports and recorded WAVs are local validation artifacts; review before publishing. This report includes no API-key values.

## Related paths

- Backend: `services/orchestrator/`
- LLM integration: `services/orchestrator/app/adapters/llm.py`
- Session handling: `services/orchestrator/app/session.py`
- Hosted-model probe: `services/orchestrator/tools/Test-HostedLlms.ps1` (local; may not yet be committed)
- Existing test instructions: `services/orchestrator/tools/HOSTED_LLM_TESTING.txt` (local; may not yet be committed)
- Unreal audio/playback flow: `docs/AUDIO_AND_CONVERSATION.md`

*Recorded from the operator's October 17, 2026 test output and Codex validation report; this document records observed evidence and recovery actions, not a vendor-confirmed outage diagnosis.*
