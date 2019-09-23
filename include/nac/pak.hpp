#ifndef __PAK_HPP__
#define __PAK_HPP__

#include <nx/type.hpp>
#include <string>
#include <vector>
#include <fstream>

/*
	Every PAK file has a header, 16 bytes long
		4 	Ident (IDPAKHEADER)
		4	directory offset
		4	directory length

	All files in the PAK has a directory entry, consisting of
		56	relative filename
		4	file data position
		4	file data length

	This data exists back-to-back in the file, the position could be anywhere,
	it is stored in the directory offset field of the pak file header.
	So, the directory can be anywhere, but it is one contiguous list wherever it is.
	The number of entries can be obtained from the header's directory length divided by
	64, as each entry is 64 bytes long.
*/

// .PAK File Header
struct nxPakHeader
{
	nxMem pSignature[4];
	nxMemInt32 pOffset;
	nxMemInt32 pLength;
};
// .PAK Directory Header
struct nxPakDirectory
{
	nxMem pName[56];
	nxMemInt32 pFilePos;
	nxMemInt32 pFileLen;
};

enum nxArchiveType { NX_UNKNOWN=0, NX_PAK };

class nxArchiveData
{
	protected:
		std::ifstream*ifsIn;
		nxULong uOffset;
		nxULong uSize;
		nxArchiveType aType;
	public:
		nxArchiveData(std::ifstream*ifs=NULL,const nxULong& offset=0,const nxULong& size=0, const nxArchiveType& type=NX_UNKNOWN)
		: ifsIn(ifs), uOffset(offset), uSize(size), aType(type)
		{ }
		bool GetData(std::vector<char>&vcData) const
		{
			if (ifsIn && *ifsIn && ifsIn->seekg(uOffset,std::ios::beg))
			{
				vcData.resize(uSize);
				if (ifsIn->read(&vcData.front(),uSize))
					return true;
			}
			return false;
		}
		inline const nxULong& GetSize(void) const
		{ return uSize; }
		inline const nxArchiveType&GetType(void) const
		{ return aType; }
};

typedef std::map<std::string,nxArchiveData> nxArchiveMap;

class nxPak
{
	std::ifstream ifsIn;
	protected:
		nxArchiveMap mpFiles;
		inline void Clear(void);
	public:
		bool Open(const std::string&strFile);
		inline const nxArchiveMap& GetFileMap(void) const
		{
			return mpFiles;
		}

};
inline void nxPak::Clear(void)
{
	ifsIn.close();
	ifsIn.clear();
	mpFiles.clear();
}
bool nxPak::Open(const std::string&strFile)
{
	Clear();
	ifsIn.open(strFile.c_str(),std::ios::binary);
	if (!ifsIn) return false;
	nxPakHeader objHead;
	ifsIn.read((char*)&objHead,sizeof(nxPakHeader));
	//check signature and that the offset is valid
	if (ifsIn && //read OK
		nxMemCmp((const nxMem*)objHead.pSignature,(const nxMem*)"PACK",4) == 0 && //right signature
		ifsIn.seekg(nxLMemToNum32<nxUInt32>(objHead.pOffset),std::ios::beg)) //valid offset
	{
		nxUInt32 uFiles = nxLMemToNum32<nxUInt32>(objHead.pLength) / sizeof(nxPakDirectory);
		nxPakDirectory objDir;
		while (uFiles--)
		{
			ifsIn.read((char*)&objDir,sizeof(nxPakDirectory));
			if (!ifsIn)
			{
				Clear();
				return false;
			}
			mpFiles[(const char*)objDir.pName] = nxArchiveData(&ifsIn,nxLMemToNum32<nxULong>(objDir.pFilePos),nxLMemToNum32<nxULong>(objDir.pFileLen), NX_PAK);
		}
		return true;
	}
	Clear();
	return false;
}

#endif // __PAK_HPP__
