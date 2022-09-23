/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/wallet_connect/wallet_connect_client.h"

#include "base/guid.h"
#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "brave/components/wallet_connect/encryptor.h"
#include "brave/components/wallet_connect/wallet_connect.h"
#include "brave/components/wallet_connect/wallet_connect_utils.h"
#include "services/network/public/mojom/network_context.mojom.h"
#include "url/url_constants.h"

namespace wallet_connect {

constexpr net::NetworkTrafficAnnotationTag kTrafficAnnotation =
    net::DefineNetworkTrafficAnnotation("wallet_connect_client", R"(
        semantics {
          sender: "Wallet Connect Client"
          description:
            "Chrome can communicate with a phone for the purpose of using "
            "the phone as a security key. This WebSocket connection is made to "
            "a rendezvous service of the phone's choosing. Mostly likely that "
            "is a Google service because the phone-side is being handled by "
            "Chrome on that device. The service carries only end-to-end "
            "encrypted data where the keys are shared directly between the "
            "client and phone via QR code and Bluetooth broadcast."
          trigger:
            "A web-site initiates a WebAuthn request and the user scans a QR "
            "code with their phone."
          data: "Only encrypted data that the service does not have the keys "
                "for."
          destination: GOOGLE_OWNED_SERVICE
        }
        policy {
          cookies_allowed: NO
          setting: "Not controlled by a setting because the operation is "
            "triggered by significant user action."
          policy_exception_justification:
            "No policy provided because the operation is triggered by "
            " significant user action. No background activity occurs."
        })");

WalletConnectClient::WalletConnectClient(
    network::mojom::NetworkContext* network_context)
    : client_id_(base::GenerateGUID()),
      websocket_client_(std::make_unique<WebSocketAdapter>(
          base::BindOnce(&WalletConnectClient::OnTunnelReady,
                         base::Unretained(this)),
          base::BindRepeating(&WalletConnectClient::OnTunnelData,
                              base::Unretained(this)))),
      network_context_(network_context) {}

WalletConnectClient::~WalletConnectClient() = default;

bool WalletConnectClient::Init(const std::string& uri) {
  auto data = ParseWalletConnectURI(uri);
  if (!data)
    return false;
  wallet_connect_uri_data_ = std::move(data);

  DCHECK(network_context_);
  DCHECK(wallet_connect_uri_data_->params &&
         wallet_connect_uri_data_->params->is_v1_params());
  const auto& v1_params = wallet_connect_uri_data_->params->get_v1_params();
  GURL url = v1_params->bridge;
  if (!url.SchemeIsWSOrWSS()) {
    GURL::Replacements scheme_replacements;
    scheme_replacements.SetSchemeStr(url::kWssScheme);
    url = url.ReplaceComponents(scheme_replacements);
  }
  network_context_->CreateWebSocket(
      url, {}, net::SiteForCookies(), net::IsolationInfo(),
      /*additional_headers=*/{}, network::mojom::kBrowserProcessId,
      url::Origin::Create(url), network::mojom::kWebSocketOptionBlockAllCookies,
      net::MutableNetworkTrafficAnnotationTag(kTrafficAnnotation),
      websocket_client_->BindNewHandshakeClientPipe(),
      /*url_loader_network_observer=*/mojo::NullRemote(),
      /*auth_handler=*/mojo::NullRemote(),
      /*header_client=*/mojo::NullRemote(),
      /*throttling_profile_id=*/absl::nullopt);

  return true;
}

void WalletConnectClient::OnTunnelReady(bool success) {
  if (success) {
    state_ = State::kConnected;
    // subscribe to handshae topic
    types::SocketMessage message;
    message.topic = wallet_connect_uri_data_->topic;
    message.type = "sub";
    message.payload = "";
    message.silent = true;
    std::string json;
    if (base::JSONWriter::Write(*message.ToValue(), &json)) {
      LOG(ERROR) << "send: " << json;
      websocket_client_->Write(std::vector<uint8_t>(json.begin(), json.end()));
    }
    // subscribe to own client id
    message.topic = client_id_;
    if (base::JSONWriter::Write(*message.ToValue(), &json)) {
      LOG(ERROR) << "send: " << json;
      websocket_client_->Write(std::vector<uint8_t>(json.begin(), json.end()));
    }
  }
}

void WalletConnectClient::OnTunnelData(
    absl::optional<base::span<const uint8_t>> data) {
  if (data) {
    const std::string data_str((const char*)data->data(), data->size());
    LOG(ERROR) << "receive: " << data_str;
    // TODO: sanitize input
    auto value = base::JSONReader::Read(data_str);
    if (!value) {
      return;
    }
    auto message = types::SocketMessage::FromValue(*value);
    // ack
    types::SocketMessage ack_msg;
    ack_msg.topic = message->topic;
    ack_msg.type = "ack";
    ack_msg.payload = "";
    ack_msg.silent = true;
    std::string ack_json;
    if (base::JSONWriter::Write(*ack_msg.ToValue(), &ack_json)) {
      LOG(ERROR) << "send: " << ack_json;
      websocket_client_->Write(
          std::vector<uint8_t>(ack_json.begin(), ack_json.end()));
    }

    // decrypting message
    std::vector<uint8_t> key_vec;
    DCHECK(wallet_connect_uri_data_->params &&
           wallet_connect_uri_data_->params->is_v1_params());
    if (!base::HexStringToBytes(
            wallet_connect_uri_data_->params->get_v1_params()->key, &key_vec)) {
      return;
    }
    std::array<uint8_t, 32> key;
    std::copy_n(key_vec.begin(), 32, key.begin());
    Encryptor encryptor(key);
    auto ciphertext_value = base::JSONReader::Read(message->payload);
    if (!ciphertext_value) {
      return;
    }
    auto ciphertext = types::EncryptionPayload::FromValue(*ciphertext_value);
    if (!ciphertext)
      return;
    auto decrypted_payload = encryptor.Decrypt(*ciphertext);
    if (!decrypted_payload.has_value()) {
      LOG(ERROR) << decrypted_payload.error();
    }
    auto decrypted_payload_value = decrypted_payload.value();
    const std::string decrypted_payload_value_str(
        decrypted_payload_value.begin(), decrypted_payload_value.end());
    LOG(ERROR) << "decrypted: " << decrypted_payload_value_str;

    // extract rpc request
    auto rpc_request_value =
        base::JSONReader::Read(decrypted_payload_value_str);
    if (!rpc_request_value) {
      LOG(ERROR) << "can't read rpc request json";
      return;
    }
    auto rpc_request = types::JsonRpcRequest::FromValue(*rpc_request_value);
    if (!rpc_request) {
      LOG(ERROR) << "rpc request from value failed"
                 << rpc_request_value->DebugString();
      return;
    }

    if (state_ == State::kConnected) {
      // extract session request
      DCHECK(rpc_request->method == "wc_sessionRequest");
      DCHECK(rpc_request->params.size() == 1);
      auto session_request =
          types::SessionRequest::FromValue(*std::move(rpc_request->params[0]));
      if (!session_request) {
        LOG(ERROR) << "session request from value failed";
        return;
      }

      // construct approved session_params
      types::SessionParams session_params;
      session_params.approved = true;
      session_params.chain_id = 1;
      session_params.network_id = 0;
      session_params.accounts.push_back(
          "0xf81229FE54D8a20fBc1e1e2a3451D1c7489437Db");
      session_params.peer_id =
          std::make_unique<std::string>(client_id_);
      session_params.rpc_url = std::make_unique<std::string>("");
      types::ClientMeta meta;
      meta.name = "Brave Wallet";
      session_params.peer_meta =
          std::make_unique<types::ClientMeta>(std::move(meta));

      // put session_params into JsonRpcResponseSuccess
      types::JsonRpcResponseSuccess response;
      response.id = rpc_request->id;
      response.jsonrpc = "2.0";
      response.result = session_params.ToValue();

      std::string response_json;
      if (!base::JSONWriter::Write(*response.ToValue(), &response_json)) {
        return;
      }
      LOG(ERROR) << "encrypting: " << response_json;
      auto encrypted_payload = encryptor.Encrypt(
          std::vector<uint8_t>(response_json.begin(), response_json.end()));
      DCHECK(encrypted_payload.has_value());
      std::string encrypted_response_json;
      if (!base::JSONWriter::Write(*encrypted_payload.value().ToValue(),
                                   &encrypted_response_json)) {
        return;
      }
      // put encrypted payload into socket message
      types::SocketMessage socket_response;
      socket_response.topic = session_request->peer_id;
      socket_response.type = "pub";
      socket_response.payload = encrypted_response_json;
      socket_response.silent = true;

      std::string socket_response_json;
      if (!base::JSONWriter::Write(*socket_response.ToValue(),
                                   &socket_response_json)) {
        return;
      }
      LOG(ERROR) << "send: " << socket_response_json;
      websocket_client_->Write(std::vector<uint8_t>(
          socket_response_json.begin(), socket_response_json.end()));
      state_ = State::kSessionEstablished;
    } else if (state_ == State::kSessionEstablished) {
      // Handle wallet request
      LOG(ERROR) << *rpc_request_value;
    }
  }
}

}  // namespace wallet_connect
