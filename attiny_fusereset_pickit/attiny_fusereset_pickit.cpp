#include "stdafx.h"
//#include <stdio.h>
#include <stdint.h>
//#include <hidapi/hidapi.h>
#include <assert.h>
#include "pickit.h"


//////////////////////////////////////////////////////////////////////////////

int pickit_write_report(hid_device * dev, unsigned char report[65])
{
	// endpoint 1 OUT??
	int xferred = 0;
	xferred = hid_write(dev, report, BUF_SIZE);
	printf("success: interrupt write %d bytes\n", xferred);
	return xferred;
}

int pickit_read_report(hid_device * dev, unsigned char report[65])
{
	// endpoint 1 IN??
	int xferred = 0;
	xferred = hid_read_timeout(dev, report, BUF_SIZE, TIMEOUT);
	printf("success: interrupt read %d bytes\n", xferred);
	return xferred;
}

hid_device* pickit_open(void)
{
	return hid_open(PICKIT3_VID, PICKIT3_PID, NULL);
}

static void pickit_close(hid_device* dev)
{
	hid_close(dev);
}

int GetVersions_MPLAB(hid_device* dev)
{
	unsigned char report[65] = { 0, CMD_GET_VERSION_MPLAB, 0, CMD_END_OF_BUFFER };
	report[64] = report[63] = report[62] = 0;
	report[61] = 2;
	printf("GetVersion MPLAB\n");
	if (pickit_write_report(dev, report) >= 0)
	{
		printf("success: GetVersion MPLAB\n");
		unsigned char report[65] = { 0 };
		if (pickit_read_report(dev, report) >= 0)
		{
			printf("%s: %s version %d.%d.%d.%d\n", "GetVersion", "MPLAB OS", (int)report[7], (int)report[8], (int)report[9], (int)report[10]);
			printf("%s: %s version %d.%d.%d.%d\n", "GetVersion", "MPLAB AP", (int)report[11], (int)report[12], (int)report[13], (int)report[14]);
			int magicKey;
			magicKey = report[31] + (report[32] << 8) + (report[33] << 16);
			printf("%s: %s = %d\n", "GetVersion", "MPLAB MagicKey", magicKey);
		}
		else {
			printf("error: GetVersion MPLAB usb read\n");
		}
	}
	else {
		printf("error: GetVersion MPLAB usb write\n");
	}
	return 0;
}

void genProgramBits(uint8_t sdi, uint8_t sii, uint8_t* bits)
{
	bits[0] = bits[1] = bits[10] = 0;
	for (int ii = 7; ii >= 0; ii--)
	{
		bits[ii+2] = ((sdi & (1 << ii))? SDI_H :0 )+((sii & (1 << ii)) ? SII_H : 0);
	}
}

int addPickitScript(pickit& pk, uint8_t* bits, size_t len)
{
	int ret = 0;
	uint8_t c;
	int rle = 0;
	for (int ii = static_cast<int>(len) - 1; ii >= 0; ii--)
	{
		if (ii == ( len -1 ) )
		{
			c = bits[ii];
			rle = 1;
		}
		else if (c == bits[ii])
		{
			rle++;
		}
		if (c != bits[ii])
		{
			ret++;
			pk.add_cmd(CMD_RUN_SCRIPT(c, rle));
			c = bits[ii];
			rle = 1;
		}
	}
	pk.add_cmd(CMD_RUN_SCRIPT(c, rle));
	return ++ret;
}

enum pickitScriptType {
	GEN_PK_SCR_WRITE_HIGH_FUSE,
	GEN_PK_SCR_WRITE_LOW_FUSE,
	GEN_PK_SCR_WRITE_EXT_FUSE,
	GEN_PK_SCR_READ_HIGH_FUSE,
	GEN_PK_SCR_READ_LOW_FUSE,
	GEN_PK_SCR_READ_EXT_FUSE,
	GEN_PK_SCR_READ_SIGN1,
	GEN_PK_SCR_READ_SIGN2,
} ;

