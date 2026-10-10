# Documentation

## Current Windows MetaHuman demonstration

1. [Demo guide](DEMO_GUIDE.md) ? operate the configured project, prepare voice questions, pace turns, and capture video.
2. [Audio and conversation flow](AUDIO_AND_CONVERSATION.md) ? startup SoundWave, dynamic WAV input, buffering, routing, and session/history behavior.
3. [Validation and limitations](VALIDATION.md) ? tested scope, saved evidence, known gaps, and future development.
4. [Unreal integration](../unreal/README.md) ? bridge versus MetaHuman receiver, required external assets, and preservation.
5. [Python backend](../services/orchestrator/README.md) ? configuration, setup, interfaces, and tests.

## Integration and diagnostic records

- [Buffered MetaHuman TTS integration](../unreal/METAHUMAN_TTS_DEMO.md) ? dated build/runtime record.
- [Background audio diagnosis](../unreal/METAHUMAN_AUDIO_OUTPUT.md) ? why facial animation could run while Unreal was muted.
- [Generic WebSocket audio tests](../unreal/WEBSOCKET_AUDIO_TEST.md) ? procedural playback diagnostics, separate from the final ACE route.
- [Microphone implementation/tests](../unreal/MICROPHONE_TEST.md) and [RDP inspection](../unreal/MICROPHONE_RDP_INSPECTION.md).
- [Remote microphone client](../services/orchestrator/tools/LIVE_MIC_CLIENT.md).
- [Windows setup report](../services/orchestrator/WINDOWS_SETUP_REPORT.md) ? historical configuration record.

## Related research and legacy deployment paths

These remain available as separate experiments. They are not prerequisites or claimed features of the current Windows presentation:

- [ACE sandbox architecture](architecture/ace-sandbox.md).
- [Tokkio reference stack](architecture/tokkio-reference-stack.md).
- [Tokkio rebuild runbook](operations/tokkio-rebuild-runbook.md).
- [Tokkio Japanese customization](operations/tokkio-japanese-customization.md).
- [Local RAG experiments](../infra/rag/README.md).
- [Japanese TTS experiments](../tts/README.md).

Older planning and diagnostic files describe their own dates and environments. Use the current demo guide and implementation sources for present behavior.
