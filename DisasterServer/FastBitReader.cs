namespace DisasterServer;

public class FastBitReader
{
	public int Position { get; set; }

	public byte ReadByte(ref byte[] data)
	{
		if (Position >= data.Length)
		{
			throw new ArgumentOutOfRangeException("data");
		}
		return data[Position++];
	}

	public bool ReadBoolean(ref byte[] data)
	{
		if (Position >= data.Length)
		{
			throw new ArgumentOutOfRangeException("data");
		}
		return Convert.ToBoolean(data[Position++]);
	}

	public char ReadChar(ref byte[] data)
	{
		if (Position >= data.Length)
		{
			throw new ArgumentOutOfRangeException("data");
		}
		return (char)data[Position++];
	}

	public short ReadShort(ref byte[] data)
	{
		if (Position >= data.Length)
		{
			throw new ArgumentOutOfRangeException("data");
		}
		short result = (short)(data[Position] | (data[Position + 1] << 8));
		Position += 2;
		return result;
	}

	public ushort ReadUShort(ref byte[] data)
	{
		if (Position >= data.Length)
		{
			throw new ArgumentOutOfRangeException("data");
		}
		ushort result = (ushort)(data[Position] | (data[Position + 1] << 8));
		Position += 2;
		return result;
	}

	public int ReadInt(ref byte[] data)
	{
		if (Position >= data.Length)
		{
			throw new ArgumentOutOfRangeException("data");
		}
		int result = data[Position] | (data[Position + 1] << 8) | (data[Position + 2] << 16) | (data[Position + 3] << 24);
		Position += 4;
		return result;
	}

	public uint ReadUInt(ref byte[] data)
	{
		if (Position >= data.Length)
		{
			throw new ArgumentOutOfRangeException("data");
		}
		int result = data[Position] | (data[Position + 1] << 8) | (data[Position + 2] << 16) | (data[Position + 3] << 24);
		Position += 4;
		return (uint)result;
	}

	public unsafe float ReadFloat(ref byte[] data)
	{
		if (Position >= data.Length)
		{
			throw new ArgumentOutOfRangeException("data");
		}
		float result;
		fixed (byte* ptr = &data[Position])
		{
			result = *(float*)ptr;
		}
		Position += 4;
		return result;
	}

	public long ReadLong(ref byte[] data)
	{
		if (Position >= data.Length)
		{
			throw new ArgumentOutOfRangeException("data");
		}
		ulong result = data[Position] | ((ulong)data[Position + 1] << 8) | ((ulong)data[Position + 2] << 16) | ((ulong)data[Position + 3] << 24) | ((ulong)data[Position + 4] << 32) | (((ulong)data[Position + 5] << 40) | (((ulong)data[Position + 6] << 48) | ((ulong)data[Position + 7] << 56)));
		Position += 8;
		return (long)result;
	}

	public ulong ReadULong(ref byte[] data)
	{
		if (Position >= data.Length)
		{
			throw new ArgumentOutOfRangeException("data");
		}
		ulong result = data[Position] | ((ulong)data[Position + 1] << 8) | ((ulong)data[Position + 2] << 16) | ((ulong)data[Position + 3] << 24) | ((ulong)data[Position + 4] << 32) | (((ulong)data[Position + 5] << 40) | (((ulong)data[Position + 6] << 48) | ((ulong)data[Position + 7] << 56)));
		Position += 8;
		return result;
	}
}
