//���ݰ��ṹ��
#pragma pack(1) 

//	����֡ͷ�ṹ 14 �ֽ�
typedef struct
{
	BYTE desmac[6];		//	Ŀ��MAC��ַ
	BYTE srcmac[6];		//	ԴMAC��ַ
	USHORT ethertype;	//	֡����
}Dlc_Header;

//	Arp֡�ṹ
typedef struct
{
	USHORT hw_type;		//	Ӳ������Ethernet:0x1
	USHORT prot_type;	//	�ϲ�Э������IP:0x0800
	BYTE hw_addr_len;	//	Ӳ����ַ����:6
	BYTE prot_addr_len;	//	Э���ַ(IP��ַ)�ĳ���:4
	USHORT flag;		//	1��ʾ����,2��ʾӦ��
	BYTE send_hw_addr[6];	//	ԴMAC��ַ
	UINT send_prot_addr;	//	ԴIP��ַ
	BYTE targ_hw_addr[6];	//	Ŀ��MAC��ַ
	UINT targ_prot_addr;	//	Ŀ��IP��ַ
	BYTE padding[18];	//	�������  
}Arp_Frame;

//	ARP��=DLCͷ+ARP֡
typedef struct
{
	Dlc_Header dlcheader;	//DLCͷ
	Arp_Frame arpframe;	//ARP֡
}ARP_Packet;

//	IP��ͷ�ṹ
typedef struct
{
	BYTE  ver_len;       //IP��ͷ������,��λ��4�ֽ�
	BYTE  tos;           //��������TOS
	USHORT total_len;    //IP���ܳ��� 
	USHORT ident;        //��ʶ
	USHORT frag_and_flags;  //��־λ
	BYTE ttl;           //����ʱ��
	BYTE proto;         //Э��
	USHORT checksum;    //IP�ײ�У���
	UINT  sourceIP;  //ԴIP��ַ(32λ)
	UINT  destIP;    //Ŀ��IP��ַ(32λ)
}Ip_Header;

//	TCP��ͷ�ṹ
typedef struct {
	USHORT srcport;   // Դ�˿�
	USHORT dstport;   // Ŀ�Ķ˿�
	UINT seqnum;      // ˳���
	UINT acknum;      // ȷ�Ϻ�
	BYTE dataoff;     // TCPͷ��
	BYTE flags;       // ��־��URG��ACK�ȣ�
	USHORT window;    // ���ڴ�С
	USHORT chksum;    // У���
	USHORT urgptr;    // ����ָ��
}Tcp_Header;

//	TCPα�ײ� ���ڽ���TCPУ��͵ļ���,��֤TCPЧ�����Ч��
typedef struct
{
	ULONG  sourceip;    //ԴIP��ַ
	ULONG  destip;      //Ŀ��IP��ַ
	BYTE mbz;           //�ÿ�(0)
	BYTE ptcl;          //Э������(IPPROTO_TCP)
	USHORT tcpl;        //TCP�����ܳ���(��λ:�ֽ�)
}Tcp_Psd_Header;

//	UDP��ͷ
typedef struct
{
	USHORT srcport;     // Դ�˿�
	USHORT dstport;     // Ŀ�Ķ˿�
	USHORT total_len;   // ����UDP��ͷ��UDP���ݵĳ���(��λ:�ֽ�)
	USHORT chksum;      // У���
}Udp_Header;

//	UDPα�ײ�-�����ڼ���У���
typedef struct tsd_hdr 
{ 
	ULONG  sourceip;    //ԴIP��ַ
	ULONG  destip;      //Ŀ��IP��ַ
	BYTE  mbz;           //�ÿ�(0)
	BYTE  ptcl;          //Э������(IPPROTO_UDP)
	USHORT udpl;         //UDP���ܳ���(��λ:�ֽ�) 
}Udp_Psd_Header;

//	ICMP��ͷ
typedef struct
{
	BYTE i_type;     //���� �����ǹؼ�:0->����Ӧ��(PingӦ��) 8->��������(Ping����)
	BYTE i_code;     //���� ����������й� ������Ϊ0��8ʱ���ﶼ��0
	USHORT i_cksum;  //ICMP��У���
	USHORT i_id;     //ʶ���(һ���ý���ID��Ϊ��ʶ��)
	USHORT i_seq;    //�������к�(һ������Ϊ0)
	//UINT timestamp;  //ʱ���
	BYTE padding[32];//�������
}Icmp_Header;

//	ICMP���ݰ�
typedef struct
{
	Dlc_Header dlc_header;		//	��̫֡
	Ip_Header  ip_header;		//	IPͷ
	Icmp_Header icmp_header;	//	ICMP֡
}Icmp_Packet;

