// hexvisDlg.cpp : implementation file
//

#include "stdafx.h"
#include "hexvis.h"
#include "hexvisDlg.h"
#include <float.h>
#include "Winuser.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHexvisDlg dialog

CHexvisDlg::CHexvisDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHexvisDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHexvisDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_processed = false;
}

void CHexvisDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHexvisDlg)
	DDX_Control(pDX, IDC_STATE, m_status);
	DDX_Control(pDX, IDC_BIN_REP, m_binRep);
	DDX_Control(pDX, IDC_SEARCH_NUMBER, m_searchNumber);
	DDX_Control(pDX, IDC_OFFSET, m_offset);
	DDX_Control(pDX, IDC_UNIX_TIME, m_unix_time);
	DDX_Control(pDX, IDC_STRING, m_string);
	DDX_Control(pDX, IDC_SHORT_LE, m_short_le);
	DDX_Control(pDX, IDC_SHORT_BE, m_short_be);
	DDX_Control(pDX, IDC_REAL, m_real);
	DDX_Control(pDX, IDC_PACKET_BODY, m_packetBody);
	DDX_Control(pDX, IDC_LONG_LE, m_long_le);
	DDX_Control(pDX, IDC_LONG_BE, m_long_be);
	DDX_Control(pDX, IDC_INT64_LE, m_int64_le);
	DDX_Control(pDX, IDC_INT64_BE, m_int64_be);
	DDX_Control(pDX, IDC_BYTE, m_byte);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CHexvisDlg, CDialog)
	//{{AFX_MSG_MAP(CHexvisDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_PROCESS, OnProcess)
	ON_EN_UPDATE(IDC_PACKET_BODY, OnUpdatePacketBody)
	ON_NOTIFY(EN_SELCHANGE, IDC_PACKET_BODY, OnSelchangePacketBody)
	ON_BN_CLICKED(IDC_CLEAR, OnClear)
	ON_BN_CLICKED(IDC_FIND_NUMBER, OnFindNumber)
	ON_BN_CLICKED(IDC_LOAD_FILE, OnLoadFile)
	ON_BN_CLICKED(IDC_COPY_TO_CLIP, OnCopyToClip)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHexvisDlg message handlers

BOOL CHexvisDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	m_packetBody.SetEventMask(m_packetBody.GetEventMask() | ENM_UPDATE | ENM_SELCHANGE);
	
	CHARFORMAT cf;
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_FACE;
	_tcsncpy(cf.szFaceName, "Courier New", LF_FACESIZE);
	m_packetBody.SetDefaultCharFormat(cf);

	m_packetBody.parent = this;
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CHexvisDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CHexvisDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CHexvisDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CHexvisDlg::OnProcess() {
	CString packet;
	m_packetBody.GetWindowText(packet);
	
	m_offsetStart = -1;
	m_digits.clear();
	m_digits.reserve(packet.GetLength()/3);
	m_lineStarts.clear();

	m_processed = false;

	
	int linelen;
	const char *start = packet.GetBuffer(0);
	const char *cur = start;
	linelen = 0;
	while(*cur != '\0') {
		if(*cur == '\r' || *cur == '\n') {
			ProcessLine(start, linelen);
			cur++;
			if(*cur == '\n')
				cur++;
			start = cur;
			linelen = 0;
		} else {
			linelen++;
			cur++;
		}
	}
	if(linelen > 0)
		ProcessLine(start, linelen);

	DisplayDigits();

	m_processed = true;

	//this will trigger the sel change
	m_packetBody.SetSel(8, 9);

	m_status.SetWindowText("Press clear to process another block");
}

