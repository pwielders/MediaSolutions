
// FileAnalyserDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FileAnalyser.h"
#include "FileAnalyserDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace Solutions;

ENUM_CONVERSION_TABLE(AVI::StreamType)
{
	{ AVI::AUDIO,   _TXT("Audio")         },
	{ AVI::VIDEO,   _TXT("Video")         },
	{ AVI::TEXT,    _TXT("Text")          },
	{ AVI::UNKNOWN, _TXT("Unknown")       }
};
//
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CFileAnalyserDlg dialog




CFileAnalyserDlg::CFileAnalyserDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFileAnalyserDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFileAnalyserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CFileAnalyserDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BROWSER, &CFileAnalyserDlg::OnBnClickedBrowser)
END_MESSAGE_MAP()


// CFileAnalyserDlg message handlers

BOOL CFileAnalyserDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
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

	// TODO: Add extra initialization here
	// Turn tracing on..
	Trace::TraceUnit::Instance().Output(Generics::NodeId("127.0.0.1", 5555));

	// We want to see the RTSP exchange.
	Trace::TraceUnit::Instance().SetCategories(true, "Messages", "Solutions::Trace::TraceInformationText");

	// We want to see session flow.
	//TraceUnit::Instance().SetCategories(true, "ProtocolSuite", "TraceSessionFlow");

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CFileAnalyserDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CFileAnalyserDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFileAnalyserDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CFileAnalyserDlg::OnBnClickedBrowser()
{
	CString currentPointer;  GetDlgItemText(IDC_FILEPATH, currentPointer); 
	CFileDialog newDialog (TRUE, NULL, currentPointer);

	if (newDialog.DoModal() == IDOK)
	{
		// Seems like a new file name has been choosen, show it.
		SetDlgItemText (IDC_FILEPATH, newDialog.GetPathName());

		// Time to parse the file and show the contents
		Generics::DataElementFile dataObject(Generics::TextFragment(newDialog.GetPathName()));

		if (dataObject.IsValid())
		{
			AnalyseMPEG(dataObject);
			// AnalyseTS(dataObject);

			//CEdit* entry = dynamic_cast<CEdit*> (GetDlgItem(IDC_ANALYSEINFO));
			////AVI::Main mainHeader = AVI::Main(dataObject);

			////if (mainHeader.IsValid())
			////{
			////	// Yip it's time to show what we got...
			////	BuildTree(mainHeader);
			////}

			//uint32 slices = 0;
			//// Solutions::MPEG::MPEGTransport source (dataObject, 0);
			//Solutions::MPEG::MPEGSource source (dataObject);
			////

			//MPEG::MPEGSource::Iterator streams = source.Streams();
			//// MPEG::MPEGStream stream;
			////MPEG::MPEGSource::Iterator streams = source.Streams();
			////MPEG::MPEGStream stream;

			//CString  text; text.Format("Channels: %d", streams.Channels());

			//while (streams.Next())
			//{
			//	CString channelText;

			//	switch (streams.Type())
			//	{
			//		case MPEG::AUDIO:

			//			//channelText.Format("Audio: ID: <%X>", streams.Pid());
			//		break;
			//		case MPEG::VIDEO:
			//			//channelText.Format("Video: ID: <%X>", streams.Pid());
			//			break;
			//		default:
			//			//channelText.Format("Unknown: ID: <%X>", streams.Pid());
			//		break;
			//	}
			//	text += "\n\r" + channelText;
			//}

			//SetDlgItemText (IDC_ANALYSEINFO, text);


			////uint32 group = (stream.CurrentGroupOfPicturesHeader().IsValid() ? 1 : 0);
			////uint32 pictures = (stream.CurrentPictureHeader().IsValid() ? 1 : 0);

			////while (stream.NextSlice())
			////{
			////	if (stream.NewGroupOfPicturesHeader())
			////	{
			////		group++;
			////	}
			////	if (stream.NewPictureHeader())
			////	{
			////		pictures++;
			////	}
			////	slices++;
			////}
			////uint64 duration = source.Duration();
			////uint8 channels = source.Channels();
			//


		}
	}
}

