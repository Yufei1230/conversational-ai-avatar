# Troubleshooting

Start with the symptom below. These notes combine the checked-in implementation with the October 17–18 local demo results.

| Symptom | First check |
|---|---|
| Correct transcript, then a long wait in `THINKING` | [Hosted LLM request](#llm-request-stalls) |
| Audio arrives but the face stops moving after switching windows | [Background CPU throttling](#face-stops-moving-in-the-background) |
| Face animates but speech is silent when Unreal loses focus | [Background audio](#animation-works-but-audio-is-silent) |
| A second answer is discarded as `busy` | [Playback completion](#second-response-is-discarded) |
| Unreal does not receive responses after restarting the backend | [Observer connection](#observer-does-not-reconnect) |

## LLM request stalls

If `asr.final` contains the expected question but no answer text or TTS follows, inspect the LLM stage before changing the MetaHuman setup. Model-list access and successful TTS requests do not prove that chat inference works.

On October 17, the configured Nemotron model repeatedly timed out. The local demo recovered using NVIDIA-hosted `openai/gpt-oss-20b` with the same API key and model-specific adapter changes. The provider-side cause was not confirmed.

See the [incident and recovery record](2026-10-17-hosted-llm-timeout-and-recovery.md) and [repository source status](../../services/orchestrator/README.md#model-configuration-and-source-status). The complete local recovery implementation is not yet published in this checkout.

## Face stops moving in the background

On October 18, speech and ACE samples arrived, but facial animation stalled when Unreal lost focus. Disabling background CPU throttling restored animation in the operator's test:

1. Open **Edit > Editor Preferences**.
2. Search for **Use Less CPU when in Background** and uncheck it.
3. Restart Play/Simulate, allow the introduction to finish, and wait for the observer-ready message.
4. Send a question, switch to PowerShell as usual, and check speech and facial movement.

The recorded failing interval ran at roughly 3 FPS. This supports background throttling as a contributor, but the active face graph's curve/pose evaluation was not captured. The successful workaround did not require changes to the Face AnimBP or character Blueprint.

## Animation works but audio is silent

Background muting is separate from CPU throttling. The configured project's `DefaultEngine.ini` uses:

```ini
[Audio]
UnfocusedVolumeMultiplier=1.0
```

Check the selected output device and Windows volume mixer as well. The [background audio record](../../unreal/METAHUMAN_AUDIO_OUTPUT.md) describes the earlier local diagnosis.

## Second response is discarded

The receiver logs:

```text
busy: Previous ACE response or standalone test still active; discarding this response.
```

Wait for the visible/audible answer to end and for:

```text
Response completed: ACE synchronized audio and animation playback ended.
```

Then resubmit the question. Backend `LISTENING`, `tts.end`, and CLI exit can all precede avatar playback completion. Discarded responses are not queued or replayed automatically.

## Observer does not reconnect

1. Start the backend with one worker.
2. Stop and restart Unreal Play/Simulate to reconnect the observer.
3. Check that the observer URL uses `?observe=metahuman-demo` and the question client uses `?publish=metahuman-demo`.
4. Wait for `Observer connected and session ready` before sending input.

Stopping Play/Simulate disconnects the observer. The current receiver does not automatically reconnect after a backend restart or a failed connection.
