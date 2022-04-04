#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int mytest_yuv420_split(const std::string &path, int w, int h)
{
    int yuv_size = w * h * 3 / 2;

    int fd = open(path.c_str(), O_RDONLY);
    int yfd = open("output_256x256_y.y", O_WRONLY | O_CREAT);
    int ufd = open("output_128x128_u.u", O_WRONLY | O_CREAT);
    int vfd = open("output_128x128_v.v", O_WRONLY | O_CREAT);

    void *buff = malloc(yuv_size);
    ssize_t size = read(fd, buff, yuv_size);

    if (size < 0)
    {
        std::cout << "read err" << std::endl;
        free(buff);
        return -1;
    }

    // Y分量为w * h个字节
    // U、V分量为w * h / 4个字节
    write(yfd, buff, w * h);
    write(ufd, (void *)(buff + w * h), w * h / 4);
    write(vfd, (void *)(buff + w * h * 5 / 4), w * h / 4);

    free(buff);
    close(fd);
    close(yfd);
    close(ufd);
    close(vfd);

    return 0;
}

int main(void)
{
    mytest_yuv420_split("../media/lena_256x256_yuv420p.yuv", 256, 256);
    return 0;
}