#pragma once
#include "hidapi/hidapi.h"

/*
	HVISP FUSE RESET(HIGH 0xDF)
	Pin connection
	PICKIT3		ATTINY85
	1.VPP/MCLR	1.RST, PB5 (12V)
	2.Vdd		8.Vdd
	3.Gnd		4.Gnd
	4.PGD		6.SII, PB1
	5.PGC		2.SCI, PB3
	6.AUX		5.SDI, PB0
*/


// Device vendor and product id.
#define PICKIT3_VID 0x04d8
#define PICKIT3_PID 0x900a
#define TIMEOUT 1500

// Device of bytes to transfer.
#define BUF_SIZE 64

#define CMD_NOP             0x5A
#define CMD_GET_VERSION     0x76
#define CMD_GET_VERSION_MPLAB     0x41
#define CMD_SET_VDD_4(v)    0xA0, (uint8_t)((v)*2048+672), (uint8_t)(((v)*2048+672)/256), (uint8_t)((v)*36)
#define CMD_SET_VDD_PK3(v)    0xA0, (uint8_t)(v / 0.125F), (uint8_t)(v / 0.125F /256)
#define CMD_SET_VPP_4(v)    0xA1, 0x40, (uint8_t)((v)*18.61), (uint8_t)((v)*13)
#define CMD_SET_VPP_PK3(v)    0xA1, (uint8_t)(v / 0.125F), (uint8_t)(v / 0.125F /256)
#define CMD_READ_STATUS		0xA2
#define CMD_READ_VDD_VPP    0xA3
#define CMD_DOWNLOAD_SCRIPT(idx, len)    0xA4, (idx), (len)
#define CMD_RUN_SCRIPT(idx, cnt)    0xA5, (idx), (cnt)
#define CMD_EXEC_SCRIPT_2(len)  0xA6, (len)
#define CMD_CLR_DLOAD_BUFF  0xA7
#define CMD_DOWNLOAD_DATA_2(len)  0xA8, (len)
#define CMD_CLR_ULOAD_BUFF  0xA9
#define CMD_UPLOAD_DATA     0xAA
#define CMD_CLR_SCRIPT_BUFF 0xAB
#define CMD_UPLOAD_DATA_NO_LEN     0xAC
#define CMD_END_OF_BUFFER   0xAD
#define CMD_RESET		    0xAE

#define SCR_VDD_ON          0xFF
#define SCR_VDD_OFF         0xFE
#define SCR_VDD_GND_ON		0xFD
#define SCR_VDD_GND_OFF		0xFC
#define SCR_VPP_ON          0xFB
#define SCR_VPP_OFF         0xFA
#define SCR_VPP_PWM_ON      0xF9
#define SCR_VPP_PWM_OFF     0xF8
#define SCR_MCLR_GND_ON     0xF7
#define SCR_MCLR_GND_OFF    0xF6
#define SCR_BUSY_LED_ON     0xF5
#define SCR_BUSY_LED_OFF    0xF4
#define SCR_WRITE_BYTE_LITERAL(v)	0xF2, (v)
#define SCR_WRITE_BYTE_BUFFER	0xF1
#define SCR_READ_BYTE_BUFFER	0xF0
#define SCR_READ_BYTE		0xEF
#define SCR_WRITE_BITS_LITTERAL(b, v)	0xEE, (b), (v)
#define SCR_WRITE_BITS_BUFFER(v)	0xED, (v)
#define SCR_READ_BITS_BUFFER(v)		0xEC, (v)
#define SCR_READ_BITS(v)			0xEB, (v)
#define SCR_SET_ICSP_SPEED(us) 0xEA,(us)
#define SCR_SET_ICSP_DELAY_2(us) 0xEA,(us)
#define SCR_SET_PINS_2(dd, cd, dv, cv) 0xF3, (((cd)!=0) | (((dd)!=0)<<1) | (((cv)!=0)<<2) | (((dv)!=0)<<3))
#define SCR_GET_PINS        0xDC
#define SCR_LOOP_3(rel, cnt)    0xE9, rel, cnt
#define SCR_DELAY_2(sec)    ((sec)>0.0054528?0xE8:0xE7), (uint8_t)((sec)>0.0054528?(.999+(sec)/.00546):(.999+(sec)/.0000213))
// SPI
#define SCR_SET_AUX_2(ad, av)   0xCF, (((ad)!=0) | (((av)!=0)<<1))
#define SCR_SPI_SETUP_PINS_4    SCR_SET_PINS_2(1,0,0,0), SCR_SET_AUX_2(0,0)
#define SCR_SPI					0xC3
#define SCR_SPI_LIT_2(v)		0xC7, (v)
#define SPI_WR_BYTE_BUF			0xC6
#define SPI_RD_BYTE_BUF			0xC5
#define SCR_SPI_RDWR_BYTE_LIT(v)	0xC4, (v)
#define SCR_SPI_RDWR_BYTE_BUF	0xC3
// I2C
#define SCR_I2C_START			0xCD
#define SCR_I2C_STOP			0xCC
#define SCR_I2C_WR_BYTE_LIT(v)	0xCB, (v)
#define SCR_I2C_WR_BYTE_BUF		0xCA
#define SCR_I2C_RD_BYTE_ACK		0xC9
#define SCR_I2C_RD_BYTE_NACK	0xC8

