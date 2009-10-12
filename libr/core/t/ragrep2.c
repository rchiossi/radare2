int show_help(int large)
{
	printf("Usage: ragrep2 [-a x86] [-L] [-xsd str] [-ft addr] [file] [...]\n");
	if (large)
	printf(
	" -a x86    select architecture for disasm search\n"
	" -L        list r_asm plugins\n"
	" -x 908e   search a hexpair string\n"
	" -s str    search a string\n"
	" -d opcode search an opcode\n"
	" -f 0x102  'from' address\n"
	" -t 0x502  'to' address\n");
}

int main(int argc, char **argv)
{
	return show_help(1);
}