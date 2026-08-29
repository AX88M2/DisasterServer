using System;

namespace Server.Game;

public class Packet
{
	private byte[] _buffer = new byte[256];

	private int _position;

	public PacketType Type { get; set; }

	public int Length { get; private set; }

	public Packet()
	{
		Write((byte)80);
		Write((byte)111);
		Write((byte)82);
		Write((byte)110);
		Write((byte)0);
	}

	public Packet(PacketType type)
	{
		Type = type;
		Write((byte)80);
		Write((byte)111);
		Write((byte)82);
		Write((byte)110);
		Write((byte)10);
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
		foreach (char c in value)
		{
			Write(c);
		}
		Write('\0');
	}

	public byte[] ToArray()
	{
		Write((byte)75);
		Write((byte)105);
		Write((byte)68);
		int len = Length;
		_position = 4;
		Write((byte)(len - 8));
		_position = len;
		Length = len;
		return _buffer;
	}
}
