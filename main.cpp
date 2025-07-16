#define UNICODE

#include <format>
#include <windows.h>
#include <shobjidl.h>
#include <comdef.h>

void QuickDialogFail(const wchar_t* msg) {
    MessageBoxW(
        nullptr,
        msg,
        L"fuck",
        MB_ICONERROR | MB_OK
        );
}

void QuickDialog(const wchar_t* msg) {
    MessageBoxW(
        nullptr,
        msg,
        L"log",
        MB_ICONINFORMATION | MB_OK
    );
}

void ResultFail(HRESULT hr, const wchar_t* msg) {
    _com_error err {hr};
    auto* errMsg = (wchar_t*) err.ErrorMessage();
    if (FAILED(hr)) {
        QuickDialogFail(std::format(L"{}, HRESULT: {}", msg, errMsg).c_str());
        exit(1);
    }
}

class DialogEventHandler : public IFileDialogEvents {
    ULONG ref = 1;
public:
    virtual ~DialogEventHandler() = default;

    HRESULT QueryInterface(const IID& riid, void** ppvObject) override {
        if (!ppvObject) {
            return E_POINTER;
        }

        if (riid == _uuidof(IFileDialogEvents) || riid == _uuidof(IUnknown)) {
            *ppvObject = (IFileDialogEvents*)this;

            AddRef();
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    ULONG AddRef() override { return ++ref; }

    ULONG Release() override {
        ULONG count = --ref;
        if (count == 0) { delete this; }
        return count;
    }

    HRESULT OnFileOk(IFileDialog* pfd) override {
        wchar_t* wMsg = nullptr;

        //ascii is still contained, relax man itll work
        const HRESULT hr = pfd->GetFileName(&wMsg);
        ResultFail(hr, L"could not extract file name from file dialog");


        QuickDialog(wMsg);

        CoTaskMemFree(wMsg);

        return S_OK;
    }

    HRESULT OnFolderChanging(IFileDialog* pfd, IShellItem* psiFolder) override {return S_OK;}

    HRESULT OnFolderChange(IFileDialog* pfd) override {return S_OK;}

    HRESULT OnSelectionChange(IFileDialog* pfd) override {return S_OK;}

    HRESULT OnShareViolation(IFileDialog* pfd, IShellItem* psi, FDE_SHAREVIOLATION_RESPONSE* pResponse) override {return S_OK;}

    HRESULT OnTypeChange(IFileDialog* pfd) override {return S_OK;}

    HRESULT OnOverwrite(IFileDialog* pfd, IShellItem* psi, FDE_OVERWRITE_RESPONSE* pResponse) override {return S_OK;}
};


int WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    HRESULT hr = 0;

    hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    ResultFail(hr, L"could not init com server");
    IFileDialog* dialog = nullptr;

    hr = CoCreateInstance(
        CLSID_FileOpenDialog,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IFileDialog,
        (void**) &dialog
    );

    ResultFail(hr, L"could not create file dialog object");

    // event handling object
    IFileDialogEvents* events = new DialogEventHandler{};
    DWORD cookie = 0;

    hr = dialog->Advise(events, &cookie);

    ResultFail(hr, L"advise");

    hr = dialog->Show(nullptr);

    ResultFail(hr, L"show");

    hr = dialog->Unadvise(cookie);

    ResultFail(hr, L"unadvise");

    dialog->Release();

    CoUninitialize();
}
