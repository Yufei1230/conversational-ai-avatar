# Troubleshooting

| Symptom | Check |
|---|---|
| Backend fails to start | Run Uvicorn from `services/orchestrator` using the virtual environment. Check `/healthz` before opening Unreal. |
| Client rejects the question WAV | Export 16 kHz mono PCM16 and use a short recording with clearly audible speech. |
| No transcript | Check the Riva endpoint, function ID, credentials, and ASR language in `.env`. |
| Transcript appears, then the backend stalls | Check backend logs for an LLM error or timeout. Test an actual completion with your configured model and API key. |
| Text appears without an answer WAV | Check the TTS endpoint, voice, and credentials. |
| Answer WAV exists, but Unreal receives nothing | Use one backend worker. Check the matching publish/observe channel and restart Play after a backend restart. Wait for observer readiness before submitting. |
| Unreal receives audio but the face does not animate | Test a standalone SoundWave with `LocalA2F-Mark`. Check the receiver target, face graph, and `ACEAudioCurveSource`. |
| Animation stalls after switching windows | Disable **Use Less CPU when in Background** in Unreal Editor Preferences. |
| Speech is silent when Unreal loses focus | Check the Windows output device and volume mixer. Set `UnfocusedVolumeMultiplier=1.0` under `[Audio]` in the Unreal project's `Config/DefaultEngine.ini` if background muting is the cause. |
| Unreal reports `busy` | Wait for `Response completed` and resend the question. |
