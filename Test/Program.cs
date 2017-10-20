using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

class Program {
	const UInt64 KiB = 0x400;
	const UInt64 MiB = 0x100000;
	const UInt64 GiB = 0x40000000;

	const Int32 Shift = 12;
	const UInt64 SizeMask = 512 - 1;

	static UInt64 CreateAddr(UInt64 Gb, UInt64 Mb, UInt64 Kb, UInt64 B) {
		return (Gb * GiB) + (Mb * MiB) + (Kb * KiB) + B;
	}

	public static void Main(string[] Args) {

		//UInt64 Addr = CreateAddr(0, 0, 8, 0x123);
		UInt64 Addr = 0x201000;
		//P(Addr);

		UInt64 Offset = Addr & (4 * KiB - 1);

		Addr = Addr >> 12;
		UInt64 PT1 = Addr & SizeMask;

		Addr = Addr >> 9;
		UInt64 PT2 = Addr & SizeMask;

		Addr = Addr >> 9;
		UInt64 PT3 = Addr & SizeMask;

		Addr = Addr >> 9;
		UInt64 PT4 = Addr & SizeMask;


		Console.WriteLine(CFMT(0), Addr);

		Console.WriteLine("Offset = {0}", Offset);
		Console.WriteLine("PT4[{0}]->PT3[{1}]->PT2[{2}]->PT1[{3}]", PT4, PT3, PT2, PT1);

		Console.ReadLine();
	}

	static string CFMT(int I) {
		return "0x{" + I + ":X}";
	}
	
	/*static void P(UInt64 I) {
		Console.Write("0x{0:X16}", I);
	}*/
}