// Control HVISP Pins: SCI, SII, SDI
#if 0
// 4.PGD=SII, 5.PGC=SCI, 6.AUX=SDI
#define	SCR_HVISP_SET_SDI(v)	SCR_SET_AUX_2(0, v)
#define	SCR_HVISP_SET_SII_SCI(sii, sci)	SCR_SET_PINS_2(0, 0, sii, sci)
// mode=0: SDI=0, SII=0, SCI=(v)
// mode=1: SDI=1, SII=0, SCI=(v)
// mode=2: SDI=0, SII=1, SCI=(v)
// mode=3: SDI=1, SII=1, SCI=(v)
#define SCR_HVISP_SET_PINS(mode) SCR_HVISP_SET_SII_SCI(mode & 2,0), SCR_HVISP_SET_SDI(mode & 1)
#define SCR_HVISP_SCI_PULSE(mode, v) SCR_HVISP_SET_SII_SCI(mode & 2, v)
#else
// 4.PGD=SII, 5.PGC=SDI, 6.AUX=SCI
#define	SCR_HVISP_SET_SCI(v)	SCR_SET_AUX_2(0, v)
#define	SCR_HVISP_SET_SII_sdi(sii, sdi)	SCR_SET_PINS_2(0, 0, sii, sdi)
// mode=0: SDI=0, SII=0, SCI=(v)
// mode=1: SDI=1, SII=0, SCI=(v)
// mode=2: SDI=0, SII=1, SCI=(v)
// mode=3: SDI=1, SII=1, SCI=(v)
#define SCR_HVISP_SET_PINS(mode) SCR_HVISP_SET_SII_sdi(mode & 2, mode & 1), SCR_HVISP_SET_SCI(0)
#define SCR_HVISP_SCI_PULSE(mode, v) SCR_HVISP_SET_SCI(v)
#endif

#define SDI_L	0
#define SDI_H	1
#define SII_L	0
#define SII_H	2
#define SCI_L	0
#define SCI_H	1
#define HVISP_POWER_UP	4
#define HVISP_POWER_DOWN	5
#define HVISP_PULSE_DELAY	0.00001

#define XMIT_STATUS_SUCCESSED_64	(64)
#define XMIT_STATUS_SUCCESSED_65	(65)
#define XMIT_STATUS_SUCCESSED_RECV_ZERO	(0)
#define XMIT_STATUS_ERROR_SEND	(-1)
#define XMIT_STATUS_ERROR_RECV	(-2)
#define XMIT_SEND_ONLY	false
#define XMIT_SEND_RECV	true

class pickit
{
public:
	pickit();
	~pickit();
private:
	hid_device * dev = nullptr;
	size_t report_position = 0;
	// USB hid_read
	int read(int timeout_msec = TIMEOUT, unsigned char* c = nullptr);
	// USB hid_write
	int write(unsigned char* c = nullptr);
	size_t script_length_position = 0;
public:
	int open(unsigned short vender_id = PICKIT3_VID, unsigned short product_id = PICKIT3_PID);
	void close();
	void clear_cmd(int val = 0);
	unsigned char report[65] = { 0 };
	size_t add_cmd(unsigned char cmd1);
	size_t add_cmd(unsigned char cmd1, unsigned char cmd2);
	size_t add_cmd(unsigned char cmd1, unsigned char cmd2, unsigned char cmd3);
	size_t add_cmd(unsigned char cmd1, unsigned char cmd2, unsigned char cmd3, unsigned char cmd4);
	size_t begin_script(int index = -1);
	size_t end_script(void);
	bool read_vol(float& vdd, float& vpp);
	unsigned int read_status();
	// mode: true = send/recv, false = only send
	int sendCmd(bool mode = XMIT_SEND_RECV);
	int recvValue();
	enum e_pickit {
		e_pk_OutOfReportSize,

	};

};



pickit::pickit()
{
	hid_init();
}


pickit::~pickit()
{
	close();
	hid_exit();
}


