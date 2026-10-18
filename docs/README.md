# Documentation

| Guide | Use it to |
|---|---|
| [Demo guide](DEMO_GUIDE.md) | Start the system, submit two questions, and record the demonstration |
| [Audio and conversation flow](AUDIO_AND_CONVERSATION.md) | Understand the introduction, dynamic responses, WebSocket routing, and playback timing |
| [Python backend](../services/orchestrator/README.md) | Install dependencies and configure speech and language services |
| [Unreal integration](../unreal/README.md) | Connect the bridge to an existing ACE-enabled MetaHuman |
| [Troubleshooting](troubleshooting/README.md) | Diagnose LLM timeouts, a stationary face, missing audio, and busy responses |
| [Validation and known limits](VALIDATION.md) | Distinguish repository tests from local runtime observations |

## Technical references

- [Protocol and architecture](architecture/ace-sandbox.md)
- [October 17 LLM timeout and recovery](troubleshooting/2026-10-17-hosted-llm-timeout-and-recovery.md)
- [Microphone client](../services/orchestrator/tools/LIVE_MIC_CLIENT.md)

## Earlier integration records

These files retain dated commands and results from the original workstation. Local paths and generated reports are not part of a fresh checkout.

- [Buffered MetaHuman TTS integration](../unreal/METAHUMAN_TTS_DEMO.md)
- [Background audio diagnosis](../unreal/METAHUMAN_AUDIO_OUTPUT.md)
- [WebSocket audio tests](../unreal/WEBSOCKET_AUDIO_TEST.md)
- [Microphone tests](../unreal/MICROPHONE_TEST.md) and [RDP device inspection](../unreal/MICROPHONE_RDP_INSPECTION.md)
- [Play-in-Editor diagnostics](../unreal/PIE_DIAGNOSTICS.md)