int genPickitScript(pickit& pk, pickitScriptType pkType)
{
	int ret = 0;
	uint8_t cmd[11 * 4] = { 0 };
	int len = 0;

	switch (pkType)
	{
	case GEN_PK_SCR_WRITE_HIGH_FUSE:
		// Hight Fuse (0xDF)
		// SDI 0x40, 0xDF, 0x00, 0x00
		// SII 0x4C, 0x2C, 0x74, 0x7C
		len = 11 * 4;
		genProgramBits(0x40, 0x4c, &cmd[11 * 3]);
		genProgramBits(0xDF, 0x2c, &cmd[11 * 2]);
		genProgramBits(0x00, 0x74, &cmd[11 * 1]);
		genProgramBits(0x00, 0x7c, &cmd[11 * 0]);
		break;
	case GEN_PK_SCR_WRITE_LOW_FUSE:
		// Low Fuse (0x62)
		// SDI 0x40, 0x62, 0x00, 0x00
		// SII 0x4C, 0x2C, 0x64, 0x6C
		len = 11 * 4;
		genProgramBits(0x40, 0x4c, &cmd[11 * 3]);
		genProgramBits(0x62, 0x2c, &cmd[11 * 2]);
		genProgramBits(0x00, 0x64, &cmd[11 * 1]);
		genProgramBits(0x00, 0x6c, &cmd[11 * 0]);
		break;
	case GEN_PK_SCR_WRITE_EXT_FUSE:
		// Ext Fuse (0xFF)
		// SDI 0x40, 0xFF, 0x00, 0x00
		// SII 0x4C, 0x2C, 0x66, 0x6E
		len = 11 * 4;
		genProgramBits(0x40, 0x4c, &cmd[11 * 3]);
		genProgramBits(0xff, 0x2c, &cmd[11 * 2]);
		genProgramBits(0x00, 0x66, &cmd[11 * 1]);
		genProgramBits(0x00, 0x6e, &cmd[11 * 0]);
		break;
	case GEN_PK_SCR_READ_HIGH_FUSE:
		// High Fuse
		// SDI 0x04, 0x00, 0x00
		// SII 0x4C, 0x7A, 0x7E
		len = 11 * 3;
		genProgramBits(0x04, 0x4c, &cmd[11 * 2]);
		genProgramBits(0x00, 0x7a, &cmd[11 * 1]);
		genProgramBits(0x00, 0x7e, &cmd[11 * 0]);
		break;
	case GEN_PK_SCR_READ_LOW_FUSE:
		// Low Fuse
		// SDI 0x04, 0x00, 0x00
		// SII 0x4C, 0x68, 0x6C
		len = 11 * 3;
		genProgramBits(0x04, 0x4c, &cmd[11 * 2]);
		genProgramBits(0x00, 0x68, &cmd[11 * 1]);
		genProgramBits(0x00, 0x6c, &cmd[11 * 0]);
		break;
	case GEN_PK_SCR_READ_EXT_FUSE:
		// Ext Fuse
		// SDI 0x04, 0x00, 0x00
		// SII 0x4C, 0x6A, 0x6E
		len = 11 * 3;
		genProgramBits(0x04, 0x4c, &cmd[11 * 2]);
		genProgramBits(0x00, 0x6a, &cmd[11 * 1]);
		genProgramBits(0x00, 0x6e, &cmd[11 * 0]);
		break;
	case GEN_PK_SCR_READ_SIGN1:
		// READ Signature1
		// SDI 0x08, 0x01, 0x00, 0x00
		// SII 0x4C, 0x0C, 0x68, 0x6C
		len = 11 * 4;
		genProgramBits(0x08, 0x4c, &cmd[11 * 3]);
		genProgramBits(0x01, 0x0c, &cmd[11 * 2]);
		genProgramBits(0x00, 0x68, &cmd[11 * 1]);
		genProgramBits(0x00, 0x6c, &cmd[11 * 0]);
		break;
	case GEN_PK_SCR_READ_SIGN2:
		// READ Signature2
		// SDI 0x08, 0x02, 0x00, 0x00
		// SII 0x4C, 0x0C, 0x68, 0x6C
		len = 11 * 4;
		genProgramBits(0x08, 0x4c, &cmd[11 * 3]);
		genProgramBits(0x02, 0x0c, &cmd[11 * 2]);
		genProgramBits(0x00, 0x68, &cmd[11 * 1]);
		genProgramBits(0x00, 0x6c, &cmd[11 * 0]);
		break;
	default:
		break;
	}
	if (len)
		ret = addPickitScript(pk, cmd, len);
	return ret;
}

