#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {

    const uint16_t BMP_FORMAT = 0x4D42;

    PACKED_STRUCT_BEGIN BitmapFileHeader{
        // поля заголовка Bitmap File Header
        // 1 Подпись — 2 байта.Символы BM.
        uint16_t type = 0;
        // 2 Суммарный размер заголовка и данных — 4 байта, беззнаковое целое.
        // Размер данных определяется как отступ, умноженный на высоту изображения.
        uint32_t size_data = 0;
        // 3 Зарезервированное пространство — 4 байта, заполненные нулями.
        uint32_t reserved = 0;
        // 4 Отступ данных от начала файла — 4 байта, беззнаковое целое.Он равен размеру двух частей заголовка.
        uint32_t indentation_data = 54;
    }
        PACKED_STRUCT_END

    PACKED_STRUCT_BEGIN BitmapInfoHeader{
        // поля заголовка Bitmap Info Header
        // 1 Размер заголовка — 4 байта, беззнаковое целое.Учитывается только размер второй части заголовка.
        uint32_t size_info = 40;
        // 2 Ширина изображения в пикселях — 4 байта, знаковое целое.
        int32_t width = 0;
        // 3 Высота изображения в пикселях — 4 байта, знаковое целое.
        int32_t height = 0;
        // 4 Количество плоскостей — 2 байта, беззнаковое целое.В нашем случае всегда 1 — одна RGB плоскость.
        uint16_t planes = 1;
        // 5 Количество бит на пиксель — 2 байта, беззнаковое целое.В нашем случае всегда 24.
        uint16_t  bit_pixel = 24;
        // 6 Тип сжатия — 4 байта, беззнаковое целое.В нашем случае всегда 0 — отсутствие сжатия.
        uint32_t compression_type = 0;
        // 7 Количество байт в данных — 4 байта, беззнаковое целое.Произведение отступа на высоту.
        uint32_t bytes = 0;
        // 8 Горизонтальное разрешение, пикселей на метр — 4 байта, знаковое целое.Нужно записать 11811, что примерно соответствует 300 DPI.
        int32_t pixel_width = 11811;
        // 9 Вертикальное разрешение, пикселей на метр — 4 байта, знаковое целое.Нужно записать 11811, что примерно соответствует 300 DPI.
        int32_t pixel_height = 11811;
        // 10 Количество использованных цветов — 4 байта, знаковое целое.Нужно записать 0 — значение не определено.
        int32_t used_colors = 0;
        // 11 Количество значимых цветов — 4 байта, знаковое целое.Нужно записать 0x1000000.
        int32_t significant_colors = 0x1000000;
    }
PACKED_STRUCT_END

// функция вычисления отступа по ширине
static int GetBMPStride(int w) {
    return 4 * ((w * 3 + 3) / 4);
}

// напишите эту функцию
bool SaveBMP(const Path& file, const Image& image) {
    const int32_t width = image.GetWidth();
    const int32_t height = image.GetHeight();
    const uint32_t step = GetBMPStride(width);
    const uint32_t stride = step * height;

    BitmapFileHeader file_header;
    file_header.type = BMP_FORMAT;
    file_header.size_data = file_header.indentation_data + stride;

    BitmapInfoHeader info_header;
    info_header.width = width;
    info_header.height = height;
    info_header.bytes = stride;

    ofstream out(file, ios::binary);
    out.write(reinterpret_cast<const char*>(&file_header), sizeof(BitmapFileHeader));
    out.write(reinterpret_cast<const char*>(&info_header), info_header.size_info);

    vector<char> buff(step);

    for (int y = height - 1; y >= 0; --y) {
        const Color* line = image.GetLine(y);
        for (int x = 0; x < width; ++x) {
            buff[x * 3 + 0] = static_cast<char>(line[x].b);
            buff[x * 3 + 1] = static_cast<char>(line[x].g);
            buff[x * 3 + 2] = static_cast<char>(line[x].r);
        }
        out.write(buff.data(), step);
    }
    return out.good();
}

// напишите эту функцию
Image LoadBMP(const Path& file) {

    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;

    ifstream ifs(file, ios::binary);
    if (!ifs) {return {};}

    ifs.read(reinterpret_cast<char*>(&file_header), sizeof(BitmapFileHeader));
    if (file_header.type != BMP_FORMAT) {return {};}

    ifs.read(reinterpret_cast<char*>(&info_header), info_header.size_info);

    const int32_t width = info_header.width;
    const int32_t height = info_header.height;
    const uint32_t step = GetBMPStride(width);

    Image result(width, height, Color::Black());
    vector<char> buff(step);

    for (int y = height - 1; y >= 0; --y) {
        Color* line = result.GetLine(y);
        ifs.read(buff.data(), step);
        for (int x = 0; x < width; ++x) {
            line[x].b = static_cast<byte>(buff[x * 3 + 0]);
            line[x].g = static_cast<byte>(buff[x * 3 + 1]);
            line[x].r = static_cast<byte>(buff[x * 3 + 2]);
        }
    }
    return result;
}

}  // namespace img_lib