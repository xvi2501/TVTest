/*
	TVTest �v���O�C���T���v��

	��莞�Ԃ��ƂɃL���v�`�����s��
*/


#include <windows.h>
#include <tchar.h>
#define TVTEST_PLUGIN_CLASS_IMPLEMENT
#include "TVTestPlugin.h"
#include "resource.h"


#define SNAPSHOT_WINDOW_CLASS TEXT("Auto Snap Shot Window")


// �v���O�C���N���X
class CAutoSnapShot : public TVTest::CTVTestPlugin
{
	DWORD m_Interval;
	HWND m_hwnd;
	bool m_fEnabled;
	static LRESULT CALLBACK EventCallback(UINT Event,LPARAM lParam1,LPARAM lParam2,void *pClientData);
	static CAutoSnapShot *GetThis(HWND hwnd);
	static LRESULT CALLBACK WndProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
	static INT_PTR CALLBACK DlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
public:
	CAutoSnapShot();
	virtual bool GetPluginInfo(TVTest::PluginInfo *pInfo);
	virtual bool Initialize();
	virtual bool Finalize();
};


CAutoSnapShot::CAutoSnapShot()
{
	m_Interval=10;	// �L���v�`���Ԋu(�b�P��)
	m_hwnd=NULL;
	m_fEnabled=false;
}


bool CAutoSnapShot::GetPluginInfo(TVTest::PluginInfo *pInfo)
{
	// �v���O�C���̏���Ԃ�
	pInfo->Type           = TVTest::PLUGIN_TYPE_NORMAL;
	pInfo->Flags          = TVTest::PLUGIN_FLAG_HASSETTINGS;
	pInfo->pszPluginName  = L"Auto Snap Shot";
	pInfo->pszCopyright   = L"Public Domain";
	pInfo->pszDescription = L"��莞�Ԃ��ƂɃL���v�`������";
	return true;
}


bool CAutoSnapShot::Initialize()
{
	// ����������

	WNDCLASS wc;

	wc.style=0;
	wc.lpfnWndProc=WndProc;
	wc.cbClsExtra=0;
	wc.cbWndExtra=0;
	wc.hInstance=g_hinstDLL;
	wc.hIcon=NULL;
	wc.hCursor=LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground=(HBRUSH)(COLOR_3DFACE+1);
	wc.lpszMenuName=NULL;
	wc.lpszClassName=SNAPSHOT_WINDOW_CLASS;
	if (::RegisterClass(&wc)==0)
		return false;

	m_hwnd=::CreateWindowEx(0,SNAPSHOT_WINDOW_CLASS,NULL,WS_POPUP,
							0,0,0,0,
							m_pApp->GetAppWindow(),NULL,g_hinstDLL,this);
	if (m_hwnd==NULL)
		return false;

	// �C�x���g�R�[���o�b�N�֐���o�^
	m_pApp->SetEventCallback(EventCallback,this);

	return true;
}


bool CAutoSnapShot::Finalize()
{
	// �I������

	::DestroyWindow(m_hwnd);

	return true;
}


// �C�x���g�R�[���o�b�N�֐�
// �����C�x���g���N����ƌĂ΂��
LRESULT CALLBACK CAutoSnapShot::EventCallback(UINT Event,LPARAM lParam1,LPARAM lParam2,void *pClientData)
{
	CAutoSnapShot *pThis=static_cast<CAutoSnapShot*>(pClientData);

	switch (Event) {
	case TVTest::EVENT_PLUGINENABLE:
		// �v���O�C���̗L����Ԃ��ω�����
		pThis->m_fEnabled=lParam1!=0;
		if (pThis->m_fEnabled)
			::SetTimer(pThis->m_hwnd,1,pThis->m_Interval*1000,NULL);
		else
			::KillTimer(pThis->m_hwnd,1);
		return TRUE;

	case TVTest::EVENT_PLUGINSETTINGS:
		// �v���O�C���̐ݒ���s��
		return ::DialogBoxParam(g_hinstDLL,MAKEINTRESOURCE(IDD_SETTINGS),
								reinterpret_cast<HWND>(lParam1),DlgProc,
								reinterpret_cast<LPARAM>(pThis))==IDOK;
	}
	return 0;
}


CAutoSnapShot *CAutoSnapShot::GetThis(HWND hwnd)
{
	return reinterpret_cast<CAutoSnapShot*>(::GetWindowLongPtr(hwnd,GWLP_USERDATA));
}


LRESULT CALLBACK CAutoSnapShot::WndProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg) {
	case WM_CREATE:
		{
			LPCREATESTRUCT pcs=reinterpret_cast<LPCREATESTRUCT>(lParam);
			CAutoSnapShot *pThis=static_cast<CAutoSnapShot*>(pcs->lpCreateParams);

			::SetWindowLongPtr(hwnd,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(pThis));
		}
		return TRUE;

	case WM_TIMER:
		{
			CAutoSnapShot *pThis=GetThis(hwnd);

			// �L���v�`�����s
			pThis->m_pApp->SaveImage();
		}
		return 0;

	case WM_DESTROY:
		{
			CAutoSnapShot *pThis=GetThis(hwnd);

			if (pThis->m_fEnabled)
				::KillTimer(hwnd,1);	// �ʂɂ��Ȃ��Ă���������...
		}
		return 0;
	}
	return ::DefWindowProc(hwnd,uMsg,wParam,lParam);
}


// �ݒ�_�C�A���O�v���V�[�W��
INT_PTR CALLBACK CAutoSnapShot::DlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			CAutoSnapShot *pThis=reinterpret_cast<CAutoSnapShot*>(lParam);

			::SetProp(hDlg,TEXT("This"),pThis);
			::SetDlgItemInt(hDlg,IDC_SETTINGS_INTERVAL,pThis->m_Interval,FALSE);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			{
				CAutoSnapShot *pThis=static_cast<CAutoSnapShot*>(::GetProp(hDlg,TEXT("This")));

				pThis->m_Interval=::GetDlgItemInt(hDlg,IDC_SETTINGS_INTERVAL,NULL,FALSE);
				if (pThis->m_fEnabled)
					::SetTimer(pThis->m_hwnd,1,pThis->m_Interval*1000,NULL);
			}
		case IDCANCEL:
			::EndDialog(hDlg,LOWORD(wParam));
			return TRUE;
		}
		return TRUE;

	case WM_NCDESTROY:
		::RemoveProp(hDlg,TEXT("This"));
		return TRUE;
	}
	return FALSE;
}




TVTest::CTVTestPlugin *CreatePluginClass()
{
	return new CAutoSnapShot;
}