int main(void)
{
	hid_device *dev = NULL; /* the device handle */
	pickit pk3;
	if (pk3.open()) {
		pk3.clear_cmd(CMD_END_OF_BUFFER);
		pk3.add_cmd(0, CMD_GET_VERSION_MPLAB, 0);
		if (pk3.sendCmd())
		{
			if (pk3.report[30]=='P' && pk3.report[31]=='k' && pk3.report[32]=='3')
			{
				/*
				report[30] = 'P';
				report[31] = 'k';
				report[32] = '3';

				report[33] = gblSysVersion.MAJOR;
				report[34] = gblSysVersion.MINOR;
				report[35] = gblSysVersion.REV;
				*/
				printf("success: Pickit2 compatible mode\n");
			}
			else
			{
				printf("success: Pickit3 MPLAB mode\n");
			}
			size_t cmd_size = 0;
			pk3.clear_cmd(CMD_END_OF_BUFFER);
			pk3.add_cmd(0);
			pk3.add_cmd(CMD_SET_VDD_PK3(5));  // Vdd  +5V
			pk3.add_cmd(CMD_SET_VPP_PK3(12)); // Vpp +12V

			pk3.add_cmd(CMD_CLR_DLOAD_BUFF);
			pk3.add_cmd(CMD_CLR_ULOAD_BUFF);
			pk3.add_cmd(CMD_CLR_SCRIPT_BUFF);
			cmd_size = pk3.add_cmd(CMD_END_OF_BUFFER);
			printf("CMD size %d\n", (int)cmd_size);
			if (pk3.sendCmd() >= 0)
			{
				printf("Setup done\n");
			}

			// download script 0: SDI_L + SII_L
			pk3.clear_cmd(CMD_END_OF_BUFFER);
			pk3.add_cmd(0);
			// 4.PGD=SII, 5.PGC=SCI, 6.AUX=SDI
			pk3.begin_script(SDI_L + SII_L);
			pk3.add_cmd(SCR_BUSY_LED_ON);
			// SDI=0, SSI=0
			pk3.add_cmd(SCR_HVISP_SET_PINS(SDI_L + SII_L));
			// SCI Pulse
			pk3.add_cmd(SCR_DELAY_2(HVISP_PULSE_DELAY));
			pk3.add_cmd(SCR_HVISP_SCI_PULSE(SDI_L + SII_L, SCI_H));
			pk3.add_cmd(SCR_DELAY_2(HVISP_PULSE_DELAY));
			pk3.add_cmd(SCR_HVISP_SCI_PULSE(SDI_L + SII_L, SCI_L));
			pk3.add_cmd(SCR_BUSY_LED_OFF);
			pk3.end_script();
			cmd_size = pk3.add_cmd(CMD_END_OF_BUFFER);
			printf("Download script 0 size %d\n", (int)cmd_size);
			if (pk3.sendCmd(XMIT_SEND_ONLY) >= 0)
			{
				printf("success: script 0\n");
			}

			// download script 1: SDI_H + SII_L
			pk3.clear_cmd(CMD_END_OF_BUFFER);
			pk3.add_cmd(0);
			pk3.begin_script(SDI_H + SII_L);
			pk3.add_cmd(SCR_BUSY_LED_ON);
			// SDI=1, SSI=0
			pk3.add_cmd(SCR_HVISP_SET_PINS(SDI_H + SII_L));
			// SCI Pulse
			pk3.add_cmd(SCR_DELAY_2(HVISP_PULSE_DELAY));
			pk3.add_cmd(SCR_HVISP_SCI_PULSE(SDI_H + SII_L, SCI_H));
			pk3.add_cmd(SCR_DELAY_2(HVISP_PULSE_DELAY));
			pk3.add_cmd(SCR_HVISP_SCI_PULSE(SDI_H + SII_L, SCI_L));
			pk3.add_cmd(SCR_BUSY_LED_OFF);
			pk3.end_script();
			cmd_size = pk3.add_cmd(CMD_END_OF_BUFFER);
			printf("Download script 1 size %d\n", (int)cmd_size);
			if (pk3.sendCmd(XMIT_SEND_ONLY) >= 0)
			{
				printf("success: script 1\n");
			}

			// download script 2: SDI_L + SII_H
			pk3.clear_cmd(CMD_END_OF_BUFFER);
			pk3.add_cmd(0);
			pk3.begin_script(SDI_L + SII_H);
			pk3.add_cmd(SCR_BUSY_LED_ON);
			// SDI=0, SSI=1
			pk3.add_cmd(SCR_HVISP_SET_PINS(SDI_L + SII_H));
			// SCI Pulse
			pk3.add_cmd(SCR_DELAY_2(HVISP_PULSE_DELAY));
			pk3.add_cmd(SCR_HVISP_SCI_PULSE(SDI_L + SII_H, SCI_H));
			pk3.add_cmd(SCR_DELAY_2(HVISP_PULSE_DELAY));
			pk3.add_cmd(SCR_HVISP_SCI_PULSE(SDI_L + SII_H, SCI_L));
			pk3.add_cmd(SCR_BUSY_LED_OFF);
			pk3.end_script();
			cmd_size = pk3.add_cmd(CMD_END_OF_BUFFER);
			printf("Download script 2 size %d\n", (int)cmd_size);
			if (pk3.sendCmd(XMIT_SEND_ONLY) >= 0)
			{
				printf("success: script 2\n");
			}

			// download script 3: SDI_H + SII_H
			pk3.clear_cmd(CMD_END_OF_BUFFER);
			pk3.add_cmd(0);
			pk3.begin_script(SDI_H + SII_H);
			pk3.add_cmd(SCR_BUSY_LED_ON);
			// SDI=1, SSI=1
			pk3.add_cmd(SCR_HVISP_SET_PINS(SDI_H + SII_H));
			// SCI Pulse
			pk3.add_cmd(SCR_DELAY_2(HVISP_PULSE_DELAY));
			pk3.add_cmd(SCR_HVISP_SCI_PULSE(SDI_H + SII_H, SCI_H));
			pk3.add_cmd(SCR_DELAY_2(HVISP_PULSE_DELAY));
			pk3.add_cmd(SCR_HVISP_SCI_PULSE(SDI_H + SII_H, SCI_L));
			pk3.add_cmd(SCR_BUSY_LED_OFF);
			pk3.end_script();

			// Power up script
			pk3.begin_script(HVISP_POWER_UP);
			pk3.add_cmd(SCR_BUSY_LED_ON);
			pk3.add_cmd(SCR_VDD_OFF);
			// SDI=0, SII=0, SCI=0
			pk3.add_cmd(SCR_HVISP_SET_PINS(SDI_L + SII_L));
			// MCLR=0
			pk3.add_cmd(SCR_VPP_OFF);
			pk3.add_cmd(SCR_MCLR_GND_ON);

			pk3.add_cmd(SCR_MCLR_GND_OFF);
			pk3.add_cmd(SCR_VPP_PWM_ON);
			pk3.add_cmd(SCR_VDD_ON);
			pk3.add_cmd(SCR_DELAY_2(0.00001));
			pk3.add_cmd(SCR_VPP_ON);
			//pk3.add_cmd(SCR_DELAY_2(0.001));
			pk3.add_cmd(SCR_DELAY_2(0.0001));
			//pk3.add_cmd(SCR_DELAY_2(0.004));
			pk3.end_script();

			// Power down script
			pk3.begin_script(HVISP_POWER_DOWN);
			pk3.add_cmd(SCR_HVISP_SET_PINS(SDI_L + SII_L));
			pk3.add_cmd(SCR_DELAY_2(0.0001));
			pk3.add_cmd(SCR_VPP_OFF);
			pk3.add_cmd(SCR_MCLR_GND_ON);
			pk3.add_cmd(SCR_VPP_PWM_OFF);
			pk3.add_cmd(SCR_DELAY_2(0.0001));
			pk3.add_cmd(SCR_VDD_OFF);
			pk3.add_cmd(SCR_DELAY_2(0.1));
			pk3.add_cmd(SCR_BUSY_LED_OFF);
			pk3.end_script();

			cmd_size = pk3.add_cmd(CMD_END_OF_BUFFER);

			printf("Download script 3 size %d\n", (int)cmd_size);
			if (pk3.sendCmd( XMIT_SEND_ONLY ) >= 0)
			{
				printf("success: script 3\n");
			}

#if 0
			// test mode
			/*
			0: pk3.add_cmd(CMD_RUN_SCRIPT(SDI_L + SII_L, 1)); // 0
			1: pk3.add_cmd(CMD_RUN_SCRIPT(SDI_H + SII_L, 1)); // 1
			2: pk3.add_cmd(CMD_RUN_SCRIPT(SDI_L + SII_H, 1)); // 2
			3: pk3.add_cmd(CMD_RUN_SCRIPT(SDI_H + SII_H, 1)); // 3
			*/
			/* 
			11bits x 4
			write Hight Fuse
			SDI 0 0100 0000 00 0 7654 3210 00 0 0000 0000 00 0 0000 0000 00
			SII 0 0100 1100 00 0 0010 1100 00 0 0111 0100 00 0 0111 1100 00
			    0 0300 2200 00 0 xxxx xxxx 00 0 0222 0200 00 0 0222 2200 00
			write Hight Fuse(0xDF)
			SDI 0 0100 0000 00 0 1101 1111 00 0 0000 0000 00 0 0000 0000 00
			SII 0 0100 1100 00 0 0010 1100 00 0 0111 0100 00 0 0111 1100 00
			    0 0300 2200 00 0 1121 3311 00 0 0222 0200 00 0 0222 2200 00
			*/
			pk3.clear_cmd(CMD_END_OF_BUFFER);
			pk3.add_cmd(0);
			pk3.add_cmd(CMD_RUN_SCRIPT(HVISP_POWER_UP, 1));
			// 0 0
			pk3.add_cmd(CMD_RUN_SCRIPT(SDI_L + SII_L, 2)); // 0-2
			// 3
			pk3.add_cmd(CMD_RUN_SCRIPT(SDI_H + SII_H, 1)); // 3-1
			// 0 0 
			pk3.add_cmd(CMD_RUN_SCRIPT(SDI_L + SII_L, 2)); // 0-2
			// 2 2
			pk3.add_cmd(CMD_RUN_SCRIPT(SDI_L + SII_H, 2)); // 2-2
			// 0 0 0 0 0
			pk3.add_cmd(CMD_RUN_SCRIPT(SDI_L + SII_L, 5)); // 0-5
			// 1 1
			pk3.add_cmd(CMD_RUN_SCRIPT(SDI_H + SII_L, 2)); // 1-2
			// 2
			pk3.add_cmd(CMD_RUN_SCRIPT(SDI_L + SII_H, 1)); // 2-1
			// 1
			pk3.add_cmd(CMD_RUN_SCRIPT(SDI_H + SII_L, 1)); // 1-1
			// 3 3
			pk3.add_cmd(CMD_RUN_SCRIPT(SDI_H + SII_H, 2)); // 3-2
			// 1 1
			pk3.add_cmd(CMD_RUN_SCRIPT(SDI_H + SII_L, 2)); // 1-2
			// 0 0 0 0
			pk3.add_cmd(CMD_RUN_SCRIPT(SDI_L + SII_L, 4)); // 0-4
			// 2 2 2
			pk3.add_cmd(CMD_RUN_SCRIPT(SDI_L + SII_H, 3)); // 2-3
			// 0
			pk3.add_cmd(CMD_RUN_SCRIPT(SDI_L + SII_L, 1)); // 0-1
			// 2
			pk3.add_cmd(CMD_RUN_SCRIPT(SDI_L + SII_H, 1)); // 2-1
			// 0 0 0 0 0 0
			pk3.add_cmd(CMD_RUN_SCRIPT(SDI_L + SII_L, 6)); // 0-6
			// 2 2 2 2 2
			pk3.add_cmd(CMD_RUN_SCRIPT(SDI_L + SII_H, 5)); // 2-5
			// 0 0 0 0
			pk3.add_cmd(CMD_RUN_SCRIPT(SDI_L + SII_L, 4)); // 0-4
			cmd_size = pk3.add_cmd(CMD_RUN_SCRIPT(HVISP_POWER_DOWN, 1));
			printf("wirte fuse size %d\n", cmd_size);
			if (pk3.sendCmd(XMIT_SEND_ONLY) >= 0)
			{
				printf("success: test mode write fuse\n");
			}
			


#endif
			pk3.clear_cmd(CMD_END_OF_BUFFER);
			pk3.add_cmd(0);
			pk3.add_cmd(CMD_RUN_SCRIPT(HVISP_POWER_UP, 1));			
			assert(cmd_size < 63);
			if (pk3.sendCmd(XMIT_SEND_ONLY) >= 0)
			{
				printf("success: HVISP start!\n");
			}
			
			pk3.clear_cmd(CMD_END_OF_BUFFER);
			pk3.add_cmd(0);
			cmd_size = genPickitScript(pk3, GEN_PK_SCR_WRITE_LOW_FUSE);
			assert(cmd_size < 63);
			if (pk3.sendCmd(XMIT_SEND_ONLY) >= 0)
			{
				printf("success: GEN_PK_SCR_WRITE_LOW_FUSE\n");
			}

			pk3.clear_cmd(CMD_END_OF_BUFFER);
			pk3.add_cmd(0);
			cmd_size = genPickitScript(pk3, GEN_PK_SCR_WRITE_HIGH_FUSE);
			assert(cmd_size < 63);
			if (pk3.sendCmd(XMIT_SEND_ONLY) >= 0)
			{
				printf("success: GEN_PK_SCR_WRITE_HIGH_FUSE\n");
			}

			pk3.clear_cmd(CMD_END_OF_BUFFER);
			pk3.add_cmd(0);
			cmd_size = genPickitScript(pk3, GEN_PK_SCR_READ_LOW_FUSE);
			assert(cmd_size < 63);
			if (pk3.sendCmd(XMIT_SEND_ONLY) >= 0)
			{
				printf("success: GEN_PK_SCR_READ_LOW_FUSE\n");
			}

			pk3.clear_cmd(CMD_END_OF_BUFFER);
			pk3.add_cmd(0);
			cmd_size = genPickitScript(pk3, GEN_PK_SCR_READ_HIGH_FUSE);
			assert(cmd_size < 63);
			if (pk3.sendCmd(XMIT_SEND_ONLY) >= 0)
			{
				printf("success: GEN_PK_SCR_READ_HIGH_FUSE\n");
			}			
			
			pk3.clear_cmd(CMD_END_OF_BUFFER);
			pk3.add_cmd(0);
			cmd_size = pk3.add_cmd(CMD_RUN_SCRIPT(HVISP_POWER_DOWN, 1));
			assert(cmd_size < 63);
			if (pk3.sendCmd(XMIT_SEND_ONLY) >= 0)
			{
				printf("success: HVISP end\n");
			}
		}
		else {
			printf("error: ");
		}
	}
	else
	{
		printf("error: opening device\n");
	}

	printf("Done.\n");

	return 0;
}
