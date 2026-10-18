# Validation and known limits

This page separates recorded local results from the source currently available on GitHub. The documentation was updated on **October 18, 2026**; that update did not rerun the Windows GPU demo or hosted services.

## Recorded results

| Date | Check | Result and scope |
|---|---|---|
| September 28 | Unreal `ACE.MetaHuman.BufferedWav` | Saved report passed PCM validation and WAV encoding checks. This NullRHI test did not run GPU facial animation. |
| September–October | Prerecorded WAV to MetaHuman | Local runs produced ASR transcripts, generated answers, TTS audio, LocalA2F-Mark samples, and playback-completion logs. Successive operator-paced questions were exercised. |
| October 10 | Python backend/client tests | 30 unittest tests passed in the existing environment; `pip check` reported no broken requirements. Tests cover protocol, session flow, response mirroring, and synthetic microphone conversion. |
| October 10 | Presentation review | The local studio scene was checked for framing, startup audio/animation, and a 1920 x 1080 capture. The public [screenshot](media/metahuman-presentation-1080.png) comes from that review. |
| October 17 | Local GPT-OSS recovery | The local report records 39 tests passed, a real WAV answer of `Tokyo.`, and matching publisher/observer PCM. The extra adapter changes and tests are not in this GitHub snapshot. See the [incident record](troubleshooting/2026-10-17-hosted-llm-timeout-and-recovery.md). |
| October 18 | Background animation workaround | The operator confirmed that disabling **Use Less CPU when in Background** restored facial animation when switching away from Unreal. This is a local workaround, not a confirmed general engine diagnosis. |

The 30-test repository baseline and the 39-test local GPT-OSS report describe different source states. Neither unit-test result establishes perceptual lip-sync quality or current provider availability.

## Reproduce the Python checks

From `services/orchestrator`, after installing the backend:

```powershell
.\.venv\Scripts\python.exe -m pip install -r tools/live_mic_requirements.txt
.\.venv\Scripts\python.exe -m unittest discover -s tests -v
.\.venv\Scripts\python.exe -m pip check
```

The microphone dependencies are needed for conversion tests even when no physical device is used. Tests use unittest, mock providers, and synthetic PCM; they do not call hosted inference to validate a complete conversation.

For Unreal build and WAV-test commands used on the original workstation, see the [integration record](../unreal/METAHUMAN_TTS_DEMO.md#build-and-validation-commands).

## Current limits

| Area | Current behavior |
|---|---|
| Reproducibility | Character assets, maps, NVIDIA packages, credentials, and local GPT-OSS adapter changes are separate from this checkout. A clean-machine reproduction has not been validated. |
| Input | Prerecorded WAV input is the demonstrated path. The microphone client and its synthetic tests do not establish a complete live microphone-to-avatar session. |
| Context | Each LLM request contains the system prompt and current question, with no previous turns. |
| Latency | ASR buffers the utterance; Unreal buffers the entire TTS response. There is no measured end-to-end latency benchmark. |
| Turn pacing | The operator waits for Unreal playback completion. There is no automatic queue, playback acknowledgement to the backend, or validated interruption workflow. |
| Animation | Speech-driven facial animation works locally; independent blinking, gaze, and idle body motion remain development goals. |
| Connectivity | The observer has no automatic reconnect or replay. Response routing is process-local and requires one backend worker. |
| Deployment | The MetaHuman receiver declares Win64 support. Packaged builds and other platforms are unverified. The WebSocket endpoint has no application authentication or TLS. |

Existing local logs also contain mapping-pose and NVIDIA scheduling warnings. Hiding screen messages does not resolve those warnings. Background CPU throttling and background audio muting are separate settings; see [troubleshooting](troubleshooting/README.md).

## Local evidence

Generated files are ignored and generally unavailable in a fresh checkout. Earlier records refer to:

- `unreal/outputs/metahuman-project-tests/index.json`
- `services/orchestrator/outputs/english-e2e.json` and its WAV
- `services/orchestrator/outputs/metahuman-demo-response.json`
- `services/orchestrator/outputs/gpt-oss-e2e.wav`
- The local Unreal project's `Saved/PresentationReview/` reports and captures

The earlier review's short 30 fps capture and internal audio/animation clock measurements are local diagnostics. They are not hardware requirements, input-to-answer latency, or an externally measured lip-sync score.
