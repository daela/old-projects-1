#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>

void StrTrim(std::string&str)
{
	std::string::size_type pos = str.find_first_not_of(" \t");
	if (pos != std::string::npos)
		str = str.substr(pos,str.find_last_not_of(" \t")-pos+1);
}
void StrNumTrim(std::string&str)
{
	std::string::size_type pos = str.find_first_not_of('0');
	if (pos == std::string::npos)
		str = "0";
	else
		str = str.substr(pos);
}
unsigned long StrCount(const std::string&str,const char&ch)
{
	unsigned long count=0;
	for (std::string::const_iterator it = str.begin();it!=str.end();++it)
	{
		if (*it == ch) ++count;
	}
	return count;
}
std::string::size_type StrFindNth(const std::string&str,const char&ch,unsigned long n)
{
	unsigned long i, len;
	if (n)
		for (i=0,len=str.size();i<len;++i)
		{
			if (str[i] == ch) --n;
			if (!n) return i;
		}
	return 0;

}
int main(int argc,char*argv[])
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind;
	std::string strBaseDir = "..\\..\\presort\\";
	std::string strOutputFile = "retag.bat";

	std::ofstream ofs(strOutputFile.c_str());
	//First, we're looking for folders named after artists
	hFind = FindFirstFile((strBaseDir+"*").c_str(),&ffd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				std::string strArtist = ffd.cFileName;
				std::string strAltArtist = "";
				if (strArtist == "." || strArtist == "..") continue;

				// Check for alternate band name
				std::string::size_type pos = strArtist.find_first_of('[');
				std::string::size_type pos2 = strArtist.find_first_of(']',pos);

				if (pos != std::string::npos && pos2 != std::string::npos)
				{
					//012345
					// [ab]
					strAltArtist = strArtist.substr(pos+1,pos2-pos-1);
					strArtist = strArtist.substr(0,pos);
					StrTrim(strArtist);
					StrTrim(strAltArtist);
				}
				/*std::cout << "ARTIST: " << strArtist << std::endl;
				if (!strAltArtist.empty())
					std::cout << "ALT ARTIST: " << strAltArtist << std::endl;
				*/

				/// Check for albums
				std::string strArtistDirectory = strBaseDir + ffd.cFileName + "\\";
				WIN32_FIND_DATA ffdAlbum;
				HANDLE hFindAlbum;
				hFindAlbum = FindFirstFile((strArtistDirectory+"*").c_str(),&ffdAlbum);
				if (hFindAlbum != INVALID_HANDLE_VALUE)
				{
					do
					{
						if (ffdAlbum.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
						{
							std::string strAlbum = ffdAlbum.cFileName;
							std::string strYear = "";
							if (strAlbum == "." || strAlbum == ".." || stricmp(strAlbum.c_str(),"misc") == 0) continue;
							std::string::size_type posYear = strAlbum.find_first_of(']');
							if (strAlbum[0] == '[' && posYear != std::string::npos)
							{
								strYear = strAlbum.substr(1,posYear-1);
								strAlbum = strAlbum.substr(posYear+1);
								StrTrim(strYear);
								StrNumTrim(strYear);
								StrTrim(strAlbum);
								//012345
								//[1996]
							}
							/*std::cout << "Album: ";
							if (!strYear.empty())
								std::cout << "[" << strYear << "] ";
							std::cout << strAlbum << std::endl;
							*/

							/// Check for songs
							std::string strAlbumDirectory = strArtistDirectory + ffdAlbum.cFileName + "\\";
							WIN32_FIND_DATA ffdSong;
							HANDLE hFindSong;
							hFindSong = FindFirstFile((strAlbumDirectory+"*.mp3").c_str(),&ffdSong);
							if (hFindSong != INVALID_HANDLE_VALUE)
							{
								do
								{
									if (ffdSong.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
									std::string strFile = ffdSong.cFileName;
									std::string strArtistName = "";
									if (strFile.size() >= strArtist.size() && strnicmp(strFile.c_str(),strArtist.c_str(),strArtist.size())==0)
									{
										strArtistName = strArtist;
									} else if (!strAltArtist.empty() && strFile.size() >= strAltArtist.size() && strnicmp(strFile.c_str(),strAltArtist.c_str(),strAltArtist.size())==0)
									{
										strArtistName = strAltArtist;
									}
									else
									{
										std::cout << "UNRECOGNIZED ARTIST: " << strFile << std::endl;
									}

									unsigned long uDashes = 2 + StrCount(strArtistName,'-') + StrCount(strAlbum,'-');
									std::string::size_type postrack = StrFindNth(strFile,'-',uDashes);
									std::string::size_type postitle = strFile.find_first_of('-',postrack+1);
									if (postitle == std::string::npos)
									{
										std::cout << "INVALID FILENAME: " << strFile << std::endl;
									}
									//012345
									//- 12 -
									++postrack;
									std::string strTrackNum = strFile.substr(postrack,postitle-postrack);
									std::string strTitle = strFile.substr(postitle+1);
									strTitle = strTitle.substr(0,strTitle.size()-4);
									StrTrim(strTrackNum);
									StrNumTrim(strTrackNum);
									StrTrim(strTitle);
									ofs << "id3.exe -d -1 -2 -3 \"" << (strAlbumDirectory + strFile) << "\"" << std::endl;
									ofs << "id3.exe -d -1 -2 -3" << " -a \"" << strArtistName << "\" -y \"" << strYear << "\" -l \"" << strAlbum << "\" -n \"" << strTrackNum << "\" -t \"" << strTitle << "\" \"" << (strAlbumDirectory + strFile) << "\"" << std::endl;
//									ofs << (strAlbumDirectory + strFile) << " ; " << strArtistName << " ; " << strYear << " ; " << strAlbum << " ; " << strTrackNum << " ; " << strTitle << std::endl;
									//check if it's alt artist, normal artist, or an error

									//count dashes in artist name and album name, and skip to the dash starting the title
									//substr after that, trim, that's the song.  we already know album and year, and we determined artist
									//so we're done.
								} while (FindNextFile(hFindSong,&ffdSong));
							}
						}
					} while (FindNextFile(hFindAlbum,&ffdAlbum));
				}

			}
		} while (FindNextFile(hFind,&ffd));
	}
	ofs.close();
	return 0;
}
