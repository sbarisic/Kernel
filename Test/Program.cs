using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

class Program {
	public static void Main(string[] Args) {

		UInt64 L = (512 * 1024 * 1024) / (0x400 * 4);
		L /= 8; // Number of bytes in the bitmap
		
		Console.WriteLine(L);


		Console.ReadLine();

	}
}