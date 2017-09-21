int x = 10.67, n = 30, k=90;
const int e = 200;
function main(int x, float z, int u) : int {
	int i = 12.5;
	x =  9*19 + 2*90.5;
	n = 10;
	if (x > 100)
		u = u * 10;
	else
		x = x / 20;

	for (i = 0; i < 100; i = i + 1)
		x = x *89;

	z = z * 5.5;

	while (n > 10) {
		u = u * 12;
		x = n * k;
		n = n - 1;
	}

	i = do_job(x, u);
}

function do_job(int m, int n) : int {
	int j;
	j = m * n + x;
	return j;
}


