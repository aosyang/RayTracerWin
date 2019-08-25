//=============================================================================
// Texture.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "Texture.h"
#include "Platform.h"
#include "Math.h"
#include "ColorBuffer.h"

#include <math.h>

#include "png.h"

RTexture::RTexture()
	: Width(0)
	, Height(0)
{

}

RVec4 RTexture::Sample(float u, float v)
{
#if DEBUG_CHECK_NAN
	if (isnan(u) || isnan(v))
	{
		RLog("Error: RTexture::Sample() - Detected NaN! (u = %f, v = %f)\n", u, v);
		DebugBreak();
	}
#endif	// DEBUG_CHECK_NAN

	// Clamp mode
	//u = RMath::Clamp(u, 0.0f, 1.0f);
	//v = RMath::Clamp(v, 0.0f, 1.0f);

	// Repeat mode
	float cu = u - floor(u);
	float cv = v - floor(v);

	float fx = cu * (Width - 1);
	float fy = cv * (Height - 1);

	int x0 = (int)floorf(fx);
	int y0 = (int)floorf(fy);
	int x1 = (int)ceilf(fx);
	int y1 = (int)ceilf(fy);

	float dx = fx - x0;
	float dy = fy - y0;

	return RVec4::Lerp(
		RVec4::Lerp(Pixels[y0 * Width + x0], Pixels[y0 * Width + x1], dx),
		RVec4::Lerp(Pixels[y1 * Width + x0], Pixels[y1 * Width + x1], dx),
		dy
	);
}

std::unique_ptr<RTexture> RTexture::LoadTexturePNG(const std::string& Filename)
{
	std::unique_ptr<RTexture> Texture;

	FILE* png_file = fopen(Filename.c_str(), "rb");
	if (png_file)
	{
		png_byte png_header[8];
		fread(png_header, 1, 8, png_file);

		if (!png_sig_cmp(png_header, 0, 8))
		{
			png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
			if (png_ptr)
			{
				png_infop info_ptr = png_create_info_struct(png_ptr);
				if (info_ptr)
				{
					if (!setjmp(png_jmpbuf(png_ptr)))
					{
						png_init_io(png_ptr, png_file);
						png_set_sig_bytes(png_ptr, 8);

						png_read_info(png_ptr, info_ptr);

						int width = png_get_image_width(png_ptr, info_ptr);
						int height = png_get_image_height(png_ptr, info_ptr);

						//   [Color types]			[Allowed bit depths]
						//   0 greyscale			1, 2, 4, 8, 16
						//   2 RGB					8, 16
						//   3 Palette index		1, 2, 4, 8
						//   4 greyscale + alpha	8, 16
						//   6 RGBA					8, 16
						int color_type = png_get_color_type(png_ptr, info_ptr);
						int bit_depth = png_get_bit_depth(png_ptr, info_ptr);

						// Only support 8-bit RGB and RGBA formats For now
						if ((color_type == 2 || color_type == 6) && bit_depth == 8)
						{
							int num_of_passes = png_set_interlace_handling(png_ptr);
							png_read_update_info(png_ptr, info_ptr);

							if (!setjmp(png_jmpbuf(png_ptr)))
							{
								size_t row_bytes = png_get_rowbytes(png_ptr, info_ptr);
								png_bytep* row_ptrs = new png_bytep[height];
								for (int y = 0; y < height; y++)
								{
									row_ptrs[y] = new png_byte[row_bytes];
								}

								png_read_image(png_ptr, row_ptrs);

								Texture = std::unique_ptr<RTexture>(new RTexture());

								Texture->Width = width;
								Texture->Height = height;
								Texture->Pixels.resize(width * height);

								if (color_type == 2)
								{
									for (int y = 0; y < height; y++)
									{
										for (int x = 0; x < width; x++)
										{
											png_byte r = *(row_ptrs[y] + 3 * x);
											png_byte g = *(row_ptrs[y] + 3 * x + 1);
											png_byte b = *(row_ptrs[y] + 3 * x + 2);

											Texture->Pixels[y * width + x] = RVec4((float)r / 255, (float)g / 255, (float)b / 255, 1.0f);
										}
									}
								}
								else if (color_type == 6)
								{
									for (int y = 0; y < height; y++)
									{
										for (int x = 0; x < width; x++)
										{
											png_byte r = *(row_ptrs[y] + 4 * x);
											png_byte g = *(row_ptrs[y] + 4 * x + 1);
											png_byte b = *(row_ptrs[y] + 4 * x + 2);
											png_byte a = *(row_ptrs[y] + 4 * x + 3);

											Texture->Pixels[y * width + x] = RVec4((float)r / 255, (float)g / 255, (float)b / 255, (float)a / 255);
										}
									}
								}
                                
                                // Release memory
                                for (int y = 0; y < height; y++)
                                {
                                    delete [] row_ptrs[y];
                                }
                                
                                delete [] row_ptrs;
							}
							else
							{
								RLog("Error during read_image: %s\n", Filename.c_str());
							}
						}
						else
						{
							RLog("Not an implemented png format (color type = %d, bit depth = %d): %s\n", color_type, bit_depth, Filename.c_str());
						}
					}
					else
					{
						RLog("Error during init_io: %s\n", Filename.c_str());
					}
				}
				else
				{
					RLog("png_create_info_struct failed: %s\n", Filename.c_str());
				}
			}
			else
			{
				RLog("png_create_read_struct failed: %s\n", Filename.c_str());
			}
		}
		else
		{
			RLog("File is not recognized as a PNG file: %s\n", Filename.c_str());
		}

		fclose(png_file);
	}
	else
	{
		RLog("File could not be opened for reading: %s\n", Filename.c_str());
	}

	return Texture;
}

