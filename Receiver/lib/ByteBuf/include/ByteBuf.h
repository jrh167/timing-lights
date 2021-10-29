#pragma once

#ifndef DISABLE_BYTEBUF_DEFAULTS
#define BYTEBUF_SLICING       //Enables slicing the ByteBuf
#define BYTEBUF_EXTENDED_PEEK //Enables extended peeking methods.
#endif

#ifdef BYTEBUF_WIDE
#define BYTEBUF_32INT
#define BYTEBUF_64LONG
#define BYTEBUF_DOUBLE
#define BYTEBUF_64DOUBLE
#endif

#ifdef ARDUINO

#include "Arduino.h"

#endif

class ByteBuf {
private:
    byte *buffer;

    size_t capacity;
    size_t size;
    size_t readerIndex;
    size_t writerIndex;
public:
    static const size_t DEFAULT_SIZE = 64;

    explicit ByteBuf(size_t buffer_size = ByteBuf::DEFAULT_SIZE);

    ~ByteBuf();

    //region sizing
    /**
     * Resize this ByteBuf.
     *
     * If the size provided is smaller than the buffer capacity,
     * the buffer is truncated.
     * If the size provided is larger than the buffer capacity,
     * the buffer is padded with zeros.
     *
     * @param buf_size The new size in bytes.
     * @return Returns 0 on success, 1 on a resize failure.
     */
    int resize(size_t buf_size);

    /**
     * Clear the buffer.
     *
     * Sets reader index, writer index and size to zero.
     *
     * This does not zero-out the entire buffer.
     */
    void clear();
    //endregion

    //region properties
    /**
     * The maximum capacity this buffer can hold.
     *
     * @return The max capacity.
     */
    size_t getMaxCapacity() const;

    /**
     * The total number of written bytes to this buffer.
     *
     * @return The size.
     */
    size_t getSize() const;


    /**
     * The number of bytes able to be read from the buffer
     * based off the current reader index.
     *
     * Effectively `getSize() - getReaderIndex();`
     *
     * @return The readable bytes.
     */
    size_t getReadableBytes() const;

    /**
     * The number of bytes able to be written to the buffer
     * based off the current writer index.
     *
     * Effectively `getMaxCapacity() - getWriterIndex();`
     *
     * @return The writeable bytes.
     */
    size_t getWriteableBytes() const;

    /**
     * The current reader index.
     *
     * @return The index.
     */
    size_t getReaderIndex() const;

    /**
     * Sets the current reader index.
     *
     * This will be clamped between zero and `getSize()`.
     *
     * @param index The index.
     */
    void setReaderIndex(size_t index);

    /**
     * The current writer index.
     *
     * @return The index.
     */
    size_t getWriterIndex() const;

    /**
     * Sets the current writer index.
     *
     * This will be clamped between zero and `getMaxCapacity()`.
     *
     * @param index The index.
     */
    void setWriterIndex(size_t index);
    //endregion

    //region slicing/skipping
    /**
     * Skips the given number of bytes.
     *
     * @param num The number of bytes to skip.
     */
    void skip(int num);

#ifdef BYTEBUF_SLICING

    /**
     * Slices the ByteBuf.
     *
     * Creates a complete clone of this ByteBuf from
     * `getReaderIndex()` to `getSize()`.
     *
     * @param trim If the new buffer should be trimmed to the minimum size possible.
     * @return The new buffer.
     */
    ByteBuf slice(bool trim = false);

    /**
     * Internally slices this ByteBuf at the reader index.
     *
     * The readerIndex will be reset to zero.
     * The writerIndex will be reset to the size.
     */
    void take();

#endif
    //endregion

    //region peeking
    /**
     * Peeks ahead into the buffer.
     *
     * Reading past the end of the buffer will return nothing but zeros.
     *
     * Bytes are the only methods with inverted S/U naming.
     *
     * @param index The offset to peek in bytes.
     * @return The peeked unsigned byte.
     */
    uint8_t peekByte(size_t index);

    /**
     * Peeks ahead into the buffer.
     *
     * Bytes are the only methods with inverted S/U naming.
     *
     * @param index The offset to peek in bytes.
     * @return The peeked signed byte.
     */
    inline int8_t peekSByte(size_t index) {
        return (int8_t) peekByte(index);
    }

#ifdef BYTEBUF_EXTENDED_PEEK

