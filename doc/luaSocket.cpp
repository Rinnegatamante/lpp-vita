/**
 * \defgroup Socket
 * Module that handles network sockets.
 */ 

/**
 * Network protocols.
 * \ingroup Socket
 */
enum NetProtocol{
	TCP_SOCKET, //!< TCP protocol
	UDP_SOCKET  //!< UDP protocol
} 
 
class Socket {
	
	public:

		/**
		 * Create a server socket.
		 * \ingroup Socket
		 *
		 * @par Usage example:
		 * @code
		 * sock = Socket.createServerSocket(5000, TCP_SOCKET)
		 * @endcode
		 *
		 * @param port - Port to use.
		 * @param protocol - Protocol to use <b>(optional)</b>.
		 *
		 * @return The created socket id.
		 *
		 * @note If not specified, \link NetProtocol::TCP_SOCKET TCP_SOCKET\endlink is used as default.
		 */
		int createServerSocket(int port, NetProtocol protocol);
		
		/**
		 * Connect to a server.
		 * \ingroup Socket
		 *
		 * @par Usage example:
		 * @code
		 * sock = Socket.connect("192.168.1.8", 5000)
		 * @endcode
		 *
		 * @param host - Host to connect to.
		 * @param port - Port to use.
		 * @param protocol - Protocol to use <b>(optional)</b>.
		 *
		 * @return A valid client socket id.
		 *
		 * @note If not specified, \link NetProtocol::TCP_SOCKET TCP_SOCKET\endlink is used as default.
		 */
		int connect(string host, int port, NetProtocol protocol);
		
		/**
		 * Send data via socket.
		 * \ingroup Socket
		 *
		 * @par Usage example:
		 * @code
		 * Socket.send(sock, "Hello World!")
		 * @endcode
		 *
		 * @param sock - A valid socket id.
		 * @param data - Data to send.
		 *
		 * @return Length of the sent data in bytes.
		 */
		int send(int sock, string data);
		
		/**
		 * Send data via socket.
		 * \ingroup Socket
		 *
		 * @par Usage example:
		 * @code
		 * data = Socket.receive(sock, 32767)
		 * @endcode
		 *
		 * @param sock - A valid socket id.
		 * @param size - Maximum size of the received data.
		 *
		 * @return The received data.
		 *
		 * @note The received data can be smaller than <b>size</b>.
		 */
		string receive(int sock, int size);
		
		/**
		 * Accept new connections for a server socket.
		 * \ingroup Socket
		 *
		 * @par Usage example:
		 * @code
		 * client = Socket.accept(server)
		 * @endcode
		 *
		 * @param sock - A valid server socket id.
		 *
		 * @return A valid client socket id.
		 */
		int accept(int sock);
		
		/**
		 * Close a socket.
		 * \ingroup Socket
		 *
		 * @par Usage example:
		 * @code
		 * Socket.close(sock)
		 * @endcode
		 *
		 * @param sock - A valid socket id.
		 */
		void close(int sock);

}