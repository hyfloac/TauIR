#include "TauIR/file/BinaryObject.hpp"
#include <ConPrinter.hpp>

namespace tau::ir::file {

void FreeFile(void* pointer) noexcept
{
    operator delete(pointer);
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
        const uSys readCount = fread_s(magicBuffer, sizeof(magicBuffer), 1, sizeof(magicBuffer), file);

        _fseeki64(file, zeroPointer, SEEK_SET);

        if(readCount != sizeof(magicBuffer))
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
    
    FileHeader* const header = new FileHeader;

    if(fread_s(header, sizeof(*header), 1, sizeof(*header), file) != sizeof(*header))
    {
        ConPrinter::PrintLn("[ReadFileHeader]: File was not long enough for header.");
        (void) _fseeki64(file, zeroPointer, SEEK_SET);
        delete header;
        return nullptr;
    }

    if(zeroPointer > 0 && static_cast<u64>(zeroPointer) != header->ZeroPointer)
    {
        ConPrinter::PrintLn("[ReadFileHeader]: The zero pointer {} in the header did not match the zero pointer of the file {}.", header->ZeroPointer, zeroPointer);
        (void) _fseeki64(file, zeroPointer, SEEK_SET);
        delete header;
        return nullptr;
    }

    if((totalFileSize - zeroPointer) < header->FileSize)
    {
        ConPrinter::PrintLn("[ReadFileHeader]: The encompassing file size {} is less than the purported file size {}.", totalFileSize, header->FileSize);
        (void) _fseeki64(file, zeroPointer, SEEK_SET);
        delete header;
        return nullptr;
    }

    if(header->FileSize <= header->SectionHeaderPointer)
    {
        ConPrinter::PrintLn("[ReadFileHeader]: The file size {} is less than the pointer to the section table {}.", header->FileSize, header->SectionHeaderPointer);
        (void) _fseeki64(file, zeroPointer, SEEK_SET);
        delete header;
        return nullptr;
    }

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

    (void) _fseeki64(file, static_cast<i64>(fileHeader->SectionHeaderPointer), SEEK_SET);

    u16 sectionCount;

    if(fread_s(&sectionCount, sizeof(sectionCount), 1, sizeof(sectionCount), file) != sizeof(sectionCount))
    {
        ConPrinter::PrintLn("[ReadSectionHeader]: Failed to read the section count.");
        return nullptr;
    }

    const uSys mappingsBufferSize = sizeof(SectionHeader::Mappings[0]) * sectionCount;

    void* const placement = operator new(sizeof(SectionHeader) + mappingsBufferSize, ::std::nothrow);
    SectionHeader* const sectionHeader = ::new(placement) SectionHeader;

    sectionHeader->SectionCount = sectionCount;

    if(fread_s(&sectionHeader->Mappings, mappingsBufferSize, 1, mappingsBufferSize, file) != mappingsBufferSize)
    {
        ConPrinter::PrintLn("[ReadSectionHeader]: Failed to read the sections map.");
        delete sectionHeader;
        return nullptr;
    }

    return sectionHeader;
}

}
