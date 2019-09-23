#include <wx/wx.h>
#include <wx/progdlg.h>
#include <wx/filedlg.h>
#include <wx/textdlg.h>
#include "passcrypt.h"

#include <string>
#include <cctype>

#include <boost/filesystem/fstream.hpp>

#include <cryptopp/serpent.h>
#include <cryptopp/files.h>
#include <cryptopp/modes.h>
#include <cryptopp/sha.h>
#include <cryptopp/gzip.h>

IMPLEMENT_APP_NO_MAIN(wxApp)

int main(int argc,char*argv[])
{
    //std::cout << "Hello World!" << std::endl;
    wxEntryStart(argc,argv);
    {


        wxFileDialog objFileDlg(NULL,wxT("nxCrypt File Selection"));
        objFileDlg.Centre();
        objFileDlg.ShowModal();

        std::wstring strFName(objFileDlg.GetFilename().wc_str());
        std::wstring strInFile(std::wstring(objFileDlg.GetDirectory().wc_str()) + L"/" + strFName);
        bool bEncrypt=true;
        if (!strFName.empty())
        {
            if (strFName.size() > 4)
            {
                const wchar_t *ptExtension = strInFile.c_str()+strInFile.size()-4;
                if (    (               ptExtension[0]  == L'.') &&
                        (std::tolower(  ptExtension[1]) == L'n') &&
                        (std::tolower(  ptExtension[2]) == L'x') &&
                        (std::tolower(  ptExtension[3]) == L'z') )
                        bEncrypt=false;
            }
        }
        else
        {
            return 0;
        }
        std::wstring strOutFile;
        if (bEncrypt)
            strOutFile = strInFile+L".nxz";
        else
            strOutFile = strInFile.substr(0,strInFile.size()-4);

        boost::filesystem::ifstream ifs(strInFile,std::ios::binary);
        boost::filesystem::ofstream ofs(strOutFile,std::ios::binary|std::ios::out);
        if (!ifs)
        {
            wxMessageBox(wxT("Error opening the input file"));
            wxMessageBox(strInFile.c_str());
            return 1;
        }
        if (!ofs)
        {
            wxMessageBox(wxT("Error opening the output file"));
            wxMessageBox(strOutFile.c_str());
            return 1;
        }

        std::wstring strPassword;

        while (true)
        {
            {
                wxPasswordEntryDialog objPassword(NULL,wxT("Enter the password"),wxT("nxCrypt Password Entry"));
                objPassword.Centre();
                if (objPassword.ShowModal() != wxID_OK)
                    return 2;
                strPassword = objPassword.GetValue().wc_str();
            }

            {
                wxPasswordEntryDialog objPassword(NULL,wxT("Confirm the password"),wxT("nxCrypt Password Confirmation"));
                objPassword.Centre();
                if (objPassword.ShowModal() != wxID_OK)
                    continue;
                if (strPassword != objPassword.GetValue().wc_str())
                {
                    wxMessageBox(wxT("ERROR: Passwords didn't match"),wxT("nxCrypt - Error"));
                    continue;
                }
            }
            break;
        }

        typedef CryptoPP::SHA512 HashType;
        typedef CryptoPP::CBC_Mode<CryptoPP::Serpent> CryptType;
        try
        {
            CryptoPP::member_ptr<CryptoPP::FileSource> ptFileCrypt;
            boost::uintmax_t uFileBlock = boost::filesystem::file_size(strInFile)/100;
            if (!uFileBlock) ++uFileBlock;
            int uProgress=0;

            if (bEncrypt)
                ptFileCrypt.reset(new CryptoPP::FileSource( ifs, false,
                    new CryptoPP::Gzip(
                        new CryptoPP::PassphrasedEncrypt<HashType,CryptType>((byte*)strPassword.c_str(),strPassword.size()*sizeof(wchar_t),
                                new CryptoPP::FileSink(
                                    ofs
                                )
                        ),
                        CryptoPP::Gzip::MAX_DEFLATE_LEVEL
                    )
                ));
            else
                ptFileCrypt.reset(new CryptoPP::FileSource( ifs, false,
                    new CryptoPP::PassphrasedDecrypt<HashType,CryptType>((byte*)strPassword.c_str(),strPassword.size()*sizeof(wchar_t),
                        new CryptoPP::Gunzip(
                                new CryptoPP::FileSink(
                                    ofs
                                )
                        )
                    )
                ));
            //ptFileCrypt->PumpAll();
            wxProgressDialog objProgress((bEncrypt?wxT("nxCrypt - Encryption"):wxT("nxCrypt - Decryption")),wxT("Processing file..."),100,NULL,wxPD_AUTO_HIDE|wxPD_SMOOTH|wxPD_REMAINING_TIME|wxPD_ELAPSED_TIME);
            while (ptFileCrypt->Pump(uFileBlock))
            {
                if (uProgress < 99)
                    objProgress.Update(++uProgress);
                else
                {
                    ptFileCrypt->PumpAll();
                    objProgress.Update(100);
                    break;
                }
            }
            wxMessageBox(wxT("Operation successful!"));
        } catch (...)
        {
            wxMessageBox(wxT("Error performing operation."));
        }
    }
    wxEntryCleanup();
    return 0;
}
