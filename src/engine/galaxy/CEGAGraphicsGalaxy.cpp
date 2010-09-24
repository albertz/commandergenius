/*
 * CEGAGraphics.cpp
 *
 *  Created on: 27.12.2009
 *      Author: gerstrong
 *
 *  Decompression and decoding routine for
 *  Commander Keen Galaxy Series.
 *  EGAHEAD and EGADICT are normally files that have been embedded
 *  into the executable.
 *  EGAGRAPH is a seperate file which contains the graphics itself
 *  All thre files are needed to extract the graphics
 */

#include "CEGAGraphicsGalaxy.h"
#include "../../graphics/CGfxEngine.h"
#include "../../FindFile.h"
#include "../../fileio/compression/CHuffman.h"
#include "../../CLogFile.h"
#include "../../StringUtils.h"
#include "../../graphics/CGfxEngine.h"
#include "../../sdl/CVideoDriver.h"
#include "../../fileio/CTileLoader.h"
#include "../../common/CObject.h"
#include "../CPlanes.h"
#include <fstream>
#include <cstring>
#include <SDL.h>

namespace galaxy
{

// The ExeImageSize fields are values from the real start of exe image,
// i.e. KEEN4E.EXE + 0x2E00 to EndOfFile=0x3D740
// It seems that the standard shareware KDREAMS.EXE starts at 0x1C00

static EpisodeInfoStruct EpisodeInfo[] = {
	{	/* Episode 4 */
		0x3D740,	/* ExeImageSize */
		0x21080,	/* OffEgaHead */
		0x358F6,	/* OffEgaDict */
		4751,		/* NumChunks */
		3, 3,		/* NumFonts, IndexFonts */
		115, 6,		/* NumBitmaps, IndexBitmaps */
		3, 121,		/* NumMaskedBitmaps, IndexMasked Bitmaps */
		397, 124,	/* NumSprites, IndexSprites */
		104, 521,	/* Num8Tiles, Index8Tiles */
		20, 522,	/* Num8MaskedTiles, Index8MaskedTiles */
		1296, 523,	/* Num16Tiles, Index16Tiles */
		2916, 1819,	/* Num16MaskedTiles, Index16MaskedTiles */
		4, 4735,	/* NumMisc, IndexMisc */
		7, 4739,	/* NumTexts, IndexTexts */
		5, 4746		/* NumDemos, IndexDemos */
	},
	{	/* Episode 5 */
		0x3E370,	/* ExeImageSize */
		0x21FC0,	/* OffEgaHead */
		0x362C4,	/* OffEgaDict */
		4931,		/* NumChunks */
		3, 3,		/* NumFonts, IndexFonts */
		93, 6,		/* NumBitmaps, IndexBitmaps */
		3, 99,		/* NumMaskedBitmaps, IndexMasked Bitmaps */
		346, 102,	/* NumSprites, IndexSprites */
		104, 448,	/* Num8Tiles, Index8Tiles */
		20, 449,	/* Num8MaskedTiles, Index8MaskedTiles */
		1512, 450,	/* Num16Tiles, Index16Tiles */
		2952, 1962,	/* Num16MaskedTiles, Index16MaskedTiles */
		5, 4921,	/* NumMisc, IndexMisc */
		7, 4914,	/* NumTexts, IndexTexts */
		5, 4926		/* NumDemos, IndexDemos */
	},
	{	/* Episode 6 */
		0x3F950,	/* ExeImageSize */
		0x20F50,	/* OffEgaHead */
		0x372EE,	/* OffEgaDict */
		5560,		/* NumChunks */
		3, 3,		/* NumFonts, IndexFonts */
		37, 6,		/* NumBitmaps, IndexBitmaps */
		3, 43,		/* NumMaskedBitmaps, IndexMasked Bitmaps */
		390, 46,	/* NumSprites, IndexSprites */
		104, 436,	/* Num8Tiles, Index8Tiles */
		12, 437,	/* Num8MaskedTiles, Index8MaskedTiles */
		2376, 438,	/* Num16Tiles, Index16Tiles */
		2736, 2814,	/* Num16MaskedTiles, Index16MaskedTiles */
		4, 5551,	/* NumMisc, IndexMisc */
		1, 5550,	/* NumTexts, IndexTexts */
		5, 5555		/* NumDemos, IndexDemos */
	},
	{	/* Episode 7 (Dreams) */
		// The following ExeImageSize = KDREAMS.EXE (213536) - 0x1C00:
		206368,		/* ExeImageSize */
		0x1CB20,	/* OffEgaHead */
		0x29F12,	/* OffEgaDict */
		3019,		/* NumChunks */
		0, 0,		/* NumFonts, IndexFonts */
		0, 0,		/* NumBitmaps, IndexBitmaps */
		0, 0,		/* NumMaskedBitmaps, IndexMasked Bitmaps */
		0, 0,		/* NumSprites, IndexSprites */
		1, 368,		/* Num8Tiles, Index8Tiles */
		1, 369,		/* Num8MaskedTiles, Index8MaskedTiles */
		1440, 370,	/* Num16Tiles, Index16Tiles */
		1224,1810 ,	/* Num16MaskedTiles, Index16MaskedTiles */
		0, 0,		/* NumMisc, IndexMisc */
		0, 0,		/* NumTexts, IndexTexts */
		0, 0		/* NumDemos, IndexDemos */
	}
};

/////
// Class members start here!
/////
/**
 * \brief	This creates the class for reading the graphical
 * \param	episode		Episode of the chosen game
 * \param	path		Path to where the game is found on the media
 * \param	ExeFile		Object to ExeFile in which the
 */
CEGAGraphicsGalaxy::CEGAGraphicsGalaxy(CExeFile &ExeFile) :
CEGAGraphics(ExeFile.getEpisode(), ExeFile.getDataDirectory()),
m_Exefile(ExeFile)
{
	createBitmapsIDs();
}

int CEGAGraphicsGalaxy::getNumSprites()
{ return 0; }
short CEGAGraphicsGalaxy::getNumTiles()
{ return 0; }

/**
 * \brief	load the data into the structure
 * \return 	returns true, if loading was successful
 */
bool CEGAGraphicsGalaxy::loadData()
{
	// Set the palette, so the proper colours are loaded
	g_pGfxEngine->Palette.setupColorPalettes(m_Exefile.getRawData(), m_episode);

	if(!begin()) return false;

	// First, retrieve the Tile properties so the tilemap gets properly formatted
	// Important especially for masks, and later in the game for the behaviours
    // of those objects
	CTileLoader TileLoader( m_Exefile );
	if(!TileLoader.load(EpisodeInfo[m_episode-4].Num16Tiles,
						EpisodeInfo[m_episode-4].Num16MaskedTiles))
		return false;

	if(!readfonts()) return false;
	if(!readBitmaps()) return false;
	if(!readMaskedBitmaps()) return false;

	g_pGfxEngine->createEmptyTilemap(4);

	if(!readTilemaps(EpisodeInfo[m_episode-4].Num16Tiles, 4, 18,
			EpisodeInfo[m_episode-4].Index16Tiles,
			g_pGfxEngine->getTileMap(0), false)) return false;
	if(!readMaskedTilemaps(EpisodeInfo[m_episode-4].Num16MaskedTiles, 4, 18,
			EpisodeInfo[m_episode-4].Index16MaskedTiles,
			g_pGfxEngine->getTileMap(1), false)) return false;
	if(!readTilemaps(EpisodeInfo[m_episode-4].Num8Tiles, 3, 1,
			EpisodeInfo[m_episode-4].Index8Tiles,
			g_pGfxEngine->getTileMap(2), true)) return false;
	if(!readMaskedTilemaps(EpisodeInfo[m_episode-4].Num8MaskedTiles, 3, 1,
			EpisodeInfo[m_episode-4].Index8MaskedTiles,
			g_pGfxEngine->getTileMap(3), true)) return false;
	if(!readSprites( EpisodeInfo[m_episode-4].NumSprites,
			EpisodeInfo[m_episode-4].IndexSprites )) return false;
	//k456_export_texts();
	//k456_export_misc();
	//k456_export_demos();
	//k456_export_end();

	return true;
}

/**
 * \brief 	This function extracts a picture from the galaxy graphics map, and converts it properly to a
 * 			SDL Surface
 */
void CEGAGraphicsGalaxy::extractPicture(SDL_Surface *sfc,
		std::vector<unsigned char> &data, size_t Width, size_t Height,
		bool masked)
{
	if(SDL_MUSTLOCK(sfc)) SDL_LockSurface(sfc);
	SDL_FillRect(sfc, NULL, 0x00);

	if(!data.empty())
	{
		// Decode the bitmap data
		for(size_t p = 0; p < 4; p++)
		{
			unsigned char *pointer;
			Uint8* pixel = (Uint8*) sfc->pixels;

			// get location of plane p
			if(masked)
			{
				pointer = &(data[0]) + (p+1) * Width * Height;

				// now try to extract the bits and pass it to the SDL-Surface
				for(size_t y = 0; y < Height; y++)
				{
					for(size_t x = 0; x < Width; x++)
					{
						Uint8 bit,b;
						for(b=0 ; b<8 ; b++)
						{
							bit = getBit(*pointer, 7-b);
							*pixel |= (bit<<p);
							pixel++;
						}
						pointer++;
					}
				}
			}
			else
			{
				pointer = &(data[0]) + p * Width * Height;

				// now try to extract the bits and pass it to the SDL-Surface
				for(size_t y = 0; y < Height; y++)
				{
					for(size_t x = 0; x < Width; x++)
					{
						Uint8 bit,b;
						for(b=0 ; b<8 ; b++)
						{
							bit = getBit(*pointer, 7-b);
							*pixel |= (bit<<p);
							pixel++;
						}
						pointer++;
					}
				}
			}
		}

		if(masked)
		{
			// This stuff is for reading the mask and apply it to the pixel map
			unsigned char *pointer = &(data[0]);
			Uint8* pixel = (Uint8*) sfc->pixels;

			// now try to extract the bits and pass it to the SDL-Surface
			for(size_t y = 0; y < Height; y++)
			{
				for(size_t x = 0; x < Width; x++)
				{
					Uint8 bit,b;
					for(b=0 ; b<8 ; b++)
					{
						bit = getBit(*pointer, 7-b);
						if(bit == 1)
							*pixel = 16;

						pixel++;
					}
					pointer++;
				}
			}
		}
	}
}

/**
 * \brief 	This function extracts a tile from the galaxy graphics map, and converts it properly to a
 * 			SDL Surface
 */
void CEGAGraphicsGalaxy::extractTile(SDL_Surface *sfc, std::vector<unsigned char> &data,
		Uint16 size, Uint16 columns, size_t tile, bool usetileoffset)
{
	if(!data.empty())
	{
		// Decode the image data
		for(size_t p = 0; p < 4; p++)
		{
			// Decode the lines of the bitmap data
			size_t tileoff = usetileoffset ? (tile*4*columns*(size/8)*size) : 0;
			Uint8 *pointer = &(data[0]) + tileoff + p * (size/8) * size;
			for(size_t y = 0; y < size; y++)
			{
				Uint8 *pixel = (Uint8*)sfc->pixels +
						size*(tile%columns) +
						size*size*columns*(tile/columns) +
						(size*columns*y);
				for(size_t x = 0; x < (size/8); x++)
				{
					Uint8 bit,b;
					for(b=0 ; b<8 ; b++)
					{
						bit = getBit(*pointer, 7-b);
						*pixel |= (bit<<p);
						pixel++;
					}
					pointer++;
				}
			}

		}
	}
}

/**
 * \brief 	This function extracts a tile from the galaxy graphics map, and converts it properly to a
 * 			SDL Surface
 */
void CEGAGraphicsGalaxy::extractMaskedTile(SDL_Surface *sfc, std::vector<unsigned char> &data,
		Uint16 size, Uint16 columns, size_t tile, bool usetileoffset)
{
	if(!data.empty())
	{
		// Decode the image data
		size_t tileoff = usetileoffset ? (tile*5*columns*(size/8)*size) : 0;
		for(size_t p = 0; p < 4; p++)
		{
			// Decode the lines of the bitmap data
			Uint8 *pointer = &(data[0]) + tileoff + (p+1) * (size/8) * size;
			for(size_t y = 0; y < size; y++)
			{
				Uint8 *pixel = (Uint8*)sfc->pixels +
						size*(tile%columns) +
						size*size*columns*(tile/columns) +
						(size*columns*y);
				for(size_t x = 0; x < (size/8); x++)
				{
					Uint8 bit,b;
					for(b=0 ; b<8 ; b++)
					{
						bit = getBit(*pointer, 7-b);
						*pixel |= (bit<<p);
						pixel++;
					}
					pointer++;
				}
			}
		}

		// now apply the mask!
		Uint8 *pointer = &(data[0]) + tileoff;
		for(size_t y = 0; y < size; y++)
		{
			Uint8 *pixel = (Uint8*)sfc->pixels +
					size*(tile%columns) +
					size*size*columns*(tile/columns) +
					(size*columns*y);
			for(size_t x = 0; x < (size/8); x++)
			{
				Uint8 bit,b;
				for(b=0 ; b<8 ; b++)
				{
					bit = getBit(*pointer, 7-b);
					if(bit == 1)
						*pixel = 16;
					pixel++;
				}
				pointer++;
			}
		}
	}
}

/**
 * \brief	prepares to load the data. Does a bit of extraction
 * \return 	returns true, if loading was successful
 */
bool CEGAGraphicsGalaxy::begin()
{
	// The stuff is Huffman compressed. Use an instance for that
	CHuffman Huffman;
	unsigned long exeheaderlen = 0;
	unsigned long exeimglen = 0;
	assert(m_episode >= 4);
	int ep = m_episode - 4; // index for EpisodeInfo; 0 - keen4, 1 - keen5, etc

	unsigned char *p_data = m_Exefile.getHeaderData();

	//if(m_episode == 7) exeheaderlen = HEADERLEN_KDREAMS;
	if(!m_Exefile.readExeImageSize( p_data, &exeimglen, &exeheaderlen))
		return false;

	// We need the EGADICT. Read it to our structure of Huffman, he needs it!
	Huffman.readDictionary(p_data, exeheaderlen + EpisodeInfo[ep].OffEgaDict);

	// Now we go to EGAHEAD
	// TODO: Implement a structure which will look for the files first and take them if possible

	// Read the EGAHEAD
	unsigned char *p_head = p_data + exeheaderlen + EpisodeInfo[ep].OffEgaHead;
	unsigned long offset = 0;
	unsigned long offset_limit;

	// For some reason, MultiMania's KDR support uses a slightly different limit
	// in offset ops. We're not in DOS, so we don't have to worry about
	// memory here >:P
	if (ep < 3) offset_limit = 0x00FFFFFF;
	else offset_limit = 0xFFFFFFFF;

	for(size_t i = 0 ; i < EpisodeInfo[ep].NumChunks ; i++)
	{
		if (ep != 3) {
			memcpy(&offset, p_head,3); // Keen 4-6
			p_head += 3;
			offset &= offset_limit;
		}
		else {
			memcpy(&offset, p_head,4); // KeenDreams
			p_head += 4;
		}
		m_egahead.push_back(offset);
	}

	// Now read the EGAGRAPH
	std::string filename;
	if (m_episode <= 6) filename =  m_path + "EGAGRAPH.CK" + itoa(m_episode);
	else filename =  m_path + "KDREAMS.EGA";

	std::ifstream File; OpenGameFileR(File, filename, std::ios::binary);

	if(!File)
	{
		g_pLogFile->textOut(RED,"Error the file \"" + filename + "\" is missing or can't be read!");
		return false;
	}

	size_t egagraphlen = 0;
	File.seekg(1,std::ios::end);
	egagraphlen = File.tellg();
	if(egagraphlen == 0)
	{
		g_pLogFile->textOut(RED,"Error the file \"" + filename + "\" is empty!");
		return false;
	}
	egagraphlen--;
	File.seekg(0,std::ios::beg);

	std::vector<unsigned char> CompEgaGraphData(egagraphlen);
	File.read((char*)&CompEgaGraphData[0], egagraphlen);

	// Make a clean memory pattern
	ChunkStruct ChunkTemplate;
	ChunkTemplate.len=0;
	m_egagraph.assign(EpisodeInfo[ep].NumChunks, ChunkTemplate);

	unsigned long inlen = 0, outlen = 0;

	// Now lets decompress the graphics
	for(size_t i = 0; i < EpisodeInfo[ep].NumChunks; i++)
	{
		// Show that something is happening
		offset = m_egahead[i];
		
		// Make sure the chunk is valid
		if(offset < offset_limit && offset + 4 <= CompEgaGraphData.size())
		{
			// Get the expanded length of the chunk
			if(i >= EpisodeInfo[ep].Index8Tiles && i < EpisodeInfo[ep].Index16MaskedTiles + EpisodeInfo[ep].Num16MaskedTiles)
			{
				// Expanded sizes of 8, 16,and 32 tiles are implicit
				if(i >= EpisodeInfo[ep].Index16MaskedTiles) // 16x16 tiles are one/chunk
					outlen = 2 * 16 * 5;
				else if(i >= EpisodeInfo[ep].Index16Tiles)
					outlen = 2 * 16 * 4;
				else if(i >= EpisodeInfo[ep].Index8MaskedTiles)	// 8x8 tiles are all in one chunk!
					outlen = EpisodeInfo[ep].Num8MaskedTiles * 8 * 5;
				else if(i >= EpisodeInfo[ep].Index8Tiles)
					outlen = EpisodeInfo[ep].Num8Tiles * 8 * 4;
			}
			else
			{
				memcpy(&outlen, &CompEgaGraphData[offset], 4);
				offset += 4;
			}

			// Allocate memory and decompress the chunk
			m_egagraph[i].len = outlen;
			m_egagraph[i].data.assign(outlen, 0);

			inlen = 0;
			// Find out the input length
			size_t j;
			for(j = i + 1; j < EpisodeInfo[ep].NumChunks; j++)
			{
				if(m_egahead[j] != offset_limit)
				{
					inlen = m_egahead[j] - offset;
					break;
				}
			}
			if(j == EpisodeInfo[ep].NumChunks)
				inlen = egagraphlen - offset;
			Huffman.expand(&CompEgaGraphData[offset], &m_egagraph[i].data[0], inlen, outlen);
		}
		else
		{
			m_egagraph[i].len = 0;
		}
	}

	// Set up pointers to bitmap and sprite tables if not doing KDR operations
	//if (ep < 4) {
		//BmpHead = (BitmapHeadStruct *)m_egagraph[0].data;
		//BmpMaskedHead = (BitmapHeadStruct *)m_EgaGraph[1].data;
		//SprHead = (SpriteHeadStruct *)m_EgaGraph[2].data;
	//}

	File.close();
	return true;
}

/**
 * \brief	This function gets the bit of an unsigned char variable at certain position
 * \param	data		variable where the bit is to be sent.
 * \param	leftshift	sets the position of the bit the function has to retrieve
 * \return 	returns 1, if the bit at position is one, else 0
 */
Uint8 CEGAGraphicsGalaxy::getBit(unsigned char data, Uint8 leftshift)
{
	Uint8 value;

	value = data & (1<<leftshift);
	value >>= leftshift;

	return value;
}

/**
 * \brief	Read the fonts to the Gfx-Engine
 * \return 	returns true, if the fonts were read successfully, else false
 */
bool CEGAGraphicsGalaxy::readfonts()
{
	int bw, y, x;

	int ep = m_episode - 4;
	SDL_Color *Palette = g_pGfxEngine->Palette.m_Palette;

	g_pGfxEngine->createEmptyFontmaps(EpisodeInfo[ep].NumFonts);
	for(Uint16 i = 0; i < EpisodeInfo[ep].NumFonts; i++)
	{
		CFont &Font = g_pGfxEngine->getFont(i);
		Font.setMonochrome(true);

		if(m_egagraph.at(EpisodeInfo[ep].IndexFonts + i).data.at(0))
		{
			FontHeadStruct *FontHead =
					(FontHeadStruct*) &(m_egagraph.at(EpisodeInfo[ep].IndexFonts + i).data.at(0));

			// Find out the maximum character width
			int maxwidth=0;
			for(Uint16 j = 0; j < 256; j++)
			{
				Font.setWidthToCharacter(FontHead->Width[j],j);
				if(FontHead->Width[j] > maxwidth)
					maxwidth = FontHead->Width[j];
			}

			Font.CreateSurface(Palette, g_pVideoDriver->getScrollSurface()->flags, 8, maxwidth*16, FontHead->Height * 16);

			SDL_Surface* sfc = Font.getSDLSurface();

			SDL_FillRect(sfc, NULL, 0x8);

			if(SDL_MUSTLOCK(sfc)) SDL_LockSurface(sfc);
			Uint8* pixel = (Uint8*) sfc->pixels;

			unsigned char *pointer = &(m_egagraph[EpisodeInfo[ep].IndexFonts + i].data.at(0));

			if(!m_egagraph.at(EpisodeInfo[ep].IndexFonts + i).data.empty())
			{
				// Decode the font data
				for(int j = 0; j < 256; j++)
				{
					// Get the width of the character in bytes
					bw = (FontHead->Width[j] + 7) / 8;

					Uint8 *pixelpos;

					if(FontHead->Width[j] > 0)
					{
						SDL_Rect rect;

						rect.x = (j%16)*maxwidth;
						rect.y = (j/16)*FontHead->Height;
						rect.w = FontHead->Width[j];
						rect.h = FontHead->Height;

						for( y = 0 ; y < rect.h ; y++ )
						{
							pixelpos = pixel + (rect.y+y)*sfc->pitch+rect.x;
							for( x = 0 ; x < rect.w ; x++ )
							{
								pixelpos[x] = getBit(*(pointer + FontHead->Offset[j] + (y*bw) + x/8 ), 7-(x%8) )*0xF;
							}
						}
					}
				}
			}
			SDL_UnlockSurface(sfc);
		}
	}
	return true;
}

/**
 * \brief	This one extracts the bitmaps used in Keen 4-6 (Maybe Dreams in future)
 * \return 	returns true, if the fonts were read successfully, else false
 */
bool CEGAGraphicsGalaxy::readBitmaps()
{
	int ep = m_episode - 4;
	BitmapHeadStruct *BmpHead = (BitmapHeadStruct *) &(m_egagraph.at(0).data.at(0));
	SDL_Color *Palette = g_pGfxEngine->Palette.m_Palette;

	g_pGfxEngine->createEmptyBitmaps(EpisodeInfo[ep].NumBitmaps);

	for(size_t i = 0; i < EpisodeInfo[ep].NumBitmaps; i++)
	{
		CBitmap &Bitmap = g_pGfxEngine->getBitmap(i);
		Bitmap.setDimensions(BmpHead[i].Width*8, BmpHead[i].Height);
		Bitmap.createSurface(g_pVideoDriver->getScrollSurface()->flags, Palette);

		extractPicture(Bitmap.getSDLSurface(),
				m_egagraph.at(EpisodeInfo[ep].IndexBitmaps + i).data,
				BmpHead[i].Width, BmpHead[i].Height);


		Bitmap.setName(m_BitmapNameMap[ep][i]);
	}

	return true;
}

bool CEGAGraphicsGalaxy::readMaskedBitmaps()
{
	int ep = m_episode - 4;
	BitmapHeadStruct *BmpMaskedHead = (BitmapHeadStruct *) &(m_egagraph.at(1).data.at(0));
	SDL_Color *Palette = g_pGfxEngine->Palette.m_Palette;

	g_pGfxEngine->createEmptyMaskedBitmaps(EpisodeInfo[ep].NumMaskedBitmaps);

	for(size_t i = 0; i < EpisodeInfo[ep].NumMaskedBitmaps; i++)
	{
		CBitmap &Bitmap = g_pGfxEngine->getMaskedBitmap(i);
		Bitmap.setDimensions(BmpMaskedHead[i].Width*8, BmpMaskedHead[i].Height);
		Bitmap.createSurface(g_pVideoDriver->getScrollSurface()->flags, Palette);

		extractPicture(Bitmap.getSDLSurface(),
				m_egagraph.at(EpisodeInfo[ep].IndexMaskedBitmaps + i).data,
				BmpMaskedHead[i].Width, BmpMaskedHead[i].Height, true);
	}
	return true;
}

bool CEGAGraphicsGalaxy::readTilemaps(	size_t NumTiles, size_t pbasetilesize,
										size_t rowlength, size_t IndexOfTiles,
										CTilemap &Tilemap, bool tileoff)
{
	Tilemap.CreateSurface( g_pGfxEngine->Palette.m_Palette, SDL_SWSURFACE,
							NumTiles, pbasetilesize, rowlength );
	SDL_Surface *sfc = Tilemap.getSDLSurface();
	SDL_FillRect(sfc,NULL, 0);
	if(SDL_MUSTLOCK(sfc))	SDL_LockSurface(sfc);

	for(size_t i = 0; i < NumTiles; i++)
	{
		extractTile(sfc, m_egagraph.at(IndexOfTiles + (tileoff ? 0 : i)).data, (1<<pbasetilesize), rowlength, i, tileoff);
	}

	SDL_UnlockSurface(sfc);

	return true;
}

bool CEGAGraphicsGalaxy::readMaskedTilemaps( size_t NumTiles, size_t pbasetilesize,
											size_t rowlength, size_t IndexOfTiles,
											CTilemap &Tilemap, bool tileoff)
{
	Tilemap.CreateSurface( g_pGfxEngine->Palette.m_Palette, SDL_SWSURFACE,
							NumTiles, pbasetilesize, rowlength );
	SDL_Surface *sfc = Tilemap.getSDLSurface();
	SDL_FillRect(sfc,NULL, 0);
	if(SDL_MUSTLOCK(sfc))	SDL_LockSurface(sfc);

	for(size_t i = 0; i < NumTiles; i++)
	{
		extractMaskedTile(sfc, m_egagraph.at(IndexOfTiles + (tileoff ? 0 : i)).data, (1<<pbasetilesize), rowlength, i, tileoff);
	}

	SDL_UnlockSurface(sfc);

	return true;
}

bool CEGAGraphicsGalaxy::readSprites( size_t NumSprites, size_t IndexSprite )
{
	// Create all the sprites
	g_pGfxEngine->createEmptySprites(NumSprites);

	SpriteHeadStruct *SprHead = (SpriteHeadStruct *)&(m_egagraph.at(2).data.at(0));

	for(size_t i = 0; i < NumSprites; i++)
	{
		SpriteHeadStruct Head = SprHead[i];
		std::vector<unsigned char> &data = m_egagraph.at(IndexSprite + i).data;

		CSprite &Sprite = g_pGfxEngine->getSprite(i);
		Sprite.setSize( Head.Width*8, Head.Height );

		// Setup the collision information
		Uint16 boxX1 = ((Head.Rx1 - Head.OrgX) << (STC-Head.Shifts));
		Uint16 boxY1 = ((Head.Ry1 - Head.OrgY) << (STC-Head.Shifts));
		Uint16 boxX2 = ((Head.Rx2 - Head.OrgX) << (STC-Head.Shifts));
		Uint16 boxY2 = ((Head.Ry2 - Head.OrgY) << (STC-Head.Shifts));

		Sprite.setBouncingBoxCoordinates( boxX1, boxY1, boxX2, boxY2 );


		Sprite.createSurface( g_pVideoDriver->BlitSurface->flags,
				g_pGfxEngine->Palette.m_Palette );

		SDL_Surface *sfc = Sprite.getSDLSurface();
		SDL_FillRect(sfc,NULL, 0);
		if(SDL_MUSTLOCK(sfc))	SDL_LockSurface(sfc);

		if(!data.empty())
		{
			// Decode the image data
			for(size_t p = 0; p < 4; p++)
			{
				// Decode the lines of the bitmap data
				Uint8 *pointer = &(data[0]) + (p+1) * Head.Width * Head.Height;
				for(size_t y = 0; y < Head.Height; y++)
				{
					Uint8 *pixel = (Uint8*)sfc->pixels +
							(Head.Width * 8 *y);
					for(size_t x = 0; x < Head.Width; x++)
					{
						Uint8 bit,b;
						for(b=0 ; b<8 ; b++)
						{
							bit = getBit(*pointer, 7-b);
							*pixel |= (bit<<p);
							pixel++;
						}
						pointer++;
					}
				}
			}

			// now apply the mask!
			Uint8 *pointer = &(data[0]);
			for(size_t y = 0; y < Head.Height; y++)
			{
				Uint8 *pixel = (Uint8*)sfc->pixels +
						(Head.Width * 8*y);
				for(size_t x = 0; x < Head.Width; x++)
				{
					Uint8 bit,b;
					for(b=0 ; b<8 ; b++)
					{
						bit = getBit(*pointer, 7-b);
						if(bit == 1)
							*pixel = 16;
						pixel++;
					}
					pointer++;
				}
			}
		}
		SDL_UnlockSurface(sfc);
	}
	return true;
}

CEGAGraphicsGalaxy::~CEGAGraphicsGalaxy()
{
}

}