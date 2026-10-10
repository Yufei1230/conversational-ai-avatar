# Validation record and known limits

Reviewed against local source, installed plugin source, configuration templates, saved test reports, and the existing MetaHuman project on **2026-10-10**. This is an evidence summary, not a claim that every feature was retested with live services.

## Evidence matrix

| Area | Evidence | What it establishes | What it does not establish |
|---|---|---|---|
| Python backend/client tests | Rerun on 2026-10-10: **30 tests passed** | Protocol, state flow, mock response routing, PCM conversion and client handling | Live service availability, physical microphone capture, perceptual lip sync |
| Python dependencies | Rerun `pip check`: no broken requirements | Installed package dependency consistency | A reproducible locked clean-machine environment |
| Installed plugin agreement | Both repository plugin `Source` trees compared byte-for-byte with the installed project; no differences | Documentation refers to the installed bridge/receiver source | That every local binary was rebuilt during this review |
| Unreal WAV adapter | Saved 2026-09-28 `ACE.MetaHuman.BufferedWav`: Success, 0 errors/warnings | Malformed PCM rejection; WAV payload/rate preservation at 16/24/44.1/48 kHz | GPU inference or audiovisual quality; test used NullRHI |
| Real backend ? Unreal responses | Earlier reports/logs show real-provider response PCM, LocalA2F-Mark samples, playback start/completion; a second separately submitted response also completed | Actual buffered integration and repeated publisher turns | Same-session conversational memory or automatic playlist behavior |
| Startup introduction provenance | SoundWave import metadata, matching source WAV hash, and saved real-turn JSON | Imported earlier LLM/TTS response is the fixed startup clip | A new LLM response on each BeginPlay |
| Reviewed presentation | Saved fresh-load comparison reports identical composition/functional settings; actual 1920?1080 viewport and non-silent master-output recording | Presentation separation, camera, startup ACE playback and local capture path | A newly exercised ASR/LLM/TTS turn in the reviewed capture |
| Natural idle | More than 30 seconds of saved idle observation; body animation class/clip absent | Independent blink/gaze/head/body idle was not functioning in this setup | That MetaHuman or ACE cannot support those features |
| External video | Screenshot and local rendering/audio diagnostics exist | A clean game frame can be captured | A finished, independently checked encoded multi-question video |

The synthetic mirror test opens two **separate publisher connections** under one observer. It should not be cited as a persistent-session conversation-history test. The inspected LLM adapter sends exactly the system prompt and current user text.

## Local evidence locations

Generated reports are intentionally ignored and normally unavailable in a fresh clone. Paths below identify the actual inspected evidence, rather than broken public download links.

| Root | Relative evidence |
|---|---|
| Source checkout | `unreal/outputs/metahuman-project-tests/index.json` |
| Source checkout | `services/orchestrator/outputs/english-e2e.json` and source WAV |
| Source checkout | `services/orchestrator/outputs/metahuman-demo-response.json` and `metahuman-visual-response.json` |
| Source checkout | `services/orchestrator/outputs/audio-output-diagnostic-fixed.json` |
| Local MetaHuman project | `Saved/PresentationReview/validation.json` |
| Local MetaHuman project | `Saved/PresentationReview/final_capture_metrics.json` |
| Local MetaHuman project | `Saved/PresentationReview/preservation.json`, motion/sync summaries and source captures |
| Local MetaHuman project | `PRESENTATION_QUALITY_REVIEW.txt` |

The public [presentation screenshot](media/metahuman-presentation-1080.png) is copied from the reviewed game capture. Raw voice recordings, logs, credentials, third-party binaries, and Unreal character/map packages are not added as documentation artifacts.

## Interpreting the presentation measurements

The earlier local review sampled a **nine-second warmed-up interval** at native 1920?1080 with a 30 fps cap: mean 30.00 fps, 95th-percentile frame time 33.80 ms. These are a short observation on one installed machine, not a throughput benchmark or hardware requirement.

