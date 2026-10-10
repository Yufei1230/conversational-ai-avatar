# Buffered backend TTS → existing MetaHuman ACE

If facial animation runs but backend audio is silent while the CLI has focus,
see [audio output diagnosis and project background-audio fix](METAHUMAN_AUDIO_OUTPUT.md).

This optional plugin reuses `ConversationBridgeComponent` and the installed NVIDIA
ACE 2.5 `UAsyncActionAnimateCharacter::AnimateCharacterFromWavFileAsync` API.
It buffers an entire ACE1 TTS response, writes PCM16 mono RIFF/WAV at the received
sample rate under `Saved/ACEConversation`, and submits it to `LocalA2F-Mark`.
NVIDIA decodes that WAV and calls `AnimateFromAudioSamples`; the character's existing
`ACEAudioCurveSource` plays the audio and supplies synchronized facial curves.
There is no second audio player, protocol change, microphone, or replacement anim graph.
Completed submissions delete their temporary WAV. An interrupted editor session can
leave a WAV in Saved; these are runtime artifacts, not assets to commit.

## Project and preserved levels

Project: `C:\Users\Administrator\Documents\Unreal Projects\MetaHumanTest\MetaHumanTest.uproject`.

Before installing the plugin, `tools/preserve_metahuman_level.py` saves the active
unsaved working world as `/Game/MetaHumanKnownGood_Preserved`. It refuses to overwrite
that backup. It does not edit the character Blueprint, Face_AnimBP, mapping pose,
provider configuration, or `/Game/TestAudio/english-e2e`.

After installation/restart, run `tools/prepare_metahuman_demo.py` in Unreal's Python
console to save a separate `/Game/MetaHumanConversationDemo` and add one receiver.
If the demo exists, it configures its single receiver without making another copy
or adding a duplicate. Open that demo level for this test.

The actor **MetaHuman TTS Observer** (`MetaHumanTtsReceiver`) should have:

- Target Character: the existing `BP_NewMetaHumanCharacter` instance.
- Provider Name: `LocalA2F-Mark`.
- Startup Delay Seconds: `25` (increase if the standalone startup test takes longer).
- ConversationBridge → Server URL: `ws://127.0.0.1:8080/ws/session?observe=metahuman-demo`.
- Is Spatially Loaded: matches the existing character (true in this map); the
  receiver sits at the character's location so they stream together.

The character remains `/Game/MetaHumans/NewMetaHumanCharacter/BP_NewMetaHumanCharacter`.
Its existing BeginPlay SoundWave test still runs. The receiver waits for its startup
delay and for observed existing playback to finish before connecting. Start the client
only after the observer-ready log. Do not trigger another standalone test while a
backend response is being sent. Overlapping responses are rejected, not queued.

These levels have already been created on this machine. For a repeatable setup in
the installed project, the editor console command is:

```text
py C:/Users/Administrator/Documents/tokkio/unreal/tools/prepare_metahuman_demo.py
```

With the demo level open and Play/Simulate stopped, this optional validation command loads its World Partition
actors, frames the existing character, and starts simulation without saving changes:

```text
py C:/Users/Administrator/Documents/tokkio/unreal/tools/validate_metahuman_demo.py
```

## Run (two PowerShell terminals)

Backend:

```powershell
Set-Location C:\Users\Administrator\Documents\tokkio\services\orchestrator
.\.venv\Scripts\python.exe -m uvicorn app.main:app --host 127.0.0.1 --port 8080 --workers 1
```

Retain the existing ignored local `.env` with the real ASR/LLM/TTS configuration.
Use one worker: observer routing is in-process. Both clients run on this Windows host.

Open the demo level and press Play. Wait for **Observer connected and session ready**,
then run:

```powershell
Set-Location C:\Users\Administrator\Documents\tokkio\services\orchestrator
.\.venv\Scripts\python.exe tools/demo_client.py --url "ws://127.0.0.1:8080/ws/session?publish=metahuman-demo" --wav outputs/demo-input.wav --require-real --output outputs/metahuman-demo-response.wav
```

`demo-input.wav` is the existing 16 kHz mono PCM16 prerecorded input. `--require-real`
checks the backend's real-provider settings. The WAV client must connect after the
observer; it publishes to the matching channel. Wait for Unreal **Response completed**
before starting another turn.

Expected Output Log sequence (`LogMetaHumanTts` and `LogACERuntime`):

1. Observer connected and session ready.
2. TTS response started; PCM bytes received, totals and sample rate.
3. ACE input created, then ACE animation request started (`LocalA2F-Mark`).
4. Native ACE session and animation/audio sample counts.
5. Audio playback started; ACE audio send completed successfully.
6. Response completed after synchronized playback ends.

ACE send completion and playback start can occur in either order. Send completion
alone is not evidence that playback finished. The visible confirmation is that the
existing MetaHuman mouth moves with the newly returned spoken answer, after the
unchanged startup sample has finished. The client also saves that answer as a WAV.

