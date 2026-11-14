# Audio and conversation flow

[View the system diagram](assets/system.pdf).

1. The WAV client sends 20 ms frames of 16 kHz mono PCM16 audio over WebSocket, then sends `mic.end` to mark the end of the recording.
2. `ConversationSession` buffers the utterance for Riva ASR. The LLM receives the transcript and the configured system prompt. The backend synthesizes sentences as they become available.
3. `ResponseMirror` forwards the response to the matching Unreal observer while the publisher receives its own copy. Control events are JSON. Audio uses binary ACE1 frames with a 32-byte header and a PCM16 payload.
4. `ConversationBridgeComponent` parses the events and audio. `MetaHumanTtsReceiver` buffers PCM through `tts.end`, writes a temporary WAV at the TTS sample rate, and submits it to `LocalA2F-Mark`.
5. The MetaHuman's `ACEAudioCurveSource` plays speech with synchronized facial curves. The receiver removes the temporary WAV after the response finishes.

| Signal | What it means |
|---|---|
| `LISTENING` | The backend is ready for input. |
| `THINKING` | The backend is processing the question. |
| `SPEAKING` / `tts.start` | The backend is sending speech. Unreal may still be buffering. |
| `tts.end` | Unreal can submit the complete WAV to ACE. |
| `Response completed` | Unreal has observed playback completion. |

The observer connects first and stays connected across successive WAV submissions. Each submission opens a new conversation session. The channel identifies where to send the response. Unreal discards responses that overlap active playback.
