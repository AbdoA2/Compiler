function main(int argc, int argv[]) : int {
	int x;
	{
		int x = 10;
		int z = 20;
	}
	x = argv[0];
	switch(x) {
		case 1:
			x = 90;
		case 4:
			x = 900;
	}
}