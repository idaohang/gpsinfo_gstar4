#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
 
#include <string>
#include <vector>

using namespace std;
typedef boost::shared_ptr<boost::asio::serial_port> serial_port_ptr;
#define SERIAL_PORT_READ_BUF_SIZE 256
#define SERIAL_START_LINE_SIZE 5

class Gstar
{
	private:
		static Gstar* m_Instance;
		boost::asio::io_service io_service_;
		string port_name_;
		serial_port_ptr port_;
		boost::mutex mutex_;
			
	public:
		std::string gps_info;
		char read_buf_raw_[SERIAL_PORT_READ_BUF_SIZE];
		char read_start_buf_raw_[SERIAL_START_LINE_SIZE];
		bool flag_gps_data;
		std::string read_buf_str_;
		char end_of_line_char_;
		char start_line_[SERIAL_START_LINE_SIZE];

	public:
		static Gstar* Instance();
		bool Start(string portname);
		void Stop();
		string parse_data(string data);
		string remove_leading_zeros(string data);

	protected:
		Gstar();
		~Gstar();
		Gstar(const Gstar & ) ;
		Gstar &operator= (const Gstar & );
	
	protected:
		virtual void async_read_some_();
		virtual void on_receive_(const boost::system::error_code& ec, size_t bytes_transferred);
		virtual void on_receive_(const std::string &data);
};