    /**
     * Peeks ahead into the buffer.
     *
     * Reading past the end of the buffer will return nothing but zeros.
     *
     * @param index The offset to peek in bytes.
     * @return The peeked short.
     */
    inline short peekShort(size_t index) {
        return (short) peekUShort(index);
    }

    /**
     * Peeks ahead into the buffer.
     *
     * Reading past the end of the buffer will return nothing but zeros.
     *
     * @param index The offset to peek in bytes.
     * @return The peeked short.
     */
    unsigned short peekUShort(size_t index) {
        unsigned short ret;
        byte *pointer = (byte *) &ret;
        pointer[0] = peekByte(index + 1);
        pointer[1] = peekByte(index);
        return ret;
    }

    /**
     * Peeks ahead into the buffer.
     *
     * Reading past the end of the buffer will return nothing but zeros.
     *
     * @param index The offset to peek in bytes.
     * @return The peeked int.
     */
    inline int peekInt(size_t index) {
        return (int) peekUInt(index);
    }

    /**
     * Peeks ahead into the buffer.
     *
     * Reading past the end of the buffer will return nothing but zeros.
     *
     * @param index The offset to peek in bytes.
     * @return The peeked unsigned int.
     */
    unsigned int peekUInt(size_t index) {
        unsigned int ret;
#ifndef BYTEBUF_32INT
        byte *pointer = (byte *) &ret;
        pointer[1] = peekByte(index);
        pointer[0] = peekByte(index + 1);
#else
        pointer[3] = peekUByte(index + 0);
        pointer[2] = peekUByte(index + 1);
        pointer[1] = peekUByte(index + 2);
        pointer[0] = peekUByte(index + 3);
#endif
        return ret;
    }

    /**
     * Peeks ahead into the buffer.
     *
     * Reading past the end of the buffer will return nothing but zeros.
     *
     * @param index The offset to peek in bytes.
     * @return The peeked long.
     */
    inline long peekLong(size_t index) {
        return (long) peekULong(index);
    }

    /**
     * Peeks ahead into the buffer.
     *
     * Reading past the end of the buffer will return nothing but zeros.
     *
     * @param index The offset to peek in bytes.
     * @return The peeked unsigned long.
     */
    unsigned long peekULong(size_t index) {
        unsigned long ret;
        byte *pointer = (byte *) &ret;
#ifndef BYTEBUF_64LONG
        pointer[3] = peekByte(index + 0);
        pointer[2] = peekByte(index + 1);
        pointer[1] = peekByte(index + 2);
        pointer[0] = peekByte(index + 3);
#else
        pointer[7] = peekUByte(index + 0);
        pointer[6] = peekUByte(index + 1);
        pointer[5] = peekUByte(index + 2);
        pointer[4] = peekUByte(index + 3);
        pointer[3] = peekUByte(index + 4);
        pointer[2] = peekUByte(index + 5);
        pointer[1] = peekUByte(index + 6);
        pointer[0] = peekUByte(index + 7);
#endif
        return ret;
    }

    /**
     * Peeks ahead into the buffer.
     *
     * Reading past the end of the buffer will return nothing but zeros.
     *
     * @param index The offset to peek in bytes.
     * @return The peeked float.
     */
    float peekFloat(size_t index) {
        float ret;
        byte *pointer = (byte *) &ret;
        pointer[0] = peekByte(index + 3);
        pointer[1] = peekByte(index + 2);
        pointer[2] = peekByte(index + 1);
        pointer[3] = peekByte(index + 0);
        return ret;
    }

#ifdef BYTEBUF_DOUBLE
    /**
     * Peeks ahead into the buffer.
     *
     * Reading past the end of the buffer will return nothing but zeros.
     *
     * @param index The offset to peek in bytes.
     * @return The peeked double.
     */
    double peekDouble(size_t index) {
        double ret;
        byte *pointer = (byte *) &ret;
#ifndef BYTEBUF_64DOUBLE
        pointer[3] = peekByte(index + 0);
        pointer[2] = peekByte(index + 1);
        pointer[1] = peekByte(index + 2);
        pointer[0] = peekByte(index + 3);
#else
        pointer[7] = peekUByte(index + 0);
        pointer[6] = peekUByte(index + 1);
        pointer[5] = peekUByte(index + 2);
        pointer[4] = peekUByte(index + 3);
        pointer[3] = peekUByte(index + 4);
        pointer[2] = peekUByte(index + 5);
        pointer[1] = peekUByte(index + 6);
        pointer[0] = peekUByte(index + 7);
#endif
        return ret;
    }
#endif
#endif
    //endregion