void CHexvisDlg::build_hex_line(const char *buffer, unsigned long length,  unsigned long offset, unsigned char padding)
{
	char out_buffer[128];
	char *ptr=out_buffer;
	int i;
	char printable[17];

	CHARFORMAT cf;
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR;
	cf.dwEffects = 0;

	m_packetBody.SetSel(-1, -1);
	sprintf(out_buffer,"%0*i:",padding, offset + m_offsetStart);
	cf.crTextColor = RGB(0,0,0);
	m_packetBody.SetSelectionCharFormat(cf);
	m_packetBody.ReplaceSel(out_buffer, false);

	m_lineStarts.push_back(m_packetBody.GetTextLength());
	//m_packetBody.SetSel(-1, -1);

	for(i=0;i<16; i++) {
		if (i==8) {
			strcpy(ptr," -");
			ptr+=2;
		}
		if (i+offset < length) {
			unsigned char c=*(const unsigned char *)(buffer+offset+i);
			ptr+=sprintf(ptr," %02x",c);
			printable[i]=isprint(c) ? c : '.';
		} else {
			ptr+=sprintf(ptr,"   ");
			printable[i]=0;
		}
	}

	//hex contents
	cf.crTextColor = RGB(0,155,0);
	m_packetBody.SetSelectionCharFormat(cf);
	m_packetBody.ReplaceSel(out_buffer, false);

	//ascii
	sprintf(out_buffer,"  | %.16s\r\n",printable);
	cf.crTextColor = RGB(0,0,155);
	m_packetBody.SetSelectionCharFormat(cf);
	m_packetBody.ReplaceSel(out_buffer, false);
}

void CHexvisDlg::DisplayDigits() {
	m_packetBody.SetSel(0, -1);
	m_packetBody.Clear();

	int len = m_digits.size();
	if(len == 0)
		return;

	const char *buf = (const char *) &m_digits[0];
	int offset;
	for(offset = 0; offset < len; offset += 16) {
		build_hex_line(buf, len, offset, 4);
	}

	/*
	CHARFORMAT cf;
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_FACE;
	_tcsncpy(cf.szFaceName, "Courier New", LF_FACESIZE);
	m_packetBody.SetSel(0, -1);
	m_packetBody.SetSelectionCharFormat(cf);*/

	
	/*CHARFORMAT cf;
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR;
	cf.dwEffects = 0;
	cf.crTextColor = RGB(0,0,155);
	m_packetBody.SetSel(0, -1);
	m_packetBody.SetSelectionCharFormat(cf);*/

	m_packetBody.SetSel(-1, -1);

	/*
	CHARFORMAT cf;
	cf.cbSize = sizeof(cf);
	
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_BOLD|CFM_COLOR|CFM_ITALIC|CFM_STRIKEOUT|CFM_UNDERLINE;
	cf.dwEffects = 0;
//per ANSI, he bold attribute changes the color, and does not actually bold the text.
//	if(f.bold)
//		cf.dwEffects |= CFE_BOLD;
	if(f.italics)
		cf.dwEffects |= CFE_ITALIC;
	if(f.underline)
		cf.dwEffects |= CFE_UNDERLINE;
	if(f.strikethrough)
		cf.dwEffects |= CFE_STRIKEOUT;
	//f.inverse
	
	cf.crTextColor = lt_colors[(f.bold?10:0) + f.color_index];
	//bg_color_index

	SetSelectionCharFormat(cf); */
}


void CHexvisDlg::ProcessLine(const char *line, int len) {
	const char *startnum = NULL;
	int numlen = 0;
	bool maybe_hex = false;
	bool found_offset = false;
	
	for(; len > 0; len--, line++) {
		switch(*line) {
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
			maybe_hex = true;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if(startnum == NULL) {
				startnum = line;
				numlen = 1;
			} else {
				numlen++;
			}
			break;
		case 'x':
			if(startnum != NULL) {
				if(numlen == 1) {
					numlen++;	//could be valid 0x notation
					maybe_hex = true;
				} else
					startnum = NULL;	//out of place.
			}
			break;
		case ':':
		case '|':
		case '>':
			if(found_offset) {
				//this is assumed to be the start of the ascii
				len = 0;	//'break'
			} else {
				if(m_offsetStart == -1)
					if(startnum != NULL) {
						//we found a leading offset
						std::string num(startnum, numlen);
						m_offsetStart = strtoul(num.c_str(), NULL, maybe_hex?16:10);
					} else {
						m_offsetStart = 0;
					}
				found_offset = true;
			}
			startnum = NULL;
			maybe_hex = false;
			break;
		case ' ':
		case '\t':
		case '-':
			if(!found_offset) {
				if(startnum != NULL) {
					if(_ProcessDigits(startnum, numlen)) {
						if(m_offsetStart == -1)
							m_offsetStart = 0;
						found_offset = true;
					}
				} else {
					//just leading junk... do nothing
				}
			} else {
				if(startnum != NULL) {
					_ProcessDigits(startnum, numlen);
				}
			}
			startnum = NULL;
			maybe_hex = false;
			break;
		default:
			startnum = NULL;
			maybe_hex = false;
			break;
		}
	}
	if(startnum != NULL) {
		if(m_offsetStart == -1)
			m_offsetStart = 0;
		_ProcessDigits(startnum, numlen);
	}
}

