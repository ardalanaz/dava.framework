//
//  ResourcePacker.cpp
//  UIEditor
//
//  Created by Denis Bespalov on 1/10/13.
//  Originaly Created by Vitaliy  Borodovsky on 3/21/10 for class ResourcePackerScreen.
//
//

#include "ResourcePacker.h"
#include <Magick++.h>
#include <magick/MagickCore.h>
#include <magick/property.h>
#include "TexturePacker/DefinitionFile.h"
#include "TexturePacker/TexturePacker.h"
#include "TexturePacker/CommandLineParser.h"

ResourcePacker::ResourcePacker()
{
	isLightmapsPacking = false;
	clearProcessDirectory = false;
}

String ResourcePacker::GetProcessFolderName()
{
	return "$process";
}

void ResourcePacker::PackResources(const String & inputDir, const String & outputDir)
{
	// Do not try to pack empty if pack directories are not set
	if (inputDir.empty() || outputDir.empty())
		return;
		
	// Create output dir for case it is not created
	FileSystem::Instance()->CreateDirectory(outputDir, true);

	CommandLineParser::Instance()->ClearFlags();

	clearProcessDirectory = true;
	inputGfxDirectory = inputDir;
	outputGfxDirectory = outputDir;
	excludeDirectory = FileSystem::Instance()->RealPath(inputDir + "/../");
	isLightmapsPacking = true;
	
	StartPacking();
}

void ResourcePacker::StartPacking()
{
	Logger::Debug("Input: %s Output: %s Exclude: %s", inputGfxDirectory.c_str(), outputGfxDirectory.c_str(), excludeDirectory.c_str());
	
	isGfxModified = false;

	String path, lastDir;
	FileSystem::SplitPath(inputGfxDirectory, path, lastDir);
	std::transform(lastDir.begin(), lastDir.end(), lastDir.begin(), ::tolower);
	gfxDirName = lastDir;


	String processDirectoryPath = excludeDirectory + String("/") + GetProcessFolderName() + String("/");
	if (FileSystem::Instance()->CreateDirectory(processDirectoryPath, true) == FileSystem::DIRECTORY_CANT_CREATE)
	{
		//Logger::Error("Can't create directory: %s", processDirectoryPath.c_str());
	}


	if (IsMD5ChangedDir(excludeDirectory + String("/") + GetProcessFolderName(), outputGfxDirectory, gfxDirName + ".md5", true))
	{
		if (Core::Instance()->IsConsoleMode())
			printf("[Gfx not available or changed - performing full repack]\n");
		isGfxModified = true;
	
		// Remove whole output directory
		bool result = FileSystem::Instance()->DeleteDirectory(outputGfxDirectory);
		if (result)
		{
			Logger::Debug("Removed output directory: %s", outputGfxDirectory.c_str());
		}
		if (!result && Core::Instance()->IsConsoleMode())
		{
			printf("[ERROR: Can't delete directory %s]\n", outputGfxDirectory.c_str());
		}
	}

	RecursiveTreeWalk(inputGfxDirectory, outputGfxDirectory);

	// Put latest md5 after convertation
	IsMD5ChangedDir(excludeDirectory + String("/") + GetProcessFolderName(), outputGfxDirectory, gfxDirName + ".md5", true);
}


bool ResourcePacker::IsMD5ChangedDir(const String & processDirectoryPath, const String & pathname, const String & name, bool isRecursive)
{
	String pathnameWithoutExtension = FileSystem::ReplaceExtension(name, "");
	String md5FileName = processDirectoryPath + String("/") + pathnameWithoutExtension + ".md5";

	uint8 oldMD5Digest[16];
	uint8 newMD5Digest[16];
	bool isChanged = false;
	File * file = File::Create(md5FileName, File::OPEN | File::READ);
	if (!file)
	{
		isChanged = true;		
	}else
	{
		int32 bytes = file->Read(oldMD5Digest, 16);
		DVASSERT(bytes == 16 && "We should always read 16 bytes from md5 file");
	}
	SafeRelease(file);


	MD5::ForDirectory(pathname, newMD5Digest, isRecursive);

	file = File::Create(md5FileName, File::CREATE | File::WRITE);
	if (!file)
	{
		isChanged = true;
	}
	else
	{
		int32 bytes = file->Write(newMD5Digest, 16);
		DVASSERT(bytes == 16 && "16 bytes should be always written for md5 file");
	}
	SafeRelease(file);

	// if already changed return without compare
	if (isChanged)
		return true;

	for (int32 k = 0; k < 16; ++k)
		if (oldMD5Digest[k] != newMD5Digest[k])
			isChanged = true;
	
	return isChanged;
}


