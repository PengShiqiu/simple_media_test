#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory.h>
#include <vector>
#include <stdint.h>
#include <memory>
#include <cmath>

int mytest_yuv420_split(const std::string &path, int w, int h)
{
    int yuv_size = w * h * 3 / 2;

    int fd = open(path.c_str(), O_RDONLY);
    int yfd = open("output/output_256x256_y.y", O_WRONLY | O_CREAT);
    int ufd = open("output/output_128x128_u.u", O_WRONLY | O_CREAT);
    int vfd = open("output/output_128x128_v.v", O_WRONLY | O_CREAT);

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

int mytest_yuv444_split(const std::string &path, int w, int h)
{
    int yuv_size = w * h * 3;

    int fd = open(path.c_str(), O_RDONLY);
    int yfd = open("output/output_256x256_y.y", O_WRONLY | O_CREAT);
    int ufd = open("output/output_256x256_u.u", O_WRONLY | O_CREAT);
    int vfd = open("output/output_256x256_v.v", O_WRONLY | O_CREAT);

    void *buff = malloc(yuv_size);
    ssize_t size = read(fd, buff, yuv_size);

    if (size < 0)
    {
        std::cout << "read err" << std::endl;
        free(buff);
        return -1;
    }

    // Y、U、V分量为w * h个字节
    write(yfd, buff, w * h);
    write(ufd, (void *)(buff + w * h), w * h);
    write(vfd, (void *)(buff + w * h * 2), w * h);

    free(buff);
    close(fd);
    close(yfd);
    close(ufd);
    close(vfd);

    return 0;
}

int mytest_yuv420_gray(const std::string &path, int w, int h)
{
    int yuv_size = w * h * 3 / 2;

    int fd = open(path.c_str(), O_RDONLY);
    int gray_fd = open("output/output_256x256_gray.yuv", O_WRONLY | O_CREAT);

    void *buff = malloc(yuv_size);
    ssize_t size = read(fd, buff, yuv_size);

    if (size < 0)
    {
        std::cout << "read err" << std::endl;
        free(buff);
        return -1;
    }

    memset(buff + w * h, 128, w * h / 2);
    write(gray_fd, buff, w * h * 3 / 2);

    free(buff);
    close(fd);
    close(gray_fd);

    return 0;
}

int mytest_rgb24_split(const std::string &path, int w, int h)
{
    int fd = open(path.c_str(), O_RDONLY);
    int rfd = open("output/output_500x500_r.y", O_WRONLY | O_CREAT);
    int gfd = open("output/output_500x500_g.y", O_WRONLY | O_CREAT);
    int bfd = open("output/output_500x500_b.y", O_WRONLY | O_CREAT);

    auto buff = std::make_shared<std::vector<uint8_t>>(w * h * 3);
    auto r_buff = std::make_shared<std::vector<uint8_t>>(w * h);
    auto g_buff = std::make_shared<std::vector<uint8_t>>(w * h);
    auto b_buff = std::make_shared<std::vector<uint8_t>>(w * h);

    ssize_t size = read(fd, buff->data(), w * h * 3);
    if (size < 0)
    {
        std::cout << "read err" << std::endl;
        return -1;
    }
    for (int i = 0, j = 0; i < w * h * 3; i += 3, j++)
    {
        (*r_buff)[j] = (*buff)[i];
        (*g_buff)[j] = (*buff)[i + 1];
        (*b_buff)[j] = (*buff)[i + 2];
    }
    write(rfd, (void *)r_buff->data(), w * h);
    write(gfd, (void *)g_buff->data(), w * h);
    write(bfd, (void *)b_buff->data(), w * h);

    close(fd);
    close(rfd);
    close(gfd);
    close(bfd);

    return 0;
}

int mytest_rgb24_psnr(const std::string &path1, const std::string &path2, int w, int h)

{
    int fd1 = open(path1.c_str(), O_RDONLY);
    int fd2 = open(path2.c_str(), O_RDONLY);

    auto buff1 = std::make_shared<std::vector<uint8_t>>(w * h);
    auto buff2 = std::make_shared<std::vector<uint8_t>>(w * h);

    auto size1 = read(fd1, buff1->data(), w * h);
    auto size2 = read(fd2, buff2->data(), w * h);

    if (size1 <= 0 || size2 <= 0)
    {
        std::cout << "read err" << std::endl;
        return -1;
    }

    double mse_sum = 0.f;
    double mes = 0.f;
    double psnr = 0.f;

    for (int i = 0; i < w * h; i++)
    {
        mse_sum += std::pow((double)((*buff1)[i] - (*buff2)[i]), 2.0);
    }
    mes = mse_sum / (w * h);
    psnr = 10 * log10(255.0 * 255.0 / mes);
    std::cout << "psnr:" << psnr << std::endl;

    close(fd1);
    close(fd2);
    return 0;
}

int mytest_rgb24_to_bmp(const std::string &rgb24_path, int w, int h)
{
    /*bmp file header*/
    typedef struct
    {
        unsigned int bfSize;        /* Size of file */
        unsigned short bfReserved1; /* Reserved */
        unsigned short bfReserved2; /* ... */
        unsigned int bfOffBits;     /* Offset to bitmap data */
    } BitmapFileHeader;

    /*bmp info header*/
    typedef struct
    {
        unsigned int biSize;         /* Size of info header */
        int biWidth;                 /* Width of image */
        int biHeight;                /* Height of image */
        unsigned short biPlanes;     /* Number of color planes */
        unsigned short biBitCount;   /* Number of bits per pixel */
        unsigned int biCompression;  /* Type of compression to use */
        unsigned int biSizeImage;    /* Size of image data */
        int biXPelsPerMeter;         /* X pixels per meter */
        int biYPelsPerMeter;         /* Y pixels per meter */
        unsigned int biClrUsed;      /* Number of colors used */
        unsigned int biClrImportant; /* Number of important colors */
    } BitmapInfoHeader;

    BitmapFileHeader bmp_head{0};
    BitmapInfoHeader bmp_info_head{0};

    const char bf_type[] = {'B', 'M'};
    int header_size = sizeof(bf_type) + sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader);

    std::cout << "header_size:" << header_size << std::endl;

    int rgb24_fd = open(rgb24_path.c_str(), O_RDONLY);
    int bmp_fd = open("output/output_rgb24_to_bmp.bmp", O_WRONLY | O_CREAT);

    if (rgb24_fd < 0 || bmp_fd < 0)
    {
        std::cout << "open error" << std::endl;
        return -1;
    }

    auto rgb_buff = std::make_shared<std::vector<uint8_t>>(w * h * 3);
    auto len = read(rgb24_fd, rgb_buff->data(), w * h * 3);
    if (len < 0)
    {
        std::cout << "read error" << std::endl;
        return -2;
    }

    bmp_head.bfReserved1 = 0;
    bmp_head.bfReserved2 = 0;
    bmp_head.bfSize = 2 + sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + w * h * 3;
    bmp_head.bfOffBits = 0x36;

    bmp_info_head.biSize = sizeof(BitmapInfoHeader);
    bmp_info_head.biWidth = w;
    bmp_info_head.biHeight = -h;
    bmp_info_head.biPlanes = 1;
    bmp_info_head.biBitCount = 24;
    bmp_info_head.biSizeImage = 0;
    bmp_info_head.biCompression = 0;
    bmp_info_head.biXPelsPerMeter = 5000;
    bmp_info_head.biYPelsPerMeter = 5000;
    bmp_info_head.biClrUsed = 0;
    bmp_info_head.biClrImportant = 0;

    write(bmp_fd, bf_type, sizeof(bf_type));
    write(bmp_fd, &bmp_head, sizeof(bmp_head));
    write(bmp_fd, &bmp_info_head, sizeof(bmp_info_head));

    // BMP save R1|G1|B1,R2|G2|B2 as B1|G1|R1,B2|G2|R2
    // It saves pixel data in Little Endian
    // So we change 'R' and 'B'
    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            std::swap((*rgb_buff)[(j * w + i) * 3 + 2], (*rgb_buff)[(j * w + i) * 3 + 0]);
        }
    }
    write(bmp_fd, rgb_buff->data(), 3 * w * h);

    close(bmp_fd);
    close(rgb24_fd);

    return 0;
}

int main(void)
{
    // 拆分yuv420图像
    mytest_yuv420_split("../media/lena_256x256_yuv420p.yuv", 256, 256);
    // 拆分yuv444图像
    mytest_yuv444_split("../media/lena_256x256_yuv444p.yuv", 256, 256);
    // yuv420转成灰度图
    mytest_yuv420_gray("../media/lena_256x256_yuv420p.yuv", 256, 256);
    // 拆分rgb图
    mytest_rgb24_split("../media/cie1931_500x500.rgb", 500, 500);
    // 计算图像Y分量的PSNR 图像质量
    mytest_rgb24_psnr("../media/lena_256x256_yuv420p.yuv", "../media/lena_distort_256x256_yuv420p.yuv", 256, 256);
    // rgb24转bmp
    mytest_rgb24_to_bmp("../media/lena_256x256_rgb24.rgb", 256, 256);
    return 0;
}