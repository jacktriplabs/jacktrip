//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008-2026 Juan-Pablo Caceres, Chris Chafe.
  SoundWIRE group at CCRMA, Stanford University.

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation
  files (the "Software"), to deal in the Software without
  restriction, including without limitation the rights to use,
  copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following
  conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.
*/
//*****************************************************************

/**
 * \file Http3Server.cpp
 * \author Mike Dickey + Claude AI
 * \date 2026
 */

#include "Http3Server.h"

#include <msquic.h>

#include <QHostAddress>
#include <cstring>
#include <iostream>

using std::cerr;
using std::cout;
using std::endl;

//*******************************************************************************
// Static msquic callbacks
//*******************************************************************************

static QUIC_STATUS QUIC_API ListenerCallback(HQUIC listener, void* context,
                                             QUIC_LISTENER_EVENT* event)
{
    Http3Server* server = static_cast<Http3Server*>(context);
    if (server) {
        return server->handleListenerEvent(listener, event);
    }
    return QUIC_STATUS_INVALID_STATE;
}

static QUIC_STATUS QUIC_API ServerConnectionCallback(HQUIC connection, void* context,
                                                     QUIC_CONNECTION_EVENT* event)
{
    Http3Server* server = static_cast<Http3Server*>(context);
    if (server) {
        return server->handleQuicConnection(connection, event);
    }
    return QUIC_STATUS_INVALID_STATE;
}

//*******************************************************************************
Http3Server::Http3Server(const QString& certFile, const QString& keyFile, int port)
    : mCertFile(certFile)
    , mKeyFile(keyFile)
    , mPort(port)
    , mQuicApi(nullptr)
    , mQuicRegistration(nullptr)
    , mQuicConfiguration(nullptr)
    , mQuicListener(nullptr)
{
}

//*******************************************************************************
Http3Server::~Http3Server()
{
    stop();
}

//*******************************************************************************
bool Http3Server::start()
{
    // Open the msquic library
    QUIC_STATUS status = MsQuicOpen2(&mQuicApi);
    if (QUIC_FAILED(status)) {
        cerr << "Http3Server: Failed to open msquic, status: 0x" << std::hex << status
             << std::dec << endl;
        return false;
    }

    // Create a registration for "JackTrip"
    const QUIC_REGISTRATION_CONFIG regConfig = {"JackTrip",
                                                QUIC_EXECUTION_PROFILE_LOW_LATENCY};
    status = mQuicApi->RegistrationOpen(&regConfig, &mQuicRegistration);
    if (QUIC_FAILED(status)) {
        cerr << "Http3Server: Failed to create msquic registration, status: 0x"
             << std::hex << status << std::dec << endl;
        MsQuicClose(mQuicApi);
        mQuicApi = nullptr;
        return false;
    }

    // Configure ALPN for HTTP/3 (WebTransport)
    const char* alpn       = "h3";
    QUIC_BUFFER alpnBuffer = {
        static_cast<uint32_t>(strlen(alpn)),
        const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(alpn))};

    QUIC_SETTINGS settings{};
    std::memset(&settings, 0, sizeof(settings));
    settings.IdleTimeoutMs                = 30000;
    settings.IsSet.IdleTimeoutMs          = TRUE;
    settings.DatagramReceiveEnabled       = TRUE;
    settings.IsSet.DatagramReceiveEnabled = TRUE;
    settings.PeerBidiStreamCount          = 10;
    settings.IsSet.PeerBidiStreamCount    = TRUE;
    settings.PeerUnidiStreamCount         = 10;
    settings.IsSet.PeerUnidiStreamCount   = TRUE;

    status = mQuicApi->ConfigurationOpen(mQuicRegistration, &alpnBuffer, 1, &settings,
                                         sizeof(settings), nullptr, &mQuicConfiguration);
    if (QUIC_FAILED(status)) {
        cerr << "Http3Server: Failed to create msquic configuration, status: 0x"
             << std::hex << status << std::dec << " (" << status << ")" << endl;
        mQuicApi->RegistrationClose(mQuicRegistration);
        MsQuicClose(mQuicApi);
        mQuicApi          = nullptr;
        mQuicRegistration = nullptr;
        return false;
    }

    // Load TLS credentials
    QUIC_CREDENTIAL_CONFIG credConfig{};
    std::memset(&credConfig, 0, sizeof(credConfig));
    credConfig.Type  = QUIC_CREDENTIAL_TYPE_CERTIFICATE_FILE;
    credConfig.Flags = QUIC_CREDENTIAL_FLAG_NONE;

    QUIC_CERTIFICATE_FILE certFile{};
    std::memset(&certFile, 0, sizeof(certFile));
    QByteArray certPath        = mCertFile.toUtf8();
    QByteArray keyPath         = mKeyFile.toUtf8();
    certFile.CertificateFile   = certPath.constData();
    certFile.PrivateKeyFile    = keyPath.constData();
    credConfig.CertificateFile = &certFile;

    status = mQuicApi->ConfigurationLoadCredential(mQuicConfiguration, &credConfig);
    if (QUIC_FAILED(status)) {
        cerr << "Http3Server: Failed to load TLS certificate, status: 0x" << std::hex
             << status << std::dec << " (" << status << ")" << endl;
        cerr << "  Certificate: " << mCertFile.toStdString() << endl;
        cerr << "  Private key: " << mKeyFile.toStdString() << endl;
        mQuicApi->ConfigurationClose(mQuicConfiguration);
        mQuicApi->RegistrationClose(mQuicRegistration);
        MsQuicClose(mQuicApi);
        mQuicApi           = nullptr;
        mQuicRegistration  = nullptr;
        mQuicConfiguration = nullptr;
        return false;
    }

    // Create listener on UDP port
    QUIC_ADDR address{};
    std::memset(&address, 0, sizeof(address));
    QuicAddrSetFamily(&address, QUIC_ADDRESS_FAMILY_UNSPEC);
    QuicAddrSetPort(&address, static_cast<uint16_t>(mPort));

    status =
        mQuicApi->ListenerOpen(mQuicRegistration, ListenerCallback, this, &mQuicListener);
    if (QUIC_FAILED(status)) {
        cerr << "Http3Server: Failed to create QUIC listener, status: 0x" << std::hex
             << status << std::dec << endl;
        mQuicApi->ConfigurationClose(mQuicConfiguration);
        mQuicApi->RegistrationClose(mQuicRegistration);
        MsQuicClose(mQuicApi);
        mQuicApi           = nullptr;
        mQuicRegistration  = nullptr;
        mQuicConfiguration = nullptr;
        return false;
    }

    status = mQuicApi->ListenerStart(mQuicListener, &alpnBuffer, 1, &address);
    if (QUIC_FAILED(status)) {
        cerr << "Http3Server: Failed to start QUIC listener, status: 0x" << std::hex
             << status << std::dec << endl;
        mQuicApi->ListenerClose(mQuicListener);
        mQuicApi->ConfigurationClose(mQuicConfiguration);
        mQuicApi->RegistrationClose(mQuicRegistration);
        MsQuicClose(mQuicApi);
        mQuicApi           = nullptr;
        mQuicRegistration  = nullptr;
        mQuicConfiguration = nullptr;
        mQuicListener      = nullptr;
        return false;
    }

    return true;
}