bool CHexvisDlg::_ProcessDigits(const char *startnum, int numlen) {
	/*if(numlen == 2) {
		std::string num(startnum, 2);
		unsigned char digit = strtoul(num.c_str(), NULL, 16);
		m_digits.push_back(digit);
		return(true);
	} else if(numlen == 4) {
		std::string num(startnum, 2);
		unsigned char digit = strtoul(num.c_str(), NULL, 16);
		m_digits.push_back(digit);

		std::string num2(startnum+2, 2);
		digit = strtoul(num2.c_str(), NULL, 16);
		m_digits.push_back(digit);
		return(true);
	} else */
	if(numlen > 0 && (numlen & 0x1) == 0) {
		//an even number of hex digits, divide them in twos
		int doffset;
		for(doffset = 0; doffset < numlen; doffset += 2) {
			std::string num(startnum+doffset, 2);
			unsigned char digit = strtoul(num.c_str(), NULL, 16);
			m_digits.push_back(digit);
		}
		return(true);
	}
	return(false);
}

void CHexvisDlg::AnalyzeBytes(int byte) {
	CString s;

	int maxlen = m_digits.size() - byte;
	const unsigned char *bptr = (const unsigned char *) &m_digits[byte];

	s.Format("%d = 0x%X", byte+m_offsetStart, byte+m_offsetStart);
	m_offset.SetWindowText(s);

	if(maxlen >= sizeof(char)) {
		s.Format("0x%02X = %d = %c", *bptr, *bptr, *bptr);
		m_byte.SetWindowText(s);

		s.Format("%.*s", maxlen, bptr);
		m_string.SetWindowText(s);

		CString b;
		s = "";
		int r;
		for(r = 0; r < 1 && r < maxlen; r++) {
			unsigned char cb = *(bptr+r);
			b.Format("%d%d%d%d %d%d%d%d",
				(cb&0x80)>>7,
				(cb&0x40)>>6,
				(cb&0x20)>>5,
				(cb&0x10)>>4,
				(cb&0x08)>>3,
				(cb&0x04)>>2,
				(cb&0x02)>>1,
				(cb&0x01) );
			if(!s.IsEmpty())
				s += " - ";
			s += b;
		}
		m_binRep.SetWindowText(s);
	} else {
		m_byte.SetWindowText("");
		m_binRep.SetWindowText("");
		m_string.SetWindowText("");
	}

	if(maxlen >= sizeof(short)) {
		const unsigned short *le = (const unsigned short *) bptr;
		signed short sle = *le;
		unsigned short be = ((*le & 0xFF)<<8) | ((*le &0xFF00)>>8);
		signed short sbe = be;

		s.Format("0x%04X = %d = %u", *le, sle, *le);
		m_short_le.SetWindowText(s);

		s.Format("0x%04X = %d = %u", be, sbe, be);
		m_short_be.SetWindowText(s);
	} else {
		m_short_le.SetWindowText("");
		m_short_be.SetWindowText("");
	}


	if(maxlen >= sizeof(long)) {
		const unsigned long *le = (const unsigned long *) bptr;
		signed long sle = *le;
		unsigned long be = 
			  ((*le & 0x000000FF) <<24) 
			| ((*le & 0x0000FF00L)<<8)
			| ((*le & 0x00FF0000L)>>8)
			| ((*le & 0xFF000000L)>>24)
			;
		signed long sbe = be;

		s.Format("0x%08lX = %ld = %lu", *le, sle, *le);
		m_long_le.SetWindowText(s);

		s.Format("0x%08lX = %ld = %lu", be, sbe, be);
		m_long_be.SetWindowText(s);
		
		std::string tt;
		time_t t = *le;
		const char *p = ctime(&t);
		if(p == NULL)
			p = "(ERROR)";
		tt = p;
		t = be;
		p = ctime(&t);
		if(p == NULL)
			p = "(ERROR)";
		s.Format("%s | %s", tt.c_str(), p);
		m_unix_time.SetWindowText(s);
	} else {
		m_long_le.SetWindowText("");
		m_long_be.SetWindowText("");
		m_unix_time.SetWindowText("");
	}

	if(maxlen >= sizeof(float)) {
		const float *f = (const float *) bptr;

		if(maxlen >= sizeof(double)) {
			const double *d = (const double *) bptr;
			s.Format("%.13f %.13f", *f, *d);
			m_real.SetWindowText(s);
		} else {
			s.Format("%.13f", *f);
			m_real.SetWindowText(s);
		}
	} else {
		m_real.SetWindowText("");
	}


	if(maxlen >= sizeof(__int64)) {
		const unsigned __int64 *le = (const unsigned __int64 *) bptr;
		signed __int64 sle = *le;
		unsigned __int64 be = 
			  ((*le & 0x00000000000000FF)<<56) 
			| ((*le & 0x000000000000FF00)<<40)
			| ((*le & 0x0000000000FF0000)<<24)
			| ((*le & 0x00000000FF000000)<<8)
			| ((*le & 0x000000FF00000000)>>8)
			| ((*le & 0x0000FF0000000000)>>24)
			| ((*le & 0x00FF000000000000)>>40)
			| ((*le & 0xFF00000000000000)>>56)
			;
		signed __int64 sbe = be;

		s.Format("0x%I64X = %I64d = %I64u", *le, sle, *le);
		m_int64_le.SetWindowText(s);

		s.Format("0x%I64X = %I64d = %I64u", be, sbe, be);
		m_int64_be.SetWindowText(s);
	} else {
		m_int64_le.SetWindowText("");
		m_int64_be.SetWindowText("");
	}

}