void CFileAnalyserDlg::BuildTree(const AVI::Main& info)
{
	CTreeCtrl* treeControl = reinterpret_cast<CTreeCtrl*>(GetDlgItem(IDC_RIFFTREE));

	HTREEITEM node = treeControl->InsertItem("MainNode");

	node = treeControl->InsertItem("Info", node);

	AVI::StreamIterator iterator (info.Streams());

	while (iterator.Next() == true)
	{
		uint64 packageSize = 0;
		uint64 packageCount = 0;

		AVI::Stream element(iterator.GetStream());
//		String streamType = "Value: " + String(Generics::EnumerateType<AVI::StreamType>(element.Type()).Text());
		String streamType = "Test";

		HTREEITEM info = treeControl->InsertItem("Streams", node);
		treeControl->InsertItem(streamType.c_str(), info);

		if (element.Type() == Solutions::AVI::AUDIO)
		{
			Solutions::AVI::AudioFormat format (element.Format());
			String text = "Content Type: " + Generics::NumberUnsigned16 (format.FormatTag()).Text();
			treeControl->InsertItem(text.c_str(), info);
			text = "Channels: " + Generics::NumberUnsigned16 (format.Channels()).Text();
			treeControl->InsertItem(text.c_str(), info);
			text = "SampleRate: " + Generics::NumberUnsigned32 (format.SampleRate()).Text();
			treeControl->InsertItem(text.c_str(), info);
			text = "BitRate: " + Generics::NumberUnsigned16 (format.BitRate()).Text();
			treeControl->InsertItem(text.c_str(), info);
			text = "BlockAlignSize: " + Generics::NumberUnsigned16 (format.BlockAlignSize()).Text();
			treeControl->InsertItem(text.c_str(), info);
			text = "ByteRate: " + Generics::NumberUnsigned32 (format.ByteRate()).Text();
			treeControl->InsertItem(text.c_str(), info);
		}
		else if (element.Type() == Solutions::AVI::VIDEO)
		{
			Solutions::AVI::VideoFormat format (element.Format());
			String text = "Content Type: Video";
			treeControl->InsertItem(text.c_str(), info);
		}



		AVI::PackageIterator indexer (element.Packages());

		while (indexer.Next() == true)
		{
			Generics::DataElementContainer package; 
			indexer.Package(package);
			packageSize += package.Size();
			packageCount++;
		}
	}
}

typedef Solutions::MPEG::AssemblerType<false,Solutions::MPEG::StreamAssemblerPolicy<1024>>	StreamCollector;
typedef std::map<int,StreamCollector*>														PIDCollector;

void CFileAnalyserDlg::AnalyseTS (const Solutions::Generics::DataElement& dataObject)
{
	uint64 startIndex = 0;
	uint8 frameSize = 0;
	Generics::ScopedStorage<256>	PATSpace;
	CTreeCtrl* filler = static_cast<CTreeCtrl*>(GetDlgItem(IDC_LOGVIEW));
	HTREEITEM parentPoint = NULL;
	PIDCollector CollectedPIDS;

	// Start looking for the PATSection
	Solutions::MPEG::PAT::Assembler associationTable (0);
	Solutions::MPEG::PMT::Assembler mappingTable;
	Solutions::MPEG::PMT pmtTable;


	while (((startIndex < dataObject.Size()) && (startIndex = dataObject.SearchNumber<uint8, Generics::ENDIAN_LITTLE>(startIndex, Solutions::MPEG::TransportPackage::ID())) < dataObject.Size()))
	{
		Generics::DataElement package(Generics::DataElement(dataObject, startIndex, frameSize));

		if (frameSize == 0)
		{
			// Determine the frameSize, 188, 204, 208
			if (package[188] == Solutions::MPEG::TransportPackage::ID())
			{
				frameSize = 188;
			}
			else if (package[204] == Solutions::MPEG::TransportPackage::ID())
			{
				frameSize = 204;
			}
			else if (package[208] == Solutions::MPEG::TransportPackage::ID())
			{
				frameSize = 208;
			}
			else
			{
				startIndex++;
			}
			package.Size(frameSize);
		}

		if (frameSize != 0)
		{
			startIndex += frameSize;

			Solutions::MPEG::TransportPackage frame (package);

			if (frame.IsValid() == true)
			{
				if (associationTable.Assemble(frame))
				{
					if (associationTable.CurrentTable().IsValid())
					{
						Solutions::MPEG::Table::Iterator	sectionEnumerator (associationTable.CurrentTable().Elements());

						ASSERT (sectionEnumerator.Count() == 1);

						// Move on to the only available section in the table.
						sectionEnumerator.Next();

						Solutions::MPEG::PAT::Iterator programEnumerator (sectionEnumerator.Element<Solutions::MPEG::PAT>().Associations());

						// As this is a file, We only expect 1 TS in the file...
						ASSERT (programEnumerator.Count() == 1);

						// Move on to the only available programm in the table.
						programEnumerator.Next();

						mappingTable.Reset(programEnumerator.Pid());

						// Requested table is loaded, display all info
						CString text; text.Format("Loaded PAT for PID <%d>", programEnumerator.Pid());
						filler->InsertItem(text, parentPoint);						
						text.Format("Loaded PAT for ProgramNumber <%d>", programEnumerator.ProgramNumber());
						filler->InsertItem(text, parentPoint);						
					}
				}
				else if (mappingTable.Assemble(frame))
				{
					if (mappingTable.CurrentTable().IsValid())
					{
						Solutions::MPEG::Table::Iterator sectionEnumerator (mappingTable.CurrentTable().Elements());

						ASSERT (sectionEnumerator.Count() == 1);

						// Move on to the only available section in the table.
						sectionEnumerator.Next();

						pmtTable = sectionEnumerator.Element<Solutions::MPEG::PMT>();

						Solutions::MPEG::PMT::Iterator streams (pmtTable.Streams());

						while (streams.Next())
						{
							CString text; text.Format("Loaded PMT Stream TYPE <%d> on  PID <%d>", streams.StreamType(), streams.Pid() );
							filler->InsertItem(text, parentPoint);						
						}
					}
				}
				else 
				{
					PIDCollector::const_iterator result = CollectedPIDS.find(frame.Pid());
					StreamCollector* aggregator = NULL;

					if (result != CollectedPIDS.end())
					{
						aggregator = result->second;
					}
					else
					{
						aggregator = new StreamCollector(frame.Pid());
						CollectedPIDS.insert(std::pair<int,StreamCollector*>(frame.Pid(), aggregator));

						CString text; text.Format("PID <%d>", frame.Pid());
						HTREEITEM result = filler->InsertItem(text, parentPoint);
					}

					if (aggregator->Assemble(frame))
					{
						Generics::OptionalType<Generics::DataElement> loadedFrame(aggregator->CompletedFrame(true));

						if (loadedFrame.IsSet())
						{
							AnalyseMPEG(loadedFrame.Value());
						}
					} 
				}
			}
		}
	}
}

