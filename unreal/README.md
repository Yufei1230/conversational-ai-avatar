# Unreal Engine integration

The current demonstration uses an existing **Unreal Engine 5.6.1 MetaHuman project** with NVIDIA ACE and `LocalA2F-Mark`. The source repository supplies two project plugins; it does not contain the finished character, presentation maps, NVIDIA model packages, or an Unreal Engine installation.

For daily operation, follow the [demo guide](../docs/DEMO_GUIDE.md). For the precise audio/session lifecycle, see [audio and conversation flow](../docs/AUDIO_AND_CONVERSATION.md).

## Two plugins, distinct responsibilities

| Plugin / component | Role |
|---|---|
| [ACEConversation](Plugins/ACEConversation) / `ConversationBridgeComponent` | WebSocket JSON/ACE1 transport, connection and TTS delegates |
| `ACEAudioPlaybackComponent` and microphone/test actors | Generic procedural playback and input diagnostics; not the final MetaHuman speaker path |
| [ACEConversationMetaHuman](Plugins/ACEConversationMetaHuman) / `MetaHumanTtsReceiver` | Receives one complete TTS response, writes a temporary WAV, submits it to the existing NVIDIA character |
| Installed NVIDIA `ACEAudioCurveSource` | Plays ACE audio and supplies synchronized facial curves to the existing face graph |

The receiver uses NVIDIA's `AnimateCharacterFromWavFileAsync`, the `LocalA2F-Mark` provider, and the character's current animation setup. It does not replace a face animation Blueprint or play a second copy through `ACEAudioPlaybackComponent`.

## Existing local project and maps

Project: `C:\Users\Administrator\Documents\Unreal Projects\MetaHumanTest\MetaHumanTest.uproject`.

| Level | Purpose |
|---|---|
| `/Game/MetaHumanKnownGood_Preserved` | Preserved original working setup |
| `/Game/MetaHumanConversationDemo` | Functional conversation/observer demo |
| `/Game/MetaHumanPresentation` | Original studio presentation, preserved |
| `/Game/MetaHumanPresentation_Reviewed` | Separate visual-review copy with matching composition and functionality |

These are local assets, not files in a fresh clone. The intended presentation camera is **Presentation_Camera**; pilot it for preview, or use the existing Player 0 activation on Play.

The existing character BeginPlay still submits the imported `/Game/TestAudio/english-e2e` SoundWave to ACE. This is a fixed earlier generated introduction, not a new backend conversation. The TTS observer waits at least 25 seconds and for observed startup playback to finish before connecting.

Receiver settings:

```text
Target Character: existing BP_NewMetaHumanCharacter in this world
Provider Name: LocalA2F-Mark
Startup Delay Seconds: 25
ConversationBridge Server URL:
ws://127.0.0.1:8080/ws/session?observe=metahuman-demo
```

For World Partition, the receiver matches the character's location and spatial-loading policy. The setup helper configures this explicitly.

## Integration on another project

This is a setup outline, **not a clean-machine validation**:

1. Install the required Unreal Engine and compatible NVIDIA ACE base/Mark provider packages through their respective distribution channels. The inspected project has `NV_ACE_Reference` 2.5.0 and `NvAudio2FaceMark`.
2. Prepare an assembled MetaHuman whose existing ACE facial graph and `ACEAudioCurveSource` already work with a known SoundWave. Preserve that working project before adding the bridge.
3. Copy the repository's `Plugins/ACEConversation` and `Plugins/ACEConversationMetaHuman` into the target project's `Plugins` directory. Do not overwrite the NVIDIA plugins or existing animation assets.
4. Build the conversation modules for the installed engine, then reopen the project. The MetaHuman adapter currently declares Win64 support.
5. Create a separate demo level, place one `MetaHumanTtsReceiver`, assign the existing character, and configure the observer URL/provider. Establish the camera/background separately.
6. Start the backend, Play the level, and wait for the observer-ready log before publishing one prepared question.

The [dated integration record](METAHUMAN_TTS_DEMO.md#build-and-validation-commands) contains the previously successful **module-limited** build and WAV unit-test commands. A full project build in that installed engine attempted to relink unavailable Epic implementation modules and failed; do not substitute it casually for the recorded module-limited build.

The manifest helper writes compatibility metadata only for the two compiled conversation plugin DLLs. It does not install NVIDIA models or make a different engine/plugin combination compatible.

## Helpers and preservation

| Helper | Scope |
|---|---|
| [preserve_metahuman_level.py](tools/preserve_metahuman_level.py) | One-time preservation of the original unsaved world; refuses to overwrite the known-good level |
| [prepare_metahuman_demo.py](tools/prepare_metahuman_demo.py) | Uses the known-good level to create/configure the separate conversation demo |
| [validate_metahuman_demo.py](tools/validate_metahuman_demo.py) | Loads demo actors, frames the character, and starts simulation; not a presentation-camera setup |
| [write_conversation_module_manifests.ps1](tools/write_conversation_module_manifests.ps1) | Metadata after the recorded module-limited build |
| [inspect_metahuman_audio.py](tools/inspect_metahuman_audio.py) | Read-only audio-state probe with ignored JSON output; Windows/UE-version-specific native exports |
| [enable_metahuman_background_audio.py](tools/enable_metahuman_background_audio.py) | Optional session change matching the existing background-audio policy |

These helpers are tailored to the local project's names and setup. Do not rerun preservation/setup scripts merely to launch an already configured demo. They are not a general MetaHuman asset installer.

World Partition maps include external actors/objects; preserve the whole project or all dependent packages, not just one `.umap`. The repository documentation update leaves every local map, character/animation asset, provider configuration, and communication implementation unchanged.

## Playback, troubleshooting, and capture

Expected runtime progression:

```text
Observer connected and session ready
TTS response started
ACE input created
ACE animation request started
Audio playback started
Response completed: ACE synchronized audio and animation playback ended.
```

Audio-send completion can precede or follow playback start. Submit the next question only after the final completion message, not after backend `turn_complete`. Busy responses are discarded rather than queued; reconnect by re-entering Play after failures.

The existing project's `[Audio] UnfocusedVolumeMultiplier=1.0` prevents the CLI taking focus from muting Unreal. See [background audio diagnosis](METAHUMAN_AUDIO_OUTPUT.md) for the evidence and local restart/runtime options.

The local `LaunchPresentationCapture.ps1` opens the reviewed map as a native 1920?1080 game window. It lives beside `MetaHumanTest.uproject`, outside this source repository. [Capture instructions](../docs/DEMO_GUIDE.md#7-capture-a-clean-19201080-presentation) explain clean framing, question audio, and external recorder verification.

Independent natural blinking/gaze/body idle is not active in the inspected scene. Existing warning messages remain in logs. The [validation record](../docs/VALIDATION.md) explains these limits and the separate startup, real-response, and rendering evidence.

## Generic diagnostics and older scaffold

- [WEBSOCKET_AUDIO_TEST.md](WEBSOCKET_AUDIO_TEST.md): generic bridge/procedural audio checks.
- [MICROPHONE_TEST.md](MICROPHONE_TEST.md): microphone implementation and automated tests.
- [MICROPHONE_RDP_INSPECTION.md](MICROPHONE_RDP_INSPECTION.md): remote-desktop device inspection.
- [PIE_DIAGNOSTICS.md](PIE_DIAGNOSTICS.md): editor diagnostics.
- `ACEAvatarSandbox`: integration scaffold whose placeholder character is not the finished ACE MetaHuman demo.

Do not interpret generic procedural-audio tests as proof of the MetaHuman's audiovisual synchronization or follow older scaffold instructions to replace the working character graph.
