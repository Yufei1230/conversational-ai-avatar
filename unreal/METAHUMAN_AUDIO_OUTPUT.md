# Mirrored TTS audio output diagnosis

The reproduced cause was Unreal's application-level background mute, not silent
PCM or a separate ACE audio route. The installed engine defaults to `[Audio]`
`UnfocusedVolumeMultiplier=0.0`. Running the prerecorded CLI, or focusing the other
open editor, put the receiving editor in the background. ACE continued generating
animation and reporting playback while the application's final volume was zero.

## Evidence before the fix

The latest saved question response was 44.1 kHz mono PCM16: 69,632 samples,
peak 0.456 and RMS 0.052 (-25.7 dBFS). A reproduced **actual temporary ACE input WAV**
contained 71,680 samples, peak 0.364 and RMS 0.0404 (-27.9 dBFS). Neither was silent.

Live inspection of the same character compared the startup SoundWave's
`AudioComponent_1` with the mirrored WAV's `AudioComponent_3`:

| Setting | Startup | Mirrored WAV |
|---|---|---|
| Source / procedural sound / component volume | 1 / 1 / 1 | 1 / 1 / 1 |
| Spatialization / attenuation override | false / false | false / false |
| Attenuation asset | None | None |
| Sound class override / sound class | None / None | None / None |
| Concurrency overrides | Empty | Empty |
| Explicit submix | None (default base submix enabled) | Same |
| World location | (17680, -2310, 0) | Same |
| Application focus during comparison | Foreground | Background |
| **FApp volume multiplier** | **1.0** | **0.0** |

Both paths call the installed NVIDIA runtime's `AnimateFromAudioSamples` and
feed the existing `ACEAudioCurveSource`. Its `CreateAudioComponent_GameThread`
creates a procedural sound on that world's audio device and attaches it to the
character component. The audio hardware log identifies **Remote Audio**. There
is no WAV-specific device, submix, sound class, concurrency, or playback-volume
branch. With attenuation disabled, distance to the character cannot explain silence.

## Minimal fix applied

Only this project configuration setting was added to MetaHumanTest's
`Config/DefaultEngine.ini`:

```ini
[Audio]
UnfocusedVolumeMultiplier=1.0
```

The original file is backed up locally at
`Saved/ACEConversation/DefaultEngine.before-background-audio.ini`. That backup
contains the original local configuration and must stay outside source control.
The setting affects all audio in this project while its window is unfocused;
component volumes and foreground behavior are unchanged. No engine-wide config,
character assets, animation setup, maps, protocol, or C++ modules were changed.
**No rebuild is necessary.**

FApp caches this setting. It was also applied to the receiving running editor
using the existing exported engine setter, without closing its unsaved level:

```text
py C:/Users/Administrator/Documents/tokkio/unreal/tools/enable_metahuman_background_audio.py
```

This optional script is Windows/UE 5.6 specific. A normal editor restart also reads
the saved project setting. Other already-running editor instances keep their own
cached value until restarted or given the same command. Two MetaHumanTest editors
were open during diagnosis; make sure to use the one with the TTS observer.

## Read-only diagnostic probe

In the receiving editor console, run:

```text
py C:/Users/Administrator/Documents/tokkio/unreal/tools/inspect_metahuman_audio.py
```

Then start Play/Simulate if stopped and run the prerecorded client. The probe logs
`METAHUMAN_AUDIO_PROBE` for 120 seconds and automatically unregisters itself. It
records FApp gain/focus, the ACE child AudioComponent state and routing settings,
and the amplitude of temporary WAVs before the adapter deletes them. It writes
`unreal/outputs/metahuman-audio-probe.json` in the repo's ignored output directory.
The probe changes no audio settings or scene assets; the enable script above is
the separate, explicit runtime setting change. Native symbol names are specific
to the inspected UE 5.6 Windows build.

Validation client command (separate output, existing demos preserved):

```powershell
cd C:\Users\Administrator\Documents\tokkio\services\orchestrator
.\.venv\Scripts\python.exe tools/demo_client.py --url 'ws://127.0.0.1:8080/ws/session?publish=metahuman-demo' --wav outputs/demo-question.wav --require-real --output outputs/audio-output-diagnostic-fixed.wav
```

Check the probe during **unfocused** playback: `app_volume=1.0`,
`unfocused_volume=1.0`, `playing=true`, and non-zero WAV peak/RMS. An
`Audio playback started` message alone does not establish audibility: the
application gain must also be non-zero. Final listening through the RDP client
remains a user-side check.

## Validation after the fix

The final real question returned "Tokyo is the capital of Japan." The actual
temporary WAV contained 143,360 PCM bytes at 44.1 kHz, peak 0.252 and RMS 0.0362
(-28.8 dBFS). During mirrored playback the receiving editor was **unfocused**,
but both `app_volume` and `unfocused_volume` were **1.0**, the AudioComponent was
playing and not virtualized, and its volume multiplier was 1.0. LocalA2F-Mark
reported 97 animation samples and 71,293 output audio samples.

The earlier `audio-output-diagnostic-after.wav` attempt ran while the observer was
stopped and is not integration evidence; the successful checked run is
`audio-output-diagnostic-fixed.wav`. No existing demo WAV was overwritten.
