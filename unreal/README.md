# Unreal Engine integration

The demo uses **Unreal Engine 5.6.1 on Windows**, MetaHuman, NVIDIA ACE, and **LocalA2F-Mark**. This repository provides two project plugins. The character, presentation maps, NVIDIA packages, and model files are installed separately.

For day-to-day operation, follow the [demo guide](../docs/DEMO_GUIDE.md).

## Components

| Component | Responsibility |
|---|---|
| [ACEConversation](Plugins/ACEConversation) / `ConversationBridgeComponent` | WebSocket control events and ACE1 audio transport |
| [ACEConversationMetaHuman](Plugins/ACEConversationMetaHuman) / `MetaHumanTtsReceiver` | Buffers a full response, writes a WAV, and submits it to the existing ACE-enabled character |
| NVIDIA `ACEAudioCurveSource` | Plays speech and provides synchronized facial curves |
| MetaHuman Face AnimBP | Applies the ACE animation to the face |

The generic `ACEAudioPlaybackComponent` and microphone test actors support diagnostics. The final MetaHuman route uses ACE's own synchronized playback, so it does not need a second procedural speaker.

## Connect a MetaHuman project

1. Install Unreal Engine and compatible NVIDIA ACE base/Mark provider packages. The original setup used `NV_ACE_Reference` version `2.5.0-20250614-2282` and `NvAudio2FaceMark`.
2. Prepare an assembled MetaHuman with a working ACE face graph and `ACEAudioCurveSource`. Verify a known SoundWave before adding the conversation bridge.
3. Copy `Plugins/ACEConversation` and `Plugins/ACEConversationMetaHuman` into the target project's `Plugins` directory, then build the conversation modules for that engine installation. The MetaHuman receiver currently declares Win64 support.
4. In a separate demo level, place one `MetaHumanTtsReceiver` and assign the existing character.
5. Configure the receiver, start the backend, and enter Play/Simulate. Wait for observer readiness before submitting a question.

| Receiver property | Value |
|---|---|
| Target Character | The ACE-enabled MetaHuman actor in the current world |
| Provider Name | `LocalA2F-Mark` |
| Startup Delay Seconds | `25` by default |
| ConversationBridge Server URL | `ws://127.0.0.1:8080/ws/session?observe=metahuman-demo` |

The startup delay lets the prepared introduction finish; connection also waits while the receiver observes active playback. The [integration record](METAHUMAN_TTS_DEMO.md#build-and-validation-commands) contains the module-limited build and WAV-test commands used on the original workstation. This outline is not a validated clean-machine installation.

## Local demo scenes

The separate local project is named `MetaHumanTest`. Its scenes are not included in this checkout:

| Level | Purpose |
|---|---|
| `/Game/MetaHumanKnownGood_Preserved` | Preserved working setup |
| `/Game/MetaHumanConversationDemo` | Conversation and observer demo |
| `/Game/MetaHumanPresentation` | Studio presentation |
| `/Game/MetaHumanPresentation_Reviewed` | Reviewed presentation copy |

The presentation uses `Presentation_Camera` and an upper-body composition. The character's BeginPlay submits the imported `/Game/TestAudio/english-e2e` SoundWave to ACE. This is the prepared introduction, separate from dynamically generated answers.

## Playback and background operation

Expected logs include:

```text
Observer connected and session ready
TTS response started
ACE input created
ACE animation request started
Audio playback started
Response completed: ACE synchronized audio and animation playback ended.
```

Wait for the final completion message and the end of visible/audible playback before sending another question. Busy responses are discarded. Stop Play/Simulate to disconnect; restart it to reconnect after a backend restart.

For the local October 18 demo, disabling **Edit > Editor Preferences > Use Less CPU when in Background** resolved a facial-animation stall when switching to PowerShell. This is separate from `[Audio] UnfocusedVolumeMultiplier=1.0`, which keeps audio audible when Unreal loses focus. See [troubleshooting](../docs/troubleshooting/README.md) for both cases.

Independent blinking, gaze, and idle body motion remain future work. Existing warnings and the scope of prior rendering checks are summarized in [validation](../docs/VALIDATION.md).

## Project helpers

| Helper | Purpose |
|---|---|
| [preserve_metahuman_level.py](tools/preserve_metahuman_level.py) | Save the original working world without overwriting the preserved level |
| [prepare_metahuman_demo.py](tools/prepare_metahuman_demo.py) | Create and configure a conversation demo from the preserved level |
| [validate_metahuman_demo.py](tools/validate_metahuman_demo.py) | Inspect demo actors and start simulation |
| [write_conversation_module_manifests.ps1](tools/write_conversation_module_manifests.ps1) | Write metadata for the built conversation plugin DLLs |
| [inspect_metahuman_audio.py](tools/inspect_metahuman_audio.py) | Inspect local audio state |
| [enable_metahuman_background_audio.py](tools/enable_metahuman_background_audio.py) | Enable background audio for the editor session |

These helpers use the original project's asset names and paths. They are setup tools, not prerequisites for each demo run. Preserve a World Partition project's dependent external actor/object packages as well as its map.

## Earlier diagnostic records

- [Buffered TTS integration](METAHUMAN_TTS_DEMO.md)
- [Background audio](METAHUMAN_AUDIO_OUTPUT.md)
- [WebSocket audio tests](WEBSOCKET_AUDIO_TEST.md)
- [Microphone tests](MICROPHONE_TEST.md) and [RDP inspection](MICROPHONE_RDP_INSPECTION.md)
- [Play-in-Editor diagnostics](PIE_DIAGNOSTICS.md)

`ACEAvatarSandbox` is an earlier integration scaffold, not the completed MetaHuman demo.
