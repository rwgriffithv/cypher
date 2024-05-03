/**
 * @file utf8.h
 * @author Rob Griffith
 */

#include "buffer.h"

/**
 * @todo as UTF-8 is needed
 * @brief decode UTF-8 character octet sequence into character number
 *
 * @param seq UTF-8 octet character sequence to decode
 * @param[out] cnum UTF-8 equivalent character number
 * @return number of bytes in UTF-8 octet sequence used, 0 if error
 */
size_t utf8_decode_char(const char *seq, size_t *cnum);

/**
 * @todo as UTF-8 is needed
 * @brief encode UTF-8 character number into octet sequence
 *
 * Provided sequence must have at least 4 bytes of memory available.
 *
 * @param cnum UTF-8 character number
 * @param[out] seq UTF-8 encoded octet sequence
 * @return number of bytes written to encoded ocatet sequence
 */
size_t utf8_encode_char(size_t cnum, char *seq);

/**
 * @todo as UTF-8 is needed (limited language support)
 * @brief decompose UTF-8 entire octet sequence by canonical equivalence
 *
 * @param seq UTF-8 octet sequence to decompose canonically
 * @param[out] nfd NFD UTF-8 octet sequence buffer (pushed to)
 * @return decomposed NFD UTF-8 octet sequence, NULL if error
 */
const char *utf8_decomp(const char *seq, buffer_t *nfd);

/**
 * @todo as UTF-8 is needed (limited language support)
 * @brief compose UTF-8 entire octet sequence by canonical equivalence
 *
 * Use @ref utf8_decomp to get input NFD UTF-8 octet sequence.
 *
 * @param nfd NFD UTF-8 octet sequence to compose canonically
 * @param[out] nfc NFC UTF-8 octet sequence buffer (pushed to)
 * @return composed NFC UTF-8 octet sequence, NULL if error
 */
const char *utf8_comp(const char *nfd, buffer_t *nfc);

/**
 * @todo as searching with UTF-8 is needed (limited language support)
 * @brief decompose UTF-8 entire octet sequence by compatibility
 *
 * @param seq UTF-8 octet sequence to decompose by compatibility
 * @param[out] nfkd NFKD UTF-8 octet sequence buffer (pushed to)
 * @return decomposed NFKD UTF-8 octet sequence, NULL if error
 */
const char *utf8_kdecomp(const char *seq, buffer_t *nfkd);

/**
 * @todo as searching with UTF-8 is needed (limited language support)
 * @brief compose UTF-8 entire octet sequence by compatibility
 *
 * Use @ref utf8_kdecomp to get input NFKD UTF-8 octet sequence.
 *
 * @param nfkd NFKD UTF-8 octet sequence to compose by compatibility
 * @param[out] nfkc NFKC UTF-8 octet sequence buffer (pushed to)
 * @return composed NFKC UTF-8 octet sequence, NULL if error
 */
const char *utf8_kcomp(const char *nfkd, buffer_t *nfkc);