#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MetaHumanTtsReceiver.generated.h"

class UConversationBridgeComponent;
class UACEAudioCurveSourceComponent;
class UAsyncActionAnimateCharacter;

// A separate observer; the existing character and its animation graph stay untouched.
UCLASS()
class ACECONVERSATIONMETAHUMAN_API AMetaHumanTtsReceiver : public AActor
{
    GENERATED_BODY()
public:
    AMetaHumanTtsReceiver();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ACE")
    TObjectPtr<UConversationBridgeComponent> Bridge;
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="ACE")
    TObjectPtr<AActor> TargetCharacter;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ACE")
    FName ProviderName = TEXT("LocalA2F-Mark");
    // Allows the character's unchanged BeginPlay SoundWave test to finish first.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ACE", meta=(ClampMin="0"))
    float StartupDelaySeconds = 25.f;
    virtual void Tick(float DeltaSeconds) override;
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
    UPROPERTY() TObjectPtr<UACEAudioCurveSourceComponent> CurveSource;
    UPROPERTY() TObjectPtr<UAsyncActionAnimateCharacter> Request;
    TArray<uint8> Pcm;
    FString WavPath;
    FGuid Turn;
    int32 Rate = 0;
    int32 Chunks = 0;
    bool bReceiving = false;
    bool bRejected = false;
    bool bPlaying = false;
    bool bOwnResponse = false;
    bool bSendFinished = false;
    bool bPlaybackFinished = false;
    float Elapsed = 0;
    bool bConnected = false;
    UFUNCTION() void Ready();
    UFUNCTION() void Started(int32 SampleRate);
    UFUNCTION() void Chunk(const TArray<uint8>& Bytes, int32 SampleRate, const FGuid& TurnId);
    UFUNCTION() void Ended();
    UFUNCTION() void Error(const FString& Code, const FString& Message);
    UFUNCTION() void Sent(bool bSuccess);
    UFUNCTION() void PlaybackStarted();
    UFUNCTION() void PlaybackEnded();
    void CompleteIfReady();
};