//*******************************************************************************
void Http3Server::stop()
{
    if (mQuicListener && mQuicApi) {
        mQuicApi->ListenerClose(mQuicListener);
        mQuicListener = nullptr;
    }
    if (mQuicConfiguration && mQuicApi) {
        mQuicApi->ConfigurationClose(mQuicConfiguration);
        mQuicConfiguration = nullptr;
    }
    if (mQuicRegistration && mQuicApi) {
        mQuicApi->RegistrationClose(mQuicRegistration);
        mQuicRegistration = nullptr;
    }
    if (mQuicApi) {
        MsQuicClose(mQuicApi);
        mQuicApi = nullptr;
    }
}

//*******************************************************************************
unsigned int Http3Server::handleListenerEvent(HQUIC listener, void* eventPtr)
{
    Q_UNUSED(listener)
    QUIC_LISTENER_EVENT* event = static_cast<QUIC_LISTENER_EVENT*>(eventPtr);

    switch (event->Type) {
    case QUIC_LISTENER_EVENT_NEW_CONNECTION: {
        HQUIC connection = event->NEW_CONNECTION.Connection;

        // Get peer address
        QUIC_ADDR peerAddr;
        uint32_t addrLen = sizeof(peerAddr);
        mQuicApi->GetParam(connection, QUIC_PARAM_CONN_REMOTE_ADDRESS, &addrLen,
                           &peerAddr);

        QHostAddress peerAddress;
        quint16 peerPort = 0;

        if (QuicAddrGetFamily(&peerAddr) == QUIC_ADDRESS_FAMILY_INET) {
            peerAddress.setAddress(ntohl(peerAddr.Ipv4.sin_addr.s_addr));
            peerPort = ntohs(peerAddr.Ipv4.sin_port);
        } else if (QuicAddrGetFamily(&peerAddr) == QUIC_ADDRESS_FAMILY_INET6) {
            peerAddress.setAddress(reinterpret_cast<quint8*>(&peerAddr.Ipv6.sin6_addr));
            peerPort = ntohs(peerAddr.Ipv6.sin6_port);
        }

        // Set the connection callback before accepting
        mQuicApi->SetCallbackHandler(connection, (void*)ServerConnectionCallback, this);

        // Accept the connection with our configuration
        QUIC_STATUS status =
            mQuicApi->ConnectionSetConfiguration(connection, mQuicConfiguration);
        if (QUIC_FAILED(status)) {
            cerr << "Http3Server: Failed to set connection configuration, status: 0x"
                 << std::hex << status << std::dec << " (" << status << ")" << endl;
            return QUIC_STATUS_CONNECTION_REFUSED;
        }

        // Notify the delegate (UdpHubListener) about the new connection
        if (mConnectionCallback) {
            mConnectionCallback(connection, peerAddress, peerPort);
        } else {
            cerr << "Http3Server: No connection callback set - refusing connection"
                 << endl;
            return QUIC_STATUS_CONNECTION_REFUSED;
        }

        return QUIC_STATUS_SUCCESS;
    }

    case QUIC_LISTENER_EVENT_STOP_COMPLETE:
        break;

    default:
        break;
    }

    return QUIC_STATUS_SUCCESS;
}

//*******************************************************************************
unsigned int Http3Server::handleQuicConnection(HQUIC /* connection */, void* eventPtr)
{
    QUIC_CONNECTION_EVENT* event = static_cast<QUIC_CONNECTION_EVENT*>(eventPtr);

    switch (event->Type) {
    case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_TRANSPORT:
        cerr << "Http3Server: Connection shutdown by transport (before session created)"
             << endl;
        cerr << "  Status: 0x" << std::hex
             << event->SHUTDOWN_INITIATED_BY_TRANSPORT.Status << std::dec << " ("
             << event->SHUTDOWN_INITIATED_BY_TRANSPORT.Status << ")" << endl;
        break;

    case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_PEER:
        break;

    case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE:
        break;

    default:
        break;
    }

    return QUIC_STATUS_SUCCESS;
}
