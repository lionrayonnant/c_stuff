

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

int main() {

    char buf[16];
    int tub1, tub2;
    int nb1, nb2, res;

    mkfifo("tube1", 0666);
    mkfifo("tube2", 0666);

    tub1 = open("tube1", O_RDONLY);
    tub2 = open("tube2", O_WRONLY);

    read(tub1, buf, sizeof(buf));
    nb1 = atoi(buf);
    printf("Nb1 : %d\n", nb1);

    read(tub1, buf, sizeof(buf));
    nb2 = atoi(buf);
    printf("Nb2 : %d\n", nb2);

    res = nb1 + nb2;
    snprintf(buf, sizeof(buf), "%d", res);

    write(tub2, buf, sizeof(buf));

    close(tub1);
    close(tub2);

    unlink("tub1");
    unlink("tub2");

}