bool ResourcePacker::IsMD5ChangedFile(const String & processDirectoryPath, const String & pathname, const String & psdName)
{
	String pathnameWithoutExtension = FileSystem::ReplaceExtension(psdName, "");
	String md5FileName = processDirectoryPath + String("/") + pathnameWithoutExtension + ".md5";

	uint8 oldMD5Digest[16];
	uint8 newMD5Digest[16];
	bool isChanged = false;
	File * file = File::Create(md5FileName, File::OPEN | File::READ);
	if (!file)
	{
		isChanged = true;		
	}else
	{
		int32 bytes = file->Read(oldMD5Digest, 16);
		DVASSERT(bytes == 16 && "We should always read 16 bytes from md5 file");
	}
	SafeRelease(file);

		
	MD5::ForFile(pathname, newMD5Digest);
	
	file = File::Create(md5FileName, File::CREATE | File::WRITE);
	if (!file)
	{
		isChanged = true;
	}
	else
	{
		int32 bytes = file->Write(newMD5Digest, 16);
		DVASSERT(bytes == 16 && "16 bytes should be always written for md5 file");
	}
	SafeRelease(file);

	for (int32 k = 0; k < 16; ++k)
		if (oldMD5Digest[k] != newMD5Digest[k])
			isChanged = true;
	
	return isChanged;
}

DefinitionFile * ResourcePacker::ProcessPSD(const String & processDirectoryPath, const String & psdPathname, const String & psdName)
{
	int32 maxTextureSize = 1024;
	if (CommandLineParser::Instance()->IsFlagSet("--tsize2048"))
	{
		maxTextureSize = 2048;
	}
	
	// TODO: Check CRC32
	std::vector<Magick::Image> layers;
	
	String psdNameWithoutExtension = FileSystem::ReplaceExtension(psdName, "");
	
	try 
	{
		Magick::readImages(&layers, psdPathname);
		
		if (layers.size() == 0)
		{
			Logger::Error("Number of layers is too low: %s", psdPathname.c_str());
			return 0;
		}
		
		if (layers.size() == 1)
		{
			layers.push_back(layers[0]);
		}
		
		int width = (int)layers[0].columns();
		int height = (int)layers[0].rows();
		
		for(int k = 1; k < (int)layers.size(); ++k)
		{
			Magick::Image & currentLayer = layers[k];			
			
			currentLayer.crop(Magick::Geometry(width,height, 0, 0));
			currentLayer.magick("PNG");
			String outputFile = processDirectoryPath + String("/") + psdNameWithoutExtension;
			outputFile += String(Format("%d.png", k - 1));
			currentLayer.write(outputFile);
		}
		
		
		DefinitionFile * defFile = new DefinitionFile;
		defFile->filename = processDirectoryPath + String("/") + psdNameWithoutExtension + String(".txt");
		defFile->spriteWidth = width;
		defFile->spriteHeight = height;
		defFile->frameCount = (int)layers.size() -1;
		defFile->frameRects = new Rect2i[defFile->frameCount];
		
		for(int k = 1; k < (int)layers.size(); ++k)
		{
			Magick::Image & currentLayer = layers[k];
			Magick::Geometry bbox = currentLayer.page();
			int xOff = (int)bbox.xOff();
			if (bbox.xNegative())
				xOff = -xOff;
			int yOff = (int)bbox.yOff();
			if (bbox.yNegative())
				yOff = -yOff;
			
			defFile->frameRects[k - 1] = Rect2i(xOff, yOff, (int32)bbox.width(), (int32)bbox.height());
			
			if ((defFile->frameRects[k - 1].dx >= maxTextureSize) || (defFile->frameRects[k - 1].dy >= maxTextureSize))
			{
				
				printf("* WARNING * - frame of %s layer %d is bigger than maxTextureSize(%d) layer exportSize (%d x %d) FORCE REDUCE TO (%d x %d). Bewarned!!! Results not guaranteed!!!\n", psdName.c_str(), k - 1, maxTextureSize
					   , defFile->frameRects[k - 1].dx, defFile->frameRects[k - 1].dy, width, height);
				defFile->frameRects[k - 1].dx = width;
				defFile->frameRects[k - 1].dy = height;
			}
				
			
			
			if (CommandLineParser::Instance()->IsFlagSet("--add0pixel"))
			{
				
			}else if (CommandLineParser::Instance()->IsFlagSet("--add1pixel"))
			{
				defFile->frameRects[k - 1].dx++;
				defFile->frameRects[k - 1].dy++;
			}
			else if (CommandLineParser::Instance()->IsFlagSet("--add2pixel"))
			{
				defFile->frameRects[k - 1].dx+=2;
				defFile->frameRects[k - 1].dy+=2;
			}
			else if (CommandLineParser::Instance()->IsFlagSet("--add4pixel"))
			{
				defFile->frameRects[k - 1].dx+=4;
				defFile->frameRects[k - 1].dy+=4;
			}else 
			{
				defFile->frameRects[k - 1].dx++;
				defFile->frameRects[k - 1].dy++;	
			}
		}
		
		return defFile;
	}
	catch( Magick::Exception &error_ )
    {
		std::cout << "Caught exception: " << error_.what() << " file: "<< psdPathname << std::endl;
		return 0;
    }
	return 0;
}

