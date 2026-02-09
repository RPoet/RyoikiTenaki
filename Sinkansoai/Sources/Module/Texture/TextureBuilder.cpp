#include <vector>
#include <fstream>
#include <cwctype>
#include <cstring>
#include <filesystem>

#include "TextureBuilder.h"
#include "../../Engine.h"
#include "../Mesh/stb_image.h"

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

struct DDS_PIXELFORMAT
{
    std::uint32_t size;
    std::uint32_t flags;
    std::uint32_t fourCC;
    std::uint32_t rgbBitCount;
    std::uint32_t rMask;
    std::uint32_t gMask;
    std::uint32_t bMask;
    std::uint32_t aMask;
};

struct DDS_HEADER
{
    std::uint32_t size;
    std::uint32_t flags;
    std::uint32_t height;
    std::uint32_t width;
    std::uint32_t pitchOrLinearSize;
    std::uint32_t depth;
    std::uint32_t mipMapCount;
    std::uint32_t reserved1[11];
    DDS_PIXELFORMAT ddspf;
    std::uint32_t caps;
    std::uint32_t caps2;
    std::uint32_t caps3;
    std::uint32_t caps4;
    std::uint32_t reserved2;
};

static void DecodeColor565(std::uint16_t c, uint8& outR, uint8& outG, uint8& outB)
{
    outR = static_cast<uint8>(((c >> 11) & 0x1F) * 255 / 31);
    outG = static_cast<uint8>(((c >> 5) & 0x3F) * 255 / 63);
    outB = static_cast<uint8>((c & 0x1F) * 255 / 31);
}

static void DecodeBC1Block(const uint8* block, uint8* dest, int32 destStride, int32 width, int32 height, int32 x, int32 y)
{
    const std::uint16_t c0 = static_cast<std::uint16_t>(block[0] | (block[1] << 8));
    const std::uint16_t c1 = static_cast<std::uint16_t>(block[2] | (block[3] << 8));

    uint8 r[4], g[4], b[4], a[4];
    DecodeColor565(c0, r[0], g[0], b[0]);
    DecodeColor565(c1, r[1], g[1], b[1]);

    if (c0 > c1)
    {
        r[2] = static_cast<uint8>((2 * r[0] + r[1]) / 3);
        g[2] = static_cast<uint8>((2 * g[0] + g[1]) / 3);
        b[2] = static_cast<uint8>((2 * b[0] + b[1]) / 3);
        r[3] = static_cast<uint8>((r[0] + 2 * r[1]) / 3);
        g[3] = static_cast<uint8>((g[0] + 2 * g[1]) / 3);
        b[3] = static_cast<uint8>((b[0] + 2 * b[1]) / 3);
        a[0] = a[1] = a[2] = a[3] = 255;
    }
    else
    {
        r[2] = static_cast<uint8>((r[0] + r[1]) / 2);
        g[2] = static_cast<uint8>((g[0] + g[1]) / 2);
        b[2] = static_cast<uint8>((b[0] + b[1]) / 2);
        r[3] = g[3] = b[3] = 0;
        a[0] = a[1] = a[2] = 255;
        a[3] = 0;
    }

    std::uint32_t indices = block[4] | (block[5] << 8) | (block[6] << 16) | (block[7] << 24);
    for (int32 row = 0; row < 4; ++row)
    {
        for (int32 col = 0; col < 4; ++col)
        {
            const int32 dstX = x + col;
            const int32 dstY = y + row;
            if (dstX >= width || dstY >= height)
            {
                indices >>= 2;
                continue;
            }

            const std::uint32_t index = indices & 0x3;
            indices >>= 2;

            uint8* pixel = dest + dstY * destStride + dstX * 4;
            pixel[0] = b[index];
            pixel[1] = g[index];
            pixel[2] = r[index];
            pixel[3] = a[index];
        }
    }
}

