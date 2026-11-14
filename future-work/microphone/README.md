# Live microphone input

This experiment has two input paths. The Python client captures a single push-to-talk turn on a Mac and can mirror the response to the standard MetaHuman receiver. The Unreal overlay captures a Windows input device and plays back the backend response through a diagnostic actor. The recorded WAV setup uses the main repository plugins directly.

## Python microphone client

On macOS, from `future-work/microphone/python`:

```bash
python3 -m venv .venv-mic
source .venv-mic/bin/activate
python3 -m pip install -r live_mic_requirements.txt
python3 live_mic_client.py --list-devices
python3 live_mic_client.py --url ws://127.0.0.1:8080/ws/session --unreal-channel avatar --output mic-answer.wav
```

Allow microphone access in macOS **Privacy & Security → Microphone**. For a backend on another host, use a private connection or an SSH tunnel to the backend's loopback port. Connect the Unreal observer to the same channel first. Press Enter to start recording and Enter again to send. Omit `--unreal-channel` to test without Unreal. Use `--test-wav path/to/speech.wav` to exercise conversion and routing without a physical microphone.

The client converts native capture to 16 kHz mono PCM16. It uses the checked-out backend's protocol module, so keep the repository layout intact. Run the isolated client tests with `python3 -m unittest discover -s . -v` from this directory.

## Unreal microphone overlay

1. In a separate Windows C++ Unreal test project, copy the standard `unreal/Plugins/ACEConversation/` plugin into `Plugins/`. Merge the files from this folder's [`unreal/ACEConversation/`](unreal/ACEConversation) over that copy and rebuild the project. The overlay enables AudioCapture and adds the microphone component, test actor, procedural playback, and automation tests.
2. In a test level, place a **Conversation Audio Test Actor**, enable **Microphone Mode**, **Show Microphone Controls**, and **Auto Start**, and set its bridge URL to `ws://127.0.0.1:8080/ws/session`.
3. Start the backend, enter Play in Editor, and wait for session readiness. Click **Start Listening**, speak, and click **Stop / Send**. Check for the transcript and audible response.
4. If Windows lists no input device, check Windows Sound Input and microphone permissions. For Remote Desktop, check microphone redirection and verify an input device appears inside the remote session.

Optional automation tests include `ACEConversation.MicrophoneConversion` and `ACEConversation.MicrophoneDeviceProbe`. Pass `-ConversationProbeMicrophone` when launching Unreal to opt into the hardware probe. The [map setup helper](unreal/tools/prepare_microphone_host.py) expects an isolated project under `future-work/microphone/unreal/outputs/MicrophoneDiagnostics/` and Unreal's Python Editor Script Plugin.

Earlier checks covered synthetic audio conversion and routing from a prerecorded input. A physical microphone-to-MetaHuman turn still needs validation.
