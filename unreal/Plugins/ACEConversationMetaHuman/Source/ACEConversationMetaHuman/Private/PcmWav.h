#pragma once
#include "CoreMinimal.h"
namespace ACEConversationWav
{
inline bool Encode(const TArray<uint8>& Pcm, int32 Rate, TArray<uint8>& Out)
{
    Out.Reset();
    if (Pcm.IsEmpty() || (Pcm.Num() & 1) || Rate < 8000 || Rate > 192000 || Pcm.Num() > 64 * 1024 * 1024) return false;
    auto U16 = [&Out](uint16 V) { Out.Add(V & 255); Out.Add(V >> 8); };
    auto U32 = [&Out](uint32 V) { for (int I = 0; I < 4; ++I) Out.Add((V >> (I * 8)) & 255); };
    auto Tag = [&Out](const ANSICHAR* S) { Out.Append(reinterpret_cast<const uint8*>(S), 4); };
    Tag("RIFF"); U32(36 + Pcm.Num()); Tag("WAVE"); Tag("fmt "); U32(16);
    U16(1); U16(1); U32(Rate); U32(Rate * 2); U16(2); U16(16);
    Tag("data"); U32(Pcm.Num()); Out.Append(Pcm);
    return true;
}
}