bool RTexture::SaveBufferToPNG(const std::string& Filename, const UINT32* Pixels, int width, int height)
{
    int code = 0;
    FILE* fp = nullptr;
    png_structp png_ptr = nullptr;
    png_infop info_ptr = nullptr;
    png_bytep row = nullptr;
    
    fp = fopen(Filename.c_str(), "wb");
    if (fp != nullptr)
    {
        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (png_ptr != nullptr)
        {
            info_ptr = png_create_info_struct(png_ptr);
            if (info_ptr != nullptr)
            {
                if (!setjmp(png_jmpbuf(png_ptr)))
                {
                    png_init_io(png_ptr, fp);
                    
                    png_set_IHDR(png_ptr, info_ptr, width, height,
                                 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
                    
                    png_write_info(png_ptr, info_ptr);
                    
                    // Allocate memory
                    row = new png_byte[3 * width * sizeof(png_byte)];
                    
                    int x, y;
                    for (y = 0; y < height; y++)
                    {
                        for (x = 0; x < width; x++)
                        {
                            UINT32 Color = Pixels[y * width + x];
                            row[x * 3] = GetUint32ColorRed(Color);
                            row[x * 3 + 1] = GetUint32ColorGreen(Color);
                            row[x * 3 + 2] = GetUint32ColorBlue(Color);
                        }
                        png_write_row(png_ptr, row);
                    }
                    
                    delete [] row;
                }
                else
                {
                    code = 1;
                    RLog("Error during png creation.\n");
                }
            }
            else
            {
                code = 1;
                RLog("Could not allocate info struct.\n");
            }
        }
        else
        {
            code = 1;
            RLog("Could not allocate write struct.\n");
        }
        
        fclose(fp);
    }
    else
    {
        code = 1;
        RLog("Failed to save to %s: Unable to open file for writing.\n", Filename.c_str());
    }
    
    if (info_ptr)
    {
        png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    }
    
    if (png_ptr)
    {
        png_destroy_write_struct(&png_ptr, (png_infopp)nullptr);
    }
    
    return code == 0;
}
