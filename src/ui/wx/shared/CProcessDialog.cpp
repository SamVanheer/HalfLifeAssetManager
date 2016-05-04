#include <wx/txtstrm.h>

#include "CProcessDialog.h"

namespace ui
{
wxBEGIN_EVENT_TABLE( CProcessDialog, wxDialog )
	EVT_TEXT_ENTER( wxID_PROCESSDLG_INPUT, CProcessDialog::OnSendInput )
	EVT_BUTTON( wxID_PROCESSDLG_SENDINPUT, CProcessDialog::OnSendInput )
	EVT_IDLE( CProcessDialog::OnIdle )
	EVT_END_PROCESS( wxID_ANY, CProcessDialog::OnTerminated )
	EVT_BUTTON( wxID_PROCESSDLG_TERMINATE, CProcessDialog::OnTerminatePressed )
	EVT_BUTTON( wxID_PROCESSDLG_CLOSE, CProcessDialog::OnClosePressed )
wxEND_EVENT_TABLE()

CProcessDialog::CProcessDialog( wxWindow* pParent, wxWindowID ID, const wxString& szTitle,
								const wxPoint& pos,
								const wxSize& size,
								long style,
								const wxString& name )
	: wxDialog( pParent, ID, szTitle, pos, size, style, name )
{
	m_pOutput = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 600, 600 ), wxTE_MULTILINE | wxTE_RICH );
	m_pOutput->SetEditable( false );

	m_pInput = new wxTextCtrl( this, wxID_PROCESSDLG_INPUT, "", wxDefaultPosition, wxSize( 600, wxDefaultSize.GetHeight() ), wxTE_PROCESS_ENTER );

	m_pSendInput = new wxButton( this, wxID_PROCESSDLG_SENDINPUT, "Send" );
	m_pSendInput->Enable( false );

	m_pTerminate = new wxButton( this, wxID_PROCESSDLG_TERMINATE, "Terminate" );
	m_pTerminate->Enable( false );

	m_pClose = new wxButton( this, wxID_PROCESSDLG_CLOSE, "Close" );
	m_pClose->Enable( false );

	//Layout
	auto pSizer = new wxBoxSizer( wxVERTICAL );

	pSizer->Add( m_pOutput, wxSizerFlags().Expand() );

	auto pInputSizer = new wxBoxSizer( wxHORIZONTAL );

	pInputSizer->Add( m_pInput, wxSizerFlags().Expand() );
	pInputSizer->Add( m_pSendInput, wxSizerFlags().Expand().Proportion( 1 ).DoubleBorder( wxLEFT ) );

	pSizer->Add( pInputSizer, wxSizerFlags().Expand().Proportion( 1 ).DoubleBorder() );

	auto pButtonsSizer = new wxBoxSizer( wxHORIZONTAL );

	pButtonsSizer->Add( m_pTerminate, wxSizerFlags() );
	pButtonsSizer->Add( m_pClose, wxSizerFlags().DoubleBorder( wxLEFT ) );

	pSizer->Add( pButtonsSizer, wxSizerFlags().Proportion( 1 ).Align( wxALIGN_RIGHT ).DoubleBorder() );

	this->SetSizer( pSizer );

	this->Fit();
	this->CenterOnScreen();

	SetInputEnabled( false );

	//Disable the button if possible.
	this->EnableCloseButton( false );
}

CProcessDialog::~CProcessDialog()
{
	SetExecuteEnv( nullptr );
}

int CProcessDialog::ShowModal()
{
	wxASSERT( !m_pProcess );

	m_pOutput->Clear();
	m_pInput->Clear();

	if( m_szCommand.IsEmpty() )
		return NO_COMMAND;

	m_pProcess = new wxProcess( this );

	m_pProcess->Redirect();

	const int iReturnCode = wxExecute( m_szCommand, wxEXEC_ASYNC | wxEXEC_HIDE_CONSOLE | wxEXEC_MAKE_GROUP_LEADER, m_pProcess, m_pEnv );

	if( iReturnCode == 0 )
		return COULD_NOT_EXECUTE;

	m_pTerminate->Enable( true );

	m_bTerminated = false;

	//Start sending idle events.
	wxWakeUpIdle();

	if( m_bInputEnabled )
	{
		m_pInput->Enable();
		m_pSendInput->Enable();
	}

	return wxDialog::ShowModal();
}

void CProcessDialog::EndModal( int iReturnCode )
{
	//If we couldn't disable the close button, ignore it.
	if( iReturnCode == wxID_CANCEL )
	{
		return;
	}

	//Still alive? Terminate.
	Terminate();

	if( m_bTerminated )
		iReturnCode = TERMINATED;

	wxDialog::EndModal( iReturnCode );
}

