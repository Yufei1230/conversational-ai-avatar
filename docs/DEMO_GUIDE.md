# Running the demo

This guide operates an already configured Windows MetaHuman project. For a new machine, complete the [backend setup](../services/orchestrator/README.md) and [Unreal integration](../unreal/README.md) first. The local GPT-OSS adapter changes are not yet included in this checkout; see [source status](../services/orchestrator/README.md#model-configuration-and-source-status).

## 1. Prepare two questions

Record short questions, for example:

- Question 1: "What is the capital of China?"
- Question 2: "Briefly explain large language models."

Save both as **16 kHz, mono, PCM16 WAV** files. With FFmpeg installed, convert each recording from its actual format:

```powershell
ffmpeg -i question1.mp3 -ar 16000 -ac 1 -c:a pcm_s16le question1.wav
```

Renaming an MP3 to `.wav` does not convert it. The commands below assume you place your prepared files at `services/orchestrator/outputs/question1.wav` and `question2.wav`. These recordings are not supplied by the repository.

## 2. Start the backend

From the repository root, open PowerShell and run:

```powershell
Set-Location services/orchestrator
.\.venv\Scripts\python.exe tools/init_storage.py
.\.venv\Scripts\python.exe -m uvicorn app.main:app --host 127.0.0.1 --port 8080 --workers 1
```

Keep this terminal open. Use one worker for publish/observe routing. Service credentials and model selection are loaded from the local `.env`.

## 3. Start Unreal and wait for the observer

1. Open the configured MetaHuman presentation or conversation level.
2. In **Edit > Editor Preferences**, search for **Use Less CPU when in Background** and disable it. This resolved the local face-animation stall when switching to PowerShell on October 18.
3. Start Play or Simulate and let the prepared introduction finish.
4. Wait for this Output Log message:

```text
Observer connected and session ready: ws://127.0.0.1:8080/ws/session?observe=metahuman-demo
```

The receiver's default startup delay is 25 seconds, and it also waits for observed startup playback to finish. Use the readiness log rather than a fixed countdown.

## 4. Submit Question 1

Open a second PowerShell terminal at the repository root, then change to the backend directory:

```powershell
Set-Location services/orchestrator
```

Submit the question using this single-line command:

```powershell
.\.venv\Scripts\python.exe tools/demo_client.py --url 'ws://127.0.0.1:8080/ws/session?publish=metahuman-demo' --wav outputs/question1.wav --require-real --output outputs/answer1.wav
```

`--require-real` checks that mock adapters are disabled and the backend reports healthy adapters. The client prints the transcript and generated text, then saves the answer WAV and JSON report. It does not play the question aloud. Reusing an output filename replaces that local output.

## 5. Wait, then submit Question 2

Wait until the MetaHuman finishes speaking and Unreal logs:

```text
Response completed: ACE synchronized audio and animation playback ended.
```

Then run:

```powershell
.\.venv\Scripts\python.exe tools/demo_client.py --url 'ws://127.0.0.1:8080/ws/session?publish=metahuman-demo' --wav outputs/question2.wav --require-real --output outputs/answer2.wav
```

Keep the Unreal observer running between questions. CLI exit and backend `LISTENING` do not mean avatar playback has ended. Sending the next question too early can cause a `busy` rejection.

## 6. Stop or reconnect

- **Disconnect Unreal:** stop Play/Simulate. The receiver's `EndPlay` closes its WebSocket.
- **Stop the backend:** press `Ctrl+C` in the Uvicorn terminal.
- **Reconnect after a backend restart or connection failure:** stop and restart Play/Simulate, then wait for the observer-ready message again. The receiver does not automatically reconnect.

## Presenting the demo

Use four short chapters: **Introduction**, **System Overview**, **End-to-End Demonstration**, and **Future Work**.

For each question, use the same visual sequence: **LISTENING**, **QUESTION**, **THINKING**, **RESPONDING**, then the **MetaHuman answer**. Show the actual transcript and response rather than assuming an exact model answer. `RESPONDING` is a presentation label; the runtime state is `SPEAKING`, and avatar playback follows full-response buffering.

During the overview, follow the narration through the backend, audio bridge, and Unreal/ACE components. Keep Future Work to two points: **live microphone interaction** and **more natural avatar animation**. Check the recorded video for audible speech, visible lip movement, and readable captions.

For failures, see [troubleshooting](troubleshooting/README.md).
