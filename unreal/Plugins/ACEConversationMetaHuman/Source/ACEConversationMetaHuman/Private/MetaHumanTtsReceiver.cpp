#include "MetaHumanTtsReceiver.h"
#include "ConversationBridgeComponent.h"
#include "ACEAudioCurveSourceComponent.h"
#include "AsyncActionAnimateCharacter.h"
#include "ACERuntimeModule.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "PcmWav.h"

DEFINE_LOG_CATEGORY_STATIC(LogMetaHumanTts, Log, All);
IMPLEMENT_MODULE(FDefaultModuleImpl, ACEConversationMetaHuman)

AMetaHumanTtsReceiver::AMetaHumanTtsReceiver()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    Bridge = CreateDefaultSubobject<UConversationBridgeComponent>(TEXT("ConversationBridge"));
    Bridge->ServerUrl = TEXT("ws://127.0.0.1:8080/ws/session?observe=metahuman-demo");
}
void AMetaHumanTtsReceiver::BeginPlay()
{
    Super::BeginPlay();
    CurveSource = TargetCharacter ? TargetCharacter->FindComponentByClass<UACEAudioCurveSourceComponent>() : nullptr;
    if (!CurveSource || !GetProviderFromName(ProviderName))
    {
        Error(TEXT("configuration"), TEXT("Assign the existing MetaHuman with ACEAudioCurveSource and an installed A2F provider."));
        SetActorTickEnabled(false); return;
    }
    Bridge->OnSessionReady.AddDynamic(this, &ThisClass::Ready);
    Bridge->OnTtsStarted.AddDynamic(this, &ThisClass::Started);
    Bridge->OnTtsAudioChunk.AddDynamic(this, &ThisClass::Chunk);
    Bridge->OnTtsEnded.AddDynamic(this, &ThisClass::Ended);
    Bridge->OnError.AddDynamic(this, &ThisClass::Error);
    CurveSource->OnAnimationStarted.AddDynamic(this, &ThisClass::PlaybackStarted);
    CurveSource->OnAnimationEnded.AddDynamic(this, &ThisClass::PlaybackEnded);
}
void AMetaHumanTtsReceiver::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    Elapsed += DeltaSeconds;
    if (!bConnected && Elapsed >= StartupDelaySeconds && !bPlaying)
    {
        bConnected = true;
        Bridge->Connect();
    }
}
void AMetaHumanTtsReceiver::Ready() { UE_LOG(LogMetaHumanTts, Display, TEXT("Observer connected and session ready: %s"), *Bridge->ServerUrl); }
void AMetaHumanTtsReceiver::Error(const FString& Code, const FString& Message)
{
    UE_LOG(LogMetaHumanTts, Error, TEXT("%s: %s"), *Code, *Message);
    bRejected = true;
    bReceiving = false;
    Pcm.Reset();
}
void AMetaHumanTtsReceiver::Started(int32 SampleRate)
{
    bRejected = bOwnResponse || bPlaying || bReceiving;
    bReceiving = true; Rate = SampleRate; Pcm.Reset(); Chunks = 0; Turn.Invalidate();
    UE_LOG(LogMetaHumanTts, Display, TEXT("TTS response started: sample rate=%d"), Rate);
    if (bRejected) Error(TEXT("busy"), TEXT("Previous ACE response or standalone test still active; discarding this response."));
}
void AMetaHumanTtsReceiver::Chunk(const TArray<uint8>& Bytes, int32 SampleRate, const FGuid& TurnId)
{
    if (bRejected) return;
    if (!bReceiving || SampleRate != Rate || (Bytes.Num() & 1) || (Turn.IsValid() && Turn != TurnId) || Pcm.Num() + int64(Bytes.Num()) > 64 * 1024 * 1024)
    { Error(TEXT("pcm"), TEXT("Invalid/mixed PCM response or 64 MiB buffer limit exceeded.")); return; }
    Turn = TurnId; Pcm.Append(Bytes); ++Chunks;
    UE_LOG(LogMetaHumanTts, Log, TEXT("PCM bytes received=%d total=%d chunks=%d sample rate=%d"), Bytes.Num(), Pcm.Num(), Chunks, Rate);
}
void AMetaHumanTtsReceiver::Ended()
{
    bReceiving = false;
    if (bRejected) return;
    TArray<uint8> Wav;
    if (!ACEConversationWav::Encode(Pcm, Rate, Wav)) { Error(TEXT("pcm"), TEXT("Empty or invalid PCM response.")); return; }
    const FString Directory = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("ACEConversation"));
    IFileManager::Get().MakeDirectory(*Directory, true);
    WavPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(Directory, FGuid::NewGuid().ToString() + TEXT(".wav")));
    if (!FFileHelper::SaveArrayToFile(Wav, *WavPath)) { Error(TEXT("wav"), TEXT("Cannot write buffered ACE input WAV.")); return; }
    UE_LOG(LogMetaHumanTts, Display, TEXT("ACE input created: PCM16 mono, sample rate=%d bytes=%d file=%s"), Rate, Pcm.Num(), *WavPath);
    Pcm.Reset();
    bOwnResponse = true; bSendFinished = false; bPlaybackFinished = false;
    Request = UAsyncActionAnimateCharacter::AnimateCharacterFromWavFileAsync(this, TargetCharacter, WavPath, FAudio2FaceEmotion(), nullptr, ProviderName);
    if (!Request)
    {
        bOwnResponse = false;
        IFileManager::Get().Delete(*WavPath); WavPath.Reset();
        Error(TEXT("ace"), TEXT("Cannot create NVIDIA async action."));
        return;
    }
    Request->AudioSendCompleted.AddDynamic(this, &ThisClass::Sent);
    UE_LOG(LogMetaHumanTts, Display, TEXT("ACE animation request started: provider=%s target=%s"), *ProviderName.ToString(), *GetNameSafe(TargetCharacter));
    Request->Activate();
}
void AMetaHumanTtsReceiver::Sent(bool bSuccess)
{
    bSendFinished = true;
    IFileManager::Get().Delete(*WavPath); WavPath.Reset(); Request = nullptr;
    if (!bSuccess)
    {
        Error(TEXT("ace"), TEXT("ACE audio send failed; inspect LogACERuntime."));
        bOwnResponse = false;
        return;
    }
    UE_LOG(LogMetaHumanTts, Display, TEXT("ACE audio send completed successfully; awaiting playback completion."));
    CompleteIfReady();
}
void AMetaHumanTtsReceiver::PlaybackStarted()
{
    bPlaying = true;
    if (bOwnResponse) UE_LOG(LogMetaHumanTts, Display, TEXT("Audio playback started; animation samples received by existing ACEAudioCurveSource (counts in LogACERuntime)."));
}
void AMetaHumanTtsReceiver::PlaybackEnded()
{
    bPlaying = false;
    if (bOwnResponse) { bPlaybackFinished = true; CompleteIfReady(); }
}
void AMetaHumanTtsReceiver::CompleteIfReady()
{
    if (bOwnResponse && bSendFinished && bPlaybackFinished)
    { bOwnResponse = false; UE_LOG(LogMetaHumanTts, Display, TEXT("Response completed: ACE synchronized audio and animation playback ended.")); }
}
void AMetaHumanTtsReceiver::EndPlay(const EEndPlayReason::Type Reason)
{
    Bridge->Disconnect();
    if (CurveSource)
    {
        CurveSource->OnAnimationStarted.RemoveDynamic(this, &ThisClass::PlaybackStarted);
        CurveSource->OnAnimationEnded.RemoveDynamic(this, &ThisClass::PlaybackEnded);
    }
    // NVIDIA's registered async action owns its worker lifetime. Never wait on it
    // here: its callbacks can require the game thread. Leave an in-flight WAV in
    // Saved until the worker has read it; completed requests delete theirs.
    if (Request) Request->AudioSendCompleted.RemoveDynamic(this, &ThisClass::Sent);
    Pcm.Reset();
    Super::EndPlay(Reason);
}
