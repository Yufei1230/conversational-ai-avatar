# Demo guide: prerecorded voice ? MetaHuman

Use this workflow for the existing Windows presentation. It requires no backend, protocol, animation-Blueprint, or audio-asset changes.

## Command status and paths

**Previously validated:** the existing backend launch, WAV publish command, observer integration, Unreal buffered-WAV test, and local capture launcher have saved execution evidence. They were not all rerun during this documentation update.

**Checked during this update:** the Python test suite, dependency consistency, input-WAV format, CLI arguments, source agreement with installed Unreal plugins, and PowerShell example syntax.

**Recipe, not end-to-end tested:** clean-machine installation, new personal WAV inputs, the operator-paced loop below, and recording/export through external capture software. Do not treat those examples as completed demo takes.

The already configured machine uses these paths (the checkout directory is still named `tokkio`):

| Item | Local path |
|---|---|
| Source repository | `C:\Users\Administrator\Documents\tokkio` |
| Orchestrator | `C:\Users\Administrator\Documents\tokkio\services\orchestrator` |
| Unreal project | `C:\Users\Administrator\Documents\Unreal Projects\MetaHumanTest\MetaHumanTest.uproject` |
| Personal questions | `C:\Users\Administrator\Documents\VoiceQuestions` |
| Ignored responses | `services\orchestrator\outputs` |
| Unreal runtime logs | Project `Saved\Logs` |

On another machine, substitute your checkout/project paths. The local project, presentation maps, demo WAVs, and capture launcher are not included in a fresh source clone.

## 1. Start the configured backend

In terminal A, use the existing virtual environment and ignored `.env`:

```powershell
Set-Location 'C:\Users\Administrator\Documents\tokkio\services\orchestrator'
.\.venv\Scripts\python.exe -m uvicorn app.main:app --host 127.0.0.1 --port 8080 --workers 1
```

If the backend is already running on port 8080, keep that instance; do not start another. Use one worker because publisher/observer routing is in memory. Keep the local listener on loopback for this demonstration.

In terminal B:

```powershell
Invoke-RestMethod 'http://127.0.0.1:8080/healthz'
Invoke-RestMethod 'http://127.0.0.1:8080/status' | ConvertTo-Json -Depth 6
```

`/healthz` checks process availability only. `/status` checks configured services, including TTS voice discovery and LLM model validation unless disabled. Hosted-ASR status checks configuration; a real audio turn is needed to verify recognition. Confirm the three mock flags are false. The CLI's `--require-real` enforces non-mock status checks before sending input.

For a new machine, first follow the [backend setup guide](../services/orchestrator/README.md). Do not copy `.env.example` over an existing working `.env`.

## 2. Open the presentation and intended camera

Open the local `MetaHumanTest.uproject`. In Unreal, use **File ? Open Level** and choose:

- `Content/MetaHumanPresentation_Reviewed.umap` ? the separately reviewed presentation, if present.
- `Content/MetaHumanPresentation.umap` ? the original presentation, preserved intact.

In the Outliner, right-click **Presentation_Camera ? Pilot** for editor preview. The existing camera activates for Player 0 when the presentation plays. Preserve its front-facing, upper-body composition.

The reviewed copy retains the character, outfit, camera, blue-gray backdrop, manual exposure, ACE provider, and observer URL. Its saved visual changes are a slightly stronger hair rim light, higher Lumen quality settings, and explicit zero film grain/chromatic aberration.

Press **Play**, not a standalone character test. The imported startup introduction plays first. The receiver waits at least **25 seconds** from BeginPlay and for observed startup playback to finish before connecting.

## 3. Confirm the observer is ready

In Unreal's Output Log, filter on `LogMetaHumanTts`. Wait for:

```text
Observer connected and session ready: ws://127.0.0.1:8080/ws/session?observe=metahuman-demo
```

The receiver should target the existing character and use `LocalA2F-Mark`. The observer must connect **before** a publisher starts.

Use one intended Unreal presentation instance during the recording. Multiple observers on the same channel can each play the answer, causing duplicate audio. If there is no ready message, verify the backend is running, stop/re-enter Play, and wait again. The receiver has no automatic reconnect.

## 4. Record and prepare your questions

