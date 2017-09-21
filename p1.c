const int a = 100;
function do_output(int x) : void {
    x = x + 5;
    switch(x) {
        case 1:
            {int y;
            y = 10; }
        case 3: {
            int z;
            z = 12; }
        default:
            x = 20;
    }
    repeat {
        x = do_input(34);
        do_input(90);
    } until (x == 10);
}

function do_input(int x) : int {
    for (i = 0; i < 10; i=i+1)
        return x*10;
}

int p = 90, c, d, f = 100, n;
int ar[100];
function main(int argc, int argv[]) : int
{
    int x;
    int y;
    x = x + y;
    {
        x[9] = 190;
        x = 90;
        y = 80;
    }
    return x + y;
}