static void DecodeBC1ColorBlock(const uint8* block, uint8* dest, int32 destStride, int32 width, int32 height, int32 x, int32 y)
{
    const std::uint16_t c0 = static_cast<std::uint16_t>(block[0] | (block[1] << 8));
    const std::uint16_t c1 = static_cast<std::uint16_t>(block[2] | (block[3] << 8));

    uint8 r[4], g[4], b[4];
    DecodeColor565(c0, r[0], g[0], b[0]);
    DecodeColor565(c1, r[1], g[1], b[1]);

    r[2] = static_cast<uint8>((2 * r[0] + r[1]) / 3);
    g[2] = static_cast<uint8>((2 * g[0] + g[1]) / 3);
    b[2] = static_cast<uint8>((2 * b[0] + b[1]) / 3);
    r[3] = static_cast<uint8>((r[0] + 2 * r[1]) / 3);
    g[3] = static_cast<uint8>((g[0] + 2 * g[1]) / 3);
    b[3] = static_cast<uint8>((b[0] + 2 * b[1]) / 3);

    std::uint32_t indices = block[4] | (block[5] << 8) | (block[6] << 16) | (block[7] << 24);
    for (int32 row = 0; row < 4; ++row)
    {
        for (int32 col = 0; col < 4; ++col)
        {
            const int32 dstX = x + col;
            const int32 dstY = y + row;
            if (dstX >= width || dstY >= height)
            {
                indices >>= 2;
                continue;
            }

            const std::uint32_t index = indices & 0x3;
            indices >>= 2;

            uint8* pixel = dest + dstY * destStride + dstX * 4;
            pixel[0] = b[index];
            pixel[1] = g[index];
            pixel[2] = r[index];
            pixel[3] = 255;
        }
    }
}

static void DecodeDXT5Alpha(const uint8* block, uint8 outAlpha[16])
{
    const uint8 a0 = block[0];
    const uint8 a1 = block[1];
    outAlpha[0] = a0;
    outAlpha[1] = a1;

    if (a0 > a1)
    {
        outAlpha[2] = static_cast<uint8>((6 * a0 + 1 * a1) / 7);
        outAlpha[3] = static_cast<uint8>((5 * a0 + 2 * a1) / 7);
        outAlpha[4] = static_cast<uint8>((4 * a0 + 3 * a1) / 7);
        outAlpha[5] = static_cast<uint8>((3 * a0 + 4 * a1) / 7);
        outAlpha[6] = static_cast<uint8>((2 * a0 + 5 * a1) / 7);
        outAlpha[7] = static_cast<uint8>((1 * a0 + 6 * a1) / 7);
    }
    else
    {
        outAlpha[2] = static_cast<uint8>((4 * a0 + 1 * a1) / 5);
        outAlpha[3] = static_cast<uint8>((3 * a0 + 2 * a1) / 5);
        outAlpha[4] = static_cast<uint8>((2 * a0 + 3 * a1) / 5);
        outAlpha[5] = static_cast<uint8>((1 * a0 + 4 * a1) / 5);
        outAlpha[6] = 0;
        outAlpha[7] = 255;
    }

    std::uint64_t alphaIndices = 0;
    for (int i = 0; i < 6; ++i)
    {
        alphaIndices |= (static_cast<std::uint64_t>(block[2 + i]) << (8 * i));
    }

    for (int i = 0; i < 16; ++i)
    {
        const std::uint32_t idx = static_cast<std::uint32_t>(alphaIndices & 0x7);
        alphaIndices >>= 3;
        outAlpha[i] = outAlpha[idx];
    }
}

static void DecodeBC3Block(const uint8* block, uint8* dest, int32 destStride, int32 width, int32 height, int32 x, int32 y)
{
    uint8 alphaLut[16];
    DecodeDXT5Alpha(block, alphaLut);

    DecodeBC1ColorBlock(block + 8, dest, destStride, width, height, x, y);

    for (int32 row = 0; row < 4; ++row)
    {
        for (int32 col = 0; col < 4; ++col)
        {
            const int32 dstX = x + col;
            const int32 dstY = y + row;
            if (dstX >= width || dstY >= height)
            {
                continue;
            }

            uint8* pixel = dest + dstY * destStride + dstX * 4;
            pixel[3] = alphaLut[row * 4 + col];
        }
    }
}

