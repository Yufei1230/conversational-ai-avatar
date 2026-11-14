#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "ConversationBridgeComponent.h"
#include "UObject/StrongObjectPtr.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FACEConversationProtocolTest, "ACEConversation.Protocol",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FACEConversationProtocolTest::RunTest(const FString& Parameters)
{
    TStrongObjectPtr<UConversationBridgeComponent> Bridge(NewObject<UConversationBridgeComponent>());
    Bridge->HandleTextMessage(TEXT("{\"type\":\"state\",\"turn_id\":null,\"payload\":{\"state\":\"LISTENING\",\"reason\":\"ready\"}}"));
    TestTrue(TEXT("Backend ready control parsed"), Bridge->bSessionReady);
    TArray<uint8> Pcm;
    Pcm.Init(0x12, 640);
    TArray<uint8> Frame = Bridge->BuildMicAudioFrame(Pcm);
    const uint8 ExpectedPrefix[] = {0x41,0x43,0x45,0x31,1,1,1,1,0,0,0x3e,0x80,0,0,2,0x80};
    TestEqual(TEXT("32 byte header plus 640 PCM bytes"), Frame.Num(), 672);
    TestTrue(TEXT("Matches Python !4sBBBBII16s wire format"), FMemory::Memcmp(Frame.GetData(), ExpectedPrefix, 16) == 0);
    for (int32 I = 16; I < 32; ++I) { TestEqual(TEXT("Mic UUID is zero"), Frame[I], uint8(0)); }
    Frame[5] = 2; // TTS
    Frame[8] = 0; Frame[9] = 0; Frame[10] = 0xac; Frame[11] = 0x44; // 44100
    int32 Rate = 0;
    TArray<uint8> Parsed;
    TestTrue(TEXT("Parses backend TTS header"), Bridge->ParseTtsAudioFrame(Frame.GetData(), Frame.Num(), Rate, Parsed));
    TestEqual(TEXT("Output rate from header"), Rate, 44100);
    TestTrue(TEXT("PCM unchanged"), Parsed == Pcm);
    Bridge->HandleBinaryMessage(Frame.GetData(), 7, false);
    TestEqual(TEXT("Fragment retained"), Bridge->BinaryMessage.Num(), 7);
    Bridge->HandleBinaryMessage(Frame.GetData() + 7, Frame.Num() - 7, true);
    TestEqual(TEXT("Complete binary message consumed"), Bridge->BinaryMessage.Num(), 0);
    Frame.Add(0);
    TestFalse(TEXT("Rejects trailing bytes"), Bridge->ParseTtsAudioFrame(Frame.GetData(), Frame.Num(), Rate, Parsed));
    Frame.RemoveAt(Frame.Num() - 1);
    Frame[6] = 2;
    TestFalse(TEXT("Rejects unsupported codec"), Bridge->ParseTtsAudioFrame(Frame.GetData(), Frame.Num(), Rate, Parsed));
    TestFalse(TEXT("Rejects truncated header"), Bridge->ParseTtsAudioFrame(Frame.GetData(), 10, Rate, Parsed));
    return true;
}

#endif
