/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

int64 InputStream::getNumBytesRemaining()
{
    auto len = getTotalLength();

    if (len >= 0)
        len -= getPosition();

    return len;
}

ssize_t InputStream::read (void* destBuffer, size_t size)
{
    ssize_t totalRead = 0;

    while (size > 0)
    {
        auto numToRead = (int) std::min (size, (size_t) 0x70000000);
        auto numRead = read (juce::addBytesToPointer (destBuffer, totalRead), numToRead);
        jassert (numRead <= numToRead);

        if (numRead < 0) return (ssize_t) numRead;
        if (numRead == 0) break;

        size -= (size_t) numRead;
        totalRead += numRead;
    }

    return totalRead;
}

char InputStream::readByte()
{
    char temp = 0;
    read (&temp, 1);
    return temp;
}

bool InputStream::readBool()
{
    return readByte() != 0;
}

short InputStream::readShort()
{
    char temp[2];

    if (read (temp, 2) == 2)
        return (short) ByteOrder::littleEndianShort (temp);

    return 0;
}

short InputStream::readShortBigEndian()
{
    char temp[2];

    if (read (temp, 2) == 2)
        return (short) ByteOrder::bigEndianShort (temp);

    return 0;
}

int InputStream::readInt()
{
    char temp[4];

    if (read (temp, 4) == 4)
        return (int) ByteOrder::littleEndianInt (temp);

    return 0;
}

int InputStream::readIntBigEndian()
{
    char temp[4];

    if (read (temp, 4) == 4)
        return (int) ByteOrder::bigEndianInt (temp);

    return 0;
}

int InputStream::readCompressedInt()
{
    auto sizeByte = (uint8) readByte();

    if (sizeByte == 0)
        return 0;

    const int numBytes = (sizeByte & 0x7f);

    if (numBytes > 4)
    {
        jassertfalse;  // trying to read corrupt data - this method must only be used
                       // to read data that was written by OutputStream::writeCompressedInt()
        return 0;
    }

    char bytes[4] = {};

    if (read (bytes, numBytes) != numBytes)
        return 0;

    auto num = (int) ByteOrder::littleEndianInt (bytes);
    return (sizeByte >> 7) ? -num : num;
}

int64 InputStream::readInt64()
{
    union { uint8 asBytes[8]; uint64 asInt64; } n;

    if (read (n.asBytes, 8) == 8)
        return (int64) ByteOrder::swapIfBigEndian (n.asInt64);

    return 0;
}

int64 InputStream::readInt64BigEndian()
{
    union { uint8 asBytes[8]; uint64 asInt64; } n;

    if (read (n.asBytes, 8) == 8)
        return (int64) ByteOrder::swapIfLittleEndian (n.asInt64);

    return 0;
}

float InputStream::readFloat()
{
    static_assert (sizeof (int32) == sizeof (float), "Union assumes float has the same size as an int32");
    union { int32 asInt; float asFloat; } n;
    n.asInt = (int32) readInt();
    return n.asFloat;
}

float InputStream::readFloatBigEndian()
{
    union { int32 asInt; float asFloat; } n;
    n.asInt = (int32) readIntBigEndian();
    return n.asFloat;
}

double InputStream::readDouble()
{
    union { int64 asInt; double asDouble; } n;
    n.asInt = readInt64();
    return n.asDouble;
}

double InputStream::readDoubleBigEndian()
{
    union { int64 asInt; double asDouble; } n;
    n.asInt = readInt64BigEndian();
    return n.asDouble;
}

String InputStream::readString()
{
    MemoryOutputStream buffer;

    for (;;)
    {
        auto c = readByte();
        buffer.writeByte (c);

        if (c == 0)
            return buffer.toUTF8();
    }
}

String InputStream::readNextLine()
{
    MemoryOutputStream buffer;

    for (;;)
    {
        auto c = readByte();

        if (c == 0 || c == '\n')
            break;

        if (c == '\r')
        {
            auto lastPos = getPosition();

            if (readByte() != '\n')
                setPosition (lastPos);

            break;
        }

        buffer.writeByte (c);
    }

    return buffer.toUTF8();
}

size_t InputStream::readIntoMemoryBlock (MemoryBlock& block, ssize_t numBytes)
{
    MemoryOutputStream mo (block, true);
    return (size_t) mo.writeFromInputStream (*this, numBytes);
}

String InputStream::readEntireStreamAsString()
{
    MemoryOutputStream mo;
    mo << *this;
    return mo.toString();
}

//==============================================================================
void InputStream::skipNextBytes (int64 numBytesToSkip)
{
    if (numBytesToSkip > 0)
    {
        auto skipBufferSize = (int) jmin (numBytesToSkip, (int64) 16384);
        HeapBlock<char> temp (skipBufferSize);

        while (numBytesToSkip > 0 && ! isExhausted())
            numBytesToSkip -= read (temp, (int) jmin (numBytesToSkip, (int64) skipBufferSize));
    }
}

} // namespace juce
