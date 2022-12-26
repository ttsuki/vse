#pragma once

#include "../../vse/decoding/WaveFileLoader.h"

namespace wave_files
{
    std::shared_ptr<vse::ISeekableByteStream> Open_36_ogg();
    std::shared_ptr<vse::ISeekableByteStream> Open_40_ogg();
    std::shared_ptr<vse::ISeekableByteStream> Open_42_ogg();
}
