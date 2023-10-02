using System.Text;

namespace DisasterServer;

public class TcpPacket
{
	private byte[] _buffer = new byte[256];

	private int _position;

	public int Length { get; private set; }

	public TcpPacket(PacketType type, params dynamic[] args)
	{
		Write((byte)104);
		Write((byte)80);
		Write((byte)75);
		Write((byte)84);
		Write((byte)0);
		Write((byte)0);
		Write((byte)0);
		Write((byte)type);
		foreach (dynamic dyn in args)
		{
			Write(dyn);
		}
	}

	public TcpPacket(PacketType type)
	{
		Write((byte)104);
		Write((byte)80);
		Write((byte)75);
		Write((byte)84);
		Write((byte)0);
		Write((byte)0);
		Write((byte)0);
		Write((byte)type);
	}

	public void Write(byte value)
	{
		lock (_buffer)
		{
			_buffer[_position++] = value;
			Length = _position;
		}
	}

	public void Write(char value)
	{
		Write((byte)value);
	}

	public void Write(sbyte value)
	{
		Write((byte)value);
	}

	public void Write(bool value)
	{
		Write((byte)(value ? 1u : 0u));
	}

	public unsafe void Write(ushort value)
	{
		byte* ptr = (byte*)(&value);
		if (BitConverter.IsLittleEndian)
		{
			Write(*ptr);
			Write(ptr[1]);
		}
		else
		{
			Write(ptr[1]);
			Write(*ptr);
		}
	}

	public unsafe void Write(short value)
	{
		byte* ptr = (byte*)(&value);
		if (BitConverter.IsLittleEndian)
		{
			Write(*ptr);
			Write(ptr[1]);
		}
		else
		{
			Write(ptr[1]);
			Write(*ptr);
		}
	}

	public unsafe void Write(float value)
	{
		byte* ptr = (byte*)(&value);
		if (BitConverter.IsLittleEndian)
		{
			Write(*ptr);
			Write(ptr[1]);
			Write(ptr[2]);
			Write(ptr[3]);
		}
		else
		{
			Write(ptr[3]);
			Write(ptr[2]);
			Write(ptr[1]);
			Write(*ptr);
		}
	}

	public unsafe void Write(uint value)
	{
		byte* ptr = (byte*)(&value);
		if (BitConverter.IsLittleEndian)
		{
			Write(*ptr);
			Write(ptr[1]);
			Write(ptr[2]);
			Write(ptr[3]);
		}
		else
		{
			Write(ptr[3]);
			Write(ptr[2]);
			Write(ptr[1]);
			Write(*ptr);
		}
	}

	public unsafe void Write(int value)
	{
		byte* ptr = (byte*)(&value);
		if (BitConverter.IsLittleEndian)
		{
			Write(*ptr);
			Write(ptr[1]);
			Write(ptr[2]);
			Write(ptr[3]);
		}
		else
		{
			Write(ptr[3]);
			Write(ptr[2]);
			Write(ptr[1]);
			Write(*ptr);
		}
	}

	public unsafe void Write(ulong value)
	{
		byte* ptr = (byte*)(&value);
		if (BitConverter.IsLittleEndian)
		{
			Write(*ptr);
			Write(ptr[1]);
			Write(ptr[2]);
			Write(ptr[3]);
			Write(ptr[4]);
			Write(ptr[5]);
			Write(ptr[6]);
			Write(ptr[7]);
		}
		else
		{
			Write(ptr[7]);
			Write(ptr[6]);
			Write(ptr[5]);
			Write(ptr[4]);
			Write(ptr[3]);
			Write(ptr[2]);
			Write(ptr[1]);
			Write(*ptr);
		}
	}

	public unsafe void Write(double value)
	{
		byte* ptr = (byte*)(&value);
		if (BitConverter.IsLittleEndian)
		{
			Write(*ptr);
			Write(ptr[1]);
			Write(ptr[2]);
			Write(ptr[3]);
			Write(ptr[4]);
			Write(ptr[5]);
			Write(ptr[6]);
			Write(ptr[7]);
		}
		else
		{
			Write(ptr[7]);
			Write(ptr[6]);
			Write(ptr[5]);
			Write(ptr[4]);
			Write(ptr[3]);
			Write(ptr[2]);
			Write(ptr[1]);
			Write(*ptr);
		}
	}

	public unsafe void Write(long value)
	{
		byte* ptr = (byte*)(&value);
		if (BitConverter.IsLittleEndian)
		{
			Write(*ptr);
			Write(ptr[1]);
			Write(ptr[2]);
			Write(ptr[3]);
			Write(ptr[4]);
			Write(ptr[5]);
			Write(ptr[6]);
			Write(ptr[7]);
		}
		else
		{
			Write(ptr[7]);
			Write(ptr[6]);
			Write(ptr[5]);
			Write(ptr[4]);
			Write(ptr[3]);
			Write(ptr[2]);
			Write(ptr[1]);
			Write(*ptr);
		}
	}

	public void Write(string value)
	{
		byte[] bytes = Encoding.UTF8.GetBytes(value);
		foreach (byte c in bytes)
		{
			Write(c);
		}
		Write('\0');
	}

	public byte[] ToArray()
	{
		_buffer[5] = (byte)(Length - 6);
		return _buffer;
	}
}
