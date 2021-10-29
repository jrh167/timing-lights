package io.github.igneel32.remote.serial

import android.hardware.usb.UsbDeviceConnection
import android.util.Log
import com.hoho.android.usbserial.driver.UsbSerialDriver
import com.hoho.android.usbserial.driver.UsbSerialPort
import com.hoho.android.usbserial.util.SerialInputOutputManager
import io.github.igneel32.remote.MainActivity
import io.netty.buffer.ByteBuf
import io.netty.buffer.Unpooled
import java.io.ByteArrayOutputStream
import java.io.IOException
import java.lang.Exception
import java.nio.ByteOrder
import java.util.*
import java.util.concurrent.CompletableFuture
import java.util.function.Consumer
import kotlin.experimental.and


/**
 * Class controlling the serial communications for the wireless controller.
 *
 * @author Julia Harrison
 */
class SerialCommunications {
    private var usbSerialPort: UsbSerialPort? = null
    val serialState : SerialState = SerialState()

    /** Uncomment to enable serial debugging messages **/
    var readThread: SerialInputOutputManager? = null

    /**
     * Init method sets the serial port's baud rate, and opens the port.
     */
    fun connect(driver: UsbSerialDriver, connection: UsbDeviceConnection) {
        usbSerialPort = driver.ports[0] // Most devices have just one port (port 0)
        usbSerialPort!!.open(connection)
        usbSerialPort!!.setParameters(BAUD_RATE, 8, UsbSerialPort.STOPBITS_1,
            UsbSerialPort.PARITY_NONE)

        /** Uncomment to enable serial debugging messages **/
        readThread = SerialInputOutputManager(usbSerialPort, object : SerialInputOutputManager.Listener {
            val buffer = StringBuffer()

            override fun onNewData(data: ByteArray?) {
                for (it in data!!) {
                    if (it.toChar() == '\n') {
                        Log.i("Serial", buffer.toString())
                        buffer.setLength(0)
                    } else {
                        buffer.append(it.toChar())
                    }
                }
            }

            override fun onRunError(e: Exception?) {

            }

        })
        readThread!!.start()
    }

    fun disconnect() {
        /** Uncomment to enable serial debugging messages **/
        readThread!!.stop()

        try {
            usbSerialPort!!.close()
        } catch (ignored : Throwable) { }
        usbSerialPort = null
    }

    fun sendState() {
        sendPacket { buf ->
            buf.writeShort(serialState.pack())
            buf.writeShort(serialState.time)
            buf.writeShort(serialState.startNumBeeps)
            buf.writeShort(serialState.endNumBeeps)
        }
    }

    /**
     * Packages the given data into packet format and sends the packet.
     * Relies on the init method being run already.
     *
     * @param func Byte buffer containing packet data
     */
    fun sendPacket(func: Consumer<ByteBuf>) {
        // Build data segment
        val dataSeg = Unpooled.buffer().order(ByteOrder.BIG_ENDIAN)
        func.accept(dataSeg) // Fill in the rest of the bytes
        // Packet must not exceed 256 bytes, including header segment
        require(dataSeg.readableBytes() + 4 + 2 + 1 <= 256) {
            "Packet too big."
        }

        // Build header segment
        val headerSeg = Unpooled.buffer().order(ByteOrder.BIG_ENDIAN)
        headerSeg.writeBytes(HEADER) // Magic header number first
        // Size of data segment + magic header + size field + checksum
        headerSeg.writeByte(dataSeg.readableBytes() + 4 + 2 + 1)
        headerSeg.writeShort(SerialUtils.checksum(dataSeg).toInt()) // Compute simple checksum of data segment
        headerSeg.writeBytes(dataSeg) // Append data segment to final packet
        val bos = ByteArrayOutputStream()
        headerSeg.readBytes(bos, headerSeg.readableBytes())
        val bytes = bos.toByteArray()
        usbSerialPort!!.write(bytes, 200)
    }

    companion object {
        private val HEADER = byteArrayOf(
            0xA4.toByte(), 0x11,
            0xE4.toByte(), 0xD8.toByte()
        )
        private const val BAUD_RATE = 9600
    }
}
