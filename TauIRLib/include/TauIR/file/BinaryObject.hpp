#pragma once

#include <NumTypes.hpp>
#include <String.hpp>
#include <cstdio>
#include <unordered_map>
#include <ConPrinter.hpp>

namespace tau::ir {

class Module;

}

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

struct FileVersion final
{
    DEFAULT_CONSTRUCT_PUC(FileVersion);
    DEFAULT_DESTRUCT(FileVersion);
    DEFAULT_CM_PUC(FileVersion);
public:
    u16 PatchVersion;
    u8 MinorVersion;
    u8 MajorVersion;

    constexpr FileVersion(const u8 majorVersion, const u8 minorVersion, const u16 patchVersion = 0) noexcept
        : PatchVersion(patchVersion)
        , MinorVersion(minorVersion)
        , MajorVersion(majorVersion)
    { }
};

static inline constexpr FileVersion BreakFileVersion(const u32 version) noexcept
{
    return FileVersion(static_cast<u8>((version >> 24) & 0xFF), static_cast<u8>((version >> 16) & 0xFF), static_cast<u16>(version & 0xFFFF));
}

inline constexpr u32 FileVersion0 = MakeFileVersion(0, 0);
inline constexpr u32 TauIRVersion0 = MakeFileVersion(0, 0);

namespace v0_0 {

inline constexpr uSys StringSectionAbsoluteIndex = 0;

inline constexpr C8ConstExprString StringSectionName(u8".strings");
inline constexpr C8ConstExprString ModuleInfoSectionName(u8".info");
inline constexpr C8ConstExprString ImportsSectionName(u8".imports");
inline constexpr C8ConstExprString ExportsSectionName(u8".exports");
inline constexpr C8ConstExprString TypesSectionName(u8".types");
inline constexpr C8ConstExprString GlobalsSectionName(u8".globals");
inline constexpr C8ConstExprString FunctionsSectionName(u8".functions");
inline constexpr C8ConstExprString CodeSectionName(u8".code");

#pragma pack(push, 1)
struct FileHeader final
{
    // Should be "TIRE".
    u8 Magic[4];
    // The version of the executable file schema.
    u32 Version;
    // The total number of bytes in the file, excluding this header.
    u64 FileSize;
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

using StringSectionMap = ::std::unordered_map<u64, C8DynString>;

struct StringSection final
{
    // The number of strings in the string section.
    u32 StringCount;
    // 32 Bytes reserved for future use.
    u64 Reserved[4];
    // A hashmap of name pointers to strings.
    StringSectionMap Strings;
};

struct ModuleInfoSection final
{
    u32 ModuleVersion;
    // The version of TauIR this module is reliant on.
    u32 TauIRVersion;
    // A pointer to the string name of this module.
    u64 NamePointer;
    // A pointer to the string description of this module.
    u64 DescriptionPointer;
    // A pointer to the string author of this module.
    u64 AuthorPointer;
    // A pointer to the string website of this module.
    u64 WebsitePointer;
    u64 Reserved[8];
};

struct FunctionPort final
{
    u64 NamePointer;
    // If the high bit is set, then this is a global variable, otherwise a function
    u32 FunctionOrGlobalIndex;
};

struct ModuleImport final
{
    u64 ModuleNamePointer;
    u32 ImportCount;
    FunctionPort Functions[];
};

struct ImportsSection final
{
    u16 ModuleCount;
    ModuleImport* Modules[];
};

struct ExportsSection final
{
    u32 FunctionCount;
    FunctionPort Functions[];
};

struct TypeData final
{
    u64 TypeNamePointer;
    u32 TypeSize;
};

struct TypesSection final
{
    u32 TypeCount;
    TypeData Types[];
};

struct GlobalsSection final
{
    u32 GlobalCount;
    u32 GlobalTypeIndexes[];
};

struct FunctionLocals final
{
    u16 LocalCount;
    u32 LocalTypeIndexes[];
};

struct FunctionArgument final
{
    bool IsRegister;
    uSys RegisterOrStackOffset;
};

struct FunctionArguments final
{
    u16 ArgumentCount;
    FunctionArgument Arguments[];
};

struct FunctionMetadata final
{
    // The pointer beginning at the start of the code section, not including the length.
    u64 CodePointer;
    // The pointer to the name. 0 is at the start of FileHeader.
    u64 NamePointer;
    // The length in the number of bytes of the code block
    u64 CodeSize;
    u32 Flags;
    u64 Reserved[8];
    FunctionLocals* Locals;
    FunctionArguments* Arguments;
};

struct FunctionsSection final
{
    u32 FunctionCount;
    FunctionMetadata Functions[];
};

struct CodeSection final
{
    u64 Length;
    u8* Code;
};
#pragma pack(pop)

void FreeFile(void* pointer) noexcept;
void FreeFile(ImportsSection* pointer) noexcept;
void FreeFile(FunctionsSection* pointer) noexcept;
void FreeFile(CodeSection* pointer) noexcept;

[[nodiscard]] FileHeader* ReadFileHeader(FILE* file) noexcept;
[[nodiscard]] SectionHeader* ReadSectionHeader(FILE* file, const FileHeader* fileHeader) noexcept;
[[nodiscard]] StringSection* ReadStringSection(FILE* file, const FileHeader* fileHeader, const SectionHeader* sectionHeader) noexcept;
[[nodiscard]] ModuleInfoSection* ReadModuleInfoSection(FILE* file, const FileHeader* fileHeader, const SectionHeader* sectionHeader, const StringSection* stringSection) noexcept;
[[nodiscard]] ImportsSection* ReadImportsSection(FILE* file, const FileHeader* fileHeader, const SectionHeader* sectionHeader, const StringSection* stringSection) noexcept;
[[nodiscard]] ExportsSection* ReadExportsSection(FILE* file, const FileHeader* fileHeader, const SectionHeader* sectionHeader, const StringSection* stringSection) noexcept;
[[nodiscard]] TypesSection* ReadTypesSection(FILE* file, const FileHeader* fileHeader, const SectionHeader* sectionHeader, const StringSection* stringSection) noexcept;
[[nodiscard]] GlobalsSection* ReadGlobalsSection(FILE* file, const FileHeader* fileHeader, const SectionHeader* sectionHeader, const StringSection* stringSection) noexcept;
[[nodiscard]] FunctionsSection* ReadFunctionsSection(FILE* file, const FileHeader* fileHeader, const SectionHeader* sectionHeader, const StringSection* stringSection) noexcept;
[[nodiscard]] CodeSection* ReadCodeSection(FILE* file, const FileHeader* fileHeader, const SectionHeader* sectionHeader, const StringSection* stringSection) noexcept;

[[nodiscard]] i64 GetAbsoluteSectionOffset(const FileHeader* fileHeader, const SectionHeader* sectionHeader, const StringSection* stringSection, const C8StringBase& targetSection) noexcept;

i64 WriteFileHeader(FILE* file) noexcept;
i64 WriteStringSection(FILE* file, i64 zeroPointer, const C8DynString* strings, u32 count, u64* pointers) noexcept;
i64 WriteSectionHeader(FILE* file, i64 zeroPointer, u64 stringSectionPointer, u64 stringSectionNamePointer, const u64* namePointers, u16 count, u64* pointers) noexcept;
i64 WriteModuleInfoSection(FILE* file, i64 zeroPointer, u64 moduleInfoSectionPointer, const ModuleInfoSection& moduleInfo) noexcept;

void WriteFunctionSection(FILE* file, i64 zeroPointer, u64 functionSectionPointer, const Module* module, const u64* namePointers, u64* codePointers) noexcept;

void WriteFinal(FILE* file, i64 zeroPointer) noexcept;

}

}


