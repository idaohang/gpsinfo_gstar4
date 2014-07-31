#include "gstar.h"
#include <sstream>
#include <iostream>

bool Gstar::Start(string portname)
{
	gps_info= "NoFix";
	end_of_line_char_ = 0x0d;
	strncpy(start_line_, "AGGPG", SERIAL_START_LINE_SIZE);
	flag_gps_data=false;
	port_name_ = portname;
	boost::system::error_code ec;
 
	port_ = serial_port_ptr(new boost::asio::serial_port(io_service_));
	port_->open(port_name_, ec);
	if (ec) {
		std::cout << "error : port_->open() failed...com_port_name="
			<< port_name_ << ", e=" << ec.message().c_str() << std::endl; 
		return false;
	}
 
	// option settings...
	port_->set_option(boost::asio::serial_port_base::baud_rate(4800));
	port_->set_option(boost::asio::serial_port_base::character_size(8));
	port_->set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
	port_->set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
	port_->set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
 
	boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service_));
	async_read_some_();
 
	return true;
}
void Gstar::Stop()
{
	boost::mutex::scoped_lock look(mutex_);
 
	if (port_) {
		port_->cancel();
		port_->close();
		port_.reset();
	}
	io_service_.stop();
	io_service_.reset();
}

Gstar* Gstar::m_Instance = 0;

Gstar* Gstar::Instance ()
{
	if (m_Instance == 0)
	{
		m_Instance = new Gstar;
	}
	return m_Instance;
}

void Gstar::async_read_some_()
{
	if (port_.get() == NULL || !port_->is_open()) return;
 
	port_->async_read_some( 
		boost::asio::buffer(read_buf_raw_, SERIAL_PORT_READ_BUF_SIZE),
		boost::bind(
			&Gstar::on_receive_,
			this, boost::asio::placeholders::error, 
			boost::asio::placeholders::bytes_transferred));
}
 
void Gstar::on_receive_(const boost::system::error_code& ec, size_t bytes_transferred)
{
	boost::mutex::scoped_lock look(mutex_);
 
	if (port_.get() == NULL || !port_->is_open()) return;
	if (ec) {
		async_read_some_();
		return;
	}
 
	for (unsigned int i = 0; i < bytes_transferred; ++i) {
		char c = read_buf_raw_[i];
		//std::cout << c;
		if(!flag_gps_data)
		{
			for(int j=0; j<SERIAL_START_LINE_SIZE-1;j++)
				read_start_buf_raw_[SERIAL_START_LINE_SIZE-1-j]=read_start_buf_raw_[SERIAL_START_LINE_SIZE-2-j];
			read_start_buf_raw_[0]=c;
			if(!strcmp(read_start_buf_raw_,"AGGPG"))
				flag_gps_data=true;
		}
		else
		{		
			if (c == end_of_line_char_)
			{
				this->on_receive_(read_buf_str_);
				read_buf_str_.clear();
			}
			else 
			{
				read_buf_str_ += c;
			}
		}
		
	}
 
	async_read_some_();
}
 
void Gstar::on_receive_(const std::string &data)
{
	gps_info =  parse_data(data);
	flag_gps_data=false;
}

string Gstar::parse_data(string data)
{
	int n_pos = data.find(",N,");
	int e_pos = data.find(",E,");
	if (n_pos>0 && e_pos>0)
	{
		std::string n_degree = data.substr(n_pos-9,2);
		std::string n_min = data.substr(n_pos-7,7);
		std::string e_degree = data.substr(e_pos-10,3);
		std::string e_min = data.substr(e_pos-7,7);
		//Remove leading zeros
		n_degree = remove_leading_zeros(n_degree);
		n_min = remove_leading_zeros(n_min);
		e_degree = remove_leading_zeros(e_degree);
		e_min = remove_leading_zeros(e_min);
		//Convertions DD
		double d_n_degree;
		double d_n_min;
		double d_e_degree;
		double d_e_min;
		std::stringstream ss;
		ss.precision(8);
		ss << n_degree; ss >> d_n_degree;
		ss.clear();
		ss << n_min; ss >> d_n_min;
		ss.clear();
		ss << e_degree; ss >> d_e_degree;
		ss.clear();
		ss << e_min; ss >> d_e_min;
		ss.str("");
		ss.clear();
		ss << "Lat:" << d_n_degree + d_n_min/60.0  <<",Lon:" << d_e_degree+d_e_min/60.0 ;
		return ss.str();
	}
	return "NoFix";
}

string Gstar::remove_leading_zeros(string data)
{
	int first_non_zero = data.find_first_not_of('0',0);
	if(first_non_zero > 0)
		return data.substr(first_non_zero,data.length()-first_non_zero);
	return data;
}
Gstar::Gstar(){}

Gstar::~Gstar(){}