void ResourcePacker::ProcessFlags(const String & flagsPathname)
{
	File * file = File::Create(flagsPathname.c_str(), File::READ | File::OPEN);
	if (!file)
	{
		Logger::Error("Failed to open file: %s", flagsPathname.c_str());
	}
	char flagsTmpBuffer[4096];
	int flagsSize = 0;
	while(!file->IsEof())
	{
		char c;
		int32 readSize = file->Read(&c, 1);
		if (readSize == 1)
		{
			flagsTmpBuffer[flagsSize++] = c;
		}	
	}
	flagsTmpBuffer[flagsSize++] = 0;
	
	currentFlags = flagsTmpBuffer;
	String flags = flagsTmpBuffer;
	
	const String & delims=" ";
	
	// Skip delims at beginning, find start of first token
	String::size_type lastPos = flags.find_first_not_of(delims, 0);
	// Find next delimiter @ end of token
	String::size_type pos     = flags.find_first_of(delims, lastPos);
	// output vector
	Vector<String> tokens;
	
	while (String::npos != pos || String::npos != lastPos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(flags.substr(lastPos, pos - lastPos));
		// Skip delims.  Note the "not_of". this is beginning of token
		lastPos = flags.find_first_not_of(delims, pos);
		// Find next delimiter at end of token.
		pos     = flags.find_first_of(delims, lastPos);
	}
	
	if (CommandLineParser::Instance()->GetVerbose())
		for (int k = 0; k < (int) tokens.size(); ++k)
		{
			Logger::Debug("Token: %s", tokens[k].c_str());
		}

	if (Core::Instance()->IsConsoleMode())
	{
		for (int k = 0; k < (int) tokens.size(); ++k)
		{
			String sub = tokens[k].substr(0, 2);
			if (sub != "--")
				printf("\n[WARNING: flag %s incorrect]\n", tokens[k].c_str());
		}
	}
	
	CommandLineParser::Instance()->SetFlags(tokens);
	
	SafeRelease(file);
}