void CHexvisDlg::OnUpdatePacketBody() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
}

void CHexvisDlg::OnSelchangePacketBody(NMHDR* pNMHDR, LRESULT* pResult) 
{
	SELCHANGE *pSelChange = reinterpret_cast<SELCHANGE *>(pNMHDR);
	// TODO: The control will not send this notification unless you override the
	// CDialog::OnInitDialog() function to send the EM_SETEVENTMASK message
	// to the control with the ENM_SELCHANGE flag ORed into the lParam mask.
	
	static int line_mid = 3*8;
	static int ascii_start = 3*16 + 3;
	static int ascii_end = 3*16 + 5 + 16;

	if(m_processed) {

		long start, end;

		m_packetBody.GetSel(start, end);
		if(start != -1) {
			int line = m_packetBody.LineFromChar(start);
			if(line >= 0 && line < m_lineStarts.size()) {
				int lstart = m_lineStarts[line]+1;
				int loffset = start - lstart;

				if(loffset > line_mid)
					loffset -= 2;

				int coffset;
				if(loffset <= 0)
					coffset = 0;
				else if(loffset < ascii_start) {
					coffset = (loffset / 3);
					if(coffset >= 16)
						coffset = 15;
				} else if(loffset < ascii_end) {
					//into the ascii section
					coffset = (loffset-ascii_start);
				} else {
					coffset = 15;
				}



				int abscoffset = coffset + line*16;

				if(abscoffset >= m_digits.size()) {
					int diff = 1 + abscoffset - m_digits.size();
					coffset -= diff;
					abscoffset -= diff;
				}

				int seloffset = lstart + 3*coffset;
				if(coffset >= 8)
					seloffset += 2;
				

				//they have selected byte 'abscoffset'
				AnalyzeBytes(abscoffset);
				m_activeByte = abscoffset;

				m_packetBody.SetSel(seloffset, seloffset + 2);
			}
		}
	}
	
	*pResult = 0;
}

void CHexvisDlg::SelectByte(int byte) {
	if(byte < 0)
		byte = 0;
	if(byte >= m_digits.size())
		byte = m_digits.size()-1;

	int seloffset = 0;
	if(byte > 0) {
		//first figure out what line its on...
		//then figure out the char offset into that line.
		int line = byte/16;
		int coffset = byte%16;
		int lstart = m_lineStarts[line]+1;
		seloffset = lstart + 3*coffset;
		if(coffset >= 8)
			seloffset += 2;

		m_activeByte = byte;
	}

	//this will trigger a sel change which will update the fields
	m_packetBody.SetSel(seloffset, seloffset + 2);
}

