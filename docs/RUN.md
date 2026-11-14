# Run a recorded conversation

Complete the [backend setup](../services/orchestrator/README.md) and the [Unreal setup](../unreal/README.md). These steps run both components on the same Windows machine.

1. **Prepare a question.** Record a short spoken question and export it as 16 kHz, mono, signed 16-bit PCM WAV. If FFmpeg is installed, convert an existing recording in PowerShell from `services/orchestrator`.

   ```powershell
   ffmpeg -i 'recording.m4a' -ar 16000 -ac 1 -c:a pcm_s16le 'question.wav'
   ```

2. **Start the backend.** From `services/orchestrator`, run:

   ```powershell
   .\.venv\Scripts\python.exe -m uvicorn app.main:app --host 127.0.0.1 --port 8080 --workers 1
   ```

3. **Connect Unreal.** Open the configured level, press Play, and wait for `Observer connected and session ready` in Unreal's Output Log. The receiver URL should end with `?observe=avatar`.
4. **Send the question.** In a second PowerShell terminal, from `services/orchestrator`, run:

   ```powershell
   .\.venv\Scripts\python.exe tools/wav_client.py --url 'ws://127.0.0.1:8080/ws/session?publish=avatar' --wav question.wav --require-real --output outputs/answer.wav
   ```

   Use the same channel name after `publish=` and `observe=`. The client saves the answer WAV and a JSON report under `outputs/`.

5. **Verify playback.** Listen for the generated answer and watch the MetaHuman's mouth. Wait for Unreal's `Response completed: ACE synchronized audio and animation playback ended.` before sending another question.

If a step fails, see [troubleshooting](TROUBLESHOOTING.md).
