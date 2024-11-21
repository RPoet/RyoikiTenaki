#include <vector>
#include <fstream>

#include "TextureBuilder.h"
#include "../../Engine.h"

IMPLEMENT_MODULE(MTextureBuilder)


typedef union PixelInfo
{
    std::uint32_t Colour;
    struct
    {
        uint8 R, G, B, A;
    };
} *PPixelInfo;


class Tga
{
private:
    std::vector<uint8> Pixels;
    bool ImageCompressed;
    std::uint32_t width, height, size, BitsPerPixel;

public:
    Tga(const char* FilePath);
    std::vector<uint8> GetPixels() { return this->Pixels; }
    std::uint32_t GetWidth() const { return this->width; }
    std::uint32_t GetHeight() const { return this->height; }
    bool HasAlphaChannel() { return BitsPerPixel == 32; }
};

Tga::Tga(const char* FilePath)
{
    std::fstream hFile(FilePath, std::ios::in | std::ios::binary);
    if (!hFile.is_open())
    {
        assert(false && " File not exists ");
    }

    std::uint8_t Header[18] = { 0 };
    std::vector<std::uint8_t> ImageData;
    static std::uint8_t DeCompressed[12] = { 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
    static std::uint8_t IsCompressed[12] = { 0x0, 0x0, 0xA, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

    hFile.read(reinterpret_cast<char*>(&Header), sizeof(Header));

    if (!std::memcmp(DeCompressed, &Header, sizeof(DeCompressed)))
    {
        BitsPerPixel = Header[16];
        width = Header[13] * 256 + Header[12];
        height = Header[15] * 256 + Header[14];

        size = ((width * BitsPerPixel + 31) / 32) * 4 * height;

        if ((BitsPerPixel != 24) && (BitsPerPixel != 32))
        {
            hFile.close();
            throw std::invalid_argument("Invalid File Format. Required: 24 or 32 Bit Image.");
        }

        ImageCompressed = false;

        if (BitsPerPixel == 24)
        {
            std::vector<std::uint8_t> ImageWOAlpha;
            ImageWOAlpha.resize(size);
            hFile.read(reinterpret_cast<char*>(ImageWOAlpha.data()), size);

            ImageData.resize(width * height * 4);
            for (int32 i = 0; i < ImageWOAlpha.size() / 3; i++)
            {
                int32 BaseIndexWOAlpha = i * 3;
                int32 BaseIndex = i * 4;
                ImageData[BaseIndex + 0] = ImageWOAlpha[BaseIndexWOAlpha + 0];
                ImageData[BaseIndex + 1] = ImageWOAlpha[BaseIndexWOAlpha + 1];
                ImageData[BaseIndex + 2] = ImageWOAlpha[BaseIndexWOAlpha + 2];
                ImageData[BaseIndex + 3] = 1;
            }
        }
        else
        {
            ImageData.resize(size);
            hFile.read(reinterpret_cast<char*>(ImageData.data()), size);
        }

    }
    else if (!std::memcmp(IsCompressed, &Header, sizeof(IsCompressed)))
    {
        BitsPerPixel = Header[16];
        width = Header[13] * 256 + Header[12];
        height = Header[15] * 256 + Header[14];
        size = ((width * BitsPerPixel + 31) / 32) * 4 * height;

        if ((BitsPerPixel != 24) && (BitsPerPixel != 32))
        {
            hFile.close();
            throw std::invalid_argument("Invalid File Format. Required: 24 or 32 Bit Image.");
        }

        PixelInfo Pixel = { 0 };
        int CurrentByte = 0;
        std::size_t CurrentPixel = 0;
        ImageCompressed = true;
        std::uint8_t ChunkHeader = { 0 };
        int BytesPerPixel = (BitsPerPixel / 8);
        ImageData.resize(width * height * sizeof(PixelInfo));

        do
        {
            hFile.read(reinterpret_cast<char*>(&ChunkHeader), sizeof(ChunkHeader));

            if (ChunkHeader < 128)
            {
                ++ChunkHeader;
                for (int I = 0; I < ChunkHeader; ++I, ++CurrentPixel)
                {
                    hFile.read(reinterpret_cast<char*>(&Pixel), BytesPerPixel);

                    ImageData[CurrentByte++] = Pixel.B;
                    ImageData[CurrentByte++] = Pixel.G;
                    ImageData[CurrentByte++] = Pixel.R;
                    if (BitsPerPixel > 24)
                    {
                        ImageData[CurrentByte++] = Pixel.A;
                    }
                    else
                    {
                        ImageData[CurrentByte++] = 1;
                    }
                }
            }
            else
            {
                ChunkHeader -= 127;
                hFile.read(reinterpret_cast<char*>(&Pixel), BytesPerPixel);

                for (int I = 0; I < ChunkHeader; ++I, ++CurrentPixel)
                {
                    ImageData[CurrentByte++] = Pixel.B;
                    ImageData[CurrentByte++] = Pixel.G;
                    ImageData[CurrentByte++] = Pixel.R;
                    if (BitsPerPixel > 24)
                    {
                        ImageData[CurrentByte++] = Pixel.A;
                    }
                    else
                    {
                        ImageData[CurrentByte++] = 1;
                    }
                }
            }
        } while (CurrentPixel < (width * height));
    }
    else
    {
        hFile.close();
        throw std::invalid_argument("Invalid File Format. Required: 24 or 32 Bit TGA File.");
    }

    hFile.close();
    this->Pixels = ImageData;
}

vector<uint8> MTextureBuilder::GenerateDefaultTexture(const uint32 Width, const uint32 Height, const uint32 PixelSizeInBytes)
{
    const uint32 rowPitch = Width * PixelSizeInBytes;
    const uint32 cellPitch = rowPitch >> 3;        // The width of a cell in the checkboard texture.
    const uint32 cellHeight = Width >> 3;    // The height of a cell in the checkerboard texture.
    const uint32 textureSize = rowPitch * Height;

    vector<uint8> data(textureSize);
    uint8* pData = &data[0];

    for (uint32 n = 0; n < textureSize; n += PixelSizeInBytes)
    {
        uint32 x = n % rowPitch;
        uint32 y = n / rowPitch;
        uint32 i = x / cellPitch;
        uint32 j = y / cellHeight;

        if (i % 2 == j % 2)
        {
            pData[n] = 0x00;        // R
            pData[n + 1] = 0x00;    // G
            pData[n + 2] = 0x00;    // B
            pData[n + 3] = 0xff;    // A
        }
        else
        {
            pData[n] = 0xff;        // R
            pData[n + 1] = 0xff;    // G
            pData[n + 2] = 0xff;    // B
            pData[n + 3] = 0xff;    // A
        }
    }

    return data;
}

vector<uint8> MTextureBuilder::GenerateDefaultColoredTexture(const uint32 Width, const uint32 Height, const uint32 PixelSizeInBytes, uint32 Color)
{
    const uint32 rowPitch = Width * PixelSizeInBytes;
    const uint32 cellPitch = rowPitch >> 3;        // The width of a cell in the checkboard texture.
    const uint32 cellHeight = Width >> 3;    // The height of a cell in the checkerboard texture.
    const uint32 textureSize = rowPitch * Height;

    vector<uint8> data(textureSize);
    uint8* pData = &data[0];

    for (uint32 n = 0; n < textureSize; n += PixelSizeInBytes)
    {
        uint32 x = n % rowPitch;
        uint32 y = n / rowPitch;
        uint32 i = x / cellPitch;
        uint32 j = y / cellHeight;

        pData[n] = Color & 0xFF;        // R
        pData[n + 1] = Color & 0xFF00;    // G
        pData[n + 2] = Color & 0xFF0000;    // B
        pData[n + 3] = Color & 0xFF000000;    // A
    }

    return data;
}

void MTextureBuilder::Init()
{
	Super::Init();
	
	cout << "Texture Builder Init" << endl;

}


void MTextureBuilder::Teardown()
{
	Super::Teardown();
}

MTexture MTextureBuilder::LoadTexture(const String& Path, const String& TextureName)
{
    // add assert to check file extension. this is for tga only.

    auto CombinedPath = Path + TextureName;
    Tga TGATexture(std::string(CombinedPath.begin(), CombinedPath.end()).c_str());

	MTexture Texture;
    Texture.Name = TextureName;
    Texture.Pixels = TGATexture.GetPixels();
    Texture.Width = TGATexture.GetWidth();
    Texture.Height = TGATexture.GetHeight();
    Texture.Size = 0;
    Texture.BitsPerPixel = 32;

	return Texture;
}
