#include "TauIR/file/BinaryObject.hpp"
#include <ConPrinter.hpp>
#include <allocator/PageAllocator.hpp>

#include "TauIR/Function.hpp"
#include "TauIR/Module.hpp"

namespace tau::ir::file {

namespace v0_0 {

void FreeFile(void* pointer) noexcept
{
    operator delete(pointer);
}

void FreeFile(ImportsSection* pointer) noexcept
{
    if(!pointer)
    {
        return;
    }

    for(u16 i = 0; i < pointer->ModuleCount; ++i)
    {
        if(pointer->Modules[i])
        {
            FreeFile(pointer->Modules[i]);
        }
    }

    operator delete(pointer);
}

void FreeFile(FunctionsSection* pointer) noexcept
{
    if(!pointer)
    {
        return;
    }

    for(u32 i = 0; i < pointer->FunctionCount; ++i)
    {
        if(pointer->Functions[i].Locals)
        {
            FreeFile(pointer->Functions[i].Locals);
        }
        if(pointer->Functions[i].Arguments)
        {
            FreeFile(pointer->Functions[i].Arguments);
        }
    }

    operator delete(pointer);
}

void FreeFile(CodeSection* pointer) noexcept
{
    if(!pointer)
    {
        return;
    }

    PageAllocator::Free(pointer->Code);

    operator delete(pointer);
}

template<typename T>
bool ReadFileT(FILE* const file, T* const t) noexcept
{
    return fread_s(t, sizeof(T), 1, sizeof(T), file) == sizeof(T);
}

template<typename T, uSys Size>
bool ReadFileTArr(FILE* const file, T(&arr)[Size]) noexcept
{
    return fread_s(arr, sizeof(T) * Size, 1, sizeof(T) * Size, file) == sizeof(T) * Size;
}

template<typename T>
bool ReadFileTBuf(FILE* const file, T* const buf, const uSys size) noexcept
{
    return fread_s(buf, sizeof(T) * size, 1, sizeof(T) * size, file) == sizeof(T) * size;
}

template<typename T>
bool WriteFileT(FILE* const file, const T* const t) noexcept
{
    return fwrite(t, sizeof(T), 1, file) == 1;
}

template<typename T, uSys Size>
bool WriteFileTArr(FILE* const file, const T(&arr)[Size]) noexcept
{
    return fwrite(arr, sizeof(T) * Size, 1, file) == Size;
}

template<typename T>
bool WriteFileTBuf(FILE* const file, const T* const buf, const uSys size) noexcept
{
    return fwrite(buf, sizeof(T) * size, 1, file) == size;
}

bool ReadFileBuf(FILE* const file, void* const buf, const uSys size) noexcept
{
    return fread_s(buf, size, 1, size, file) == size;
}

static constexpr void GenerateCRCTable(u32(&table)[256])
{
    constexpr u32 polynomial = 0xEDB88320;

    for(u32 i = 0; i < 256; ++i)
    {
        u32 c = i;
        for(uSys j = 0; j < 8; ++j)
        {
            if(c & 1)
            {
                c = polynomial ^ (c >> 1);
            }
            else
            {
                c >>= 1;
            }
        }

        table[i] = c;
    }
}

static u32 CRCUpdate(const u32(&table)[256], const u32 initial, const void* const buf, const uSys len)
{
    u32 c = initial ^ 0xFFFFFFFF;
    // u32 c = initial;
    const u8* u = static_cast<const u8*>(buf);

    for(uSys i = 0; i < len; ++i)
    {
        c = table[(c ^ u[i]) & 0xFF] ^ (c >> 8);
    }

    return c ^ 0xFFFFFFFF;
    // return c;
}

static u32 CRCFile(FILE* const file, const i64 zeroPointer, const i64 fileSize) noexcept
{
    static u32 CrcTable[256];
    GenerateCRCTable(CrcTable);

    (void) _fseeki64(file, zeroPointer + static_cast<i64>(sizeof(FileHeader)), SEEK_SET);

    u64 remainingBytes = fileSize;

    u8 crcBuffer[256];

    u32 crc32 = 0;

    while(remainingBytes)
    {
        if(remainingBytes >= sizeof(crcBuffer))
        {
            ReadFileTArr(file, crcBuffer);
            crc32 = CRCUpdate(CrcTable, crc32, crcBuffer, sizeof(crcBuffer));

            remainingBytes -= sizeof(crcBuffer);
        }
        else
        {
            ReadFileTBuf(file, crcBuffer, remainingBytes);
            crc32 = CRCUpdate(CrcTable, crc32, crcBuffer, remainingBytes);
            break;
        }
    }

    return crc32;
}

FileHeader* ReadFileHeader(FILE* const file) noexcept
{
    if(!file)
    {
        ConPrinter::PrintLn("[ReadFileHeader]: File was null.");
        return nullptr;
    }

    const i64 zeroPointer = _ftelli64(file);

    if(_fseeki64(file, 0, SEEK_END))
    {
        ConPrinter::PrintLn("[ReadFileHeader]: File does not support seeking.");
        return nullptr;
    }

    const i64 totalFileSize = _ftelli64(file) + 1;
    (void) _fseeki64(file, zeroPointer, SEEK_SET);

    u8 magicBuffer[4];

    {
        const bool readSuccess = ReadFileTArr(file, magicBuffer);

        (void) _fseeki64(file, zeroPointer, SEEK_SET);

        if(!readSuccess)
        {
            ConPrinter::PrintLn("[ReadFileHeader]: File was not long enough for the magic.");
            return nullptr;
        }

        if(!CheckMagic(magicBuffer))
        {
            ConPrinter::PrintLn("[ReadFileHeader]: File magic did not match the designated magic 0x{XP} (\"{}{}{}{}\").", *reinterpret_cast<const u32*>(FileMagic), static_cast<c8>(FileMagic[0]), static_cast<c8>(FileMagic[1]), static_cast<c8>(FileMagic[2]), static_cast<c8>(FileMagic[3]));
            return nullptr;
        }
    }

    FileHeader* const header = new(::std::nothrow) FileHeader;

    if(!header)
    {
        (void) _fseeki64(file, zeroPointer, SEEK_SET);
        ConPrinter::PrintLn("[ReadFileHeader]: Failed to allocate space for file header.");
        return nullptr;
    }

    if(!ReadFileT(file, header))
    {
        delete header;
        (void) _fseeki64(file, zeroPointer, SEEK_SET);
        ConPrinter::PrintLn("[ReadFileHeader]: File was not long enough for header.");
        return nullptr;
    }

    if(zeroPointer > 0 && static_cast<u64>(zeroPointer) != header->ZeroPointer)
    {
        delete header;
        (void) _fseeki64(file, zeroPointer, SEEK_SET);
        ConPrinter::PrintLn("[ReadFileHeader]: The zero pointer {} in the header did not match the zero pointer of the file {}.", header->ZeroPointer, zeroPointer);
        return nullptr;
    }

    if((totalFileSize - zeroPointer) < static_cast<i64>(header->FileSize))
    {
        delete header;
        (void) _fseeki64(file, zeroPointer, SEEK_SET);
        ConPrinter::PrintLn("[ReadFileHeader]: The encompassing file size {} is less than the purported file size {}.", totalFileSize, header->FileSize);
        return nullptr;
    }

    if(header->FileSize <= header->SectionHeaderPointer)
    {
        delete header;
        (void) _fseeki64(file, zeroPointer, SEEK_SET);
        ConPrinter::PrintLn("[ReadFileHeader]: The file size {} is less than the pointer to the section table {}.", header->FileSize, header->SectionHeaderPointer);
        return nullptr;
    }

    const u32 crc32 = CRCFile(file, zeroPointer, static_cast<i64>(header->FileSize));

    if(crc32 != header->Checksum)
    {
        delete header;
        (void) _fseeki64(file, zeroPointer, SEEK_SET);
        ConPrinter::PrintLn("[ReadFileHeader]: The computed checksum {XP} does not match the target checksum {XP}.", crc32, header->Checksum);
        return nullptr;
    }

    (void) _fseeki64(file, zeroPointer + static_cast<i64>(sizeof(FileHeader)), SEEK_SET);

    return header;
}

SectionHeader* ReadSectionHeader(FILE* const file, const FileHeader* const fileHeader) noexcept
{
    if(!file)
    {
        ConPrinter::PrintLn("[ReadSectionHeader]: File was null.");
        return nullptr;
    }

    if(!fileHeader)
    {
        ConPrinter::PrintLn("[ReadSectionHeader]: File header was null.");
        return nullptr;
    }

    (void) _fseeki64(file, static_cast<i64>(fileHeader->SectionHeaderPointer + fileHeader->ZeroPointer), SEEK_SET);

    u16 sectionCount;

    if(!ReadFileT(file, &sectionCount))
    {
        ConPrinter::PrintLn("[ReadSectionHeader]: Failed to read the section count.");
        return nullptr;
    }

    const uSys mappingsBufferSize = sizeof(SectionHeader::Mappings[0]) * sectionCount;

    void* const placement = operator new(sizeof(SectionHeader) + mappingsBufferSize, ::std::nothrow);

    if(!placement)
    {
        ConPrinter::PrintLn("[ReadSectionHeader]: Failed to allocate space for section header.");
        return nullptr;
    }

    SectionHeader* const sectionHeader = ::new(placement) SectionHeader;

    sectionHeader->SectionCount = sectionCount;

    if(!ReadFileTBuf(file, sectionHeader->Mappings, sectionCount))
    {
        delete sectionHeader;
        ConPrinter::PrintLn("[ReadSectionHeader]: Failed to read the sections map.");
        return nullptr;
    }

    return sectionHeader;
}

StringSection* ReadStringSection(FILE* const file, const FileHeader* const fileHeader, const SectionHeader* const sectionHeader) noexcept
{
    if(!file)
    {
        ConPrinter::PrintLn("[ReadStringSection]: File was null.");
        return nullptr;
    }

    if(!fileHeader)
    {
        ConPrinter::PrintLn("[ReadStringSection]: File header was null.");
        return nullptr;
    }

    if(!sectionHeader)
    {
        ConPrinter::PrintLn("[ReadStringSection]: Section header was null.");
        return nullptr;
    }

    (void) _fseeki64(file, static_cast<i64>(sectionHeader->Mappings[StringSectionAbsoluteIndex].SectionPointer + fileHeader->ZeroPointer), SEEK_SET);

    u32 stringCount;

    if(!ReadFileT(file, &stringCount))
    {
        ConPrinter::PrintLn("[ReadStringSection]: Failed to read the string count.");
        return nullptr;
    }

    StringSection* stringSection = new(::std::nothrow) StringSection;

    if(!stringSection)
    {
        ConPrinter::PrintLn("[ReadStringSection]: Failed to allocate space for string section.");
        return nullptr;
    }

    stringSection->StringCount = stringCount;

    for(u32 i = 0; i < stringCount; ++i)
    {
        const u64 stringPointer = static_cast<u64>(_ftelli64(file)) - fileHeader->ZeroPointer;

        u32 stringLength;

        if(!ReadFileT(file, &stringLength))
        {
            delete stringSection;
            ConPrinter::PrintLn("[ReadStringSection]: Failed to read the string length for string #{}.", i);
            return nullptr;
        }

        void* const placement = ::TauUtilsAllocate(sizeof(c8) * stringLength + sizeof(ReferenceCounter::Type));

        if(!placement)
        {
            delete stringSection;
            ConPrinter::PrintLn("[ReadStringSection]: Failed to allocate space for string.");
            return nullptr;
        }

        ReferenceCounter::Type* refCount = ::new(placement) ReferenceCounter::Type(1);
        c8* const strBuf = reinterpret_cast<c8*>(refCount + 1);

        if(!ReadFileTBuf(file, strBuf, stringLength))
        {
            delete stringSection;
            ::TauUtilsDeallocate(placement);
            ConPrinter::PrintLn("[ReadStringSection]: Failed to read string #{}.", i);
            return nullptr;
        }

        stringSection->Strings[stringPointer] = C8DynString::PassControl(refCount, strBuf, stringLength - 1, nullptr, [](ReferenceCounter::Type* const rc) { ::TauUtilsDeallocate(rc); });
    }

    return stringSection;
}

ModuleInfoSection* ReadModuleInfoSection(FILE* file, const FileHeader* fileHeader, const SectionHeader* sectionHeader, const StringSection* stringSection) noexcept
{
    if(!file)
    {
        ConPrinter::PrintLn("[ReadModuleInfoSection]: File was null.");
        return nullptr;
    }

    if(!fileHeader)
    {
        ConPrinter::PrintLn("[ReadModuleInfoSection]: File header was null.");
        return nullptr;
    }

    if(!sectionHeader)
    {
        ConPrinter::PrintLn("[ReadModuleInfoSection]: Section header was null.");
        return nullptr;
    }

    if(!stringSection)
    {
        ConPrinter::PrintLn("[ReadModuleInfoSection]: String section was null.");
        return nullptr;
    }

    const i64 moduleInfoSectionPointer = GetAbsoluteSectionOffset(fileHeader, sectionHeader, stringSection, ModuleInfoSectionName);

    (void) _fseeki64(file, moduleInfoSectionPointer, SEEK_SET);

    ModuleInfoSection* moduleInfoSection = new(::std::nothrow) ModuleInfoSection;

    if(!moduleInfoSection)
    {
        ConPrinter::PrintLn("[ReadModuleInfoSection]: Failed to allocate space for module info section.");
        return nullptr;
    }

    if(!ReadFileT(file, moduleInfoSection))
    {
        delete moduleInfoSection;
        ConPrinter::PrintLn("[ReadModuleInfoSection]: Failed to load module info section.");
        return nullptr;
    }

    return moduleInfoSection;
}

ImportsSection* ReadImportsSection(FILE* const file, const FileHeader* const fileHeader, const SectionHeader* const sectionHeader, const StringSection* const stringSection) noexcept
{
    if(!file)
    {
        ConPrinter::PrintLn("[ReadImportsSection]: File was null.");
        return nullptr;
    }

    if(!fileHeader)
    {
        ConPrinter::PrintLn("[ReadImportsSection]: File header was null.");
        return nullptr;
    }

    if(!sectionHeader)
    {
        ConPrinter::PrintLn("[ReadImportsSection]: Section header was null.");
        return nullptr;
    }

    if(!stringSection)
    {
        ConPrinter::PrintLn("[ReadImportsSection]: String section was null.");
        return nullptr;
    }

    const i64 importsSectionPointer = GetAbsoluteSectionOffset(fileHeader, sectionHeader, stringSection, ImportsSectionName);

    (void) _fseeki64(file, importsSectionPointer, SEEK_SET);

    u16 moduleCount;

    if(!ReadFileT(file, &moduleCount))
    {
        ConPrinter::PrintLn("[ReadImportsSection]: Failed to read the module import count.");
        return nullptr;
    }

    const uSys moduleBufferSize = sizeof(ImportsSection::Modules[0]) * moduleCount;

    void* const placement = operator new(sizeof(ImportsSection) + moduleBufferSize, ::std::nothrow);

    if(!placement)
    {
        ConPrinter::PrintLn("[ReadImportsSection]: Failed to allocate space for section header.");
        return nullptr;
    }

    ImportsSection* const importsSection = ::new(placement) ImportsSection;
    importsSection->ModuleCount = moduleCount;
    (void) ::std::memset(importsSection->Modules, 0, moduleBufferSize);

    for(u16 i = 0; i < moduleCount; ++i)
    {
        u32 functionCount;

        if(!ReadFileT(file, &functionCount))
        {
            FreeFile(importsSection);
            ConPrinter::PrintLn("[ReadImportsSection]: Failed to read the function import count for module #{}.", i);
            return nullptr;
        }

        const uSys functionBufferSize = sizeof(ModuleImport::Functions[0]) * functionCount;

        void* const modulePlacement = operator new(sizeof(ModuleImport) + functionBufferSize, ::std::nothrow);

        if(!modulePlacement)
        {
            ConPrinter::PrintLn("[ReadImportsSection]: Failed to allocate space for module #{} imports.", i);
            return nullptr;
        }

        ModuleImport* const moduleImport = ::new(modulePlacement) ModuleImport;
        moduleImport->ImportCount = functionCount;

        if(!ReadFileTBuf(file, moduleImport->Functions, functionCount))
        {
            FreeFile(importsSection);
            ConPrinter::PrintLn("[ReadImportsSection]: Failed to read the imports list for module #{}.", i);
            return nullptr;
        }
    }

    return importsSection;
}

ExportsSection* ReadExportsSection(FILE* const file, const FileHeader* const fileHeader, const SectionHeader* const sectionHeader, const StringSection* const stringSection) noexcept
{
    if(!file)
    {
        ConPrinter::PrintLn("[ReadExportsSection]: File was null.");
        return nullptr;
    }

    if(!fileHeader)
    {
        ConPrinter::PrintLn("[ReadExportsSection]: File header was null.");
        return nullptr;
    }

    if(!sectionHeader)
    {
        ConPrinter::PrintLn("[ReadExportsSection]: Section header was null.");
        return nullptr;
    }

    if(!stringSection)
    {
        ConPrinter::PrintLn("[ReadExportsSection]: String section was null.");
        return nullptr;
    }

    const i64 exportsSectionPointer = GetAbsoluteSectionOffset(fileHeader, sectionHeader, stringSection, ExportsSectionName);

    (void) _fseeki64(file, exportsSectionPointer, SEEK_SET);

    u32 functionCount;

    if(!ReadFileT(file, &functionCount))
    {
        ConPrinter::PrintLn("[ReadExportsSection]: Failed to read the export count.");
        return nullptr;
    }

    const uSys functionBufferSize = sizeof(ExportsSection::Functions[0]) * functionCount;

    void* const placement = operator new(sizeof(ExportsSection) + functionBufferSize, ::std::nothrow);

    if(!placement)
    {
        ConPrinter::PrintLn("[ReadExportsSection]: Failed to allocate space for exports section.");
        return nullptr;
    }

    ExportsSection* const exportsSection = ::new(placement) ExportsSection;
    exportsSection->FunctionCount = functionCount;

    if(!ReadFileTBuf(file, exportsSection->Functions, functionCount))
    {
        operator delete(exportsSection);
        ConPrinter::PrintLn("[ReadExportsSection]: Failed to read the exports list.");
        return nullptr;
    }

    return exportsSection;
}

TypesSection* ReadTypesSection(FILE* const file, const FileHeader* const fileHeader, const SectionHeader* const sectionHeader, const StringSection* const stringSection) noexcept
{
    if(!file)
    {
        ConPrinter::PrintLn("[ReadTypesSection]: File was null.");
        return nullptr;
    }

    if(!fileHeader)
    {
        ConPrinter::PrintLn("[ReadTypesSection]: File header was null.");
        return nullptr;
    }

    if(!sectionHeader)
    {
        ConPrinter::PrintLn("[ReadTypesSection]: Section header was null.");
        return nullptr;
    }

    if(!stringSection)
    {
        ConPrinter::PrintLn("[ReadTypesSection]: String section was null.");
        return nullptr;
    }

    const i64 typesSectionPointer = GetAbsoluteSectionOffset(fileHeader, sectionHeader, stringSection, TypesSectionName);

    (void) _fseeki64(file, typesSectionPointer, SEEK_SET);

    u32 typeCount;

    if(!ReadFileT(file, &typeCount))
    {
        ConPrinter::PrintLn("[ReadTypesSection]: Failed to read the type count.");
        return nullptr;
    }

    const uSys typeBufferSize = sizeof(TypesSection::Types[0]) * typeCount;

    void* const placement = operator new(sizeof(TypesSection) + typeBufferSize, ::std::nothrow);

    if(!placement)
    {
        ConPrinter::PrintLn("[ReadTypesSection]: Failed to allocate space for types section.");
        return nullptr;
    }

    TypesSection* const typesSection = ::new(placement) TypesSection;
    typesSection->TypeCount = typeCount;

    if(!ReadFileTBuf(file, typesSection->Types, typeCount))
    {
        operator delete(typesSection);
        ConPrinter::PrintLn("[ReadTypesSection]: Failed to read the types list.");
        return nullptr;
    }

    return typesSection;
}

GlobalsSection* ReadGlobalsSection(FILE* const file, const FileHeader* const fileHeader, const SectionHeader* const sectionHeader, const StringSection* const stringSection) noexcept
{
    if(!file)
    {
        ConPrinter::PrintLn("[ReadGlobalsSection]: File was null.");
        return nullptr;
    }

    if(!fileHeader)
    {
        ConPrinter::PrintLn("[ReadGlobalsSection]: File header was null.");
        return nullptr;
    }

    if(!sectionHeader)
    {
        ConPrinter::PrintLn("[ReadGlobalsSection]: Section header was null.");
        return nullptr;
    }

    if(!stringSection)
    {
        ConPrinter::PrintLn("[ReadGlobalsSection]: String section was null.");
        return nullptr;
    }

    const i64 globalsSectionPointer = GetAbsoluteSectionOffset(fileHeader, sectionHeader, stringSection, GlobalsSectionName);

    (void) _fseeki64(file, globalsSectionPointer, SEEK_SET);

    u32 globalCount;

    if(!ReadFileT(file, &globalCount))
    {
        ConPrinter::PrintLn("[ReadGlobalsSection]: Failed to read the globals count.");
        return nullptr;
    }

    const uSys typeBufferSize = sizeof(GlobalsSection::GlobalTypeIndexes[0]) * globalCount;

    void* const placement = operator new(sizeof(GlobalsSection) + typeBufferSize, ::std::nothrow);

    if(!placement)
    {
        ConPrinter::PrintLn("[ReadGlobalsSection]: Failed to allocate space for globals section.");
        return nullptr;
    }

    GlobalsSection* const globalsSection = ::new(placement) GlobalsSection;
    globalsSection->GlobalCount = globalCount;

    if(!ReadFileTBuf(file, globalsSection->GlobalTypeIndexes, globalCount))
    {
        operator delete(globalsSection);
        ConPrinter::PrintLn("[ReadGlobalsSection]: Failed to read the globals list.");
        return nullptr;
    }

    return globalsSection;
}

static FunctionLocals* ReadFunctionLocals(FILE* const file, const u32 functionIndex) noexcept
{
    if(!file)
    {
        ConPrinter::PrintLn("[ReadFunctionLocals]: File was null.");
        return nullptr;
    }
    
    u16 localCount;

    if(!ReadFileT(file, &localCount))
    {
        ConPrinter::PrintLn("[ReadFunctionLocals]: Failed to read the locals count for function #{}.", functionIndex);
        return nullptr;
    }

    const uSys typeBufferSize = sizeof(FunctionLocals::LocalTypeIndexes[0]) * localCount;

    void* const placement = operator new(sizeof(FunctionLocals) + typeBufferSize, ::std::nothrow);

    if(!placement)
    {
        ConPrinter::PrintLn("[ReadFunctionLocals]: Failed to allocate space for locals for function #{}.", functionIndex);
        return nullptr;
    }

    FunctionLocals* const locals = ::new(placement) FunctionLocals;
    locals->LocalCount = localCount;

    if(!ReadFileTBuf(file, locals->LocalTypeIndexes, localCount))
    {
        operator delete(locals);
        ConPrinter::PrintLn("[ReadFunctionLocals]: Failed to read the locals list for function #{}.", functionIndex);
        return nullptr;
    }

    return locals;
}

static FunctionArguments* ReadFunctionArguments(FILE* const file, const u32 functionIndex) noexcept
{
    if(!file)
    {
        ConPrinter::PrintLn("[ReadFunctionArguments]: File was null.");
        return nullptr;
    }

    u16 argumentCount;

    if(!ReadFileT(file, &argumentCount))
    {
        ConPrinter::PrintLn("[ReadFunctionArguments]: Failed to read the arguments count for function #{}.", functionIndex);
        return nullptr;
    }

    const uSys typeBufferSize = sizeof(FunctionArguments::Arguments[0]) * argumentCount;

    void* const placement = operator new(sizeof(FunctionArguments) + typeBufferSize, ::std::nothrow);

    if(!placement)
    {
        ConPrinter::PrintLn("[ReadFunctionArguments]: Failed to allocate space for arguments for function #{}.", functionIndex);
        return nullptr;
    }

    FunctionArguments* const arguments = ::new(placement) FunctionArguments;
    arguments->ArgumentCount = argumentCount;

    if(!ReadFileTBuf(file, arguments->Arguments, argumentCount))
    {
        operator delete(arguments);
        ConPrinter::PrintLn("[ReadFunctionArguments]: Failed to read the arguments list for function #{}.", functionIndex);
        return nullptr;
    }

    return arguments;
}

static bool ReadFunctionMetadata(FILE* const file, const u32 functionIndex, FunctionMetadata* const placement) noexcept
{
    if(!file)
    {
        ConPrinter::PrintLn("[ReadFunctionMetadata]: File was null.");
        return false;
    }
    
    if(!ReadFileT(file, &placement->CodePointer))
    {
        ConPrinter::PrintLn("[ReadFunctionMetadata]: Failed to read the code pointer for function #{}.", functionIndex);
        return false;
    }

    if(!ReadFileT(file, &placement->NamePointer))
    {
        ConPrinter::PrintLn("[ReadFunctionMetadata]: Failed to read the name pointer for function #{}.", functionIndex);
        return false;
    }

    if(!ReadFileT(file, &placement->CodeSize))
    {
        ConPrinter::PrintLn("[ReadFunctionMetadata]: Failed to read the code size for function #{}.", functionIndex);
        return false;
    }

    if(!ReadFileT(file, &placement->Flags))
    {
        ConPrinter::PrintLn("[ReadFunctionMetadata]: Failed to read the flags for function #{}.", functionIndex);
        return false;
    }

    if(!ReadFileT(file, &placement->Reserved))
    {
        ConPrinter::PrintLn("[ReadFunctionMetadata]: Failed to read the reserved data for function #{}.", functionIndex);
        return false;
    }

    placement->Locals = ReadFunctionLocals(file, functionIndex);

    if(!placement->Locals)
    {
        ConPrinter::PrintLn("[ReadFunctionMetadata]: Failed to read the locals for function #{}.", functionIndex);
        return false;
    }

    placement->Arguments = ReadFunctionArguments(file, functionIndex);

    if(!placement->Arguments)
    {
        FreeFile(placement->Locals);
        placement->Locals = nullptr;
        ConPrinter::PrintLn("[ReadFunctionMetadata]: Failed to read the arguments for function #{}.", functionIndex);
        return false;
    }

    return  true;
}

FunctionsSection* ReadFunctionsSection(FILE* const file, const FileHeader* const fileHeader, const SectionHeader* const sectionHeader, const StringSection* const stringSection) noexcept
{
    if(!file)
    {
        ConPrinter::PrintLn("[ReadFunctionsSection]: File was null.");
        return nullptr;
    }

    if(!fileHeader)
    {
        ConPrinter::PrintLn("[ReadFunctionsSection]: File header was null.");
        return nullptr;
    }

    if(!sectionHeader)
    {
        ConPrinter::PrintLn("[ReadFunctionsSection]: Section header was null.");
        return nullptr;
    }

    if(!stringSection)
    {
        ConPrinter::PrintLn("[ReadFunctionsSection]: String section was null.");
        return nullptr;
    }

    const i64 globalsSectionPointer = GetAbsoluteSectionOffset(fileHeader, sectionHeader, stringSection, FunctionsSectionName);

    (void) _fseeki64(file, globalsSectionPointer, SEEK_SET);

    u32 functionCount;

    if(!ReadFileT(file, &functionCount))
    {
        ConPrinter::PrintLn("[ReadFunctionsSection]: Failed to read the function count.");
        return nullptr;
    }

    const uSys functionBufferSize = sizeof(FunctionsSection::Functions[0]) * functionCount;

    void* const placement = operator new(sizeof(FunctionsSection) + functionBufferSize, ::std::nothrow);

    if(!placement)
    {
        ConPrinter::PrintLn("[ReadFunctionsSection]: Failed to allocate space for functions section.");
        return nullptr;
    }

    FunctionsSection* const functionsSection = ::new(placement) FunctionsSection;
    functionsSection->FunctionCount = functionCount;

    (void) ::std::memset(&functionsSection->Functions, 0, functionBufferSize);

    for(u32 i = 0; i < functionCount; ++i)
    {
        if(!ReadFunctionMetadata(file, i, &functionsSection->Functions[i]))
        {
            FreeFile(functionsSection);
            ConPrinter::PrintLn("[ReadFunctionsSection]: Failed to read function metadata.");
            return nullptr;
        }
    }

    return functionsSection;
}

CodeSection* ReadCodeSection(FILE* const file, const FileHeader* const fileHeader, const SectionHeader* const sectionHeader, const StringSection* const stringSection) noexcept
{
    if(!file)
    {
        ConPrinter::PrintLn("[ReadCodeSection]: File was null.");
        return nullptr;
    }

    if(!fileHeader)
    {
        ConPrinter::PrintLn("[ReadCodeSection]: File header was null.");
        return nullptr;
    }

    if(!sectionHeader)
    {
        ConPrinter::PrintLn("[ReadCodeSection]: Section header was null.");
        return nullptr;
    }

    if(!stringSection)
    {
        ConPrinter::PrintLn("[ReadCodeSection]: String section was null.");
        return nullptr;
    }

    const i64 codeSectionPointer = GetAbsoluteSectionOffset(fileHeader, sectionHeader, stringSection, CodeSectionName);

    (void) _fseeki64(file, codeSectionPointer, SEEK_SET);

    u64 codeLength;

    if(!ReadFileT(file, &codeLength))
    {
        ConPrinter::PrintLn("[ReadCodeSection]: Failed to read the export count.");
        return nullptr;
    }

    CodeSection* const codeSection = new(::std::nothrow) CodeSection;

    if(!codeSection)
    {
        ConPrinter::PrintLn("[ReadCodeSection]: Failed to allocate space for code section.");
        return nullptr;
    }

    codeSection->Length = codeLength;
    codeSection->Code = reinterpret_cast<u8*>(PageAllocator::Alloc(AlignTo(codeLength, PageAllocator::PageSize()) / PageAllocator::PageSize()));
    
    if(!codeSection->Code)
    {
        delete codeSection;
        ConPrinter::PrintLn("[ReadCodeSection]: Failed to allocate space for code data.");
        return nullptr;
    }
    
    if(!ReadFileTBuf(file, &codeSection->Code, codeLength))
    {
        FreeFile(codeSection);
        ConPrinter::PrintLn("[ReadCodeSection]: Failed to read the code data.");
        return nullptr;
    }

    return codeSection;
}

i64 GetAbsoluteSectionOffset(const FileHeader* const fileHeader, const SectionHeader* const sectionHeader, const StringSection* const stringSection, const C8StringBase& targetSection) noexcept
{
    for(uSys i = 0; i < sectionHeader->SectionCount; ++i)
    {
        const auto stringIter = stringSection->Strings.find(sectionHeader->Mappings[i].NamePointer);
        if(stringIter != stringSection->Strings.end())
        {
            if(stringIter->second.Equals(targetSection))
            {
                return static_cast<i64>(fileHeader->ZeroPointer + sectionHeader->Mappings[i].SectionPointer);
            }
        }
    }

    return -1;
}

i64 WriteFileHeader(FILE* const file) noexcept
{
    if(!file)
    {
        ConPrinter::PrintLn("[WriteFileHeader]: File was null.");
        return -1;
    }

    const i64 zeroPointer = _ftelli64(file);

    FileHeader header;
    (void) ::std::memcpy(header.Magic, FileMagic, sizeof(FileMagic));
    header.Version = FileVersion0;
    header.FileSize = 0;
    header.Checksum = 0;
    header.ZeroPointer = static_cast<u64>(zeroPointer);
    header.SectionHeaderPointer = 0;
    (void) ::std::memset(header.Reserved, 0, sizeof(header.Reserved));

    if(!WriteFileT(file, &header))
    {
        ConPrinter::PrintLn("[WriteFileHeader]: Failed to write the file header.");
        return -1;
    }

    return zeroPointer;
}

i64 WriteStringSection(FILE* const file, const i64 zeroPointer, const C8DynString* const strings, const u32 count, u64* const pointers) noexcept
{
    if(!file)
    {
        ConPrinter::PrintLn("[WriteStringSection]: File was null.");
        return -1;
    }

    const i64 stringSectionPointer = _ftelli64(file) - zeroPointer;
    
    WriteFileT(file, &count);

    u8 reservedBuf[sizeof(StringSection::Reserved)];
    (void) ::std::memset(reservedBuf, 0, sizeof(reservedBuf));
    WriteFileTArr(file, reservedBuf);
    
    for(u32 i = 0; i < count; ++i)
    {
        pointers[i] = static_cast<u64>(_ftelli64(file) - zeroPointer);
    
        const u32 stringLength = static_cast<u32>(strings[i].Length());
        WriteFileT(file, &stringLength);
    
        WriteFileTBuf(file, strings[i].String(), strings[i].Length() + 1);
    }

    return stringSectionPointer;
}

i64 WriteSectionHeader(FILE* const file, const i64 zeroPointer, const u64 stringSectionPointer, const u64 stringSectionNamePointer, const u64* const namePointers, const u16 count, u64* const pointers) noexcept
{
    if(!file)
    {
        ConPrinter::PrintLn("[WriteSectionHeader]: File was null.");
        return -1;
    }

    i64 sectionHeaderPointer = _ftelli64(file);

    const i64 remainingBytes = 8 - (sectionHeaderPointer % 8);

    if(remainingBytes != 2)
    {
        constexpr u8 alignmentBuffer[8] = {};

        if(remainingBytes < 2)
        {
            WriteFileTBuf(file, alignmentBuffer, remainingBytes + 6);
        }
        else
        {
            WriteFileTBuf(file, alignmentBuffer, remainingBytes - 2);
        }

        sectionHeaderPointer = _ftelli64(file);
    }

    sectionHeaderPointer -= zeroPointer;

    (void) _fseeki64(file, zeroPointer + static_cast<i64>(offsetof(FileHeader, SectionHeaderPointer)), SEEK_SET);
    WriteFileT(file, &sectionHeaderPointer);
    (void) _fseeki64(file, sectionHeaderPointer + zeroPointer, SEEK_SET);

    WriteFileT(file, &count);
    WriteFileT(file, &stringSectionNamePointer);
    WriteFileT(file, &stringSectionPointer);

    constexpr u64 defaultSectionPointer = 0;

    for(u32 i = 0; i < count; ++i)
    {
        pointers[i] = static_cast<u64>(_ftelli64(file) - zeroPointer) + sizeof(u64);

        WriteFileT(file, &namePointers[i]);
        WriteFileT(file, &defaultSectionPointer);
    }

    return sectionHeaderPointer;
}

i64 WriteModuleInfoSection(FILE* const file, const i64 zeroPointer, const u64 moduleInfoSectionPointer, const ModuleInfoSection& moduleInfo) noexcept
{
    if(!file)
    {
        ConPrinter::PrintLn("[WriteModuleInfoSection]: File was null.");
        return -1;
    }

    i64 moduleSectionPointer = _ftelli64(file);

    const i64 remainingBytes = 8 - (moduleSectionPointer % 8);

    if(remainingBytes != 4)
    {
        constexpr u8 alignmentBuffer[8] = {};

        if(remainingBytes < 4)
        {
            WriteFileTBuf(file, alignmentBuffer, remainingBytes + 4);
        }
        else
        {
            WriteFileTBuf(file, alignmentBuffer, remainingBytes - 4);
        }
        
        moduleSectionPointer = _ftelli64(file);
    }

    moduleSectionPointer -= zeroPointer;

    (void) _fseeki64(file, zeroPointer + static_cast<i64>(moduleInfoSectionPointer), SEEK_SET);
    WriteFileT(file, &moduleSectionPointer);
    (void) _fseeki64(file, moduleSectionPointer + zeroPointer, SEEK_SET);

    WriteFileT(file, &moduleInfo);
    
    return moduleSectionPointer;
}

void WriteFunctionSection(FILE* file, i64 zeroPointer, u64 functionSectionPointer, const Module* module, const u64* const namePointers, u64* const codePointers) noexcept
{
    i64 codeSectionPointer = _ftelli64(file);

    const i64 remainingBytes = 4096 - (codeSectionPointer % 4096);

    if(remainingBytes != 4)
    {
        static constexpr u8 alignmentBuffer[4096] = {};

        if(remainingBytes < 4)
        {
            WriteFileTBuf(file, alignmentBuffer, remainingBytes + 4092);
        }
        else
        {
            WriteFileTBuf(file, alignmentBuffer, remainingBytes - 4);
        }
        
        codeSectionPointer = _ftelli64(file);
    }

    codeSectionPointer -= zeroPointer;

    (void) _fseeki64(file, zeroPointer + static_cast<i64>(functionSectionPointer), SEEK_SET);
    WriteFileT(file, &codeSectionPointer);
    (void) _fseeki64(file, codeSectionPointer + zeroPointer, SEEK_SET);

    const u32 functionCount = static_cast<u32>(module->Functions().Count());

    WriteFileT(file, &functionCount);

    for(uSys i = 0; i < module->Functions().Count(); ++i)
    {
        const Function* const function = module->Functions()[i];

        constexpr u64 codePointer = 0;

        codePointers[i] = static_cast<u64>(_ftelli64(file) - zeroPointer);

        WriteFileT(file, &codePointer);
        WriteFileT(file, &namePointers[i]);

        const u64 codeSize = function->CodeSize();

        WriteFileT(file, &codeSize);

        const u32 flags = function->Flags().Packed;

        WriteFileT(file, &flags);

        u8 reservedBuf[sizeof(FunctionMetadata::Reserved)] = {};
        WriteFileTArr(file, reservedBuf);

        const u16 localCount = static_cast<u16>(function->LocalTypes().Count());

        WriteFileT(file, &localCount);
    }
}

void WriteFinal(FILE* file, const i64 zeroPointer) noexcept
{
    static u32 CrcTable[256];
    GenerateCRCTable(CrcTable);

    (void) fflush(file);

    const i64 fileSize = _ftelli64(file) - zeroPointer - static_cast<i64>(sizeof(FileHeader));
    
    (void) _fseeki64(file, zeroPointer + static_cast<i64>(offsetof(FileHeader, FileSize)), SEEK_SET);
    WriteFileT(file, &fileSize);

    const u32 crc32 = CRCFile(file, zeroPointer, fileSize);

    (void) _fseeki64(file, zeroPointer + static_cast<i64>(offsetof(FileHeader, Checksum)), SEEK_SET);
    WriteFileT(file, &crc32);

    (void) _fseeki64(file, zeroPointer + fileSize + static_cast<i64>(sizeof(FileHeader)), SEEK_SET);
}

}

}
