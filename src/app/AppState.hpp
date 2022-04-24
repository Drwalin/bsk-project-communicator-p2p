
#ifndef PG_BSK_PROJECT_APP_STATE_HPP
#define PG_BSK_PROJECT_APP_STATE_HPP

#include <vector>
#include <string>
#include <array>
#include <queue>
#include <mutex>

#include "../crypto/Crypto.hpp"

#include <rpc/client.h>
#include <rpc/server.h>
#include <rpc/rpc_error.h>

#include <stdexcept>

#include "Codes.hpp"

using Array12 = std::array<uint8_t, 12>;
using Array16 = std::array<uint8_t, 16>;
using Array32 = std::array<uint8_t, 32>;
using Array33 = std::array<uint8_t, 33>;
using Array64 = std::array<uint8_t, 64>;

using EcPublicKey = std::array<uint8_t, ec::PUBLIC_KEY_SIZE>;
using EcPrivateKey = std::array<uint8_t, ec::PRIVATE_KEY_SIZE>;
using EcSignature = std::array<uint8_t, ec::SIGNATURE_SIZE>;
using ChachaNonce = std::array<uint8_t, chacha::NONCE_SIZE>;

class KexError: public std::runtime_error{using std::runtime_error::runtime_error;};
class KexKeygenFailed: public KexError{using KexError::KexError;};
class KexSignFailed: public KexError{using KexError::KexError;};
class KexConnectionFailed: public KexError{using KexError::KexError;};


struct KexMessage {
	ERROR_CODE error_code;
	EcPublicKey publicKey;
	EcPublicKey publicEcdheKey;
	std::string ipaddress;
	int port;
	EcSignature signature;
	
	void GenerateDigest(Array32& hash);
	bool Verify();
	bool Sign(EcPrivateKey& privkey);

	MSGPACK_DEFINE_ARRAY(error_code, publicKey, publicEcdheKey, ipaddress, port, signature);
};

struct Message {
    MSG_TYPE msg_type;
    ENCRYPTION_MODE cipher_variant;
	ChachaNonce nonce;
    std::vector<uint8_t> encrypted_data;

	MSGPACK_DEFINE_ARRAY(msg_type, cipher_variant, nonce, encrypted_data);
};

class AppState {
public:
	
	static AppState* singleton;
	
	AppState(std::string myIp, int32_t port);
	~AppState();
	
	ERROR_CODE ConnectAndHandshake(std::string ip, int32_t port);
	
	void BindAll();
	
	void GenerateKey();
	void LoadKey(std::string keyFilePath, const std::string& passphrase);
	void SaveKey(std::string keyFilePath, const std::string& passphrase);
	
	KexMessage ReceiveKex(KexMessage kex);
	
	
	
	uint32_t SendMessage(std::string message);
	uint32_t ReceiveMessage(Message message);
	void PushMessage(const std::string& message);
	bool PopMessage(std::string& message);
	
public:
	
	void EncryptMessage(MSG_TYPE type, const void* plaintext, size_t length,
			Message& message);
	bool DecryptMessage(const Message& message,
			std::vector<uint8_t>& plaintext);
	
	void Encrypt(const void* plaintext, size_t plaintextLength,
			ChachaNonce& nonce, std::vector<uint8_t>& ciphertext,
			const void* ad, size_t adLength, ENCRYPTION_MODE encryptionMode);
	bool Decrypt(const void* ciphertext, size_t ciphertextLength,
			const ChachaNonce& nonce, std::vector<uint8_t>& plaintext,
			const void* ad, size_t adLength, ENCRYPTION_MODE encryptionMode);
	
public:
	
	ENCRYPTION_MODE currentEncryptionMode;
	
	rpc::server rpcServer;
	rpc::client* client;
	
	EcPublicKey publicKey;
	EcPrivateKey privateKey;
	Array32 sharedKey;
	EcPublicKey theirPublicKey;
	
	std::queue<std::string> receivedMessages;
	std::mutex mutex;
	std::string ipAddress;
	int32_t port;
};

#endif

