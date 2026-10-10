#include "PcmWav.h"
#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMetaHumanPcmWavTest, "ACE.MetaHuman.BufferedWav", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMetaHumanPcmWavTest::RunTest(const FString& Parameters)
{
    TArray<uint8> Out;
    TestFalse(TEXT("Empty response rejected"), ACEConversationWav::Encode({}, 44100, Out));
    TestFalse(TEXT("Partial sample rejected"), ACEConversationWav::Encode({1}, 44100, Out));
    TestFalse(TEXT("Invalid rate rejected"), ACEConversationWav::Encode({0,0}, 0, Out));
    const TArray<uint8> Pcm = {0,0,255,127,0,128};
    for (int32 Rate : {16000, 24000, 44100, 48000})
    {
        TestTrue(TEXT("Encode"), ACEConversationWav::Encode(Pcm, Rate, Out));
        TestEqual(TEXT("WAV length"), Out.Num(), 44 + Pcm.Num());
        TestEqual(TEXT("Rate preserved"), int32(Out[24] | Out[25]<<8 | Out[26]<<16 | Out[27]<<24), Rate);
        TestEqual(TEXT("Mono"), Out[22], uint8(1));
        TestEqual(TEXT("16 bit"), Out[34], uint8(16));
        TestTrue(TEXT("PCM unchanged"), FMemory::Memcmp(Out.GetData()+44, Pcm.GetData(), Pcm.Num()) == 0);
    }
    return true;
}
#endif
