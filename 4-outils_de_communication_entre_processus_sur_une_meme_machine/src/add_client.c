#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

int main() {

    char buf[16];
    int tub1, tub2;
    int nb1, nb2;

    tub1 = open("tube1", O_WRONLY);
    tub2 = open("tube2", O_RDONLY);

    printf("A : "); scanf("%d",&nb1);
    printf("B : "); scanf("%d",&nb2);

    snprintf(buf, sizeof(buf), "%d", nb1);
    write(tub1, buf, sizeof(buf));

    snprintf(buf, sizeof(buf), "%d", nb2);
    write(tub1, buf, sizeof(buf));

    read(tub2, buf, sizeof(buf));

    printf("Résultat : %s\n", buf);

    close(tub1);
    close(tub2);
}