// USB hid_read
int pickit::read(int timeout_msec, unsigned char* c)
{
	int xferred = 0;
	if ( c == nullptr)
		xferred = hid_read_timeout(dev, report, BUF_SIZE, timeout_msec);
	else
		xferred = hid_read_timeout(dev, c, BUF_SIZE, timeout_msec);
	if (xferred >= 0)
		printf("success: interrupt read %d bytes\n", xferred);
	else {
		printf("error: interrupt read %d bytes\n", xferred);
		xferred = XMIT_STATUS_ERROR_RECV;
	}
	return xferred;
}


// USB hid_write
int pickit::write(unsigned char* c)
{
	int xferred = 0;
	if (c == nullptr)
		xferred = hid_write(dev, report, BUF_SIZE);
	else
		xferred = hid_write(dev, c, BUF_SIZE);
	if (xferred > 0)
		printf("success: interrupt write %d bytes\n", xferred);
	else{
		printf("error: interrupt write %d bytes\n", xferred);
		xferred = XMIT_STATUS_ERROR_SEND;
	}
	return xferred;
}


int pickit::open(unsigned short vender_id, unsigned short product_id)
{
	dev = hid_open(vender_id, product_id, nullptr);
	if (dev)
		hid_set_nonblocking(dev, 1);
	return (dev != nullptr ? 1: 0);
}

void pickit::close()
{
	if (dev)
		hid_close(dev);
	dev = nullptr;
}


void pickit::clear_cmd(int val)
{
	memset(report, val, sizeof(report));
	report_position = 0;
	script_length_position = 0;
}

size_t pickit::add_cmd(unsigned char cmd1)
{
	if (report_position > (BUF_SIZE - 1))
		throw(e_pk_OutOfReportSize);
	report[report_position++] = cmd1;
	return report_position;
}

size_t pickit::add_cmd(unsigned char cmd1, unsigned char cmd2)
{
	if (report_position > (BUF_SIZE - 2))
		throw(e_pk_OutOfReportSize);
	report[report_position++] = cmd1;
	report[report_position++] = cmd2;
	return report_position;
}

size_t pickit::add_cmd(unsigned char cmd1, unsigned char cmd2, unsigned char cmd3)
{
	if (report_position > (BUF_SIZE - 3))
		throw(e_pk_OutOfReportSize);
	report[report_position++] = cmd1;
	report[report_position++] = cmd2;
	report[report_position++] = cmd3;
	return report_position;
}

size_t pickit::add_cmd(unsigned char cmd1, unsigned char cmd2, unsigned char cmd3, unsigned char cmd4)
{
	if (report_position > (BUF_SIZE - 4))
		throw(e_pk_OutOfReportSize);
	report[report_position++] = cmd1;
	report[report_position++] = cmd2;
	report[report_position++] = cmd3;
	report[report_position++] = cmd4;
	return report_position;
}

size_t pickit::begin_script(int index)
{
	if (index >= 0)
		script_length_position = add_cmd(CMD_DOWNLOAD_SCRIPT(index, 0)) - 1;
	else
		script_length_position = add_cmd(CMD_EXEC_SCRIPT_2(0)) - 1;
	return script_length_position;
}

size_t pickit::end_script(void)
{
	size_t script_length = 0;
	if (script_length_position > 0) 
	{
		script_length = report_position - script_length_position -1;
		report[script_length_position] = static_cast<unsigned char>(script_length);
	}
	return script_length;
}

bool pickit::read_vol(float& vdd_, float& vpp_)
{
	bool ret = false;
	clear_cmd(CMD_END_OF_BUFFER);
	add_cmd(0);
	add_cmd(CMD_READ_VDD_VPP);
	if ( write() > 0 )
	{
		clear_cmd();
		if ( read() > 0 )
		{
			vdd_ = (float)(report[0] * 256 + report[1]) * 0.125F; //pk3
			vpp_ = (float)(report[2] * 256 + report[3] ) * 0.125F; //pk3
			ret = true;
		}
	}
	return ret;
}


unsigned int pickit::read_status()
{
	unsigned int ret = 0;
	clear_cmd(CMD_END_OF_BUFFER);
	add_cmd(0);
	add_cmd(CMD_READ_STATUS);
	if (write() > 0)
	{
		clear_cmd();
		if (read() > 0)
		{
			ret = report[1] | (report[2] << 8);
		}
	}
	return ret;
}

// mode true = write->read , false = write only
int pickit::sendCmd(bool mode)
{
	int ret = XMIT_STATUS_ERROR_SEND;
	if (( ret = write() ) > 0)
	{
		if (mode)
		{
			ret = recvValue();
		}
	}
	return ret;
}

int pickit::recvValue()
{
	clear_cmd(0);
	return read();
}
