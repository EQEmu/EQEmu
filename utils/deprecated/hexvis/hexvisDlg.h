// hexvisDlg.h : header file
//

#if !defined(AFX_HEXVISDLG_H__CA785941_36B6_4928_A1EB_B189A9AE2EA2__INCLUDED_)
#define AFX_HEXVISDLG_H__CA785941_36B6_4928_A1EB_B189A9AE2EA2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "HexRichEdit.h"

#include <vector>

/////////////////////////////////////////////////////////////////////////////
// CHexvisDlg dialog

class CHexvisDlg : public CDialog
{
// Construction
public:
	CHexvisDlg(CWnd* pParent = NULL);	// standard constructor

	void PrevByte();
	void NextByte();
	void PrevLine();
	void NextLine();

// Dialog Data
	//{{AFX_DATA(CHexvisDlg)
	enum { IDD = IDD_HEXVIS_DIALOG };
	CStatic	m_status;
	CEdit	m_binRep;
	CEdit	m_searchNumber;
	CEdit	m_offset;
	CEdit	m_unix_time;
	CEdit	m_string;
	CEdit	m_short_le;
	CEdit	m_short_be;
	CEdit	m_real;
	HexRichEdit	m_packetBody;
	CEdit	m_long_le;
	CEdit	m_long_be;
	CEdit	m_int64_le;
	CEdit	m_int64_be;
	CEdit	m_byte;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHexvisDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	bool m_processed;
	int m_offsetStart;
	int m_activeByte;
	std::vector<unsigned char> m_digits;
	std::vector<unsigned long> m_lineStarts;

	void ProcessLine(const char *line, int len);
	bool _ProcessDigits(const char *startnum, int numlen);

	void DisplayDigits();
	void AnalyzeBytes(int byte);
	void SelectByte(int byte);
	void build_hex_line(const char *buffer, unsigned long length, unsigned long offset, unsigned char padding);
	
	typedef struct {
		unsigned __int64 intval;
		double dval;
		float fval;
	} FindSpec;
	bool FindNumber(int offset, const FindSpec *f);

	// Generated message map functions
	//{{AFX_MSG(CHexvisDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnProcess();
	afx_msg void OnUpdatePacketBody();
	afx_msg void OnSelchangePacketBody(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClear();
	afx_msg void OnFindNumber();
	afx_msg void OnLoadFile();
	afx_msg void OnCopyToClip();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HEXVISDLG_H__CA785941_36B6_4928_A1EB_B189A9AE2EA2__INCLUDED_)
