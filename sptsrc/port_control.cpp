/************************************************************************
    > File Name: port_control.cpp
    > Author: Jintao Yang
    > Mail: 18608842770@163.com 
    > Created Time: Sun Feb 23 19:22:50 2020
 ************************************************************************/

/* 
* MIT License
* 
* Copyright (c) 2020 Joker2770
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/


#include "port_control.h"

int AUX_split_str(string strSrc, STRVECTOR& vecDest, char cSep)
{
	//Current 'cSep' position and previous 'cSep' position. 
	STRPOS pos = 0;
	STRPOS prev_pos = 0;
	//Search by turn. 
	while ((pos = strSrc.find_first_of(cSep, pos)) != string::npos)
	{
		string strTemp = strSrc.substr(prev_pos, pos - prev_pos);
		vecDest.push_back(strTemp);
		prev_pos = ++pos;
	}
	//Even after 'cSep' is NULL, as a NULL string. 
	if (!strSrc.empty())
		vecDest.push_back(&strSrc[prev_pos]);
	//Return the count of the string. 
	return vecDest.size();
}

void HexToAscii(unsigned char *pSrc, unsigned char *pDest, unsigned int nLen)
{
	unsigned char Nibble[2];
	unsigned int i, j;
	for (i = 0; i < nLen; i++) {
		Nibble[0] = (pSrc[i] & 0xF0) >> 4;
		Nibble[1] = pSrc[i] & 0x0F;
		for (j = 0; j < 2; j++) {
			if (Nibble[j] < 10) {
				Nibble[j] += 0x30;
			}
			else {
				if (Nibble[j] < 16)
					Nibble[j] = Nibble[j] - 10 + 'A';
			}
			*pDest++ = Nibble[j];
		}               // for (int j = ...)
	}           // for (int i = ...)
}

int StringToHex(char *pSrc, unsigned char *pDest, unsigned int *uilen)
{
	char *p = pSrc;
	char high = 0, low = 0;
	int tmplen = strlen(p), cnt = 0;
	tmplen = strlen(p);
	while (cnt < (tmplen / 2))
	{
		high = ((*p > '9') && ((*p <= 'F') || (*p <= 'f'))) ? *p - 48 - 7 : *p - 48;
		low = (*(++p) > '9' && ((*p <= 'F') || (*p <= 'f'))) ? *(p)-48 - 7 : *(p)-48;
		pDest[cnt] = ((high & 0x0f) << 4 | (low & 0x0f));
		p++;
		cnt++;
	}
	if (tmplen % 2 != 0) pDest[cnt] = ((*p > '9') && ((*p <= 'F') || (*p <= 'f'))) ? *p - 48 - 7 : *p - 48;

	if (uilen != NULL) *uilen = tmplen / 2 + tmplen % 2;
	return tmplen / 2 + tmplen % 2;
}

my_serial_ctrl::my_serial_ctrl()
{
	this->m_serial = new serial::Serial();
	serial::Timeout timeout = serial::Timeout::simpleTimeout(1000);
	this->m_serial->setTimeout(timeout);
}

my_serial_ctrl::~my_serial_ctrl()
{
	if (nullptr != this->m_serial) delete this->m_serial;
}

// void my_serial_ctrl::my_sleep(unsigned long milliseconds) {
// #ifdef _WIN32
// 	Sleep(milliseconds); // 100 ms
// #else
// 	usleep(milliseconds * 1000); // 100 ms
// #endif
// }

int my_stricmp(const char* str1, const char* str2) {
#ifdef _WIN32
	return _stricmp(str1, str2); // 100 ms
#else
	return strcasecmp(str1, str2); // 100 ms
#endif
}


void my_serial_ctrl::enumerate_ports()
{
	vector<serial::PortInfo> devices_found = serial::list_ports();

	vector<serial::PortInfo>::iterator iter = devices_found.begin();

	int i = 0;
	while (iter != devices_found.end())
	{
		serial::PortInfo device = *iter++;
		i++; 
		printf("%d. Port - <%s>\n\tDescription: %s\n\tHardware_id: %s\n\n", i, device.port.c_str(), device.description.c_str(), device.hardware_id.c_str());
	}
	printf("Total %d ports could be connect. \n", i);
}

int my_serial_ctrl::open_port()
{
	char szPort[128] = "";
	memset(szPort, 0, sizeof(szPort));
	memcpy(szPort, this->m_serial->getPort().c_str(), 128);
	if (0 == my_stricmp(szPort, ""))
	{
		printf("PORT can not be NULL! \n");
		return -1;
	}
	if (!m_serial->isOpen())
	{
		printf("[%s] is not open! \n", szPort);

		try {
			this->m_serial->open();
			printf("Open ...\n");
		}
		catch (exception &e) {
			printf("Unhandled Exception: %s\n", e.what());
		}

		if (!this->m_serial->isOpen())
		{
			printf("[%s] open failed!\n", szPort);
			return -1;
		}
	}
	printf("[%s] open succeed!\n", szPort);
	return 0;
}

int my_serial_ctrl::close_port()
{
	if (this->m_serial->isOpen())
	{
		printf("Port is open!\n");

		try {
			this->m_serial->close();
			printf("Close ...\n");
		}
		catch (exception &e) {
			printf("Unhandled Exception: %s\n", e.what());
		}

		if (this->m_serial->isOpen())
		{
			printf("Close port failed!\n");
			return -1;
		}
	}
	printf("Close port succeed!\n");
	return 0;
}

void my_serial_ctrl::show_port_set()
{
	printf("---------------------------------------------\n");
	printf("\tPORT: %s\n\tBAUDRATE: %u\n\tTIMEOUT: %u, %u, %u, %u, %u\n\tBYTESIZE: %d\n\tPARITY: %d\n\tSTOPBITS: %d\n\tFLOWCONTROL: %d\n",
		this->m_serial->getPort().c_str(),
		this->m_serial->getBaudrate(),
		//Number of milliseconds between bytes received to timeout on. 
		this->m_serial->getTimeout().inter_byte_timeout,
		//A constant number of milliseconds to wait after calling read. 
		this->m_serial->getTimeout().read_timeout_constant,
		//A multiplier against the number of requested bytes to wait after calling read. 
		this->m_serial->getTimeout().read_timeout_multiplier,
		//A constant number of milliseconds to wait after calling write. 
		this->m_serial->getTimeout().write_timeout_constant,
		//A multiplier against the number of requested bytes to wait after calling write. 
		this->m_serial->getTimeout().write_timeout_multiplier,
		this->m_serial->getBytesize(),
		this->m_serial->getParity(),
		this->m_serial->getStopbits(),
		this->m_serial->getFlowcontrol()
		//true == this->m_serial->getCTS()?1:0,
		//true == this->m_serial->getDSR() ? 1 : 0,
		//true == this->m_serial->getCD() ? 1 : 0,
		//true == this->m_serial->getRI() ? 1 : 0
	);
	printf("---------------------------------------------\n");
}

int my_serial_ctrl::port_set(const char* szCommend, const char* szPara)
{
	if (nullptr == szCommend || nullptr == szPara || 0 == my_stricmp(szCommend, "") || 0 == my_stricmp(szPara, ""))
	{
		return -1;
	}
	if (0 == my_stricmp(szCommend, "SETPORT")) this->m_serial->setPort(std::string(szPara));
	else if (0 == my_stricmp(szCommend, "SETBAUDRATE")) this->m_serial->setBaudrate(atol(szPara));
	else if (0 == my_stricmp(szCommend, "SETTIMEOUT"))
	{
		STRVECTOR vDes;
		AUX_split_str(string(szPara), vDes, ',');
		if (0 == my_stricmp(szPara, "") || 5 != vDes.size())
		{
			printf("e.g.: SETTIMEOUT:10000,250,0,250,0\n");
			return -1;
		}
		this->m_serial->setTimeout(atol(vDes[0].c_str()), atol(vDes[1].c_str()), atol(vDes[2].c_str()), atol(vDes[3].c_str()), atol(vDes[4].c_str()));
	}
	else if (0 == my_stricmp(szCommend, "SETBYTESIZE"))
	{
		if (5 > atoi(szPara) || 8 < atoi(szPara)) return -1;
		this->m_serial->setBytesize((serial::bytesize_t)atoi(szPara));
	}
	else if (0 == my_stricmp(szCommend, "SETPARITY"))
	{
		if (0 > atoi(szPara) || 4 < atoi(szPara)) return -1;
		this->m_serial->setParity((serial::parity_t)atoi(szPara));
	}
	else if (0 == my_stricmp(szCommend, "SETSTOPBITS"))
	{
		if (1 > atoi(szPara) || 3 < atoi(szPara)) return -1;
		this->m_serial->setStopbits((serial::stopbits_t)atoi(szPara));
	}
	else if (0 == my_stricmp(szCommend, "SETFLOWCONTROL"))
	{
		if (0 > atoi(szPara) || 2 < atoi(szPara)) return -1;
		this->m_serial->setFlowcontrol((serial::flowcontrol_t)atoi(szPara));
	}
	else if (0 == my_stricmp(szCommend, "SETRTS"))
	{
		if (0 == atoi(szPara))
			this->m_serial->setRTS(false);
		else
			this->m_serial->setRTS(true);
	}
	else if (0 == my_stricmp(szCommend, "SETDTR"))
	{
		if (0 == atoi(szPara))
			this->m_serial->setDTR(false);
		else
			this->m_serial->setDTR(true);
	}
	else if (0 == my_stricmp(szCommend, "SETCD"))
	{
		if (0 == atoi(szPara))
			this->m_serial->setBreak(false);
		else
			this->m_serial->setBreak(true);
	}
	else
	{
	}

	return 0;
}

int my_serial_ctrl::send_data(const char* szData, bool b_hex)
{
	try
	{
		char szDest[1024 * 100] = "";
		memset(szDest, 0, sizeof(szDest));
		uint32_t ilen = 0;
		if (b_hex)
		{
			StringToHex((char*)szData, (unsigned char*)szDest, &ilen);
			size_t bytes_wrote = this->m_serial->write(szDest);
			char szTmp[1024 * 100] = "";
			memset(szTmp, 0, sizeof(szTmp));
			memcpy(szTmp, szData, bytes_wrote);
			printf(">>(hex)%s\n", szTmp);
		}
		else
		{
			size_t bytes_wrote = this->m_serial->write(szData);
			char szTmp[1024 * 100] = "";
			memset(szTmp, 0, sizeof(szTmp));
			memcpy(szTmp, szData, bytes_wrote);
			printf(">>(visual)%s\n", szTmp);
		}
	}
	catch (exception &e) {
		printf("Unhandled Exception: %s\n", e.what());
	}

	return 0;
}

int my_serial_ctrl::receive_data(uint32_t ulength, bool b_hex)
{
	try
	{
		string result = this->m_serial->read(ulength);
		if (b_hex)
		{
			unsigned char szdest[1024 * 100] = "";
			memset(szdest, 0, sizeof(szdest));
			HexToAscii((unsigned char*)result.c_str(), szdest, result.length());
			printf("<<(hex)%s\n", szdest);
		}
		else
			printf("<<(visual)%s\n", result.c_str());
	}
	catch (exception &e) {
		printf("Unhandled Exception: %s\n", e.what());
	}

	return 0;
}