#ifndef CRYPTO_H
#define CRYPTO_H

#include <cstdlib>
#include <cstdio>
#include <ctime>

#include <string>
#include <vector>
#include <sstream>
#include <exception>

#include <botan/botan.h>
#include <botan/pubkey.h>
#include <botan/rsa.h>


namespace xblab {


class crypto_exception : public std::exception {
public:
  crypto_exception(){
    message_ = std::string("xblab crypto exception");
  }
  crypto_exception(std::string err_msg){
    message_ = err_msg;
  }
  ~crypto_exception() throw(){};
  virtual const char* what() const throw(){
    return this->message_.c_str();
  }
private:
  std::string message_;
};


class Crypto {

public:
  
  static void
  generateKey(std::string& pr, std::string& pu);

  static std::string
  generateNonce();

  static std::string
  sign(Botan::AutoSeeded_RNG&, Botan::RSA_PrivateKey*&, std::string&);

  static std::string
  sign(std::string& privateKey, std::string& message);

  static bool
  verify(std::string message, std::string signature);

  static bool
  verify(std::string publicKey, std::string message, std::string signature);

  static bool
  verify(Botan::RSA_PublicKey* rsakey,
    std::string message, std::string signature);

  static void
  hybridEncrypt(std::stringstream& in, std::stringstream& out);

  static void
  hybridEncrypt(
    std::string& pu, std::stringstream& in, std::stringstream& out);

  static std::string
  hybridEncrypt(std::string& publicKey, std::string& plaintext);

  static std::string
  hybridDecrypt(std::string& privateKey, std::string& ciphertext);

  static std::string
  hashPassword(std::string& pw);

  static bool
  checkPasshash(std::string pw, std::string ph);

  static int
  init();


  /* Templates */

  template <class T>
  static T
  simpleRandom(T n) {
    int rnd, limit = RAND_MAX - RAND_MAX % n;
    
    do {
      rnd = random();
    } while (rnd >= limit);
    return rnd % n;
  }


  template <class T>
  static T
  generateRandomInt(){
    unsigned char buf[sizeof(T)];
    Botan::AutoSeeded_RNG rng;
    rng.randomize(buf, sizeof(T));
    T random = T(*((T *)buf));
    return random;
  }


  template <class T>
  static std::vector<T>
  generateRandomInts(size_t n){
    std::vector<T> retval(n, 0);
    Botan::AutoSeeded_RNG rng;
    rng.randomize((unsigned char *)&retval[0], n * sizeof(T));
    return retval;
  }


  template <class T>
  static void
  initShuffle(T *array, size_t n) {
    T i, r;

    array[0] = 0;

    for (i = 1; i < n; i++) {
      r = simpleRandom(i);
      /* swap possibly unitialized value to avoid duplicates
       * (likelihood of array[r] being unitialized will decrease
       * as i (original number source) increases)
       */
      array[i] = array[r]; 
      array[r] = i;
    }
  }


  template <class T>
  static void
  shuffle(T *array, size_t n) {
    T i, r, t;

    for (i = n - 1; i > 0; i--) {
      r = simpleRandom(i + 1);
      t = array[r];
      array[r] = array[i];
      array[i] = t;
    }
  }


  template <class T>
  static void
  fillDisjointVectors(
    std::vector< std::vector<T>* >& vecs, size_t len) {

    srandom(time(NULL));

    size_t i, j, n = vecs.size();
    printf("vecs.size = %lu\n", n);

    T *arr = (T*)malloc(sizeof(T) * n);
    initShuffle<T>(arr, n);

    for (i = 0; i < len; i++) {
      for (j = 0; j < n; j++) {
        (*vecs.at(j))[i] = arr[j];
      }
      shuffle(arr, n);
    }
    free(arr);
  }


  #ifndef XBLAB_CLIENT

  // TODO: are these necessary?
  static std::string
  sign(std::string& message);

  static std::string
  hybridDecrypt(std::string& ciphertext);

  #endif

private:
  Crypto(){};
  ~Crypto(){};

  // Convenience wrappers for Botan Pipe functions
  static std::string
  b64Encode(const Botan::SecureVector<unsigned char>&);

  static Botan::SecureVector<unsigned char>
  b64Decode(const std::string& in);

  static Botan::SymmetricKey
  deriveSymmetricKey(const std::string&,
    const Botan::SymmetricKey&, unsigned int);

  static void
  hybridEncrypt(Botan::RSA_PublicKey* rsakey,
    std::stringstream& in, std::stringstream& out);

  static void
  hybridDecrypt(Botan::AutoSeeded_RNG&,
    Botan::RSA_PrivateKey*&, std::stringstream&, std::stringstream&);
};

} //namespace xblab

#endif