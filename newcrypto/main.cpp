#include "passcrypt.h"
#include <cryptopp/sha.h>
#include <cryptopp/files.h>
#include <cryptopp/gzip.h>
#include <cryptopp/panama.h>
#include <cryptopp/serpent.h>
#include <cryptopp/sosemanuk.h>
#include <cryptopp/salsa.h>
#include <cryptopp/blowfish.h>

int main(int argc,char*argv[])
{
    typedef CryptoPP::SHA HashType;
    //typedef CryptoPP::CBC_Mode<CryptoPP::Serpent> CryptType;  // WORKS
    //typedef CryptoPP::CBC_Mode<CryptoPP::Blowfish> CryptType; // WORKS
    typedef CryptoPP::PanamaCipher<> CryptType;               // CRASH: Passes keycheck for decryption, but then gzip detects corruption and throws
    //typedef CryptoPP::Sosemanuk CryptType;                    // WORKS
    //typedef CryptoPP::Salsa20 CryptType;                      // WORKS

    CryptoPP::FileSource( "PStart.exe", true,
        new CryptoPP::Gzip(
            new CryptoPP::PassphrasedEncrypt<HashType,CryptType>((byte*)"password",8,
                    new CryptoPP::FileSink(
                        "PStart.exe.crypt"
                    )
            ),
            CryptoPP::Gzip::MAX_DEFLATE_LEVEL
        )
    );

    CryptoPP::FileSource( "PStart.exe.crypt", true,
        new CryptoPP::PassphrasedDecrypt<HashType,CryptType>((byte*)"password",8,
            new CryptoPP::Gunzip(
                new CryptoPP::FileSink(
                        "PStart.decrypt.exe"
                )
            )
        )
    );
    return 0;
}


/// Crypto++ Multiple operations on the same file
/*
	std::ifstream ifs("/home/nacitar/test.xml",std::ios::binary);
	CryptoPP::FileSource(ifs,true,new CryptoPP::HashFilter(objSHA,new CryptoPP::FileSink("/home/nacitar/test.out")));
	ifs.clear();
	ifs.seekg(0,std::ios::beg);
	CryptoPP::FileSource(ifs,true,new CryptoPP::HashFilter(objSHA,new CryptoPP::FileSink("/home/nacitar/test.out2")));
	ifs.close();
*/

/*
		//CryptoPP::SHA256 objSHA;
		CryptoPP::SocketSource
		(
			socket.native(),true,
			//new CryptoPP::HashFilter(objSHA,
			new CryptoPP::Gzip(
				new CryptoPP::Base64Encoder(
					new CryptoPP::FileSink(
						std::cout//"/home/nacitar/test.out"
					)
				),
				CryptoPP::Gzip::MAX_DEFLATE_LEVEL
			)
			//)
		);
		*/