Record one complete, self-contained question per file. Export **uncompressed WAV, signed 16-bit PCM, 16,000 Hz, one channel (mono)**. Renaming an MP3 to `.wav` does not convert it. The client rejects stereo, float PCM, and other sample rates; it does not resample.

Place your files anywhere accessible, for example:

```text
C:\Users\Administrator\Documents\VoiceQuestions\question1.wav
C:\Users\Administrator\Documents\VoiceQuestions\question2.wav
C:\Users\Administrator\Documents\VoiceQuestions\question3.wav
```

Trim unnecessary leading/trailing silence. Avoid long pauses inside a question: the current VAD end-of-speech threshold is 500 ms and can end input before the rest of a long recording is sent. No endpointing setting needs to change for this workflow.

Check the file before submitting it (this validation pattern was checked against the existing demo WAV):

```powershell
Set-Location 'C:\Users\Administrator\Documents\tokkio\services\orchestrator'
$question = 'C:\Users\Administrator\Documents\VoiceQuestions\question1.wav'
.\.venv\Scripts\python.exe -c "import sys,wave; w=wave.open(sys.argv[1],'rb'); p=(w.getnchannels(),w.getsampwidth(),w.getframerate(),w.getcomptype()); print(p); assert p==(1,2,16000,'NONE'), 'Required: mono PCM16 at 16000 Hz'; print('seconds:',w.getnframes()/w.getframerate()); w.close()" $question
```

Expected tuple: `(1, 2, 16000, 'NONE')`. Sample width `2` means two bytes / 16 bits.

You do not need to overwrite `demo-input.wav` or `demo-question.wav`, import your question into Unreal, or change a configuration file. Select it with `--wav`.

## 5. Submit a question

Previously exercised command pattern, using the existing local capital-of-Japan question. A unique output filename protects earlier answers:

```powershell
Set-Location 'C:\Users\Administrator\Documents\tokkio\services\orchestrator'
$answer = Join-Path 'outputs' ('demo-answer-' + [guid]::NewGuid().ToString('N') + '.wav')
.\.venv\Scripts\python.exe tools/demo_client.py `
    --url 'ws://127.0.0.1:8080/ws/session?publish=metahuman-demo' `
    --wav 'outputs/demo-question.wav' --require-real --output $answer
```

For your own recording, use the same interface (personal input not validated by this review):

```powershell
$question = 'C:\Users\Administrator\Documents\VoiceQuestions\question1.wav'
$answer = Join-Path 'outputs' ('answer-' + [guid]::NewGuid().ToString('N') + '.wav')
.\.venv\Scripts\python.exe tools/demo_client.py `
    --url 'ws://127.0.0.1:8080/ws/session?publish=metahuman-demo' `
    --wav $question --require-real --output $answer
if ($LASTEXITCODE -ne 0) { throw 'Question failed; inspect the CLI and Unreal logs.' }
```

The CLI prints the ASR transcript, LLM text, and state events, and writes the synthesized response WAV plus a JSON report. It does not play the input question or returned answer through the PC speakers; Unreal plays the answer through ACE.

The `publish=` and `observe=` channel values must match. Omitting `publish=metahuman-demo` runs a backend-only CLI turn with no mirrored response to this observer.

## 6. Wait for Unreal before the next question

Backend completion is **not** playback completion:

```text
CLI/backend: state LISTENING, reason turn_complete
Unreal: ACE audio send completed successfully; awaiting playback completion.
Unreal: Response completed: ACE synchronized audio and animation playback ended.
```

Wait for the **last Unreal message** and for the response to audibly/visibly end. Then submit `question2.wav` using the same command with a new output filename. Keep the Unreal scene playing; do not replay the startup introduction between questions.

The observer receives later responses automatically, but **there is no LLM conversation history**. Make questions self-contained. For example, use ?Which city is the capital of Japan, and why is it important?? rather than an unexplained ?Why is it important??

This optional operator-paced loop is a **recipe, not a tested automatic playlist**. The operator checks Unreal before advancing:

```powershell
Set-Location 'C:\Users\Administrator\Documents\tokkio\services\orchestrator'
$questionDir = 'C:\Users\Administrator\Documents\VoiceQuestions'
$questions = @('question1.wav', 'question2.wav', 'question3.wav')
foreach ($name in $questions) {
    $question = Join-Path $questionDir $name
    if (-not (Test-Path -LiteralPath $question)) { throw "Missing WAV: $question" }
    [void](Read-Host "Confirm observer ready / previous Unreal Response completed, then press Enter for $name")
    $answer = Join-Path 'outputs' ('answer-' + [guid]::NewGuid().ToString('N') + '.wav')
    .\.venv\Scripts\python.exe tools/demo_client.py `
        --url 'ws://127.0.0.1:8080/ws/session?publish=metahuman-demo' `
        --wav $question --require-real --output $answer
    if ($LASTEXITCODE -ne 0) { throw "Turn failed: $name. Recheck observer before continuing." }
}
[void](Read-Host 'Wait for the final Unreal Response completed message before ending the recording')
```

Each invocation still creates a new backend session. A fully automatic sequence needs a playback-finished acknowledgement/queue; genuine contextual follow-ups additionally need LLM message history.

## 7. Capture a clean 1920?1080 presentation

On the existing project, this **previously validated local launcher** opens the reviewed scene in a 1920?1080 game window:

```powershell
& 'C:\Users\Administrator\Documents\Unreal Projects\MetaHumanTest\LaunchPresentationCapture.ps1'
```

This helper lives with the local project, not in this repository. It selects the existing camera, applies session-only TSR/hair/shadow quality settings, caps rendering at 30 fps, and hides on-screen messages/stats. It does not modify the backend, WebSocket, character animation, or saved map. The original presentation remains available.

For editor capture, use the project's `PresentationCapture.py`, then **Play ? New Editor Window (PIE)** with a 1920?1080 viewport. See the local `PRESENTATION_QUALITY_REVIEW.txt` for the exact rendering profile and restoration command. Avoid forcing face LOD 0: the inspected character has an existing eye-rendering issue there.

Recording procedure (external recorder/export **not validated by this review**):

1. Launch the backend and intended game instance. Wait for the introduction, observer readiness, and a few more seconds for temporal rendering history to settle.
2. Capture only the game's 1920?1080 client area at 30 fps; exclude the window border, terminal, cursor, editor panels, and desktop notifications. Use the capture software already available on your machine.
3. Select the actual Unreal output device as an audio source. Confirm the recording meters react and listen to a short test; an ACE ?playback started? log alone does not prove the recorder receives audio.
4. If the audience should hear your questions, play each question once through a captured output device **before** submitting that same WAV, or add the question recordings in editing. The CLI does not play them. For manual playback in PowerShell, `(New-Object System.Media.SoundPlayer -ArgumentList $question).PlaySync()` is an optional, unvalidated capture recipe.
5. Submit questions one at a time, waiting for Unreal playback completion each time. Avoid recording terminal input in the video.
6. Listen to and inspect the exported file for speech clarity, clipping, duplicate voices, frame drops, and lip/audio offset. The internal ACE timing check does not certify an external recorder's synchronization.

The project already has `[Audio] UnfocusedVolumeMultiplier=1.0`, so focusing the CLI should not mute Unreal. If audio is missing, consult the [audio-output diagnosis](../unreal/METAHUMAN_AUDIO_OUTPUT.md). Remote-desktop audio still requires a listening check on the recording machine.

Hiding screen messages produces a clean frame; it does not fix underlying warnings. Check logs during rehearsal, then hide overlays for capture. Existing mapping-pose and NVIDIA runtime diagnostics are documented in [validation](VALIDATION.md).

## 8. Preserve the working demo

Keep these local levels intact:

- `/Game/MetaHumanKnownGood_Preserved`
- `/Game/MetaHumanConversationDemo`
- `/Game/MetaHumanPresentation`

The reviewed scene is `/Game/MetaHumanPresentation_Reviewed`. The earlier presentation backup is under project `Saved/PresentationReview/Backup_20261010_064041`; its manifest includes the map and external actor/object packages. World Partition maps cannot be preserved reliably by copying only the `.umap`.

Before future visual edits, back up the whole project or use the editor to create a separate level with its dependencies. Keep the character, face animation Blueprint, ACE mapping/provider, WebSocket URL, and working `.env` intact. Save responses under unique filenames. Do not rerun preservation/setup scripts just to operate an already configured demo.

No map, audio file, runtime configuration, backend source, or animation Blueprint was modified during this repository documentation update.