    //region reading
    /**
     * Gets the next unsigned byte in the buffer.
     *
     * Reading past the end of the buffer will return nothing but zeros.
     *
     * Bytes are the only methods with inverted S/U naming.
     *
     * @return The unsigned byte.
     */
    uint8_t readByte();

    /**
     * Gets the next signed byte in the buffer.
     *
     * Reading past the end of the buffer will return nothing but zeros.
     *
     * Bytes are the only methods with inverted S/U naming.
     *
     * @return The signed byte.
     */
    inline int8_t readSByte() {
        return (int8_t) readByte();
    }

    /**
     * Gets the next signed short in the buffer.
     *
     * Reading past the end of the buffer will return nothing but zeros.
     *
     * @return The signed short.
     */
    inline short readShort() {
        return (short) readUShort();
    }

    /**
     * Gets the next unsigned short in the buffer.
     *
     * Reading past the end of the buffer will return nothing but zeros.
     *
     * @return The unsigned short.
     */
    unsigned short readUShort() {
        unsigned short ret;
        byte *pointer = (byte *) &ret;
        pointer[1] = readByte();
        pointer[0] = readByte();
        return ret;
    }

    /**
     * Gets the next signed int in the buffer.
     *
     * Reading past the end of the buffer will return nothing but zeros.
     *
     * @return The signed int.
     */
    inline int readInt() {
        return (int) readUInt();
    }

    /**
     * Gets the next unsigned int in the buffer.
     *
     * Reading past the end of the buffer will return nothing but zeros.
     *
     * @return The unsigned int.
     */
    unsigned int readUInt() {
        unsigned int ret;
        byte *pointer = (byte *) &ret;
#ifndef BYTEBUF_32INT
        pointer[1] = readByte();
        pointer[0] = readByte();
#else
        pointer[3] = readUByte();
        pointer[2] = readUByte();
        pointer[1] = readUByte();
        pointer[0] = readUByte();
#endif
        return ret;
    }

    /**
     * Gets the next signed long in the buffer.
     *
     * Reading past the end of the buffer will return nothing but zeros.
     *
     * @return The signed long.
     */
    inline long readLong() {
        return (long) readULong();
    }

    /**
     * Gets the next unsigned long in the buffer.
     *
     * Reading past the end of the buffer will return nothing but zeros.
     *
     * @return The unsigned long.
     */
    unsigned long readULong() {
        unsigned long ret;
        byte *pointer = (byte *) &ret;
#ifndef BYTEBUF_64LONG
        pointer[3] = readByte();
        pointer[2] = readByte();
        pointer[1] = readByte();
        pointer[0] = readByte();
#else
        pointer[7] = readUByte();
        pointer[6] = readUByte();
        pointer[5] = readUByte();
        pointer[4] = readUByte();
        pointer[3] = readUByte();
        pointer[2] = readUByte();
        pointer[1] = readUByte();
        pointer[0] = readUByte();
#endif
        return ret;
    }

    /**
     * Gets the next float in the buffer.
     *
     * Reading past the end of the buffer will return nothing but zeros.
     *
     * @return The float.
     */
    float readFloat() {
        float ret;
        byte *pointer = (byte *) &ret;
        pointer[3] = readByte();
        pointer[2] = readByte();
        pointer[1] = readByte();
        pointer[0] = readByte();
        return ret;
    }

#ifdef BYTEBUF_DOUBLE
    /**
     * Gets the next double in the buffer.
     *
     * Reading past the end of the buffer will return nothing but zeros.
     *
     * @return The double.
     */
    double readDouble() {
        double ret;
        byte *pointer = (byte *) &ret;
#ifndef BYTEBUF_64DOUBLE
        pointer[3] = readByte();
        pointer[2] = readByte();
        pointer[1] = readByte();
        pointer[0] = readByte();
#else
        pointer[7] = readUByte();
        pointer[6] = readUByte();
        pointer[5] = readUByte();
        pointer[4] = readUByte();
        pointer[3] = readUByte();
        pointer[2] = readUByte();
        pointer[1] = readUByte();
        pointer[0] = readUByte();
#endif
        return ret;
    }
#endif
    //endregion