Errors are explicit for missing target/provider, invalid/mixed-rate PCM, responses
larger than 64 MiB, overlapping playback, failed WAV writes, and failed ACE requests.
The existing bridge reports disconnect codes and reasons. Re-enter Play to reconnect
after a connection failure; this demo does not add automatic reconnect or VAD.

## Build and validation commands

Install `Plugins/ACEConversation` and `Plugins/ACEConversationMetaHuman` into the
project's Plugins directory. Keep installed `NV_ACE_Reference` and `NvAudio2FaceMark`.
Close the editor only after saving the working map; Live Coding otherwise blocks build.

```powershell
& 'C:/Program Files/Epic Games/UE_5.6/Engine/Build/BatchFiles/Build.bat' UnrealEditor Win64 Development '-Project=C:/Users/Administrator/Documents/Unreal Projects/MetaHumanTest/MetaHumanTest.uproject' -Module=ACEConversation -Module=ACEConversationMetaHuman -NoHotReload -NoUBTMakefiles -NoUBA
& C:/Users/Administrator/Documents/tokkio/unreal/tools/write_conversation_module_manifests.ps1 -ProjectDir 'C:/Users/Administrator/Documents/Unreal Projects/MetaHumanTest'

& 'C:/Program Files/Epic Games/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe' 'C:/Users/Administrator/Documents/Unreal Projects/MetaHumanTest/MetaHumanTest.uproject' -unattended -nop4 -NullRHI '-ExecCmds=Automation RunTests ACE.MetaHuman.BufferedWav' '-TestExit=Automation Test Queue Empty' '-ReportExportPath=C:/Users/Administrator/Documents/tokkio/unreal/outputs/metahuman-project-tests' '-abslog=C:/Users/Administrator/Documents/tokkio/unreal/outputs/metahuman-project-tests.log'

Set-Location C:\Users\Administrator\Documents\tokkio\services\orchestrator
.\.venv\Scripts\python.exe -m unittest discover -s tests -v
```

The C++ test verifies rejection of malformed PCM and WAV sample-rate/payload
preservation for 16, 24, 44.1 and 48 kHz. The existing Python tests cover ACE1 and
mirrored synthetic PCM. Real GPU inference and visible lip sync require the editor
demo, not the NullRHI unit test.

Use the module-limited build above: a full `UnrealEditor` build on this installed
engine attempts to relink Epic MetaHuman modules whose complete implementation
sources are not shipped, producing unresolved symbols. Module-limited builds omit
UBT's metadata action; the script writes manifests only for the two new plugin DLLs,
using the installed engine BuildId. It does not change NVIDIA/engine manifests.

## Validation on this machine (2026-09-28)

- Module-limited build: succeeded, including UnrealHeaderTool and both plugin DLLs.
- `ACE.MetaHuman.BufferedWav`: passed in both an isolated host and MetaHumanTest
  itself; the project command above exited 0.
- Backend `unittest discover`: all 30 tests passed. An initial `pytest` attempt
  could not run because pytest is not installed; this repository's suite uses unittest.
- Unchanged standalone SoundWave: LocalA2F-Mark returned 468 animation samples and
  343,980 audio samples in the copied level.
- Real prerecorded round trip, first response: 471,480 PCM bytes at 44,100 Hz;
  LocalA2F-Mark returned 320 animation samples and 235,199 audio samples. The
  observer logged successful audio send, playback start, and response completion.
- Second real turn (same command with output `outputs/metahuman-visual-response.wav`):
  491,520 input PCM bytes; 334 animation samples and 245,488 output audio samples;
  synchronized playback completed. Both completed temporary ACE input WAVs were deleted.
- The captured viewport was a distant side view. A close-up visual/audio sync check
  remains manual; sample counts alone do not establish perceptual lip-sync quality.

The initial full-project build attempted to relink binary-distributed Epic modules
and failed. Fourteen engine DLLs were restored from Epic's official CDN and verified
against the installed engine manifest before the successful project runtime/tests.
No launcher repair action is still required. Use the limited build command above.

Repo files added: this guide; `ACEConversationMetaHuman.uplugin`; its module
`ACEConversationMetaHuman.Build.cs`, `Public/MetaHumanTtsReceiver.h`,
`Private/MetaHumanTtsReceiver.cpp`, `Private/PcmWav.h`, `Private/PcmWavTests.cpp`;
`tools/preserve_metahuman_level.py`, `tools/prepare_metahuman_demo.py`,
`tools/validate_metahuman_demo.py`, and `tools/write_conversation_module_manifests.ps1`.
`unreal/README.md` links this guide. The existing generic bridge, microphone code,
demo client, backend protocol, and NVIDIA plugin sources were not edited.

Project-local additions: `Plugins/ACEConversation`, `Plugins/ACEConversationMetaHuman`,
the two saved maps and their World Partition external actor/object packages. Generated
build outputs, repair tooling, logs, screenshots, and response WAVs remain outside the
source change list. The `.uproject`, existing character assets, Face_AnimBP, mapping
pose, provider configuration, and original SoundWave were not edited.
