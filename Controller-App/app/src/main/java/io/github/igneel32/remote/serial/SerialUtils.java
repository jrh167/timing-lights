package io.github.igneel32.remote.serial;

import io.netty.buffer.ByteBuf;


/**
 * SerialUtils is a utility class in Java for serial communications methods
 * which I could not write in Kotlin (unexpected behaviour).
 *
 * @author Julia Harrison
 */
public class SerialUtils {

    /**
     * Basic checksum calculation via byte sum
     *
     * @param data a ByteBuf containing the data to sum
     * @return the checksum of the data
     */
    public static short checksum(ByteBuf data) {
        data.markReaderIndex();
        short c = 0;
        int len = data.readableBytes();
        for (int i = 0; i < len; i++) {
            c += (data.readByte() & 0xFF);
        }
        data.resetReaderIndex();
        return c;
    }
}
