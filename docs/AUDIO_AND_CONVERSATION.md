# Audio and conversation flow

The demo has two audio paths: a prepared introduction and a dynamically generated answer.

## Startup introduction

The local character Blueprint calls `AnimateCharacterFromSoundWaveAsync` at BeginPlay, using the imported `/Game/TestAudio/english-e2e` SoundWave and `LocalA2F-Mark`.

The clip introduces Kagawa as a conversational virtual assistant. It was created through an earlier LLM/TTS run, then imported into Unreal. Starting the scene replays that clip and generates its facial animation; it does not make a new ASR, LLM, or TTS request.

These `/Game/` assets belong to the separate local MetaHuman project. Replacing the original WAV on disk does not automatically reimport the SoundWave.

## Dynamic question and answer

1. **Input:** `demo_client.py` reads a 16 kHz, mono, PCM16 WAV and sends 20 ms audio frames, followed by `mic.end`.
2. **ASR:** the backend buffers the utterance and transcribes it through Riva's `offline_recognize` call.
3. **LLM:** the system prompt and current transcript are sent to the configured model. Streamed answer text is assembled into sentences.
4. **TTS:** each sentence is synthesized. Its PCM audio is sent to the question client and Unreal observer.
5. **Audio bridge:** Unreal buffers all response PCM until `tts.end`, preserves its sample rate, and writes a temporary WAV.
6. **Animation and playback:** `MetaHumanTtsReceiver` submits the WAV to Audio2Face-3D using `LocalA2F-Mark`. The existing `ACEAudioCurveSource` supplies synchronized speech and facial curves to the MetaHuman.

Audio travels over the network in chunks, but the MetaHuman starts processing the answer only after the complete response has arrived. This is buffered playback, not continuous token-to-mouth animation.

## What the WebSocket does

WebSocket keeps a connection open for control events and audio frames. Both clients connect to the Python backend:

| Client | URL | Role |
|---|---|---|
| Question CLI | `ws://127.0.0.1:8080/ws/session?publish=metahuman-demo` | Sends the question and receives the generated answer |
| Unreal observer | `ws://127.0.0.1:8080/ws/session?observe=metahuman-demo` | Receives a copy of the response for avatar playback |

Connect the observer before submitting a question. `metahuman-demo` is a routing label, not conversation memory. Routing is held in process memory, so the backend must use one worker.

The CLI closes after one turn. Unreal can stay connected across successive CLI runs. Each run creates a new session, and the current LLM adapter sends only the system prompt and current question.

## Reading the states

| Signal | Meaning |
|---|---|
| `Observer connected and session ready` | Unreal is connected and ready to receive responses |
| `LISTENING` | The backend is ready for input |
| `THINKING` | The backend is processing the question |
| `SPEAKING` / `tts.start` | Response speech is being generated and transmitted; Unreal may still be buffering |
| `tts.end` | All response audio has been sent; Unreal can submit its WAV to ACE |
| `LISTENING` with `turn_complete` | Backend generation is finished; avatar playback may still be active |
| `Response completed: ACE synchronized audio and animation playback ended.` | Unreal's receiver has observed both audio-send completion and playback end |

**Wait for the last message and the visible/audible end of the answer before sending the next question.** Backend completion and CLI exit do not acknowledge Unreal playback. There is no playback-aware question queue; the receiver discards overlapping responses.

`RESPONDING` is a viewer-facing label used in the demo video. The backend's corresponding state is named `SPEAKING`; the two labels should not be confused with the actual start of avatar playback.

## Audio files

| File | Role |
|---|---|
| Your question WAV | CLI input: 16 kHz, mono, PCM16 |
| CLI `--output` WAV | Generated answer, at the received TTS sample rate; a JSON report is saved beside it |
| Imported introduction SoundWave | Prepared startup speech in the local Unreal project |
| Unreal `Saved/ACEConversation/<GUID>.wav` | Temporary complete-response input to Audio2Face-3D |

The demonstrated English TTS configuration uses 44.1 kHz mono PCM16. Completed audio submissions remove their temporary WAV; interrupted runs may leave files in `Saved`.

The WAV client does not resample input or play it through speakers. `--realtime` paces the upload; it does not enable microphone capture. `--generate-input` creates a fixed test question through TTS and can replace `demo-input.wav` beside the selected output. Use `--wav` for your own recordings.

## Implementation references

| Responsibility | Source |
|---|---|
| WAV validation and CLI | [demo_client.py](../services/orchestrator/tools/demo_client.py) |
| Endpoints and response routing | [main.py](../services/orchestrator/app/main.py), [response_mirror.py](../services/orchestrator/app/response_mirror.py) |
| Turn lifecycle and synthesis queue | [session.py](../services/orchestrator/app/session.py) |
| Service adapters | [asr.py](../services/orchestrator/app/adapters/asr.py), [llm.py](../services/orchestrator/app/adapters/llm.py), [tts.py](../services/orchestrator/app/adapters/tts.py) |
| Unreal transport | [ConversationBridgeComponent.cpp](../unreal/Plugins/ACEConversation/Source/ACEConversation/Private/ConversationBridgeComponent.cpp) |
| WAV buffering and playback completion | [MetaHumanTtsReceiver.cpp](../unreal/Plugins/ACEConversationMetaHuman/Source/ACEConversationMetaHuman/Private/MetaHumanTtsReceiver.cpp) |

For operational steps, see the [demo guide](DEMO_GUIDE.md).