void CFileAnalyserDlg::AnalyseMPEG (const Solutions::Generics::DataElement& dataObject)
{
	HTREEITEM	parentPoint = NULL;
	uint64 offset = 0;
	uint64 sectionSize = 0;
	CTreeCtrl* filler = static_cast<CTreeCtrl*>(GetDlgItem(IDC_LOGVIEW));

	// Find the first Header
	offset = Solutions::MPEG::MPEGHeader::FindTag (dataObject, offset);

	DisplaySection (Generics::DataElement(dataObject, offset), filler, parentPoint);
}


void CFileAnalyserDlg::DisplaySection (const Solutions::Generics::DataElement& dataObject, CTreeCtrl* filler, const HTREEITEM startingPoint)
{
	uint32 sections = 0;
	uint32 entries = 200;
	uint64 offset = Solutions::MPEG::MPEGHeader::FindTag (dataObject, 0);
	uint64 sectionSize = 0;

	if (offset != 0)
	{
		CString text;
		text.Format("# Padding {0/%d}", offset);
		filler->InsertItem(text, startingPoint);
	}

	while ( (offset < dataObject.Size()) && (entries-- != 0))
	{
		// Display the header.
		Solutions::MPEG::MPEGHeader startPoint (dataObject, offset);

		if (startPoint.IsValid())
		{
			sections++;
			CString text;

			text.Format("%s <%X>", startPoint.TagText(), startPoint.TagId());
			HTREEITEM result = filler->InsertItem(text, startingPoint);

			if (Solutions::MPEG::PictureHeader::IsValid(startPoint.TagId()))
			{
				startPoint = Solutions::MPEG::PictureHeader (startPoint);
			}
			else if (Solutions::MPEG::SliceHeader::IsValid(startPoint.TagId()))
			{
				startPoint = Solutions::MPEG::SliceHeader (startPoint);
			}
			else if (Solutions::MPEG::UserDataHeader::IsValid(startPoint.TagId()))
			{
				startPoint = Solutions::MPEG::UserDataHeader (startPoint);
			}
			else if (Solutions::MPEG::SequenceHeader::IsValid(startPoint.TagId()))
			{
				startPoint = Solutions::MPEG::SequenceHeader (startPoint);
			}
			else if (Solutions::MPEG::ExtendedSequenceHeader::IsValid(startPoint.TagId()))
			{
				Solutions::MPEG::ExtendedSequenceHeader frame = Solutions::MPEG::ExtendedSequenceHeader (startPoint);
				startPoint = frame;

				if (frame.IsDisplayExtension())
				{
					text += " <Display>";
				}
				else if (frame.IsPictureExtension())
				{
					text += " <Picture>";
				}
				else if (frame.IsSequenceExtension())
				{
					text += " <Sequence>";
				}
			
				filler->SetItemText(result, text);
			}
			else if (Solutions::MPEG::GroupOfPicturesHeader::IsValid(startPoint.TagId()))
			{
				startPoint = Solutions::MPEG::GroupOfPicturesHeader (startPoint);
			}
			else if (Solutions::MPEG::PackStreamHeader::IsValid(startPoint.TagId()))
			{
				startPoint = Solutions::MPEG::PackStreamHeader (startPoint);
			}
			else if (Solutions::MPEG::SystemStreamHeader::IsValid(startPoint.TagId()))
			{
				startPoint = Solutions::MPEG::SystemStreamHeader (startPoint);
			}
			else if (Solutions::MPEG::PaddingStreamHeader::IsValid(startPoint.TagId()))
			{
				startPoint = Solutions::MPEG::PaddingStreamHeader (startPoint);
			}			
			else if (Solutions::MPEG::ElementaryStreamHeader::IsValid(startPoint.TagId()))
			{
				Solutions::MPEG::ElementaryStreamHeader frame = Solutions::MPEG::ElementaryStreamHeader (startPoint);
				startPoint = frame;

				if (frame.IsValid())
				{
					DisplaySection(frame.Frame(), filler, result);
					text = filler->GetItemText(result);
				}
				else
				{
					filler->SetItemState(result, TVIS_BOLD, TVIS_BOLD);
					filler->InsertItem("Invalid Elementary Stream !!!", result, TVI_LAST);
				}
			}

			CString additionalText;

			additionalText.Format(_T(" {%d/"), offset);
			text.Append(additionalText);  
			additionalText.Format(_T("%d}"), startPoint.Size());
			text.Append(additionalText);  

			// Check for the length of the frame if it is oke...
			uint32 endPackage = static_cast<uint32>(offset) + static_cast<uint32>(startPoint.Size());

			if ( ( (endPackage + 3) <= dataObject.Size() ) &&
				 ( (dataObject[endPackage + 0] != 0)      ||
			 	   (dataObject[endPackage + 1] != 0)      ||
				   (dataObject[endPackage + 2] != 1) )    )
			{
				Generics::DataElement element (startPoint.Data());
				filler->SetItemState(result, TVIS_BOLD, TVIS_BOLD);

				offset = Solutions::MPEG::MPEGHeader::FindTag (dataObject, offset+3);

				additionalText.Format(_T(" (%s/%s)"),ByteArray(dataObject, endPackage-4, 4), ByteArray(dataObject, endPackage, 4));
				text.Append(additionalText);
				filler->Expand(startingPoint, TVE_EXPAND);
				if (offset < endPackage)
				{
					text += " <Next Header In package !!>";
				}
			}
			else if (offset == endPackage)
			{
				offset++;

				text += " <invalidated package>";

				filler->Expand(startingPoint, TVE_EXPAND);
			}
			else
			{
				offset = endPackage;
			}
			
			filler->SetItemText(result, text);
 		}
		else
		{
			CString text;
			text.Format(_T("Invalid Header {%d/%d} ("), offset, (dataObject.Size() - offset));
			if (offset > 4)
			{
				text.Append(ByteArray(dataObject, offset-4, 4));
				text += _T("/");
			}
			text.Append(ByteArray(dataObject, offset, 4));
			text += _T(")"); 

			HTREEITEM result = filler->InsertItem(text, startingPoint, TVI_LAST);
			filler->SetItemState(result, TVIS_BOLD, TVIS_BOLD);
			filler->Expand(startingPoint, TVE_EXPAND);
			offset = Solutions::MPEG::MPEGHeader::FindTag (dataObject, offset+1);
		}
	}

	if (startingPoint != NULL)
	{
		CString text;
		text.Format(_T("(%d) "), sections);
		text.Append(filler->GetItemText(startingPoint));
		filler->SetItemText(startingPoint, text);
	}
}

CString CFileAnalyserDlg::ByteArray(const Solutions::Generics::DataElement& dataObject, uint64 offset, uint32 size) const
{
	CString text;
	bool moreToFill = ((size != 0) && (offset < dataObject.Size()));

	while (moreToFill == true)
	{
		CString digit;

		digit.Format(_T("%02X"), dataObject[static_cast<uint32>(offset)]);

		text.Append(digit);

		moreToFill = ((--size != 0) && (++offset < dataObject.Size()));

		if (moreToFill)
		{
			text = text + ":";
		}
	}

	return (text);
}
