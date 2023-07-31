#include <iostream>
#include <string>
#include <memory>
#include <limits>
#include <stdexcept>

#include <openssl/evp.h>
#include <openssl/rand.h>

static const unsigned int KEY_SIZE = 16;
static const unsigned int BLOCK_SIZE = 16;

template <typename T>
struct zallocator
{
public:
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    pointer address (reference v) const {return &v;}
    const_pointer address (const_reference v) const {return &v;}

    pointer allocate (size_type n, const void* hint = 0) {
        if (n > std::numeric_limits<size_type>::max() / sizeof(T))
            throw std::bad_alloc();
        return static_cast<pointer> (::operator new (n * sizeof (value_type)));
    }

    void deallocate(pointer p, size_type n) {
        OPENSSL_cleanse(p, n*sizeof(T));
        ::operator delete(p); 
    }
    
    size_type max_size() const {
        return std::numeric_limits<size_type>::max() / sizeof (T);
    }
    
    template<typename U>
    struct rebind
    {
        typedef zallocator<U> other;
    };

    void construct (pointer ptr, const T& val) {
        new (static_cast<T*>(ptr) ) T (val);
    }

    void destroy(pointer ptr) {
        static_cast<T*>(ptr)->~T();
    }

#if __cpluplus >= 201103L
    template<typename U, typename... Args>
    void construct (U* ptr, Args&&  ... args) {
        ::new (static_cast<void*> (ptr) ) U (std::forward<Args> (args)...);
    }

    template<typename U>
    void destroy(U* ptr) {
        ptr->~U();
    }
#endif
};

typedef unsigned char byte;
typedef std::basic_string<char, std::char_traits<char>, zallocator<char> > secure_string;
using EVP_CIPHER_CTX_free_ptr = std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)>;

void aes_encrypt(const byte key[KEY_SIZE], const secure_string& ptext, secure_string& ctext);
void aes_decrypt(const byte key[KEY_SIZE], const secure_string& ctext, secure_string& rtext);
void b64_encode(const secure_string& ptext, secure_string& ctext);
void b64_decode(const secure_string& etext, secure_string& dtext);

int main(int argc, char* argv[])
{
    // Load the necessary cipher
    EVP_add_cipher(EVP_aes_128_ecb());

    // encoded ciphertext
    secure_string ectext(std::istreambuf_iterator<char>(std::cin), {});

    // ciphertext, recovered text
    secure_string ctext, rtext;
    
    b64_decode(ectext, ctext);

    byte key[KEY_SIZE] = {
      'Y', 'E', 'L', 'L', 'O', 'W', ' ', 'S',
      'U', 'B', 'M', 'A', 'R', 'I', 'N', 'E'};

    aes_decrypt(key, ctext, rtext);
    
    OPENSSL_cleanse(key, KEY_SIZE);

    std::cout << "Recovered message:\n" << rtext << std::endl;

    return 0;
}

void b64_encode(const secure_string& ptext, secure_string& etext)
{
  EVP_ENCODE_CTX *ctx = EVP_ENCODE_CTX_new();
  EVP_EncodeInit(ctx);

  int rc;
  etext.resize(ptext.size()+ BLOCK_SIZE);
  int out_len1 = (int) etext.size();
  rc = EVP_EncodeUpdate(ctx, (byte *) &etext[0], &out_len1,
			(const byte *) &ptext[0], (int) ptext.size());
  if (rc != 1)
    throw std::runtime_error("EVP_EncodeUpdate failed");

  int out_len2 = (int) etext.size() - out_len1;
  EVP_EncodeFinal(ctx, (byte *) &etext[0] + out_len1, &out_len2);

  etext.resize(out_len1 + out_len2);
}

void b64_decode(const secure_string& etext, secure_string& dtext)
{
  EVP_ENCODE_CTX *ctx = EVP_ENCODE_CTX_new();
  EVP_DecodeInit(ctx);

  int rc;
  dtext.resize(etext.size() + BLOCK_SIZE);
  int out_len1 = (int) dtext.size();
  rc = EVP_DecodeUpdate(ctx, (byte *) &dtext[0], &out_len1,
			(const byte *) &etext[0], (int) etext.size());
  if (rc == -1)
    throw std::runtime_error("EVP_DecodeUpdate failed");

  int out_len2 = (int) dtext.size() - out_len1;
  EVP_DecodeFinal(ctx, (byte *) &dtext[0] + out_len1, &out_len2);

  dtext.resize(out_len1 + out_len2);
}

void aes_encrypt(const byte key[KEY_SIZE], const secure_string& ptext, secure_string& ctext)
{
    EVP_CIPHER_CTX_free_ptr ctx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free);
    int rc = EVP_EncryptInit_ex(ctx.get(), EVP_aes_128_ecb(), NULL, key, NULL);
    if (rc != 1)
      throw std::runtime_error("EVP_EncryptInit_ex failed");

    // Recovered text expands upto BLOCK_SIZE
    ctext.resize(ptext.size()+BLOCK_SIZE);
    int out_len1 = (int)ctext.size();

    rc = EVP_EncryptUpdate(ctx.get(), (byte*)&ctext[0], &out_len1, (const byte*)&ptext[0], (int)ptext.size());
    if (rc != 1)
      throw std::runtime_error("EVP_EncryptUpdate failed");
  
    int out_len2 = (int)ctext.size() - out_len1;
    rc = EVP_EncryptFinal_ex(ctx.get(), (byte*)&ctext[0]+out_len1, &out_len2);
    if (rc != 1)
      throw std::runtime_error("EVP_EncryptFinal_ex failed");

    // Set cipher text size now that we know it
    ctext.resize(out_len1 + out_len2);
}

void aes_decrypt(const byte key[KEY_SIZE], const secure_string& ctext, secure_string& rtext)
{
    EVP_CIPHER_CTX_free_ptr ctx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free);
    int rc = EVP_DecryptInit_ex(ctx.get(), EVP_aes_128_ecb(), NULL, key, NULL);
    if (rc != 1)
      throw std::runtime_error("EVP_DecryptInit_ex failed");

    // Recovered text contracts upto BLOCK_SIZE
    rtext.resize(ctext.size());
    int out_len1 = (int)rtext.size();

    rc = EVP_DecryptUpdate(ctx.get(), (byte*)&rtext[0], &out_len1, (const byte*)&ctext[0], (int)ctext.size());
    if (rc != 1)
      throw std::runtime_error("EVP_DecryptUpdate failed");
  
    int out_len2 = (int)rtext.size() - out_len1;
    rc = EVP_DecryptFinal_ex(ctx.get(), (byte*)&rtext[0]+out_len1, &out_len2);
    if (rc != 1)
      throw std::runtime_error("EVP_DecryptFinal_ex failed");

    // Set recovered text size now that we know it
    rtext.resize(out_len1 + out_len2);
}