static void DecodeBC5Block(const uint8* block, uint8* dest, int32 destStride, int32 width, int32 height, int32 x, int32 y)
{
    uint8 redLut[16];
    uint8 greenLut[16];
    DecodeDXT5Alpha(block, redLut);
    DecodeDXT5Alpha(block + 8, greenLut);

    for (int32 row = 0; row < 4; ++row)
    {
        for (int32 col = 0; col < 4; ++col)
        {
            const int32 dstX = x + col;
            const int32 dstY = y + row;
            if (dstX >= width || dstY >= height)
            {
                continue;
            }

            const int idx = row * 4 + col;
            uint8* pixel = dest + dstY * destStride + dstX * 4;
            pixel[0] = 255;
            pixel[1] = greenLut[idx];
            pixel[2] = redLut[idx];
            pixel[3] = 255;
        }
    }
}

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
    auto CombinedPath = Path + TextureName;
    std::wstring Ext;
    {
        auto Dot = TextureName.find_last_of(L'.');
        if (Dot != String::npos)
        {
            Ext = TextureName.substr(Dot);
            for (auto& Ch : Ext)
            {
                Ch = static_cast<wchar_t>(towlower(Ch));
            }
        }
    }

    MTexture Texture;
    Texture.Name = TextureName;
    Texture.BitsPerPixel = 32;

    auto ResolvePath = [&](const String& Base, const String& Name) -> std::string
    {
        auto ToString = [](const String& Value)
        {
            return std::string(Value.begin(), Value.end());
        };

        String Candidate = Base + Name;
        if (std::filesystem::exists(std::filesystem::path(Candidate)))
        {
            return ToString(Candidate);
        }

        const size_t SlashPos = Name.find_last_of(L"/\\");
        const String FileName = (SlashPos != String::npos) ? Name.substr(SlashPos + 1) : Name;

        Candidate = Base + FileName;
        if (std::filesystem::exists(std::filesystem::path(Candidate)))
        {
            return ToString(Candidate);
        }

        Candidate = Base + L"Textures/" + FileName;
        if (std::filesystem::exists(std::filesystem::path(Candidate)))
        {
            return ToString(Candidate);
        }

        Candidate = Base + L"textures/" + FileName;
        if (std::filesystem::exists(std::filesystem::path(Candidate)))
        {
            return ToString(Candidate);
        }

        Candidate = Base + L"Texture/" + FileName;
        if (std::filesystem::exists(std::filesystem::path(Candidate)))
        {
            return ToString(Candidate);
        }

        return ToString(Base + Name);
    };

    const std::string FilePath = ResolvePath(Path, TextureName);

    if (Ext == L".tga")
    {
        Tga TGATexture(FilePath.c_str());
        Texture.Pixels = TGATexture.GetPixels();
        Texture.Width = TGATexture.GetWidth();
        Texture.Height = TGATexture.GetHeight();
        Texture.Size = 0;
        return Texture;
    }

    if (Ext == L".dds")
    {
        std::ifstream DDSFile(FilePath, std::ios::binary);
        if (!DDSFile)
        {
            cout << "Texture load failed: " << FilePath << endl;
            return Texture;
        }

        DDSFile.seekg(0, std::ios::end);
        const size_t Size = static_cast<size_t>(DDSFile.tellg());
        DDSFile.seekg(0, std::ios::beg);

        std::vector<uint8> Data(Size);
        if (!DDSFile.read(reinterpret_cast<char*>(Data.data()), Size))
        {
            cout << "Texture load failed: " << FilePath << endl;
            return Texture;
        }

        if (Size < 4 + sizeof(DDS_HEADER))
        {
            cout << "DDS header too small: " << FilePath << endl;
            return Texture;
        }

        if (std::memcmp(Data.data(), "DDS ", 4) != 0)
        {
            cout << "Invalid DDS magic: " << FilePath << endl;
            return Texture;
        }

        DDS_HEADER Header{};
        std::memcpy(&Header, Data.data() + 4, sizeof(DDS_HEADER));
        if (Header.size != 124 || Header.ddspf.size != 32)
        {
            cout << "Invalid DDS header: " << FilePath << endl;
            return Texture;
        }

        const uint32 Width = Header.width;
        const uint32 Height = Header.height;
        if (Width == 0 || Height == 0)
        {
            cout << "Invalid DDS dimensions: " << FilePath << endl;
            return Texture;
        }

        const char FourCC[5] = {
            static_cast<char>(Header.ddspf.fourCC & 0xFF),
            static_cast<char>((Header.ddspf.fourCC >> 8) & 0xFF),
            static_cast<char>((Header.ddspf.fourCC >> 16) & 0xFF),
            static_cast<char>((Header.ddspf.fourCC >> 24) & 0xFF),
            0
        };

        const uint8* PixelData = Data.data() + 4 + sizeof(DDS_HEADER);
        const size_t PixelSize = Size - (4 + sizeof(DDS_HEADER));

        Texture.Pixels.clear();
        Texture.Pixels.resize(static_cast<size_t>(Width) * static_cast<size_t>(Height) * 4);
        const int32 DestStride = static_cast<int32>(Width) * 4;

        const int32 BlocksX = static_cast<int32>((Width + 3) / 4);
        const int32 BlocksY = static_cast<int32>((Height + 3) / 4);

        if (std::strncmp(FourCC, "DXT1", 4) == 0)
        {
            const size_t Required = static_cast<size_t>(BlocksX) * static_cast<size_t>(BlocksY) * 8;
            if (PixelSize < Required)
            {
                cout << "DDS data size mismatch: " << FilePath << endl;
                return Texture;
            }

            const uint8* Block = PixelData;
            for (int32 by = 0; by < BlocksY; ++by)
            {
                for (int32 bx = 0; bx < BlocksX; ++bx)
                {
                    DecodeBC1Block(Block, Texture.Pixels.data(), DestStride, Width, Height, bx * 4, by * 4);
                    Block += 8;
                }
            }
        }
        else if (std::strncmp(FourCC, "DXT5", 4) == 0)
        {
            const size_t Required = static_cast<size_t>(BlocksX) * static_cast<size_t>(BlocksY) * 16;
            if (PixelSize < Required)
            {
                cout << "DDS data size mismatch: " << FilePath << endl;
                return Texture;
            }

            const uint8* Block = PixelData;
            for (int32 by = 0; by < BlocksY; ++by)
            {
                for (int32 bx = 0; bx < BlocksX; ++bx)
                {
                    DecodeBC3Block(Block, Texture.Pixels.data(), DestStride, Width, Height, bx * 4, by * 4);
                    Block += 16;
                }
            }
        }
        else if (std::strncmp(FourCC, "ATI2", 4) == 0 || std::strncmp(FourCC, "BC5U", 4) == 0 || std::strncmp(FourCC, "BC5S", 4) == 0)
        {
            const size_t Required = static_cast<size_t>(BlocksX) * static_cast<size_t>(BlocksY) * 16;
            if (PixelSize < Required)
            {
                cout << "DDS data size mismatch: " << FilePath << endl;
                return Texture;
            }

            const uint8* Block = PixelData;
            for (int32 by = 0; by < BlocksY; ++by)
            {
                for (int32 bx = 0; bx < BlocksX; ++bx)
                {
                    DecodeBC5Block(Block, Texture.Pixels.data(), DestStride, Width, Height, bx * 4, by * 4);
                    Block += 16;
                }
            }
        }
        else
        {
            cout << "Unsupported DDS format: " << FourCC << " (" << FilePath << ")" << endl;
            return Texture;
        }

        Texture.Width = Width;
        Texture.Height = Height;
        Texture.Size = 0;
        return Texture;
    }

    int Width = 0;
    int Height = 0;
    int Components = 0;
    unsigned char* Pixels = stbi_load(FilePath.c_str(), &Width, &Height, &Components, 4);
    if (!Pixels || Width <= 0 || Height <= 0)
    {
        cout << "Texture load failed: " << FilePath << endl;
        return Texture;
    }

    const size_t PixelCount = static_cast<size_t>(Width) * static_cast<size_t>(Height);
    Texture.Pixels.resize(PixelCount * 4);
    for (size_t i = 0; i < PixelCount; ++i)
    {
        const size_t Src = i * 4;
        const size_t Dst = i * 4;
        // Convert RGBA -> BGRA for DXGI_FORMAT_B8G8R8A8_UNORM
        Texture.Pixels[Dst + 0] = Pixels[Src + 2];
        Texture.Pixels[Dst + 1] = Pixels[Src + 1];
        Texture.Pixels[Dst + 2] = Pixels[Src + 0];
        Texture.Pixels[Dst + 3] = Pixels[Src + 3];
    }

    stbi_image_free(Pixels);

    Texture.Width = static_cast<uint32>(Width);
    Texture.Height = static_cast<uint32>(Height);
    Texture.Size = 0;

	return Texture;
}