void ResourcePacker::RecursiveTreeWalk(const String & inputPath, const String & outputPath)
{
	uint64 packTime = SystemTimer::Instance()->AbsoluteMS();

	FileList * fileList = new FileList(inputPath);

	// New $process folder structure
	String dataSourceRelativePath = inputPath;
	StringReplace(dataSourceRelativePath, excludeDirectory, std::string(""));

	String processDirectoryPath = excludeDirectory + String("/") + GetProcessFolderName() + String("/") + dataSourceRelativePath;
	if (FileSystem::Instance()->CreateDirectory(processDirectoryPath, true) == FileSystem::DIRECTORY_CANT_CREATE)
	{
		//Logger::Error("Can't create directory: %s", processDirectoryPath.c_str());
	}

	if(clearProcessDirectory)
	{
		FileSystem::Instance()->DeleteDirectoryFiles(processDirectoryPath, false);
	}

	//String outputPath = outputPath;
	if (FileSystem::Instance()->CreateDirectory(outputPath) == FileSystem::DIRECTORY_CANT_CREATE)
	{
		//Logger::Error("Can't create directory: %s", outputPath.c_str());
	}
	
	CommandLineParser::Instance()->ClearFlags();
	std::list<DefinitionFile *> definitionFileList;

	// Find flags and setup them
	for (int fi = 0; fi < fileList->GetCount(); ++fi)
	{
		if (!fileList->IsDirectory(fi))
		{
			if (fileList->GetFilename(fi) == "flags.txt")
			{
				String fullname = inputPath + String("/") + fileList->GetFilename(fi);
				ProcessFlags(fullname);
				break;
			}
		}
	}
	
	bool modified = isGfxModified;
	// Process all psd / png files

	if (IsMD5ChangedDir(processDirectoryPath, inputPath, "dir.md5", false))
	{
		modified = true;
	}

	if (modified)
	{
		FileSystem::Instance()->DeleteDirectoryFiles(outputPath, false);
		
		for (int fi = 0; fi < fileList->GetCount(); ++fi)
		{
			if (!fileList->IsDirectory(fi))
			{
				String fullname = inputPath + String("/") + fileList->GetFilename(fi);
				if (FileSystem::GetExtension(fullname) == ".psd")
				{
					DefinitionFile * defFile = ProcessPSD(processDirectoryPath, fullname, fileList->GetFilename(fi));
					definitionFileList.push_back(defFile);
				}
				else if(isLightmapsPacking && FileSystem::GetExtension(fullname) == ".png")
				{
					DefinitionFile * defFile = new DefinitionFile();
					defFile->LoadPNG(fullname, processDirectoryPath);
					definitionFileList.push_back(defFile);
				}
				else if (FileSystem::GetExtension(fullname) == ".pngdef")
				{
					DefinitionFile * defFile = new DefinitionFile();
					if (defFile->LoadPNGDef(fullname, processDirectoryPath))
					{
						definitionFileList.push_back(defFile);
					}
					else 
					{
						SafeDelete(defFile);
					}
				}
			}
		}

		// 
		if (definitionFileList.size() > 0 && modified)
		{
			TexturePacker packer;
			String outputPathWithSlash = outputPath + String("/");
				
			if(isLightmapsPacking)
			{
				packer.UseOnlySquareTextures();
				packer.SetMaxTextureSize(2048);
			}

			if (CommandLineParser::Instance()->IsFlagSet("--split"))
			{
				packer.PackToTexturesSeparate(excludeDirectory.c_str(), outputPathWithSlash.c_str(), definitionFileList);
			}
			else
			{
				packer.PackToTextures(excludeDirectory.c_str(), outputPathWithSlash.c_str(), definitionFileList);
			}
		}
	}	

	packTime = SystemTimer::Instance()->AbsoluteMS() - packTime;

	if (Core::Instance()->IsConsoleMode())
	{
		if (CommandLineParser::Instance()->IsExtendedOutput())
		{
			printf("[%d files packed with flags: %s]\n", (int)definitionFileList.size(), currentFlags.c_str());
		}
	
		String result = "[unchanged]";
		if (modified)
			result = "[REPACKED]";

		printf("[%s - %.2lf secs] - %s\n", inputPath.c_str(), (float64)packTime / 1000.0f, result.c_str());
	}

	
	for (std::list<DefinitionFile*>::iterator it = definitionFileList.begin(); it != definitionFileList.end(); ++it)
	{
		DefinitionFile * file = *it;
		SafeDelete(file);
	}
	definitionFileList.clear();
	
	for (int fi = 0; fi < fileList->GetCount(); ++fi)
	{
		if (fileList->IsDirectory(fi))
		{
			String filename = fileList->GetFilename(fi);
			if ((filename != ".") && (filename != "..") && (filename != "$process") && (filename != ".svn"))
			{
				if ((filename.size() > 0) && (filename[0] != '.'))
					RecursiveTreeWalk(inputPath + String("/") + fileList->GetFilename(fi),
									  outputPath + String("/") + fileList->GetFilename(fi));
			}
		}
	}
	
	SafeRelease(fileList);
}