    //region writing
    /**
     * Writes an unsigned byte to the buffer
     *
     * Writing past the end of the buffer results in no operations being performed.
     *
     * Bytes are the only methods with inverted S/U naming.
     *
     * @param b The unsigned byte.
     */
    void writeByte(uint8_t b);

    /**
     * Writes a signed byte to the buffer
     *
     * Writing past the end of the buffer results in no operations being performed.
     *
     * Bytes are the only methods with inverted S/U naming.
     *
     * @param b The signed byte.
     */
    inline void writeSByte(int8_t b) {
        writeByte(b);
    }

    /**
     * Writes a signed short to the buffer
     *
     * Writing past the end of the buffer results in no operations being performed.
     *
     * @param s The signed short.
     */
    inline void writeShort(short s) {
        writeUShort(s);
    }

    /**
     * Writes an unsigned short to the buffer
     *
     * Writing past the end of the buffer results in no operations being performed.
     *
     * @param s The unsigned short.
     */
    void writeUShort(unsigned short s) {
        byte *pointer = (byte *) &s;
        writeByte(pointer[1]);
        writeByte(pointer[0]);
    }

    /**
     * Writes a signed int to the buffer
     *
     * Writing past the end of the buffer results in no operations being performed.
     *
     * @param i The signed int.
     */
    inline void writeInt(int i) {
        writeUInt(i);
    }

    /**
     * Writes an unsigned int to the buffer
     *
     * Writing past the end of the buffer results in no operations being performed.
     *
     * @param i The unsigned int.
     */
    void writeUInt(unsigned int i) {
        byte *pointer = (byte *) &i;
#ifndef BYTEBUF_32INT
        writeByte(pointer[1]);
        writeByte(pointer[0]);
#else
        writeUByte(pointer[3]);
        writeUByte(pointer[2]);
        writeUByte(pointer[1]);
        writeUByte(pointer[0]);
#endif
    }

    /**
     * Writes a signed long to the buffer
     *
     * Writing past the end of the buffer results in no operations being performed.
     *
     * @param l The signed long.
     */
    inline void writeLong(long l) {
        writeULong(l);
    }

    /**
     * Writes an unsigned long to the buffer
     *
     * Writing past the end of the buffer results in no operations being performed.
     *
     * @param l The unsigned long.
     */
    void writeULong(unsigned long l) {
        byte *pointer = (byte *) &l;
#ifndef BYTEBUF_64LONG
        writeByte(pointer[3]);
        writeByte(pointer[2]);
        writeByte(pointer[1]);
        writeByte(pointer[0]);
#else
        writeUByte(pointer[7]);
        writeUByte(pointer[6]);
        writeUByte(pointer[5]);
        writeUByte(pointer[4]);
        writeUByte(pointer[3]);
        writeUByte(pointer[2]);
        writeUByte(pointer[1]);
        writeUByte(pointer[0]);
#endif
    }

    /**
     * Writes an float to the buffer
     *
     * Writing past the end of the buffer results in no operations being performed.
     *
     * @param f The float.
     */
    void writeFloat(float f) {
        byte *pointer = (byte *) &f;
        writeByte(pointer[3]);
        writeByte(pointer[2]);
        writeByte(pointer[1]);
        writeByte(pointer[0]);
    }

#ifdef BYTEBUF_DOUBLE
    /**
     * Writes a double to the buffer
     *
     * Writing past the end of the buffer results in no operations being performed.
     *
     * @param d The double.
     */
    void writeDouble(double d) {
        byte *pointer = (byte *) &d;
#ifndef BYTEBUF_64DOUBLE
        writeByte(pointer[3]);
        writeByte(pointer[2]);
        writeByte(pointer[1]);
        writeByte(pointer[0]);
#else
        writeUByte(pointer[7]);
        writeUByte(pointer[6]);
        writeUByte(pointer[5]);
        writeUByte(pointer[4]);
        writeUByte(pointer[3]);
        writeUByte(pointer[2]);
        writeUByte(pointer[1]);
        writeUByte(pointer[0]);
#endif
    }
#endif
    //endregion
};