wxString CProcessDialog::GetErrorString( const int iErrorCode ) const
{
	switch( iErrorCode )
	{
	case SUCCESS:			return wxString::Format( "The command \"%s\" completed successfully", GetCommand() );

	case NO_COMMAND:		return wxString::Format( "No command was specified (command was \"%s\")", GetCommand() );

	case COULD_NOT_EXECUTE:	return wxString::Format( "Could not execute command \"%s\"", GetCommand() );

	case TERMINATED:		return wxString::Format( "Process was terminated before it could finish (command was \"%s\")", GetCommand() );

	default:				return wxString::Format( "Unknown error code \"%d\" (command was \"%s\")", iErrorCode, GetCommand() );
	}
}

bool CProcessDialog::IsInputEnabled() const
{
	return m_bInputEnabled;
}

void CProcessDialog::SetInputEnabled( const bool bEnabled )
{
	m_bInputEnabled = true;
}

void CProcessDialog::SetCommand( const wxString& szCommand )
{
	m_szCommand = szCommand;

	m_szCommand.Trim();
	m_szCommand.Trim( false );
}

void CProcessDialog::SetExecuteEnv( wxExecuteEnv* pEnv )
{
	if( m_pEnv )
	{
		delete m_pEnv;
	}

	m_pEnv = pEnv;
}

bool CProcessDialog::SendInput( const wxString& szInput )
{
	if( !m_pInput->IsEnabled() || !m_pProcess || !m_pProcess->GetOutputStream() )
	{
		return false;
	}

	wxTextOutputStream out( *m_pProcess->GetOutputStream() );

	out.WriteString( szInput );
	out.Flush();

	return true;
}

void CProcessDialog::Terminate()
{
	m_pTerminate->Enable( false );

	if( !m_pProcess )
		return;

	//Pull remaining output. This won't ever get all output due to race conditions, but it will get most of it.
	PullOutput();

	wxProcess::Kill( m_pProcess->GetPid(), wxSIGTERM, wxKILL_CHILDREN );

	delete m_pProcess;
	m_pProcess = nullptr;

	m_bTerminated = true;

	Ended();
}

void CProcessDialog::OnIdle( wxIdleEvent& event )
{
	if( !m_pProcess )
	{
		event.Skip();
		return;
	}

	PullOutput();

	event.RequestMore();
}

void CProcessDialog::PullOutput()
{
	//Pull input from the input and error streams.
	if( m_pProcess->GetInputStream() && m_pProcess->IsInputAvailable() )
	{
		//Use black text color.
		m_pOutput->SetDefaultStyle( wxTextAttr( wxColor( 0, 0, 0 ) ) );

		wxTextInputStream in( *m_pProcess->GetInputStream() );

		while( !in.GetInputStream().Eof() )
		{
			m_pOutput->AppendText( in.ReadLine() + '\n' );
		}
	}

	if( m_pProcess->GetErrorStream() && m_pProcess->IsErrorAvailable() )
	{
		//Use red text color.
		m_pOutput->SetDefaultStyle( wxTextAttr( wxColor( 255, 0, 0 ) ) );

		wxTextInputStream in( *m_pProcess->GetErrorStream() );

		while( !in.GetInputStream().Eof() )
		{
			m_pOutput->AppendText( in.ReadLine() + '\n' );
		}
	}
}

void CProcessDialog::SendCurrentInput()
{
	if( SendInput( m_pInput->GetValue() ) )
		m_pInput->Clear();
}

void CProcessDialog::Ended()
{
	m_pInput->Enable( false );
	m_pSendInput->Enable( false );
}

void CProcessDialog::OnSendInput( wxCommandEvent& event )
{
	SendCurrentInput();
}

void CProcessDialog::OnTerminated( wxProcessEvent& event )
{
	//If the process finishes before we've pulled all output, finish doing so now.
	PullOutput();

	m_pOutput->SetDefaultStyle( wxTextAttr( wxColor( 0, 0, 0 ) ) );
	m_pOutput->AppendText( wxString::Format( "The program exited with exit code %d\n", event.GetExitCode() ) );

	delete m_pProcess;
	m_pProcess = nullptr;

	Ended();

	m_pTerminate->Enable( false );
	m_pClose->Enable( true );
}

void CProcessDialog::OnTerminatePressed( wxCommandEvent& event )
{
	Terminate();

	m_pClose->Enable( true );
}

void CProcessDialog::OnClosePressed( wxCommandEvent& event )
{
	EndModal( SUCCESS );
}
}