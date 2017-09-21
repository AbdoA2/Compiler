function main(int argc, int argv[]) : int {
	int x, y;

	int i = 0;
	
	while (i < 100) {
		x = 15;
		i = i + 1;
	}


	for (i = 0; i < 100; i = i + 1) {
		x = 15;
	}

	i = 0;

	repeat {
		x = 15;
		i = i + 1;
	} until (x == 100);
}

