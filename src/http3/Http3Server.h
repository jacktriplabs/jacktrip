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
 * \file Http3Server.h
 * \author Mike Dickey + Claude AI
 * \date 2026
 *
 * QUIC/HTTP3 server lifecycle: msquic registration, configuration, TLS, and
 * listener management extracted from UdpHubListener.
 */

#ifndef __HTTP3SERVER_H__
#define __HTTP3SERVER_H__

#include <QHostAddress>
#include <QString>
#include <functional>

// Forward-declare msquic types so callers don't need to include msquic.h
struct QUIC_API_TABLE;
struct QUIC_HANDLE;
typedef QUIC_HANDLE* HQUIC;

/** \brief QUIC/HTTP3 server lifecycle manager.
 *
 * Owns the msquic registration, configuration, and listener.  Callers supply
 * a new-connection callback that is invoked (from a msquic thread) whenever a
 * new QUIC connection is accepted.
 *
 * Usage:
 * \code
 *   Http3Server server(certFile, keyFile, port);
 *   server.setConnectionCallback([](HQUIC conn, const QHostAddress& addr, quint16 port) {
 *       // create a WebTransportSession for this connection
 *   });
 *   server.start();
 *   // ...
 *   server.stop();  // or just let the destructor run
 * \endcode
 */
class Http3Server
{
   public:
    using ConnectionCallback =
        std::function<void(HQUIC connection, const QHostAddress& addr, quint16 port)>;

    /** \brief Construct the server.
     *
     * \param certFile  Path to the PEM certificate file.
     * \param keyFile   Path to the PEM private key file.
     * \param port      UDP port to listen on.
     */
    Http3Server(const QString& certFile, const QString& keyFile, int port);
    ~Http3Server();

    /** \brief Set the callback invoked for each new QUIC connection. */
    void setConnectionCallback(ConnectionCallback cb) { mConnectionCallback = cb; }

    /** \brief Initialize msquic and start listening.  Returns true on success. */
    bool start();

    /** \brief Stop listening and release all msquic resources. */
    void stop();

    /** \brief Returns true if the server is currently listening. */
    bool isRunning() const { return mQuicListener != nullptr; }

    /** \brief Expose the API table so sessions can use it. */
    const QUIC_API_TABLE* quicApi() const { return mQuicApi; }

    /** \brief Expose the configuration handle so connections can be accepted. */
    HQUIC quicConfiguration() const { return mQuicConfiguration; }

    // These must be public so the static msquic callbacks can reach them.
    unsigned int handleListenerEvent(HQUIC listener, void* event);
    unsigned int handleQuicConnection(HQUIC connection, void* event);

   private:
    QString mCertFile;
    QString mKeyFile;
    int mPort;

    const QUIC_API_TABLE* mQuicApi;
    HQUIC mQuicRegistration;
    HQUIC mQuicConfiguration;
    HQUIC mQuicListener;

    ConnectionCallback mConnectionCallback;
};

#endif  // __HTTP3SERVER_H__
