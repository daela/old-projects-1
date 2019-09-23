#ifndef PASSCRYPT_H_INCLUDED
#define PASSCRYPT_H_INCLUDED
#include <cryptopp/rsa.h> // required or pwdbased errors
#include <cryptopp/pwdbased.h>
#include <cryptopp/filters.h>
#include <ctime>


NAMESPACE_BEGIN(CryptoPP)

template <class HashType,class CryptType>
class PassphrasedBase
{
    protected:
        typedef PKCS5_PBKDF2_HMAC<HashType> PBKDF;

        CryptType m_cipher;
        SecByteBlock m_passphrase;

        const size_t DIGESTSIZE;
        const size_t KEYSIZE;
        const size_t IVSIZE;
        const size_t BLOCKSIZE;
        CRYPTOPP_CONSTANT(KEY_ITERATIONS = 2000 );

        PassphrasedBase(const byte *passphrase, size_t passphraseLength)
        : m_passphrase(passphrase, passphraseLength)
        , DIGESTSIZE(HashType().DigestSize())
        , KEYSIZE(m_cipher.DefaultKeyLength())
        , IVSIZE(m_cipher.IVSize())
        , BLOCKSIZE(m_cipher.MandatoryBlockSize())
        {
          /*  std::cout << "BLOCKSIZE: " << BLOCKSIZE << std::endl;
            std::cout << "KEYSIZE: " << KEYSIZE << std::endl;
            std::cout << "DIGESTSIZE: " << DIGESTSIZE << std::endl;
            std::cout << "ALIGNEDDIGEST: " << BlockAlign(DIGESTSIZE) << std::endl;*/
        }

        PassphrasedBase(const char *passphrase)
        : m_passphrase(passphrase, strlen(passphrase))
        , DIGESTSIZE(HashType().DigestSize())
        , KEYSIZE(m_cipher.DefaultKeyLength())
        , IVSIZE(m_cipher.IVSize())
        , BLOCKSIZE(m_cipher.MandatoryBlockSize())
        {
        }

        size_t BlockAlign(const size_t&uSize) const
        {
            return (BLOCKSIZE*((uSize+BLOCKSIZE-1)/BLOCKSIZE));
        }
        void AlignedPut(SecByteBlock&uBlock,BufferedTransformation*transformation)
        {
            transformation->Put(uBlock,uBlock.size());
            for (size_t count=(BlockAlign(uBlock.size())-uBlock.size());count;--count)
                transformation->Put('\0');
        }
        void AlignedGet(SecByteBlock&uBlock,BufferedTransformation*transformation)
        {
            transformation->Get(uBlock,uBlock.size());
            transformation->Skip(BlockAlign(uBlock.size())-uBlock.size());
        }

};

template <class HashType,class CryptType>
class PassphrasedEncrypt : public PassphrasedBase<HashType,typename CryptType::Encryption>, public ProxyFilter
{
    private:
        typedef PassphrasedBase<HashType,typename CryptType::Encryption> Base;
        typedef typename Base::PBKDF PBKDF;
        using Base::m_cipher;
        using Base::m_passphrase;
        using Base::DIGESTSIZE;
        using Base::KEYSIZE;
        using Base::IVSIZE;
        using Base::KEY_ITERATIONS;
        using Base::AlignedPut;
        using Base::AlignedGet;
        using Base::BlockAlign;

    public:
        PassphrasedEncrypt(const byte *passphrase, size_t passphraseLength, BufferedTransformation *attachment = NULL)
        : Base(passphrase,passphraseLength)
        , ProxyFilter(NULL,0,0,attachment)
        {
            /*std::string strAlgo=ptCipher->AlgorithmName();
            if (strAlgo.size()>=6 &*/
        }

        void FirstPut(const byte *inString)
        {
            HashType hash;
            SecByteBlock salt(DIGESTSIZE), keyCheck(DIGESTSIZE);

            // use hash(passphrase | time | clock) as salt
            hash.Update(m_passphrase, m_passphrase.size());
            std::time_t t=std::time(0);
            hash.Update((byte *)&t, sizeof(t));
            std::clock_t c=std::clock();
            hash.Update((byte *)&c, sizeof(c));
            hash.Final(salt);

            // use hash(passphrase | salt) as key check
            hash.Update(m_passphrase, m_passphrase.size());
            hash.Update(salt, salt.size());
            hash.Final(keyCheck);

            // put the salt into the output stream, unencrypted
            AlignedPut(salt,AttachedTransformation());

            // Buffer with room for the key and the initial value
            SecByteBlock keyIV(KEYSIZE+IVSIZE);
            // Derive the key
            PBKDF().DeriveKey(keyIV,keyIV.size(),0,m_passphrase,m_passphrase.size(),salt,salt.size(),KEY_ITERATIONS);

            // Prepare the cipher for use given our key and initial value
            m_cipher.SetKeyWithIV(keyIV,KEYSIZE,keyIV+KEYSIZE);

            // Set the cipher as the filter
            SetFilter(new StreamTransformationFilter(m_cipher));
            // Output the keyCheck value before all others
            AlignedPut(keyCheck,m_filter.get());
        }
        void LastPut(const byte *inString, size_t length)
        {
            m_filter->MessageEnd();
        }
};