A stationary 12-frame comparison of 32 versus 16 TSR history samples measured lower temporal pixel standard deviation in selected regions: face 0.539 ? 0.388 and hair 0.854 ? 0.589 on an 8-bit pixel scale. This does not establish equivalent improvement during every animation or on other hardware.

The final known-speech check sampled 161 internal audio/animation clock pairs: mean difference 8.45 ms, maximum 31.61 ms. This is an internal engine timing diagnostic, **not** input-to-response latency or measured lip/audio offset in an externally encoded video. No general low-latency claim is made.

## Limitations and development work

| Limitation | Current behavior | Work required |
|---|---|---|
| Conversation history | Only system prompt + current transcript sent to LLM | Explicit message storage, session identity/lifecycle and context tests |
| Multi-question automation | One WAV and new session per CLI run; operator waits for Unreal | Playback-ended acknowledgement, queue, failure handling, persistent client if desired |
| Buffered playback | Utterance buffered for ASR; full TTS response buffered in Unreal | Incremental provider/integration work with timing and quality regression tests |
| Interruption | Busy MetaHuman receiver discards overlapping responses | Explicit cancellation/barge-in protocol and audio/animation cleanup |
| Endpointing | VAD can end an utterance after 500 ms silence | Test recordings and configurable UX; no change made in this update |
| Idle animation | No independent natural blink/gaze/body idle; settled ACE face may retain an open jaw | A safely blended listening/idle layer; preserve existing face Blueprint until tested |
| Connectivity | No automatic Unreal reconnect or replay; slow observers disconnect | Reconnect/state recovery and bounded playback queue |
| Deployment | In-process channel state; no application auth/TLS; Win64 receiver | Authentication, protected transport, shared routing for multiple workers, platform validation |
| Reproducibility | Local assets/models/configuration are external; dependencies use version ranges | Documented asset acquisition/setup and a validated clean-machine build |
| Capture | Internal clock checks and clean screenshots, not a finished video validation | Record and inspect actual encoded output, including RDP/audio-device behavior |

Existing logs include an ACE mapping-pose source-animation mismatch, NVIDIA CUDA/graphics scheduling fallback, and assumed 30 fps input timing. These were not repaired by the visual review. `DisableAllScreenMessages` hides overlays only. The face's existing minimum-LOD workaround is retained; forcing face LOD 0 produced an eye-rendering issue in the inspected project.

RAG/Tokkio and Japanese TTS experiments elsewhere in the repository are separate work. Their results do not establish that retrieval, contextual memory, or those voices are integrated into this Windows demonstration.

## Reproduce the offline Python checks

From `services/orchestrator`, with the existing environment:

```powershell
.\.venv\Scripts\python.exe -m unittest discover -s tests -v
.\.venv\Scripts\python.exe -m pip check
```

For a fresh test environment, install the backend plus `tools/live_mic_requirements.txt`; the tests import the microphone client's conversion dependencies even when no device is used. These tests use unittest, not pytest. Do not infer live NVIDIA service success from mock-based tests.

The [MetaHuman integration record](../unreal/METAHUMAN_TTS_DEMO.md) contains the previously executed module-limited Unreal build and NullRHI test commands. They were not rerun for this documentation-only update.

## Repository publication boundary

The local checkout still contains older Tokkio/Linux experiments and some previously tracked WAVs. This update does not remove that work or rewrite history. Runtime `.env` files, `outputs`, backend `runtime`, and Unreal generated directories remain ignored.

A prior repository audit found a credential-bearing `.env` in Git history; removing a file from the current index does not remove historical copies. This documentation update does not rotate credentials or rewrite Git history. Review that existing exposure before distributing the old history. A changes-only patch contains the selected updated files without transferring prior commits.

No license is invented for the project or its dependencies. NVIDIA/Epic model, engine, and character packages require their own acquisition and terms. No production-security, cross-platform, packaged-build, or external-video certification is claimed.