void CHexvisDlg::OnClear() 
{
	m_processed = false;
	m_activeByte = -1;
	m_packetBody.SetSel(0, -1);
	m_packetBody.Clear();

	m_status.SetWindowText("Paste text above an process");
}

static __int64 _strtoll(char *str)
{
        __int64 result = 0;
        int negative=0;

        while (*str == ' ' || *str == '\t')
                str++;
        if (*str == '+')
                str++;
        else if (*str == '-') {
                negative = 1;
                str++;
        }

        while (*str >= '0' && *str <= '9') {
                result = (result*10)+(*str++ - '0');
        }
        return negative ? -result : result;
}


void CHexvisDlg::OnFindNumber() 
{
	if(!m_processed || m_digits.empty())
		return;

	CString num;
	m_searchNumber.GetWindowText(num);
	const char *buf = num.GetBuffer(0);
	if(buf[0] == '\0')
		return;

	FindSpec s;
	//we dont handle unsigned 64 bit ints very well.
	s.intval = strtol(buf, NULL, 0);
	s.dval = strtod(buf, NULL);
	s.fval = s.dval;	//only cast it down once.

	int bytecount = m_digits.size();
	int sval = m_activeByte;
	int eval = bytecount-1;
	if(sval < 1) {
		sval = 0;
	} else {
		eval = sval;
		sval++;
	}
	
	m_status.SetWindowText("Searching...");

	bool found = false;
	while(true) {
		if(sval >= bytecount)
			sval = 0;

		if(FindNumber(sval, &s)) {
			SelectByte(sval);
			found = true;
			break;
		}

		if(sval == eval)
			break;
		sval++;
	}
	if(!found)
		m_status.SetWindowText("Not found");
}

static bool isnormal(double f) {
	int c = _fpclass(f);
	return(
		(c & (_FPCLASS_SNAN | _FPCLASS_QNAN | _FPCLASS_NINF | _FPCLASS_PINF)) == 0
	);
}

static void _findDebug() {
	static int x;
	x++;
}

bool CHexvisDlg::FindNumber(int byte, const FindSpec *f) {

	int maxlen = m_digits.size() - byte;
	const unsigned char *bptr = (const unsigned char *) &m_digits[byte];

	if(maxlen >= sizeof(char)) {
		if(*bptr == f->intval) {
			_findDebug();
			m_status.SetWindowText("Found as a byte");
			return(true);
		}
	}

	if(maxlen >= sizeof(short)) {
		const unsigned short *le = (const unsigned short *) bptr;
		signed short sle = *le;
		unsigned short be = ((*le & 0xFF)<<8) | ((*le &0xFF00)>>8);
		signed short sbe = be;

		if(*le == f->intval) {
			m_status.SetWindowText("Found as a native short");
			_findDebug();
			return(true);
		}
		if(sle == f->intval) {
			m_status.SetWindowText("Found as a signed native short");
			_findDebug();
			return(true);
		}
		if(be == f->intval) {
			m_status.SetWindowText("Found as a swapped short");
			_findDebug();
			return(true);
		}
		if(sbe == f->intval) {
			m_status.SetWindowText("Found as a signed swapped short");
			_findDebug();
			return(true);
		}
	}


	if(maxlen >= sizeof(long)) {
		const unsigned long *le = (const unsigned long *) bptr;
		signed long sle = *le;
		unsigned long be = 
			  ((*le & 0x000000FF) <<24) 
			| ((*le & 0x0000FF00L)<<8)
			| ((*le & 0x00FF0000L)>>8)
			| ((*le & 0xFF000000L)>>24)
			;
		signed long sbe = be;

		if(*le == f->intval) {
			m_status.SetWindowText("Found as a native long");
			_findDebug();
			return(true);
		}
		if(sle == f->intval) {
			m_status.SetWindowText("Found as a signed native long");
			_findDebug();
			return(true);
		}
		if(be == f->intval) {
			m_status.SetWindowText("Found as a swapped long");
			_findDebug();
			return(true);
		}
		if(sbe == f->intval) {
			m_status.SetWindowText("Found as a signed swapped long");
			_findDebug();
			return(true);
		}
	}

	
	if(f->fval != 0.0f && maxlen >= sizeof(float)) {
		const float *fv = (const float *) bptr;

		if(isnormal(*fv)) {
			if(*fv == f->fval) {
				m_status.SetWindowText("Found as a float");
				_findDebug();
				return(true);
			}
			if(__int64(*fv) == f->intval) {
				m_status.SetWindowText("Found as a rounded float");
				_findDebug();
				return(true);
			}
		}
	}


	if(f->dval != 0.0f && maxlen >= sizeof(double)) {
		const double *d = (const double *) bptr;
		if(isnormal(*d)) {
			if(*d == f->dval) {
				m_status.SetWindowText("Found as a double");
				_findDebug();
				return(true);
			}
			if(__int64(*d) == f->intval) {
				m_status.SetWindowText("Found as a rounded double");
				_findDebug();
				return(true);
			}
		}
	}

	if(maxlen >= sizeof(__int64)) {
		const unsigned __int64 *le = (const unsigned __int64 *) bptr;
		signed __int64 sle = *le;
		unsigned __int64 be = 
			  ((*le & 0x00000000000000FF)<<56) 
			| ((*le & 0x000000000000FF00)<<40)
			| ((*le & 0x0000000000FF0000)<<24)
			| ((*le & 0x00000000FF000000)<<8)
			| ((*le & 0x000000FF00000000)>>8)
			| ((*le & 0x0000FF0000000000)>>24)
			| ((*le & 0x00FF000000000000)>>40)
			| ((*le & 0xFF00000000000000)>>56)
			;
		signed __int64 sbe = be;
		
		if(*le == f->intval) {
			m_status.SetWindowText("Found as a native int64");
			_findDebug();
			return(true);
		}
		if(sle == f->intval) {
			m_status.SetWindowText("Found as a signed native int64");
			_findDebug();
			return(true);
		}
		if(be == f->intval) {
			m_status.SetWindowText("Found as a swapped int64");
			_findDebug();
			return(true);
		}
		if(sbe == f->intval) {
			m_status.SetWindowText("Found as a signed swapped int64");
			_findDebug();
			return(true);
		}
	}

	return(false);
}


