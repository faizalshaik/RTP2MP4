// rtp2mp4.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// Crtp2mp4App:
// See rtp2mp4.cpp for the implementation of this class
//

class Crtp2mp4App : public CWinApp
{
public:
	Crtp2mp4App();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern Crtp2mp4App theApp;


#pragma pack(push)
#pragma pack(1)
typedef struct pcap_file_head
{
	int	  magic;
	short minor_version,magor_version;
	int	  timezone;
	int	  sigflags;
	int	  snaplen; 
	int	  linktype;
}PCAP_FILE_HDR, *PPCAP_FILE_HDR;

typedef struct pcap_data_head
{
	int timestamp_ms;
	int timestamp_us;
	int pack_length;
	int real_length;
}PCAP_DATA_HDR, *PPCAP_DATA_HDR;


#define	ETHERTYPE_PUP	0x0200		/* PUP protocol */
#define	ETHERTYPE_IP	0x0800		/* IP protocol */
#define ETHERTYPE_ARP	0x0806		/* Addr. resolution protocol */

typedef struct	ether_header {
	BYTE	ether_dhost[6];
	BYTE	ether_shost[6];
	WORD	ether_type;
}ETHER_HDR,*PETHER_HDR;

#define IP_PROT_UDP	17
typedef struct ip_header{
	BYTE	ip_hl:4,		/* header length */
			ip_v:4;			/* version */
	BYTE	ip_tos;			/* type of service */
	WORD	ip_len;			/* total length */
	WORD	ip_id;			/* identification */
	SHORT	ip_off;			/* fragment offset field */
#define	IP_DF 0x4000			/* dont fragment flag */
#define	IP_MF 0x2000			/* more fragments flag */
	BYTE	ip_ttl;			/* time to live */
	BYTE	ip_p;			/* protocol */
	WORD	ip_sum;			/* checksum */
	DWORD   ip_src;
	DWORD   ip_dst;
}IP_HDR,*PIP_HDR ;

typedef struct udp_header{
	WORD    srcPort;
	WORD    dstPort;
	WORD    length;
	WORD    checksum;
}UDP_HDR, *PUDP_HDR; 


typedef struct rtpHeader {
	//For big endian
	unsigned char cc:4,
				  extension:1,
				  padding:1,
				  version:2;       // Version, currently 2

	unsigned char payload:7,       // Marker bit
				  marker:1;        // Payload type
	unsigned short sequence;       // sequence number
	unsigned int  timestamp;       //  timestamp
	unsigned int  sources;         // contributing sources
}RTP_HDR, *PRTP_HDR;

#pragma pack(pop)
