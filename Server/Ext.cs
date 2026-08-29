using System.Collections.Generic;
using System.IO;
using System.Text;

namespace Server;

public static class Ext
{
	public static string ReadStringNull(this BinaryReader reader)
	{
		List<byte> bytes = new List<byte>();
		byte c;
		while (reader.BaseStream.Position < reader.BaseStream.Length && (c = reader.ReadByte()) != 0)
		{
			bytes.Add(c);
		}
		return Encoding.UTF8.GetString(bytes.ToArray());
	}
}
