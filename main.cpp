#include "App.h"
#include "DB.h"
#include "Models.h"
#include "Websocks.h"
#include "Encrypter.h"
int openssl_strerror_r(int errnum, char *buf, size_t buflen)
{
#if defined(_MSC_VER) && _MSC_VER >= 1400 && !defined(_WIN32_WCE)
    return !strerror_s(buf, buflen, errnum);
#elif defined(_GNU_SOURCE)
    char *err;

    /*
     * GNU strerror_r may not actually set buf.
     * It can return a pointer to some (immutable) static string in which case
     * buf is left unused.
     */
    err = strerror_r(errnum, buf, buflen);
    if (err == NULL || buflen == 0)
        return 0;
    /*
     * If err is statically allocated, err != buf and we need to copy the data.
     * If err points somewhere inside buf, OPENSSL_strlcpy can handle this,
     * since src and dest are not annotated with __restrict and the function
     * reads src byte for byte and writes to dest.
     * If err == buf we do not have to copy anything.
     */
    if (err != buf)
        OPENSSL_strlcpy(buf, err, buflen);
    return 1;
#elif (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200112L) || \
    (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 600)
    /*
     * We can use "real" strerror_r. The OpenSSL version differs in that it
     * gives 1 on success and 0 on failure for consistency with other OpenSSL
     * functions. Real strerror_r does it the other way around
     */
    return !strerror_r(errnum, buf, buflen);
#else
    char *err;

    /* Fall back to non-thread safe strerror()...its all we can do */
    if (buflen < 2)
        return 0;
    err = strerror(errnum);
    /* Can this ever happen? */
    if (err == NULL)
        return 0;
    OPENSSL_strlcpy(buf, err, buflen);
    return 1;
#endif
}
int main(int, char **)
{
    // std::string word = "u3Owj0kjGn5ESEXiEelX4gAFI5rbflwG2ACdIMFnnZHzroqcex5L1PvJajN93IrJXzN_q9DPN1pmkjfth0SRsw";
    // auto dec = Encrypter::Decrypt(word);
    // auto enc=Encrypter::Encrypt("A good and God fearing person will go to heaven");
    RunSocks();

    initializeDB();

    // Runnable run;
    // bool deleted = deleteRunnable(1);
    // auto runnable = Runnable{
    //     10, generate_uuid_v4(), false, true, "executor2.exe", "https://app.com/executor.exe", Status::UNDEFINEDSTATUS, 0, 0};
    // auto res = UpdateRunnable(runnable);
    // std::map<std::string, DBValue> queryVals;
    // DBValue dbVal;
    // dbVal.equality = DBEquality::GREATERTHAN;
    // dbVal.intValue = std::make_shared<int>(3);
    // queryVals["id"] = dbVal;
    // auto runnables = findRunnables(queryVals,200,"id");
    // // uploadFile(std::string(R"(C:\Users\James\Downloads\AvaloniaVS.VS2022.zip)"));
    // WaitForConnnection();
    // Start();

    return 0;
}