//	������Ϣ
typedef struct
{
	unsigned char flag;		//	�������ݰ�����1-arp,2-tcp,3-udp
	unsigned int srcip;		//	������IP
	unsigned char code[33];		//	����������
}Attack_Infor;

#pragma pack()





//-------------------------------------------------------------------------
// PacketCheckSum
// �������ݰ���У���
// ����:packet-����������(����װ�õ����ݰ���ָ��)
//-------------------------------------------------------------------------
void PacketCheckSum(unsigned char packet[])
{
	Dlc_Header * pdlc_header	= NULL;		//	��̫ͷָ��
	Ip_Header * pip_header		= NULL;		//	IPͷָ��
	unsigned short attachsize	= 0;		//	�����Э��ͷ�Լ��������ݵ��ܳ���

	pdlc_header	= (Dlc_Header *)packet;

	//	�ж� ethertype, ������� IP �����账��
	if ( ntohs( pdlc_header->ethertype ) != 0x0800 )
	{
		return;
	}

	pip_header	= (Ip_Header*)( packet + 14 );


	if ( 0x06 == pip_header->proto )
	{
		//
		//	TCP��
		//
		Tcp_Header * ptcp_header		= NULL; //TCPͷָ��
		Tcp_Psd_Header * ptcp_psd_header	= NULL;
		UINT uIpHeaderLen;
		UINT uTcpHeaderOffset;

		//	IPͷ����
		uIpHeaderLen		= ( ( pip_header->ver_len ) & 15 ) * 4;
		uTcpHeaderOffset	= 14 + uIpHeaderLen;
		ptcp_header		= (Tcp_Header*)( packet + uTcpHeaderOffset );

		//	�����Э��ͷ�Լ��������ݵ��ܳ���
		//	TCPͷ��+TCP������������
		attachsize		= ntohs( pip_header->total_len ) - uIpHeaderLen;
		ptcp_psd_header		= (Tcp_Psd_Header*)malloc( attachsize + sizeof(Tcp_Psd_Header) );

		if ( ptcp_psd_header )
		{
			memset( ptcp_psd_header, 0, attachsize + sizeof(Tcp_Psd_Header) );

			//	���α TCP ͷ
			ptcp_psd_header->destip		= pip_header->destIP;
			ptcp_psd_header->sourceip	= pip_header->sourceIP;
			ptcp_psd_header->mbz		= 0;
			ptcp_psd_header->ptcl		= 0x06;
			ptcp_psd_header->tcpl		= htons( attachsize );

			//	����TCPУ���
			ptcp_header->chksum		= 0;
			memcpy( (unsigned char*)ptcp_psd_header + sizeof(Tcp_Psd_Header), (unsigned char*)ptcp_header, attachsize );
			ptcp_header->chksum		= checksum( (unsigned short *)ptcp_psd_header, attachsize+sizeof(Tcp_Psd_Header) );

			//	����ipͷ��У���
			pip_header->checksum		= 0;
			pip_header->checksum		= checksum( (unsigned short *)pip_header, 20 );
		}
	}
	else if ( 0x11 == pip_header->proto )
	{
		//
		//	UDP��
		//
		Udp_Header * pudp_header		= NULL;	//	UDPͷָ��
		Udp_Psd_Header * pudp_psd_header	= NULL;
		pudp_header	= (Udp_Header*)( packet + 14 + ((pip_header->ver_len)&15 ) * 4 );
		attachsize	= ntohs( pip_header->total_len ) - ( ( pip_header->ver_len ) & 15 ) * 4;
		pudp_psd_header	= (Udp_Psd_Header*)malloc( attachsize+sizeof(Udp_Psd_Header) );
		if ( pudp_psd_header )
		{
			memset( pudp_psd_header, 0, attachsize+sizeof(Udp_Psd_Header) );
			
			//	���αUDPͷ
			pudp_psd_header->destip		= pip_header->destIP;
			pudp_psd_header->sourceip	= pip_header->sourceIP;
			pudp_psd_header->mbz		= 0;
			pudp_psd_header->ptcl		= 0x11;
			pudp_psd_header->udpl		= htons(attachsize);

			//	����UDPУ���
			pudp_header->chksum		= 0;
			memcpy( (unsigned char *)pudp_psd_header+sizeof(Udp_Psd_Header), (unsigned char *)pudp_header, attachsize );
			pudp_header->chksum		= checksum( (unsigned short *)pudp_psd_header, attachsize + sizeof(Udp_Psd_Header) );

			//	����ipͷ��У���
			pip_header->checksum		= 0;
			pip_header->checksum		= checksum( (unsigned short *)pip_header, 20 );
		}
	}

	return;
}