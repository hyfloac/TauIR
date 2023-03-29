#pragma once

#include <Objects.hpp>
#include <NumTypes.hpp>
#include <cstdio>

namespace tau::ir::file {

inline constexpr u8 FileMagic[4] = { 'T', 'I', 'R', 'E' };

inline bool CheckMagic(const u8 magic[4])
{
    return magic[0] == FileMagic[0] && magic[1] == FileMagic[1] && magic[2] == FileMagic[2] && magic[3] == FileMagic[3];
}

static inline constexpr u32 MakeFileVersion(const u8 majorVersion, const u8 minorVersion, const u16 patchVersion = 0) noexcept
{
    return (static_cast<u32>(majorVersion) << 24) | (static_cast<u32>(minorVersion) << 16) | static_cast<u32>(patchVersion);
}

inline constexpr u32 FileVersion0 = MakeFileVersion(0, 0);

#pragma pack(push, 1)
struct FileHeader final
{
    // Should be "TIRE".
    u8 Magic[4];
    // The version of the executable file schema.
    u32 Version;
    // The total number of bytes in the file, excluding this header.
    u32 FileSize;
    // A CRC32 checksum of this file. This does not include this header.
    u32 Checksum;
    // A pointer from the true beginning of the file to the start of this header. This is used to package a binary file inside an PE file.
    u64 ZeroPointer;
    // An offset from the beginning of this header to the Section header.
    u64 SectionHeaderPointer;
    // 64 bytes reserved for future use.
    u64 Reserved[8];
};

struct SectionMapping final
{
    // The pointer to the name. 0 is at the start of FileHeader.
    u64 NamePointer;
    // The pointer to the section. 0 is at the start of FileHeader.
    u64 SectionPointer;
};

struct SectionHeader final
{
    // The number of sections.
    u16 SectionCount;
    // A flexible array of the mappings.
    SectionMapping Mappings[];
};
#pragma pack(pop)

void FreeFile(void* pointer) noexcept;
[[nodiscard]] FileHeader* ReadFileHeader(FILE* file) noexcept;
[[nodiscard]] SectionHeader* ReadSectionHeader(FILE* file, const FileHeader* fileHeader) noexcept;

}
