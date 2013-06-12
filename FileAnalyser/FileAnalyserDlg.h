
// FileAnalyserDlg.h : header file
//

#pragma once


// CFileAnalyserDlg dialog
class CFileAnalyserDlg : public CDialog
{
// Construction
public:
	CFileAnalyserDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_FILEANALYSER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBrowser();

private:
	void AnalyseMPEG    (const Solutions::Generics::DataElement& dataObject);
	void AnalyseTS      (const Solutions::Generics::DataElement& dataObject);
	void BuildTree      (const Solutions::AVI::Main& info);
	void DisplaySection (const Solutions::Generics::DataElement& dataObject, CTreeCtrl* filler, const HTREEITEM startingPoint);
	CString ByteArray(const Solutions::Generics::DataElement& dataObject, uint64 offset, uint32 size) const;
};
