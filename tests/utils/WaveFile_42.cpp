#include "./WaveFiles.h"

namespace wave_files
{
    std::shared_ptr<vse::ISeekableByteStream> Open_42_ogg()
    {
        static const unsigned char file_data[] = {
            0x4f, 0x67, 0x67, 0x53, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x97, 0x0a,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x27, 0x49, 0xb3, 0x64, 0x01, 0x1e, 0x01, 0x76, 0x6f, 0x72,
            0x62, 0x69, 0x73, 0x00, 0x00, 0x00, 0x00, 0x02, 0x44, 0xac, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
            0x00, 0xfa, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xb8, 0x01, 0x4f, 0x67, 0x67, 0x53, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x97, 0x0a, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
            0x7f, 0x59, 0x80, 0xd5, 0x0f, 0x44, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0x7a, 0x03, 0x76, 0x6f, 0x72, 0x62, 0x69, 0x73, 0x34, 0x00, 0x00, 0x00, 0x58,
            0x69, 0x70, 0x68, 0x2e, 0x4f, 0x72, 0x67, 0x20, 0x6c, 0x69, 0x62, 0x56, 0x6f, 0x72, 0x62, 0x69,
            0x73, 0x20, 0x49, 0x20, 0x32, 0x30, 0x32, 0x30, 0x30, 0x37, 0x30, 0x34, 0x20, 0x28, 0x52, 0x65,
            0x64, 0x75, 0x63, 0x69, 0x6e, 0x67, 0x20, 0x45, 0x6e, 0x76, 0x69, 0x72, 0x6f, 0x6e, 0x6d, 0x65,
            0x6e, 0x74, 0x29, 0x00, 0x00, 0x00, 0x00, 0x01, 0x05, 0x76, 0x6f, 0x72, 0x62, 0x69, 0x73, 0x21,
            0x42, 0x43, 0x56, 0x01, 0x00, 0x00, 0x01, 0x00, 0x18, 0x63, 0x54, 0x29, 0x46, 0x99, 0x52, 0xd2,
            0x4a, 0x89, 0x19, 0x73, 0x94, 0x31, 0x46, 0x99, 0x62, 0x92, 0x4a, 0x89, 0xa5, 0x84, 0x16, 0x42,
            0x48, 0x9d, 0x73, 0x14, 0x53, 0xa9, 0x39, 0xd7, 0x9c, 0x6b, 0xac, 0xb9, 0xb5, 0x20, 0x84, 0x10,
            0x1a, 0x53, 0x50, 0x29, 0x05, 0x99, 0x52, 0x8e, 0x52, 0x69, 0x19, 0x63, 0x90, 0x29, 0x05, 0x99,
            0x52, 0x10, 0x4b, 0x49, 0x25, 0x74, 0x12, 0x3a, 0x27, 0x9d, 0x63, 0x10, 0x5b, 0x49, 0xc1, 0xd6,
            0x98, 0x6b, 0x8b, 0x41, 0xb6, 0x1c, 0x84, 0x0d, 0x9a, 0x52, 0x4c, 0x29, 0xc4, 0x94, 0x52, 0x8a,
            0x42, 0x08, 0x19, 0x53, 0x8c, 0x29, 0xc5, 0x94, 0x52, 0x4a, 0x42, 0x07, 0x25, 0x74, 0x0e, 0x3a,
            0xe6, 0x1c, 0x53, 0x8e, 0x4a, 0x28, 0x41, 0xb8, 0x9c, 0x73, 0xab, 0xb5, 0x96, 0x96, 0x63, 0x8b,
            0xa9, 0x74, 0x92, 0x4a, 0xe7, 0x24, 0x64, 0x4c, 0x42, 0x48, 0x29, 0x85, 0x92, 0x4a, 0x07, 0xa5,
            0x53, 0x4e, 0x42, 0x48, 0x35, 0x96, 0xd6, 0x52, 0x29, 0x1d, 0x73, 0x52, 0x52, 0x6a, 0x41, 0xe8,
            0x20, 0x84, 0x10, 0x42, 0xb6, 0x20, 0x84, 0x0d, 0x82, 0xd0, 0x90, 0x55, 0x00, 0x00, 0x01, 0x00,
            0xc0, 0x40, 0x10, 0x1a, 0xb2, 0x0a, 0x00, 0x50, 0x00, 0x00, 0x10, 0x8a, 0xa1, 0x18, 0x8a, 0x02,
            0x84, 0x86, 0xac, 0x02, 0x00, 0x32, 0x00, 0x00, 0x04, 0xa0, 0x28, 0x8e, 0xe2, 0x28, 0x8e, 0x23,
            0x39, 0x92, 0x63, 0x49, 0x16, 0x10, 0x1a, 0xb2, 0x0a, 0x00, 0x00, 0x02, 0x00, 0x10, 0x00, 0x00,
            0xc0, 0x70, 0x14, 0x49, 0x91, 0x14, 0xc9, 0xb1, 0x24, 0x4b, 0xd2, 0x2c, 0x4b, 0xd3, 0x44, 0x51,
            0x55, 0x7d, 0xd5, 0x36, 0x55, 0x55, 0xf6, 0x75, 0x5d, 0xd7, 0x75, 0x5d, 0xd7, 0x75, 0x20, 0x34,
            0x64, 0x15, 0x00, 0x00, 0x01, 0x00, 0x40, 0x48, 0xa7, 0x99, 0xa5, 0x1a, 0x20, 0xc2, 0x0c, 0x64,
            0x18, 0x08, 0x0d, 0x59, 0x05, 0x00, 0x20, 0x00, 0x00, 0x00, 0x46, 0x28, 0xc2, 0x10, 0x03, 0x42,
            0x43, 0x56, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x62, 0x28, 0x39, 0x88, 0x26, 0xb4, 0xe6, 0x7c,
            0x73, 0x8e, 0x83, 0x66, 0x39, 0x68, 0x2a, 0xc5, 0xe6, 0x74, 0x70, 0x22, 0xd5, 0xe6, 0x49, 0x6e,
            0x2a, 0xe6, 0xe6, 0x9c, 0x73, 0xce, 0x39, 0x27, 0x9b, 0x73, 0xc6, 0x38, 0xe7, 0x9c, 0x73, 0x8a,
            0x72, 0x66, 0x31, 0x68, 0x26, 0xb4, 0xe6, 0x9c, 0x73, 0x12, 0x83, 0x66, 0x29, 0x68, 0x26, 0xb4,
            0xe6, 0x9c, 0x73, 0x9e, 0xc4, 0xe6, 0x41, 0x6b, 0xaa, 0xb4, 0xe6, 0x9c, 0x73, 0xc6, 0x39, 0xa7,
            0x83, 0x71, 0x46, 0x18, 0xe7, 0x9c, 0x73, 0x9a, 0xb4, 0xe6, 0x41, 0x6a, 0x36, 0xd6, 0xe6, 0x9c,
            0x73, 0x16, 0xb4, 0xa6, 0x39, 0x6a, 0x2e, 0xc5, 0xe6, 0x9c, 0x73, 0x22, 0xe5, 0xe6, 0x49, 0x6d,
            0x2e, 0xd5, 0xe6, 0x9c, 0x73, 0xce, 0x39, 0xe7, 0x9c, 0x73, 0xce, 0x39, 0xe7, 0x9c, 0x73, 0xaa,
            0x17, 0xa7, 0x73, 0x70, 0x4e, 0x38, 0xe7, 0x9c, 0x73, 0xa2, 0xf6, 0xe6, 0x5a, 0x6e, 0x42, 0x17,
            0xe7, 0x9c, 0x73, 0x3e, 0x19, 0xa7, 0x7b, 0x73, 0x42, 0x38, 0xe7, 0x9c, 0x73, 0xce, 0x39, 0xe7,
            0x9c, 0x73, 0xce, 0x39, 0xe7, 0x9c, 0x73, 0x82, 0xd0, 0x90, 0x55, 0x00, 0x00, 0x10, 0x00, 0x00,
            0x41, 0x18, 0x36, 0x86, 0x71, 0xa7, 0x20, 0x48, 0x9f, 0xa3, 0x81, 0x18, 0x45, 0x88, 0x69, 0xc8,
            0xa4, 0x07, 0xdd, 0xa3, 0xc3, 0x24, 0x68, 0x0c, 0x72, 0x0a, 0xa9, 0x47, 0xa3, 0xa3, 0x91, 0x52,
            0xea, 0x20, 0x94, 0x54, 0xc6, 0x49, 0x29, 0x9d, 0x20, 0x34, 0x64, 0x15, 0x00, 0x00, 0x08, 0x00,
            0x00, 0x21, 0x84, 0x14, 0x52, 0x48, 0x21, 0x85, 0x14, 0x52, 0x48, 0x21, 0x85, 0x14, 0x52, 0x88,
            0x21, 0x86, 0x18, 0x62, 0xc8, 0x29, 0xa7, 0x9c, 0x82, 0x0a, 0x2a, 0xa9, 0xa4, 0xa2, 0x8a, 0x32,
            0xca, 0x2c, 0xb3, 0xcc, 0x32, 0xcb, 0x2c, 0xb3, 0xcc, 0x32, 0xeb, 0xb0, 0xb3, 0xce, 0x3a, 0xec,
            0x30, 0xc4, 0x10, 0x43, 0x0c, 0xad, 0xb4, 0x12, 0x4b, 0x4d, 0xb5, 0xd5, 0x58, 0x63, 0xad, 0xb9,
            0xe7, 0x9c, 0x6b, 0x0e, 0xd2, 0x5a, 0x69, 0xad, 0xb5, 0xd6, 0x4a, 0x29, 0xa5, 0x94, 0x52, 0x4a,
            0x29, 0x08, 0x0d, 0x59, 0x05, 0x00, 0x80, 0x00, 0x00, 0x10, 0x08, 0x19, 0x64, 0x90, 0x41, 0x46,
            0x21, 0x85, 0x14, 0x52, 0x88, 0x21, 0xa6, 0x9c, 0x72, 0xca, 0x29, 0xa8, 0xa0, 0x02, 0x42, 0x43,
            0x56, 0x01, 0x00, 0x80, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x3c, 0xc9, 0x73, 0x44, 0x47, 0x74,
            0x44, 0x47, 0x74, 0x44, 0x47, 0x74, 0x44, 0x47, 0x74, 0x44, 0xc7, 0x73, 0x3c, 0x47, 0x94, 0x44,
            0x49, 0x94, 0x44, 0x49, 0xb4, 0x4c, 0xcb, 0xd4, 0x4c, 0x4f, 0x15, 0x55, 0xd5, 0x95, 0x5d, 0x5b,
            0xd6, 0x65, 0xdd, 0xf6, 0x6d, 0x61, 0x17, 0x76, 0xdd, 0xf7, 0x75, 0xdf, 0xf7, 0x75, 0xe3, 0xd7,
            0x85, 0x61, 0x59, 0x96, 0x65, 0x59, 0x96, 0x65, 0x59, 0x96, 0x65, 0x59, 0x96, 0x65, 0x59, 0x96,
            0x65, 0x59, 0x82, 0xd0, 0x90, 0x55, 0x00, 0x00, 0x08, 0x00, 0x00, 0x80, 0x10, 0x42, 0x08, 0x21,
            0x85, 0x14, 0x52, 0x48, 0x21, 0xa5, 0x18, 0x63, 0xcc, 0x31, 0xe7, 0xa0, 0x93, 0x50, 0x42, 0x20,
            0x34, 0x64, 0x15, 0x00, 0x00, 0x08, 0x00, 0x20, 0x00, 0x00, 0x00, 0xc0, 0x51, 0x1c, 0xc5, 0x71,
            0x24, 0x47, 0x72, 0x24, 0xc9, 0x92, 0x2c, 0x49, 0x93, 0x34, 0x4b, 0xb3, 0x3c, 0xcd, 0xd3, 0x3c,
            0x4d, 0xf4, 0x44, 0x51, 0x14, 0x4d, 0xd3, 0x54, 0x45, 0x57, 0x74, 0x45, 0xdd, 0xb4, 0x45, 0xd9,
            0x94, 0x4d, 0xd7, 0x74, 0x4d, 0xd9, 0x74, 0x55, 0x59, 0xb5, 0x5d, 0x59, 0xb6, 0x6d, 0xd9, 0xd6,
            0x6d, 0x5f, 0x96, 0x6d, 0xdf, 0xf7, 0x7d, 0xdf, 0xf7, 0x7d, 0xdf, 0xf7, 0x7d, 0xdf, 0xf7, 0x7d,
            0xdf, 0xf7, 0x75, 0x1d, 0x08, 0x0d, 0x59, 0x05, 0x00, 0x48, 0x00, 0x00, 0xe8, 0x48, 0x8e, 0xa4,
            0x48, 0x8a, 0xa4, 0x48, 0x8e, 0xe3, 0x38, 0x92, 0x24, 0x01, 0xa1, 0x21, 0xab, 0x00, 0x00, 0x19,
            0x00, 0x00, 0x01, 0x00, 0x28, 0x8a, 0xa3, 0x38, 0x8e, 0xe3, 0x48, 0x92, 0x24, 0x49, 0x96, 0xa4,
            0x49, 0x9e, 0xe5, 0x59, 0xa2, 0x66, 0x6a, 0xa6, 0x67, 0x7a, 0xaa, 0xa8, 0x02, 0xa1, 0x21, 0xab,
            0x00, 0x00, 0x40, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x9a, 0xe2, 0x29, 0xa6,
            0xe2, 0x29, 0xa2, 0xe2, 0x39, 0xa2, 0x23, 0x4a, 0xa2, 0x65, 0x5a, 0xa2, 0xa6, 0x6a, 0xae, 0x28,
            0x9b, 0xb2, 0xeb, 0xba, 0xae, 0xeb, 0xba, 0xae, 0xeb, 0xba, 0xae, 0xeb, 0xba, 0xae, 0xeb, 0xba,
            0xae, 0xeb, 0xba, 0xae, 0xeb, 0xba, 0xae, 0xeb, 0xba, 0xae, 0xeb, 0xba, 0xae, 0xeb, 0xba, 0xae,
            0xeb, 0xba, 0xae, 0xeb, 0xba, 0x2e, 0x10, 0x1a, 0xb2, 0x0a, 0x00, 0x90, 0x00, 0x00, 0xd0, 0x91,
            0x1c, 0xc9, 0x91, 0x1c, 0x49, 0x91, 0x14, 0x49, 0x91, 0x1c, 0xc9, 0x01, 0x42, 0x43, 0x56, 0x01,
            0x00, 0x32, 0x00, 0x00, 0x02, 0x00, 0x70, 0x0c, 0xc7, 0x90, 0x14, 0xc9, 0xb1, 0x2c, 0x4b, 0xd3,
            0x3c, 0xcd, 0xd3, 0x3c, 0x4d, 0xf4, 0x44, 0x4f, 0xf4, 0x4c, 0x4f, 0x15, 0x5d, 0xd1, 0x05, 0x42,
            0x43, 0x56, 0x01, 0x00, 0x80, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x24, 0xc3,
            0x52, 0x2c, 0x47, 0x73, 0x34, 0x49, 0x94, 0x54, 0x4b, 0xb5, 0x54, 0x4d, 0xb5, 0x54, 0x4b, 0x15,
            0x55, 0x4f, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
            0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
            0x55, 0x55, 0x55, 0x35, 0x4d, 0xd3, 0x34, 0x4d, 0x20, 0x34, 0x64, 0x25, 0x00, 0x10, 0x05, 0x00,
            0x40, 0x29, 0x8b, 0xb1, 0xf6, 0x20, 0x1c, 0x81, 0x1c, 0x83, 0x96, 0x73, 0x68, 0x10, 0x64, 0xd0,
            0x7a, 0x51, 0x15, 0x33, 0xca, 0x51, 0x2d, 0x26, 0x52, 0x08, 0x31, 0xa9, 0xc1, 0x44, 0x8c, 0x29,
            0x26, 0xb1, 0xa7, 0x88, 0x31, 0xe6, 0xa4, 0xe5, 0x58, 0x31, 0x84, 0x18, 0xb4, 0xd8, 0x3b, 0xa8,
            0x14, 0x83, 0xd2, 0x02, 0xa1, 0x21, 0x2b, 0x04, 0x80, 0xd0, 0x0c, 0x00, 0x83, 0x24, 0x01, 0x92,
            0xa6, 0x01, 0x92, 0xa6, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xe4, 0x69, 0x80, 0x26,
            0x8a, 0x80, 0xe6, 0x89, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x69, 0x1a, 0xa0, 0x89,
            0x1e, 0xa0, 0x89, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x92, 0xa7, 0x01, 0x9e, 0x28, 0x02, 0x9e, 0x28,
            0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x26, 0x8a, 0x80, 0x68, 0x9a, 0x80, 0x68, 0x9a,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa0, 0x89, 0x22, 0xe0, 0x99, 0x22, 0x20, 0x9a, 0x26,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x92, 0xe6, 0x01, 0x9a, 0x28, 0x02, 0x9e, 0x28, 0x02, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x80, 0x26, 0x8a, 0x80, 0x68, 0x9a, 0x80, 0x28, 0x9a, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0xa0, 0x89, 0x22, 0x20, 0x9a, 0x26, 0x20, 0x9a, 0x26, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x80, 0x00, 0x00, 0x80, 0x00, 0x07, 0x00, 0x80, 0x00, 0x0b, 0xa1, 0xd0, 0x90, 0x15,
            0x01, 0x40, 0x9c, 0x00, 0x80, 0xc1, 0x71, 0x2c, 0x0b, 0x00, 0x00, 0x1c, 0x49, 0xd2, 0x34, 0x00,
            0x00, 0x70, 0x24, 0x49, 0xd3, 0x00, 0x00, 0xc0, 0xd2, 0x34, 0x51, 0x04, 0x00, 0x00, 0x4d, 0xd3,
            0x44, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x40, 0x00, 0x00, 0xc0, 0x80, 0x03, 0x00, 0x40, 0x80, 0x09, 0x65, 0xa0, 0xd0, 0x90,
            0x95, 0x00, 0x40, 0x14, 0x00, 0x80, 0x41, 0x51, 0x34, 0x0d, 0x48, 0x8e, 0xa6, 0x01, 0x49, 0xd2,
            0x34, 0x60, 0x69, 0x9e, 0x07, 0xf0, 0x3c, 0x80, 0x68, 0x02, 0x44, 0x11, 0xa0, 0xaa, 0x00, 0x40,
            0x00, 0x00, 0x40, 0x81, 0x03, 0x00, 0x40, 0x80, 0x0d, 0x9a, 0x12, 0x8b, 0x03, 0x14, 0x1a, 0xb2,
            0x12, 0x00, 0x88, 0x02, 0x00, 0x30, 0x38, 0x8a, 0x65, 0x69, 0x9a, 0x28, 0x92, 0x24, 0x4d, 0xf3,
            0x34, 0x51, 0x24, 0x49, 0x9a, 0xe6, 0x79, 0xa2, 0x48, 0xd3, 0x3c, 0xcf, 0xf3, 0x4c, 0x13, 0x9e,
            0xe7, 0x79, 0xa6, 0x09, 0x51, 0x14, 0x45, 0x55, 0x85, 0x28, 0x8a, 0xa2, 0xaa, 0xc2, 0x34, 0x4d,
            0x53, 0x55, 0x81, 0x28, 0xaa, 0xaa, 0x00, 0x00, 0x80, 0x02, 0x07, 0x00, 0x80, 0x00, 0x1b, 0x34,
            0x25, 0x16, 0x07, 0x28, 0x34, 0x64, 0x25, 0x00, 0x10, 0x12, 0x00, 0x60, 0x70, 0x1c, 0xcb, 0xf2,
            0x3c, 0xcf, 0xf3, 0x3c, 0x51, 0x34, 0x4d, 0x55, 0x65, 0x59, 0x9a, 0xe6, 0x79, 0xa2, 0x28, 0x8a,
            0xa6, 0x69, 0xaa, 0xaa, 0x4a, 0xb2, 0x34, 0xcd, 0xf3, 0x44, 0x51, 0x14, 0x4d, 0xd3, 0x54, 0x55,
            0x95, 0xa6, 0x79, 0x9e, 0xe7, 0x89, 0xa2, 0x28, 0x9a, 0xa6, 0xaa, 0xba, 0x2e, 0x3c, 0xcf, 0xf3,
            0x44, 0x51, 0x14, 0x4d, 0x53, 0x55, 0x5d, 0x17, 0x9e, 0x27, 0x8a, 0xa6, 0x69, 0x9a, 0xaa, 0xaa,
            0xaa, 0xae, 0x0b, 0xcf, 0x13, 0x45, 0xd3, 0x34, 0x4d, 0x55, 0x55, 0x55, 0xd7, 0x85, 0x28, 0x8a,
            0xa2, 0x69, 0x9a, 0xa6, 0xaa, 0xba, 0xae, 0x2b, 0x03, 0x51, 0x34, 0x4d, 0xd3, 0x54, 0x55, 0xd7,
            0x95, 0x65, 0x20, 0x8a, 0xa6, 0xa9, 0xaa, 0xaa, 0xea, 0xba, 0xb2, 0x0c, 0x44, 0xd1, 0x34, 0x55,
            0x53, 0x55, 0x5d, 0x57, 0x96, 0x81, 0x69, 0x9a, 0xa6, 0xaa, 0xba, 0xae, 0x2b, 0xcb, 0x32, 0xc0,
            0x34, 0x55, 0xd5, 0x75, 0x5d, 0x59, 0x96, 0x01, 0xaa, 0xea, 0xba, 0xae, 0x2b, 0xcb, 0xb6, 0x0d,
            0x50, 0x55, 0x57, 0x75, 0x5d, 0x59, 0x96, 0x65, 0x80, 0xeb, 0xba, 0xae, 0x2c, 0xcb, 0xb2, 0x6d,
            0x03, 0x70, 0x5d, 0xd7, 0x95, 0x65, 0xdb, 0x16, 0x00, 0x00, 0x70, 0xe0, 0x00, 0x00, 0x10, 0x60,
            0x04, 0x9d, 0x64, 0x54, 0x59, 0x84, 0x8d, 0x26, 0x5c, 0x78, 0x00, 0x0a, 0x0d, 0x59, 0x11, 0x00,
            0x44, 0x01, 0x00, 0x00, 0xc6, 0x30, 0xa5, 0x98, 0x52, 0x86, 0x31, 0x09, 0x21, 0x85, 0xd0, 0x30,
            0x26, 0x21, 0xa4, 0x10, 0x32, 0x29, 0x25, 0xa5, 0x54, 0x4a, 0x05, 0x21, 0xa5, 0x92, 0x4a, 0xa9,
            0x20, 0xa4, 0x92, 0x52, 0x29, 0x19, 0x95, 0x94, 0x52, 0x4a, 0xa9, 0x82, 0x90, 0x4a, 0x49, 0xa9,
            0x54, 0x10, 0x52, 0x29, 0xa9, 0xa4, 0x02, 0x00, 0xc0, 0x0e, 0x1c, 0x00, 0xc0, 0x0e, 0x2c, 0x84,
            0x42, 0x43, 0x56, 0x02, 0x00, 0x79, 0x00, 0x00, 0x84, 0x31, 0x4a, 0x31, 0xc6, 0x18, 0x73, 0x10,
            0x21, 0xa5, 0x18, 0x73, 0xce, 0x39, 0x88, 0x90, 0x52, 0x8c, 0x39, 0xe7, 0x9c, 0x64, 0x8c, 0x31,
            0xc6, 0x9c, 0x73, 0x4e, 0x4a, 0xc9, 0x18, 0x63, 0xce, 0x39, 0x27, 0xa5, 0x74, 0xce, 0x39, 0xe7,
            0x9c, 0x93, 0x52, 0x3a, 0xe7, 0x9c, 0x73, 0xce, 0x49, 0x29, 0x9d, 0x73, 0xce, 0x39, 0xe7, 0xa4,
            0x94, 0x52, 0x3a, 0xe7, 0x9c, 0x73, 0x52, 0x4a, 0x29, 0x21, 0x74, 0xce, 0x39, 0x29, 0xa5, 0x94,
            0xce, 0x39, 0xe7, 0x9c, 0x00, 0x00, 0xa0, 0x02, 0x07, 0x00, 0x80, 0x00, 0x1b, 0x45, 0x36, 0x27,
            0x18, 0x09, 0x2a, 0x34, 0x64, 0x25, 0x00, 0x90, 0x0a, 0x00, 0x60, 0x70, 0x1c, 0xcb, 0xd2, 0x34,
            0x4d, 0xf3, 0x3c, 0x51, 0xd4, 0x24, 0x49, 0xd3, 0x3c, 0xcf, 0xf3, 0x44, 0xd1, 0x34, 0x35, 0x4b,
            0xd2, 0x34, 0xcf, 0xf3, 0x3c, 0x51, 0x34, 0x4d, 0x9e, 0xe7, 0x79, 0xa2, 0x28, 0x8a, 0xa6, 0xa9,
            0xaa, 0x3c, 0xcf, 0xf3, 0x44, 0x51, 0x14, 0x4d, 0x53, 0x55, 0xb9, 0xae, 0x28, 0x9a, 0xa6, 0x69,
            0xaa, 0xaa, 0xaa, 0x92, 0x65, 0x51, 0x14, 0x45, 0xd3, 0x54, 0x55, 0xd5, 0x85, 0x68, 0x9a, 0xa6,
            0xaa, 0xba, 0xaa, 0xeb, 0xc2, 0x34, 0x45, 0x51, 0x55, 0x5d, 0xd7, 0x75, 0x21, 0xcb, 0xa6, 0xa9,
            0xaa, 0xae, 0x2b, 0xcb, 0xb0, 0x6d, 0xd3, 0x54, 0x55, 0xd7, 0x95, 0x65, 0xa0, 0xba, 0xaa, 0x2a,
            0xbb, 0xb2, 0x0c, 0x5c, 0x57, 0x55, 0x65, 0xd7, 0xb6, 0x05, 0x00, 0x80, 0x27, 0x38, 0x00, 0x00,
            0x15, 0xd8, 0xb0, 0x3a, 0xc2, 0x49, 0xd1, 0x58, 0x60, 0xa1, 0x21, 0x2b, 0x01, 0x80, 0x0c, 0x00,
            0x00, 0xc2, 0x18, 0x63, 0x14, 0x42, 0x08, 0x29, 0x84, 0x10, 0x52, 0x4a, 0x21, 0xa4, 0x94, 0x42,
            0x48, 0x00, 0x00, 0xc0, 0x80, 0x03, 0x00, 0x40, 0x80, 0x09, 0x65, 0xa0, 0xd0, 0x90, 0x95, 0x00,
            0x40, 0x14, 0x00, 0x00, 0x40, 0x08, 0xa5, 0x94, 0x52, 0x4a, 0x98, 0xa2, 0x94, 0x52, 0x4a, 0x8d,
            0x63, 0x94, 0x52, 0x4a, 0x29, 0xa5, 0x94, 0x52, 0x4a, 0x29, 0xa5, 0x94, 0x52, 0x4a, 0x29, 0xa5,
            0x94, 0x52, 0x6a, 0xad, 0xb5, 0xd6, 0x5a, 0x6b, 0xad, 0xb5, 0xd6, 0x5a, 0x6b, 0xad, 0xb5, 0xd6,
            0x5a, 0x6b, 0xad, 0xb5, 0xd6, 0x5a, 0x2b, 0x00, 0x40, 0x77, 0xc2, 0x01, 0x40, 0xf7, 0xc1, 0x06,
            0x4d, 0x89, 0xc5, 0x01, 0x0a, 0x0d, 0x59, 0x09, 0x00, 0xa4, 0x02, 0x00, 0x00, 0xc6, 0x28, 0xc5,
            0x18, 0x84, 0x92, 0x5a, 0xab, 0x10, 0x62, 0xcc, 0x39, 0x29, 0x2d, 0xb5, 0x56, 0x21, 0xc4, 0x98,
            0x73, 0x52, 0x5a, 0x6a, 0x2d, 0x68, 0xcc, 0x39, 0x08, 0xa5, 0xb4, 0x16, 0x63, 0xd1, 0x98, 0x63,
            0x10, 0x4a, 0x69, 0xad, 0xc5, 0x64, 0x4a, 0xe7, 0xa4, 0xa4, 0xd4, 0x5a, 0xac, 0x49, 0x95, 0x8e,
            0x49, 0x49, 0xa9, 0xb5, 0xd8, 0x92, 0x52, 0xa6, 0x94, 0x92, 0x52, 0x6b, 0x31, 0x26, 0xa5, 0x54,
            0x48, 0xa1, 0xb6, 0xd8, 0x62, 0x4c, 0xce, 0xc8, 0x9a, 0x52, 0x6b, 0x31, 0xd6, 0xd8, 0x9c, 0xd3,
            0x31, 0x95, 0x98, 0x62, 0xac, 0xb1, 0x39, 0xe7, 0x9c, 0xac, 0xad, 0xc5, 0x18, 0x63, 0x73, 0xce,
            0x39, 0x19, 0x5b, 0xeb, 0x31, 0xc7, 0x02, 0x00, 0x30, 0x1b, 0x1c, 0x00, 0x20, 0x12, 0x6c, 0x58,
            0x1d, 0xe1, 0xa4, 0x68, 0x2c, 0xb0, 0xd0, 0x90, 0x95, 0x00, 0x40, 0x48, 0x00, 0x00, 0x81, 0x90,
            0x52, 0x8c, 0x31, 0xe6, 0x9c, 0x73, 0xce, 0x39, 0x27, 0x95, 0x62, 0x8c, 0x31, 0xe7, 0x20, 0x84,
            0x10, 0x42, 0x08, 0xa5, 0x52, 0x8c, 0x39, 0xe6, 0x1c, 0x84, 0x10, 0x42, 0x08, 0xa1, 0x64, 0x8c,
            0x31, 0xe7, 0x1c, 0x84, 0x10, 0x42, 0x08, 0x21, 0x94, 0x52, 0x32, 0xe6, 0x1c, 0x74, 0x10, 0x42,
            0x28, 0x21, 0x94, 0x52, 0x52, 0xe7, 0x9c, 0x83, 0x10, 0x42, 0x08, 0xa1, 0x84, 0x52, 0x4a, 0xe9,
            0x9c, 0x73, 0x10, 0x42, 0x08, 0x21, 0x84, 0x52, 0x52, 0x29, 0x9d, 0x83, 0x10, 0x42, 0x08, 0x21,
            0x84, 0x50, 0x4a, 0x29, 0x25, 0xa5, 0xce, 0x41, 0x08, 0x21, 0x84, 0x10, 0x42, 0x49, 0x29, 0xa5,
            0x14, 0x42, 0x08, 0x21, 0x84, 0x50, 0x42, 0x09, 0x29, 0x95, 0x94, 0x42, 0x08, 0x21, 0x84, 0x10,
            0x42, 0x28, 0x21, 0xa5, 0x92, 0x52, 0x08, 0x21, 0x84, 0x10, 0x42, 0x08, 0x25, 0xa4, 0x92, 0x52,
            0x4a, 0x29, 0x84, 0x12, 0x42, 0x08, 0x21, 0x84, 0x92, 0x4a, 0x4a, 0x29, 0x95, 0x52, 0x4a, 0x08,
            0x21, 0x84, 0x50, 0x52, 0x4a, 0x29, 0xa5, 0x50, 0x42, 0x08, 0x21, 0x84, 0x10, 0x52, 0x2a, 0x29,
            0xa5, 0x52, 0x4a, 0x08, 0x21, 0x84, 0x10, 0x42, 0x49, 0x25, 0xa5, 0x94, 0x52, 0x0a, 0x21, 0x84,
            0x10, 0x42, 0x28, 0x00, 0x00, 0xe0, 0xc0, 0x01, 0x00, 0x20, 0xc0, 0x08, 0x3a, 0xc9, 0xa8, 0xb2,
            0x08, 0x1b, 0x4d, 0xb8, 0xf0, 0x00, 0x14, 0x1a, 0xb2, 0x12, 0x00, 0x88, 0x02, 0x00, 0x80, 0x0c,
            0x94, 0x50, 0x52, 0x8b, 0x0d, 0x40, 0x8c, 0x41, 0x6a, 0xb5, 0x43, 0x0c, 0x3a, 0x89, 0x31, 0x96,
            0x0c, 0x1a, 0xc5, 0xa4, 0xd5, 0x50, 0x31, 0xa5, 0x98, 0xb4, 0x16, 0x3a, 0xc8, 0x14, 0x73, 0xd4,
            0x52, 0x4a, 0xa1, 0x63, 0x4e, 0x5a, 0x8b, 0xb5, 0xa5, 0x10, 0x42, 0x6b, 0x41, 0xe8, 0xde, 0x4a,
            0x8a, 0x01, 0x00, 0x00, 0x10, 0x04, 0x00, 0x04, 0x98, 0x00, 0x02, 0x03, 0x04, 0x05, 0x5f, 0x08,
            0x01, 0x31, 0x06, 0x00, 0x20, 0x08, 0x91, 0x19, 0x22, 0xa1, 0xb0, 0x0a, 0x16, 0x18, 0x94, 0x41,
            0x83, 0xc3, 0x3c, 0x00, 0x78, 0x80, 0x88, 0x90, 0x08, 0x00, 0x12, 0x13, 0x14, 0x69, 0x17, 0x17,
            0xd0, 0x65, 0x80, 0x0b, 0xba, 0xb8, 0xeb, 0x40, 0x08, 0x41, 0x08, 0x42, 0x10, 0x8b, 0x03, 0x28,
            0x20, 0x01, 0x07, 0x27, 0xdc, 0xf0, 0xc4, 0x1b, 0x9e, 0x70, 0x83, 0x13, 0x74, 0x8a, 0x4a, 0x1d,
            0x08, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x02, 0x00, 0x3c, 0x00, 0x00, 0x20, 0x14, 0x40, 0x44, 0x44,
            0x33, 0x57, 0x61, 0x71, 0x81, 0x91, 0xa1, 0xb1, 0xc1, 0xd1, 0xe1, 0xf1, 0x01, 0x22, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x58, 0x00, 0xf0, 0x01, 0x00, 0x80, 0x84, 0x00, 0x11, 0x11, 0xcd, 0x5c, 0x85,
            0xc5, 0x05, 0x46, 0x86, 0xc6, 0x06, 0x47, 0x87, 0xc7, 0x07, 0x48, 0x00, 0x00, 0x20, 0x80, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x08, 0x20, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00,
            0x00, 0x00, 0x00, 0x01, 0x01, 0x4f, 0x67, 0x67, 0x53, 0x00, 0x04, 0xd0, 0x22, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x97, 0x0a, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0xd3, 0x31, 0x8f, 0x42, 0x0c,
            0x2c, 0x24, 0x27, 0xab, 0x98, 0x93, 0x9d, 0x8d, 0x74, 0x01, 0x01, 0x01, 0xe4, 0x4e, 0xee, 0x69,
            0x5a, 0xee, 0xe4, 0x9e, 0xa6, 0xed, 0x4d, 0x2a, 0x21, 0x33, 0x31, 0x81, 0x8a, 0xaa, 0xaa, 0x22,
            0xe6, 0xff, 0xe7, 0xfb, 0xf5, 0x61, 0xf2, 0x2f, 0xbf, 0xa7, 0xd1, 0xfd, 0xfc, 0xf9, 0xf3, 0xff,
            0x22, 0xc9, 0x24, 0xa1, 0xd2, 0x2e, 0xc3, 0x0e, 0x3c, 0x6d, 0x6c, 0x88, 0x3f, 0x6d, 0x6c, 0x88,
            0xaf, 0x5e, 0x22, 0x2b, 0x9a, 0x86, 0xd8, 0x2d, 0x22, 0x72, 0x7a, 0x26, 0xf3, 0x9c, 0x74, 0x44,
            0xf8, 0xfe, 0x7b, 0x7e, 0xac, 0xd9, 0x6e, 0xe3, 0x5d, 0x59, 0xe7, 0x16, 0x2c, 0x6d, 0x6c, 0xd4,
            0x2c, 0x6d, 0x6c, 0xd4, 0xcc, 0x66, 0x88, 0x99, 0x88, 0x62, 0x33, 0x4d, 0x87, 0x43, 0x28, 0xc5,
            0x11, 0xa2, 0x99, 0x34, 0x7a, 0xfc, 0x66, 0x95, 0xf7, 0xe3, 0xd2, 0x79, 0xcf, 0xb7, 0xfb, 0x9a,
            0x09, 0xe5, 0x02, 0xfa, 0x38, 0x0e, 0x80, 0xca, 0x4d, 0x2a, 0xd0, 0x08, 0x80, 0xd1, 0xe3, 0x38,
            0x00, 0x2a, 0x37, 0xa9, 0x40, 0x23, 0x00, 0x46, 0xfe, 0x48, 0x19, 0x59, 0xcd, 0x86, 0x32, 0xca,
            0x22, 0x35, 0x16, 0x65, 0x81, 0x32, 0x20, 0x82, 0x25, 0xc8, 0x58, 0x9e, 0x3d, 0xfb, 0xbb, 0x99,
            0x89, 0x98, 0x19, 0xea, 0x86, 0x08, 0xa6, 0xae, 0x66, 0x86, 0xa9, 0xb9, 0x9b, 0x45, 0x26, 0x44,
            0x43, 0x9a, 0x44, 0x9a, 0x24, 0x69, 0x4a, 0xa8, 0x2c, 0x44, 0x64, 0x79, 0x67, 0x8a, 0xb3, 0x59,
            0x1e, 0xc7, 0x62, 0x62, 0x8b, 0xda, 0x51, 0xe6, 0xb7, 0x9e, 0x12, 0xdb, 0x5e, 0x81, 0x64, 0xd0,
            0xd3, 0x74, 0x30, 0xd3, 0x1e, 0x24, 0xe4, 0xbb, 0x19, 0xa6, 0x58, 0xa3, 0xe3, 0xea, 0xdd, 0x77,
            0x2a, 0x92, 0x71, 0xcb, 0x0d, 0xd3, 0x6a, 0xd4, 0xb8, 0x33, 0x36, 0xe3, 0x6c, 0x09, 0x07, 0xab,
            0xf9, 0xa1, 0xf3, 0xa0, 0x56, 0xe5, 0xcb, 0xa5, 0x9f, 0xdb, 0x8d, 0x2c, 0x9b, 0xec, 0xdc, 0xde,
            0x37, 0xdb, 0xef, 0x19, 0xb7, 0x41, 0x48, 0x1f, 0x7a, 0x5e, 0x1a, 0xaf, 0x5b, 0xd7, 0x5d, 0xc6,
            0x25, 0xd2, 0x9c, 0xa9, 0x46, 0x43, 0x61, 0x16, 0x27, 0xc5, 0x2e, 0x17, 0x5b, 0x01, 0xde, 0x78,
            0xb5, 0xb4, 0x4a, 0x27, 0x00, 0x16, 0xe1, 0x83, 0xc6, 0xab, 0xa5, 0x55, 0x3a, 0x01, 0xb0, 0x08,
            0x1f, 0x38, 0xe5, 0x67, 0x39, 0xcb, 0xf3, 0x3f, 0x0d, 0x51, 0xa6, 0xc8, 0xac, 0xae, 0xa7, 0x9e,
            0xc2, 0x8a, 0xac, 0xa8, 0xac, 0x28, 0xca, 0xc8, 0xda, 0x8a, 0x8c, 0x52, 0xd6, 0x54, 0xa6, 0xad,
            0x68, 0x22, 0xd1, 0xa6, 0x49, 0xce, 0x44, 0x11, 0x27, 0xe3, 0x76, 0xf6, 0xc4, 0xfa, 0x6e, 0x6f,
            0x89, 0x65, 0x35, 0x97, 0x19, 0xed, 0xaf, 0xb7, 0xa3, 0x6e, 0xfa, 0x78, 0x2a, 0x2f, 0x7b, 0x4b,
            0xb4, 0xdf, 0xed, 0xd3, 0xae, 0x9c, 0xda, 0xa7, 0x9d, 0x4f, 0x97, 0x24, 0x62, 0xcd, 0x76, 0x09,
            0x46, 0xdd, 0x7c, 0x0b, 0xb1, 0x84, 0xd5, 0x0e, 0xe5, 0x5a, 0x1d, 0xda, 0x47, 0xa3, 0xd9, 0x39,
            0x20, 0xbc, 0xdf, 0xa1, 0x38, 0xcc, 0x78, 0x63, 0xbb, 0xb0, 0xaf, 0x3d, 0x5e, 0x72, 0x75, 0xc6,
            0x98, 0x6d, 0x22, 0x41, 0x4c, 0xb3, 0x8f, 0x61, 0xc8, 0x91, 0xa4, 0x59, 0x21, 0x37, 0x56, 0x16,
            0xef, 0xd0, 0xc7, 0x42, 0x4e, 0x04, 0xfe, 0x18, 0x45, 0xc9, 0xe6, 0x41, 0x01, 0x89, 0xf0, 0xc1,
            0x63, 0x14, 0x25, 0x9b, 0x07, 0x05, 0x24, 0xc2, 0x07, 0x3e, 0x10, 0x18, 0x82, 0x2c, 0x9b, 0xcd,
            0x7f, 0xa2, 0x80, 0x58, 0x45, 0xd5, 0x44, 0x9d, 0x9c, 0xd4, 0x21, 0xd5, 0x45, 0x5d, 0x11, 0x15,
            0x75, 0x6b, 0xaa, 0x23, 0xa2, 0xac, 0x94, 0xe4, 0x94, 0x23, 0x1b, 0x69, 0x4b, 0xa3, 0x49, 0x1c,
            0xf4, 0x67, 0xe3, 0xbd, 0x0c, 0xce, 0xd9, 0x27, 0x71, 0x57, 0x39, 0xe9, 0xd0, 0xce, 0xb6, 0x99,
            0xe3, 0x3d, 0xcd, 0x66, 0xfa, 0x31, 0x17, 0x8e, 0x3e, 0x04, 0xa5, 0x54, 0xef, 0x61, 0x48, 0xd2,
            0x51, 0x23, 0x5b, 0x20, 0x77, 0x54, 0xff, 0xe8, 0x4e, 0xf6, 0xc0, 0xfe, 0xa2, 0xe3, 0xe1, 0xaa,
            0xef, 0xdb, 0x71, 0x10, 0xca, 0xea, 0x91, 0xa0, 0x7f, 0x39, 0xab, 0xdd, 0x8f, 0x28, 0x88, 0xbb,
            0x7b, 0xd6, 0x7e, 0xe7, 0x0a, 0xb9, 0x5e, 0xa0, 0xae, 0xa8, 0x28, 0xf5, 0x25, 0x30, 0x53, 0x30,
            0x42, 0x25, 0x95, 0x3e, 0xb9, 0x2c, 0x2a, 0xd5, 0x05, 0x7e, 0x38, 0x75, 0xa3, 0xca, 0x97, 0xcc,
            0xd2, 0x07, 0x44, 0xf3, 0x98, 0x0b, 0x0e, 0xa7, 0x6e, 0x54, 0xf9, 0x92, 0x59, 0xfa, 0x80, 0x68,
            0x1e, 0x73, 0x81, 0xd3, 0x9e, 0x60, 0x46, 0xf2, 0xff, 0x1f, 0x90, 0xaa, 0xea, 0x5b, 0x53, 0x55,
            0x6f, 0x51, 0x19, 0x45, 0x44, 0x64, 0x51, 0x99, 0x75, 0xb2, 0xa8, 0x24, 0xca, 0x48, 0xca, 0x9a,
            0xb2, 0xa2, 0x2c, 0x32, 0x41, 0x97, 0x9b, 0x91, 0xa7, 0x18, 0xf9, 0xce, 0x15, 0x1f, 0x4b, 0xab,
            0x07, 0xd9, 0xe5, 0xb8, 0xdc, 0x2f, 0x7d, 0x6e, 0x38, 0x87, 0x9e, 0x75, 0x10, 0xfb, 0x25, 0x54,
            0x5e, 0x04, 0x41, 0xc8, 0x46, 0xa3, 0xd8, 0x19, 0x9c, 0x62, 0xc5, 0x71, 0x32, 0x96, 0x2f, 0xe2,
            0x4d, 0x0e, 0x5f, 0x8e, 0x8c, 0x20, 0x5b, 0xd6, 0xbc, 0xe8, 0xa7, 0x95, 0x0d, 0x37, 0xcd, 0x1d,
            0x0a, 0x6a, 0x22, 0x2b, 0x59, 0x8f, 0x37, 0xe5, 0x72, 0x62, 0x33, 0xb9, 0x1d, 0x15, 0x7d, 0x8b,
            0x50, 0xd7, 0x43, 0xae, 0xbc, 0xf2, 0x25, 0xc7, 0x80, 0xf2, 0x25, 0x7c, 0xf5, 0x05, 0x0e, 0x13,
            0x3a, 0x30, 0xbc, 0x8b, 0xc3, 0x02, 0x5e, 0xa8, 0x5d, 0xae, 0xf0, 0x9e, 0x0c, 0xd1, 0x98, 0x18,
            0xe6, 0x16, 0x6a, 0x97, 0x2b, 0xbc, 0x27, 0x43, 0x34, 0x26, 0x86, 0xb9, 0x42, 0x72, 0x9e, 0x9f,
            0x9d, 0x7f, 0x4c, 0xb1, 0x9f, 0x62, 0x6f, 0xd2, 0x34, 0x69, 0x26, 0x2b, 0x52, 0x11, 0x91, 0x95,
            0x66, 0xda, 0x6c, 0x26, 0xa7, 0x24, 0x0d, 0x1a, 0x3d, 0x9e, 0xa9, 0xbe, 0xc1, 0xdb, 0xfa, 0x15,
            0x6d, 0x9c, 0x94, 0x3b, 0xf4, 0x27, 0xde, 0xce, 0x19, 0x7a, 0x3a, 0xc6, 0x43, 0x4a, 0x4b, 0x7b,
            0x7e, 0x38, 0x7c, 0x1c, 0x81, 0xc3, 0xc9, 0x2c, 0xf8, 0x18, 0xe8, 0x58, 0xd1, 0xda, 0x7c, 0xca,
            0x90, 0x24, 0x72, 0x57, 0xd8, 0xd2, 0x36, 0x86, 0xba, 0xb2, 0x2f, 0xdc, 0xee, 0xfb, 0xbe, 0x07,
            0xd8, 0x5f, 0xd0, 0xf7, 0x21, 0x54, 0x0c, 0xf5, 0xf8, 0x1d, 0x06, 0xb9, 0xaf, 0xef, 0xaf, 0xf4,
            0x2f, 0xc9, 0x15, 0x84, 0x71, 0x0e, 0x8e, 0x4f, 0xb6, 0x21, 0x9b, 0xf3, 0xc6, 0xe2, 0x06, 0x4b,
            0xba, 0x67, 0x00, 0x9e, 0xc8, 0xfd, 0xbb, 0xf5, 0x5f, 0x84, 0x81, 0xa6, 0x80, 0x37, 0x91, 0xfb,
            0x77, 0x1b, 0xbf, 0x08, 0x03, 0xa3, 0x6b, 0xe0, 0x05, 0xc8, 0x43, 0xa1, 0x43, 0xd3, 0x23, 0x22,
            0x4d, 0xac, 0xec, 0xfe, 0x66, 0xc0, 0x0d, 0xde, 0x39, 0x5f, 0x8f, 0x4f, 0x72, 0xb7, 0x64, 0x0f,
            0x4d, 0xa5, 0xa3, 0x71, 0x9a, 0xa6, 0xd1, 0xff, 0x32, 0x93, 0x25, 0xe3, 0x41, 0x83, 0x34, 0x24,
            0x8b, 0xc3, 0xc9, 0xc8, 0x71, 0x5c, 0x59, 0xac, 0x89, 0xc3, 0x5b, 0x86, 0x8f, 0xa6, 0xd2, 0x14,
            0x8e, 0xdf, 0x1d, 0xee, 0xd7, 0x0e, 0xbe, 0x04, 0x3a, 0xc6, 0x61, 0x02, 0x5f, 0xa9, 0x9c, 0x74,
            0x3c, 0x81, 0x5c, 0x5f, 0x6e, 0x02, 0x56, 0xf6, 0xeb, 0x21, 0xfa, 0x72, 0x4c, 0x18, 0x5f, 0x42,
            0x1b, 0x74, 0xa0, 0x8d, 0x02, 0xd5, 0x05, 0x0e, 0x0e, 0x0e
        };
        return vse::OpenFile(std::shared_ptr<const void>(file_data, [](auto) {}), std::size(file_data));
    }
}
