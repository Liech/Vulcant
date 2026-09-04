#pragma once

namespace Vulcant
{
    enum class VulcantImageUsage
    {
        WriteAndRead,   // General / Storage Image
        SampleOnly,     // ShaderReadOnly / Combined Sampler
        TransferSource, // ReadByte (für Screenshots/CPU-Readback)
        TransferDest    // WriteByte (für Textur-Uploads)
    };
}