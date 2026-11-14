# Unreal Engine and MetaHuman setup

Use Windows with an NVIDIA GPU supported by local Audio2Face-3D. The reference environment used Unreal Engine 5.6.1, Visual Studio C++ build tools, NVIDIA ACE 2.5 (`NV_ACE_Reference`), and the `NvAudio2FaceMark` provider package.

## 1. Verify a standalone MetaHuman

1. Create a C++ Unreal project and assemble a MetaHuman in a level, or start from NVIDIA's ACE sample project.
2. Install the ACE plugin and its local Mark provider for your engine version. Configure the MetaHuman with `ACEAudioCurveSource` and the ACE face animation graph and mapping pose.
3. Import a short spoken WAV as a SoundWave and test **Animate Character From Sound Wave Async** with provider `LocalA2F-Mark`. Confirm that you can hear speech and see the face move. The [NVIDIA ACE setup guide](https://docs.nvidia.com/ace/latest/workflows/kairos/ace-unreal-plugin-audio2face.html) describes the character and provider setup.

## 2. Build the repository plugins

Close Unreal Editor and copy [`ACEConversation`](Plugins/ACEConversation) and [`ACEConversationMetaHuman`](Plugins/ACEConversationMetaHuman) into the Unreal project's `Plugins/` directory alongside the NVIDIA plugins. Generate the project files and build the project's **Development Editor / Win64** target.

If the installed-engine build tries to relink MetaHuman modules and fails, use the module-only build below. Run this from the cloned repository root and set the two paths for your machine.

```powershell
$engine = 'C:/Program Files/Epic Games/UE_5.6/Engine'
$project = 'C:/Unreal/MyAvatar/MyAvatar.uproject'
& "$engine/Build/BatchFiles/Build.bat" UnrealEditor Win64 Development "-Project=$project" -Module=ACEConversation -Module=ACEConversationMetaHuman -NoHotReload -NoUBTMakefiles -NoUBA
if ($LASTEXITCODE -ne 0) { throw 'Plugin build failed' }
& ./unreal/tools/write_conversation_module_manifests.ps1 -ProjectDir (Split-Path $project) -EngineDir $engine
```

## 3. Connect the level to the backend

1. Reopen the project and ensure both conversation plugins are enabled.
2. Place one **MetaHuman TTS Receiver** (`MetaHumanTtsReceiver`) actor in the same level as your MetaHuman.
3. Set the following values on the receiver and its **ConversationBridge** component.

| Property | Value |
|---|---|
| Target Character | The ACE-enabled MetaHuman actor in the level |
| Provider Name | `LocalA2F-Mark` |
| Startup Delay Seconds | `0` for a clean level, or a delay long enough for any standalone startup audio |
| ConversationBridge → Server URL | `ws://127.0.0.1:8080/ws/session?observe=avatar` |

Save the level. Follow the [run guide](../docs/RUN.md) to start the backend and send a question.