void CHexvisDlg::PrevByte() {
	if(m_activeByte > 0)
		SelectByte(m_activeByte-1);
}

void CHexvisDlg::NextByte() {
	int t = m_activeByte+1;
	if(t < m_digits.size())
		SelectByte(t);
}

void CHexvisDlg::PrevLine() {
	if(m_activeByte == 0)
		return;
	if(m_activeByte > 15)
		SelectByte(m_activeByte-16);
	else
		SelectByte(0);
}

void CHexvisDlg::NextLine() {
	int max = m_digits.size();
	if((m_activeByte+16) < max)
		SelectByte(m_activeByte+16);
	else
		SelectByte(max-1);
}

void CHexvisDlg::OnLoadFile() 
{
	CFileDialog *fileopendialog = new CFileDialog( true, NULL, NULL, 
		OFN_HIDEREADONLY , 
		"Any Raw File (*.*)|*.*||", this );

	if ( fileopendialog->DoModal() == IDOK ) {
		HANDLE hFile;
		hFile = CreateFile(fileopendialog->GetPathName(), 
                     GENERIC_READ,
                     FILE_SHARE_READ, 
                     NULL,
                     OPEN_EXISTING, 
                     FILE_ATTRIBUTE_NORMAL, 
                     NULL);
		if(hFile == INVALID_HANDLE_VALUE) {
			m_status.SetWindowText("Failed to open file.");
			return;
		}

		int size = GetFileSize(hFile, NULL);

		m_offsetStart = 0;
		m_digits.clear();
		m_digits.resize(size);
		m_lineStarts.clear();

		m_processed = false;

		DWORD out;
		if(!ReadFile(hFile, &m_digits[0], size, &out, NULL)) {
			m_status.SetWindowText("Failed to load file.");
			m_digits.resize(0);
			return;
		}

		CloseHandle(hFile);
		
		DisplayDigits();
		
		m_processed = true;
		
		//this will trigger the sel change
		m_packetBody.SetSel(8, 9);
		
		m_status.SetWindowText("Press clear to process another block");
	}
}

void CHexvisDlg::OnCopyToClip() 
{
	bool v = m_processed;
	if(v) {
		m_processed = false;
	}

	m_packetBody.SetSel(0, -1);
	m_packetBody.Copy();

	if(v) {
		m_processed = true;
		m_packetBody.SetSel(8, 9);
	}
}
