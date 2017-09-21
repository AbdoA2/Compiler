int a = 10;
function main(int argc, int argv[]) : int {
	int x, y;

	x = 12;
	
	switch(x) {
		case 12: {
			a = 9 * a;
		}
		case 17: {
			a = a / 4;
		}
		default: {
			a = 200;
		}
	}
}

