#ifndef UI_WX_SHARED_CPROCESSDIALOG_H
#define UI_WX_SHARED_CPROCESSDIALOG_H

#include "../wxInclude.h"

#include <wx/process.h>

namespace ui
{
/**
*	@brief A dialog that is used to display output from a child process that runs for a short time.
*
*	Useful for spawning compilers and other utility programs.
*	Supports input. This is disabled by default.
*	Output piped through stderror is displayed as red, whereas regular output is displayed as black.
*	Note: you must use ShowModal.
*/
class CProcessDialog : public wxDialog
{
public:
	/**
	*	Return codes for ShowModal()
	*/
	enum ReturnCode
	{
		/**
		*	Execution was successful.
		*/
		SUCCESS,

		/**
		*	No command was specified.
		*/
		NO_COMMAND,

		/**
		*	An error occurred while trying to start the process.
		*/
		COULD_NOT_EXECUTE,

		/**
		*	The process was terminated before it could finish.
		*/
		TERMINATED,
	};

public:
	CProcessDialog( wxWindow* pParent, wxWindowID ID, const wxString& szTitle,
					const wxPoint& pos = wxDefaultPosition,
					const wxSize& size = wxDefaultSize,
					long style = wxDEFAULT_DIALOG_STYLE,
					const wxString& name = wxDialogNameStr );
	~CProcessDialog();

	//Overridden to spawn the process.
	virtual int ShowModal() override final;

	//Overridden to terminate the process if needed.
	virtual void EndModal( int iReturnCode ) override final;

	/**
	*	Gets a string describing the given error code using the current settings.
	*	@param iErrorCode Error Code. Must be one of the ReturnCode enum.
	*	@return String.
	*	@see ReturnCode
	*/
	wxString GetErrorString( const int iErrorCode ) const;

	/**
	*	Returns whether input is enabled.
	*/
	bool IsInputEnabled() const;

	/**
	*	Sets whether input is enabled.
	*	@param bEnabled Whether input is enabled.
	*/
	void SetInputEnabled( const bool bEnabled );

	/**
	*	Gets the command that will be executed.
	*/
	const wxString& GetCommand() const { return m_szCommand; }

	/**
	*	Sets the command that will be executed.
	*	@param szCommand Command.
	*/
	void SetCommand( const wxString& szCommand );

	/**
	*	Gets the execute environment, if any.
	*/
	wxExecuteEnv* GetExecuteEnv() const { return m_pEnv; }

	/**
	*	Sets the execute environment. By default, none is set.
	*	@param pEnv Environment. May be null. Must point to a heap allocated object.
	*/
	void SetExecuteEnv( wxExecuteEnv* pEnv );

	/**
	*	@return Whether or not output files will be copied on completion.
	*	Default false.
	*/
	bool ShouldCopyFiles() const { return m_bShouldCopyFiles; }

	/**
	*	Sets whether or not output files will be copied on completion.
	*/
	void SetShouldCopyFiles( const bool bEnable )
	{
		m_bShouldCopyFiles = bEnable;
	}

	/**
	*	@return The directory that output files are copied to.
	*/
	const wxString& GetOutputDirectory() const { return m_szOutputDir; }

	/**
	*	Sets the directory that output files are copied to.
	*/
	void SetOutputDirectory( const wxString& szOutputDir )
	{
		m_szOutputDir = szOutputDir;
	}

	/**
	*	@return List of filters used to copy output files.
	*/
	const wxArrayString& GetOutputFileFilters() const { return m_OutputFileFilters; }

	/**
	*	Sets the list of filters used to copy output files.
	*/
	void SetOutputFileFilters( const wxArrayString& filters )
	{
		m_OutputFileFilters = filters;
	}

	/**
	*	Sends input to the process.
	*	@param szInput Input to send.
	*	@return true if the input was sent, false otherwise.
	*/
	bool SendInput( const wxString& szInput );

	/**
	*	Terminates the process if it is running.
	*/
	void Terminate();

protected:
	wxDECLARE_EVENT_TABLE();

	virtual void OnIdle( wxIdleEvent& event );

private:
	void PullOutput();

	void SendCurrentInput();

	void Ended();

	void CopyOutputFiles();

	void OnSendInput( wxCommandEvent& event );

	void OnTerminated( wxProcessEvent& event );

	void OnTerminatePressed( wxCommandEvent& event );

	void OnClosePressed( wxCommandEvent& event );

private:
	wxTextCtrl* m_pOutput;
	wxTextCtrl* m_pInput;
	wxButton* m_pSendInput;

	wxButton* m_pTerminate;
	wxButton* m_pClose;

	/**
	*	The process object used to communicate with the process.
	*/
	wxProcess* m_pProcess = nullptr;

	bool m_bInputEnabled = false;

	wxString m_szCommand;

	wxExecuteEnv* m_pEnv = nullptr;

	bool m_bShouldCopyFiles = false;

	wxString m_szOutputDir;

	wxArrayString m_OutputFileFilters;

	bool m_bTerminated = false;

private:
	CProcessDialog( const CProcessDialog& ) = delete;
	CProcessDialog& operator=( const CProcessDialog& ) = delete;
};
}

#endif //UI_WX_SHARED_CPROCESSDIALOG_H