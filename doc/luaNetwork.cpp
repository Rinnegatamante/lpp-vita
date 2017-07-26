/**
 * \defgroup Network
 * Module that handles network and online features.
 */ 
 
/**
 * HTTP methods.
 * \ingroup Network
 */
enum HTTPMethod{
	GET_METHOD,     //!< GET method
	POST_METHOD,    //!< POST method
	HEAD_METHOD,    //!< HEAD method
	OPTIONS_METHOD, //!< OPTIONS method
	PUT_METHOD,     //!< PUT method
	DELETE_METHOD,  //!< DELETE method
	TRACE_METHOD,   //!< TRACE method
	CONNECT_METHOD  //!< CONNECT method
};

class Network {
	
	public:
	
		/**
		 * Initialize network system.
		 * \ingroup Network
		 *
		 * @par Usage example:
		 * @code
		 * Network.init()
		 * @endcode
		 */
		void init(void){}
		
		/**
		 * Terminate network system.
		 * \ingroup Network
		 *
		 * @par Usage example:
		 * @code
		 * Network.term()
		 * @endcode
		 */
		void term(void){}
		
		/**
		 * Initialize debug FTP.
		 * \ingroup Network
		 *
		 * @par Usage example:
		 * @code
		 * Network.initFTP()
		 * @endcode
		 */
		void initFTP(void){}
		
		/**
		 * Terminate debug FTP.
		 * \ingroup Network
		 *
		 * @par Usage example:
		 * @code
		 * Network.termFTP()
		 * @endcode
		 */
		void termFTP(void){}
		
		/**
		 * Get console IP address.
		 * \ingroup Network
		 *
		 * @par Usage example:
		 * @code
		 * ip_addr = Network.getIPAddress()
		 * @endcode
		 *
		 * @return The IP Address of the console.
		 */
		string getIPAdddress(void){}
		
		/**
		 * Get console Mac address.
		 * \ingroup Network
		 *
		 * @par Usage example:
		 * @code
		 * mac_addr = Network.getMacAddress()
		 * @endcode
		 *
		 * @return The Mac Address of the console.
		 */
		string getMacAdddress(void){}
		
		/**
		 * Check if WiFi is available.
		 * \ingroup Network
		 *
		 * @par Usage example:
		 * @code
		 * if Network.isWifiEnabled() then
		 * 		Network.downloadFile("http://www.website.com/file.zip", "ux0:/data/file.zip")
		 * end
		 * @endcode
		 *
		 * @return true if available, false otherwise.
		 */
		bool isWifiEnabled(void){}
		
		/**
		 * Get WiFi strength level.
		 * \ingroup Network
		 *
		 * @par Usage example:
		 * @code
		 * wifi_lv = Network.getWifiLevel()
		 * @endcode
		 *
		 * @return The WiFi strength level percentage.
		 */
		int getWifiLevel(void){}
		
		/**
		 * Download a file via HTTP protocol.
		 * \ingroup Network
		 *
		 * @par Usage example:
		 * @code
		 * Network.downloadFile("http://www.website.com/file.zip", "ux0:/data/file.zip")
		 * @endcode
		 *
		 * @param url - The url from where to download the file.
		 * @param file - Filename where to save the downloaded file.
		 * @param useragent - Custom useragent to use <b>(optional)</b>.
		 * @param method - Method to use to perform the HTTP request <b>(optional)</b>.
		 * @param postdata - POST data to send with the HTTP request <b>(optional)</b>.
		 *
		 * @note <b>postdata</b> will work only with <b>method</b> set to \link HTTPMethod::POST_METHOD POST_METHOD\endlink.
		 * @note If not specified, \link HTTPMethod::GET_METHOD GET_METHOD\endlink is used as default.
		 */
		void downloadFile(string url, string file, string useragent, HTTPMethod method, string postdata){}
		
		/**
		 * Get an HTTP request result.
		 * \ingroup Network
		 *
		 * @par Usage example:
		 * @code
		 * result = Network.requestString("http://www.website.com/file.txt")
		 * @endcode
		 *
		 * @param url - The url where to send the HTTP request.
		 * @param useragent - Custom useragent to use <b>(optional)</b>.
		 * @param method - Method to use to perform the HTTP request <b>(optional)</b>.
		 * @param postdata - POST data to send with the HTTP request <b>(optional)</b>.
		 *
		 * @return The HTTP request result.
		 *
		 * @note <b>postdata</b> will work only with <b>method</b> set to \link HTTPMethod::POST_METHOD POST_METHOD\endlink.
		 * @note If not specified, \link HTTPMethod::GET_METHOD GET_METHOD\endlink is used as default.
		 */
		string requestString(string url, string useragent, HTTPMethod method, string postdata){}

}

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
		int createServerSocket(int port, NetProtocol protocol){}
		
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
		int connect(string host, int port, NetProtocol protocol){}
		
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
		int send(int sock, string data){}
		
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
		string receive(int sock, int size){}
		
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
		int accept(int sock){}
		
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
		void close(int sock){}

}