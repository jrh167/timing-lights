
#ifdef ARDUINO

#include "Arduino.h"

#endif

#include "ByteBuf.h"

ByteBuf::ByteBuf(size_t buffer_size) {
    buffer = (byte *) malloc(sizeof(byte) * buffer_size);
    capacity = buffer_size;
    size = 0;
    readerIndex = 0;
    writerIndex = 0;
}

ByteBuf::~ByteBuf() {
    free(buffer);
    capacity = -1;
}

int ByteBuf::resize(size_t buf_size) {
    //Attempt to realloc the buffer
    byte *newBuf = (byte *) realloc(buffer, sizeof(byte) * buf_size);
    if (!newBuf) return 1; //Fail
    //Set new buffer and capacity.
    free(buffer);
    buffer = newBuf;
    capacity = buf_size;

    //Clamp size, reader index and writer index.
    if (buf_size < size) size = buf_size;
    if (readerIndex < buf_size) readerIndex = buf_size;
    if (writerIndex < buf_size) writerIndex = buf_size;
    return 0;
}

void ByteBuf::clear() {
    size = 0;
    readerIndex = 0;
    writerIndex = 0;
}

size_t ByteBuf::getMaxCapacity() const {
    return capacity;
}

size_t ByteBuf::getSize() const {
    return size;
}

size_t ByteBuf::getReadableBytes() const {
    return getSize() - getReaderIndex();
}

size_t ByteBuf::getWriteableBytes() const {
    return getMaxCapacity() - getWriterIndex();
}

size_t ByteBuf::getReaderIndex() const {
    return readerIndex;
}

void ByteBuf::setReaderIndex(size_t index) {
    readerIndex = index;
    if (readerIndex > size) {
        readerIndex = size;
    }
}

size_t ByteBuf::getWriterIndex() const {
    return writerIndex;
}

void ByteBuf::setWriterIndex(size_t index) {
    writerIndex = index;
    if (writerIndex > capacity) {
        writerIndex = capacity;
    }
}

void ByteBuf::skip(int num) {
    setReaderIndex(getReaderIndex() + num);
}

ByteBuf ByteBuf::slice(bool trim) {
    ByteBuf slice(trim ? getSize() : getMaxCapacity());
    slice.size = getReadableBytes();
    memcpy(slice.buffer, buffer, slice.size);
    return slice;
}

void ByteBuf::take() {
    //TODO, dont use slice here, we can shuffle the data without an intermediate buffer.
    ByteBuf sl = slice(false);
    free(buffer);
    buffer = sl.buffer;
    size = sl.size;
    readerIndex = 0;
    writerIndex = size;
}

uint8_t ByteBuf::peekByte(size_t index) {
    if (readerIndex + index >= size) return 0;
    return buffer[readerIndex + index];
}

uint8_t ByteBuf::readByte() {
    if (readerIndex >= size) return 0;
    return buffer[readerIndex++];
}

void ByteBuf::writeByte(uint8_t b) {
    if (writerIndex >= capacity) return;
    buffer[writerIndex++] = b;
    size++;
}