template <class HashType,class CryptType>
class PassphrasedDecrypt : public PassphrasedBase<HashType,typename CryptType::Decryption>,  public ProxyFilter
{
    private:
        typedef PassphrasedBase<HashType,typename CryptType::Decryption> Base;
        typedef typename Base::PBKDF PBKDF;
        using Base::m_cipher;
        using Base::m_passphrase;
        using Base::DIGESTSIZE;
        using Base::KEYSIZE;
        using Base::IVSIZE;
        using Base::KEY_ITERATIONS;
        using Base::AlignedPut;
        using Base::AlignedGet;
        using Base::BlockAlign;
        bool m_throwException;
    public:
        class Err : public Exception
        {
            public:
            Err(const std::string &s)
            : Exception(DATA_INTEGRITY_CHECK_FAILED, s) {}
        };
        class KeyBadErr : public Err {public: KeyBadErr() : Err("PassphrasedDecryptor: cannot decrypt message with this passphrase") {}};
        enum State {WAITING_FOR_KEYCHECK, KEY_GOOD, KEY_BAD};
    protected:
        State m_state;
    public:
        State CurrentState() const {return m_state;}

        PassphrasedDecrypt(const byte *passphrase, size_t passphraseLength, BufferedTransformation *attachment = NULL, bool throwException=true)
        : Base(passphrase,passphraseLength)
        , ProxyFilter(NULL,2*BlockAlign(DIGESTSIZE),0,attachment)
        , m_throwException(throwException)
        , m_state(WAITING_FOR_KEYCHECK)
        {
        }
        void FirstPut(const byte *inString)
        {
            CheckKey(inString, inString+BlockAlign(DIGESTSIZE));
        }
        void LastPut(const byte *inString, size_t length)
        {
            if (m_filter.get() == NULL)
            {
                m_state = KEY_BAD;
                if (m_throwException)
                    throw KeyBadErr();
            }
            else
            {
                m_filter->MessageEnd();
                m_state = WAITING_FOR_KEYCHECK;
            }
        }

        void CheckKey(const byte *salt, const byte *encCheck)
        {
            HashType hash;
            SecByteBlock calcCheck(DIGESTSIZE), readCheck(DIGESTSIZE);
            // Calculate the expected keyCheck value using the salt from the input stream

            hash.Update(m_passphrase, m_passphrase.size());
            hash.Update(salt, DIGESTSIZE);
            hash.Final(calcCheck);

            // Prepare the cipher using the key derived from the input password and the salt

            // Buffer with room for the key and the initial value
            SecByteBlock keyIV(KEYSIZE+IVSIZE);
            PBKDF().DeriveKey(keyIV,keyIV.size(),0,m_passphrase,m_passphrase.size(),salt,DIGESTSIZE,KEY_ITERATIONS);
            m_cipher.SetKeyWithIV(keyIV,KEYSIZE,keyIV+KEYSIZE);

            // Make an instance of the decryption cipher
            std::auto_ptr<StreamTransformationFilter> decryptor(new StreamTransformationFilter(m_cipher));

            // Put the keyCheck read in from the file into the decryption cipher
            decryptor->Put(encCheck, BlockAlign(DIGESTSIZE));
            // Force it to be processed now.
            decryptor->ForceNextPut();
            // Read back the decrypted keyCheck value.
            AlignedGet(readCheck,decryptor.get());

            // Set the decryption cipher as the filter for this object.
            SetFilter(decryptor.release());

            // Compare the calculated keyCheck with the one decrypted from the file
            if (memcmp(calcCheck, readCheck, calcCheck.size()))
            {
                m_state = KEY_BAD;
                if (m_throwException)
                    throw KeyBadErr();
            }
            else
                m_state = KEY_GOOD;

        }

};

NAMESPACE_END

#endif // PASSCRYPT_H_INCLUDED
