#pragma once

namespace Vulcant
{
    enum class VulcantResourceLayout
    {
        Undefined,       // Erster Frame, Inhalt egal
        General,         // Für Compute-Shader (Raycast) Read/Write (Storage)
        ColorAttachment, // Wenn der Shader direkt hineinzeichnet (Rasterizer)
        ShaderReadOnly,  // Wenn der nächste Pass daraus liest (Sampler/Deferred)
        TransferSrc,     // Bilddaten von der GPU kopieren (z.B. für Screenshots)
        TransferDst,     // Daten auf die GPU kopieren
        Present,          // Das fertige Bild an den Monitor/Godot senden
        DepthStencilAttachment
    };
}