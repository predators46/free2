#pragma once
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */
#ifdef WITH_TLS
/**
 * $Id$
 *
 * @file lib/tls/conf.h
 * @brief Structures for session-resumption management.
 *
 * @copyright 2021 Arran Cudbard-Bell (a.cudbardb@freeradius.org)
 */
RCSIDH(conf_h, "$Id$")

#include <openssl/ssl.h>
#include <openssl/err.h>

#ifdef __cplusplus
extern "C" {
#endif

#if 1
/** OCSP Configuration
 *
 */
typedef struct {
	bool		enable;				//!< Enable OCSP checks
	char const	*cache_server;			//!< Virtual server to restore retrieved OCSP status.
	bool		override_url;			//!< Always use the configured OCSP URL even if the
							//!< certificate contains one.
	char const	*url;
	bool		use_nonce;
	X509_STORE	*store;
	uint32_t	timeout;
	bool		softfail;


	fr_tls_cache_t	cache;				//!< Cached cache section pointers.  Means we don't have
							///< to look them up at runtime.
} fr_tls_ocsp_conf_t;
#endif

/** Different chain building modes
 *
 */
typedef enum {
	FR_TLS_CHAIN_VERIFY_INVALID = 0,

	FR_TLS_CHAIN_VERIFY_HARD,			//!< Fail if we can't build a complete chain from
							///< the leaf cert back to a root.
	FR_TLS_CHAIN_VERIFY_SOFT,			//!< Warn if we can't build a complete chain from
							///< the leaf cert back to a root.
	FR_TLS_CHAIN_VERIFY_NONE			//!< Don't verify/build the chain.
} fr_tls_chain_verify_mode_t;

/** Structure representing a certificate chain configuration
 *
 */
typedef struct {
	int		file_format;			//!< Whether the file is expected to be PEM encoded.
							///< This allows us to load multiple chained PEM certificates
							///< from a single file.

	char const	*certificate_file;		//!< Path to certificate.

	char const	*password;			//!< Password to decrypt the certificate(s).
	char const	*private_key_file;		//!< Path to certificate.

	char const	**ca_files;			//!< Extra certificates to load.
	fr_tls_chain_verify_mode_t	verify_mode;	//!< How hard we try to build up a complete certificate
							///< chain.
	bool		include_root_ca;		//!< Include the root ca in the chain we built.
} fr_tls_chain_conf_t;

/** Control what types of session resumption we allow
 *
 */
typedef enum {
	FR_TLS_CACHE_DISABLED	= 0x00,			//!< Disabled
	FR_TLS_CACHE_STATEFUL	= 0x01,			//!< Session resumption information is stored on the server
							///< and an identifier is provided to the client.
	FR_TLS_CACHE_STATELESS	= 0x02,			//!< A session ticket is provided to the client.
	FR_TLS_CACHE_AUTO	= FR_TLS_CACHE_STATEFUL |	//!< We pick the correct cache type
								///< based on the TLS version and current
				  FR_TLS_CACHE_STATELESS	///< configuration.
} fr_tls_cache_mode_t;

/** Cache configuration
 *
 */
typedef struct {
	fr_tls_cache_mode_t mode;			//!< What type of session resumption we're going to use.

	tmpl_t		*id_name;			//!< Context ID to allow multiple sessions stores to be defined.
	char		context_id[SSL_MAX_SSL_SESSION_ID_LENGTH];

	uint32_t	lifetime;			//!< The maximum period a session can be resumed after.

	bool		verify;				//!< Revalidate any sessions read in from the cache.

	bool		require_extms;			//!< Only allow session resumption if the client/server
							//!< supports the extended master session key.  This protects
							//!< against the triple handshake attack.

	bool		require_pfs;			//!< Only allow session resumption if a cipher suite that
							//!< supports perfect forward secrecy.
} fr_tls_cache_conf_t;

/* configured values goes right here */
struct fr_tls_conf_s {
	CONF_SECTION	*virtual_server;		//!< The virtual server containing certificate validation
							///< policies, and cache control sections.

	CONF_SECTION	*cs;				//!< configuration section this tls config is based on.

	fr_tls_chain_conf_t	**chains;		//!< One or more certificates

	char const	*random_file;			//!< If set, we read 10K of data (or the complete file)
							//!< and use it to seed OpenSSL's PRNG.
	char const	*ca_path;
	char const	*ca_file;

	char const	*dh_file;			//!< File to load DH Parameters from.

	uint32_t	verify_depth;			//!< Maximum number of certificates we can traverse
							//!< when attempting to reach the presented certificate
							//!< from our Root CA.
	bool		auto_chain;			//!< Allow OpenSSL to build certificate chains
							//!< from all certificates it has available.
							//!< If false, the complete chain must be provided in
							//!< certificate file.
	bool		disable_single_dh_use;

	float		tls_max_version;		//!< Maximum TLS version allowed.
	float		tls_min_version;		//!< Minimum TLS version allowed.

	uint32_t	fragment_size;			//!< Maximum record fragment, or record size.
	uint32_t	padding_block_size;		//!< for TLS 1.3, pad blocks to multiple of this size.

	bool		check_crl;			//!< Check certificate revocation lists.
	bool		allow_expired_crl;		//!< Don't error out if CRL is expired.
	char const	*check_cert_cn;			//!< Verify cert CN matches the expansion of this string.

	char const	*cipher_list;			//!< Acceptable ciphers.
	bool		cipher_server_preference;	//!< use server preferences for cipher selection
#ifdef SSL3_FLAGS_NO_RENEGOTIATE_CIPHERS
	bool		allow_renegotiation;		//!< Whether or not to allow cipher renegotiation.
#endif
	char const	*check_cert_issuer;		//!< Verify cert issuer matches the expansion of this string.

	char const	*verify_tmp_dir;
	char const	*verify_client_cert_cmd;
	bool		require_client_cert;

#if 1
	fr_tls_ocsp_conf_t	ocsp;			//!< Configuration for validating client certificates
							//!< with ocsp.
	fr_tls_ocsp_conf_t	staple;			//!< Configuration for validating server certificates
							//!< with ocsp.
#endif

#ifndef OPENSSL_NO_ECDH
	char const		*ecdh_curve;
#endif

#ifdef PSK_MAX_IDENTITY_LEN
	char const		*psk_identity;
	char const		*psk_password;
	char const		*psk_query;
#endif

	fr_tls_cache_conf_t	cache;			//!< Session cache configuration.
};

typedef struct fr_tls_conf_s fr_tls_conf_t;

fr_tls_conf_t	*fr_tls_conf_alloc(TALLOC_CTX *ctx);

fr_tls_conf_t	*fr_tls_conf_parse_server(CONF_SECTION *cs);

fr_tls_conf_t	*fr_tls_conf_parse_client(CONF_SECTION *cs);

#ifdef __cplusplus
}
#endif
#endif /* WITH_TLS */