namespace ConPrinter {

inline u32 Print(const ::tau::ir::file::FileVersion d) noexcept { return Print(u8"{}.{}.{}", d.MajorVersion, d.MinorVersion, d.PatchVersion); }

inline u32 Print(const ::tau::ir::file::v0_0::FileHeader* fileHeader) noexcept
{
    u32 ret = 0;
    ret += ConPrinter::PrintLn("File Header:");
    ret += ConPrinter::PrintLn("    Magic: {}{}{}{}", static_cast<c8>(fileHeader->Magic[0]), static_cast<c8>(fileHeader->Magic[1]), static_cast<c8>(fileHeader->Magic[2]), static_cast<c8>(fileHeader->Magic[3]));
    ret += ConPrinter::PrintLn("    Version: {}", ::tau::ir::file::BreakFileVersion(fileHeader->Version));
    ret += ConPrinter::PrintLn("    File Size: {}", fileHeader->FileSize);
    ret += ConPrinter::PrintLn("    Checksum: {XP}", fileHeader->Checksum);
    ret += ConPrinter::PrintLn("    Zero Pointer: {}", fileHeader->ZeroPointer);
    ret += ConPrinter::PrintLn("    Section Header Pointer: {}", fileHeader->SectionHeaderPointer);
    return ret;
}

}
