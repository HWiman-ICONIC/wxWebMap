#include	<wxWebMapFrame.h>
#include    <Defines.h>
#include    <wxMapMarker.h>
#include    <SourceViewDialog.h>
#include    <wx/sizer.h>
#include    <wx/panel.h>
#include    <wx/menu.h>
#include    <wx/filename.h>
#include    <wx/numdlg.h>
#include    <wx/textdlg.h>
#include    <wx/webview.h>
#if wxUSE_WEBVIEW_IE
#include    <wx/msw/webview_ie.h>
#endif
#include    <wx/webviewarchivehandler.h>
#include    <wx/webviewfshandler.h>

wxBEGIN_EVENT_TABLE(WebFrame, wxFrame)
    EVT_MENU(WX_WBMAP_STOCKHOLM, WebFrame::OnMarkStockholm)
wxEND_EVENT_TABLE()

WebFrame::WebFrame(const wxString& url) :
    wxFrame(NULL, wxID_ANY, "wxWebView Sample")
{
    // set the frame icon
    SetIcon(wxICON(sample));
    SetTitle("wxWebView Sample");

    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);

    // Create the toolbar
    m_toolbar = CreateToolBar(wxTB_TEXT);
    m_toolbar->SetToolBitmapSize(wxSize(32, 32));

    wxBitmap back = wxArtProvider::GetBitmap(wxART_GO_BACK, wxART_TOOLBAR);
    wxBitmap forward = wxArtProvider::GetBitmap(wxART_GO_FORWARD, wxART_TOOLBAR);
#ifdef __WXGTK__
    wxBitmap stop = wxArtProvider::GetBitmap("gtk-stop", wxART_TOOLBAR);
#else
    wxBitmap stop = wxBitmap(stop_xpm);
#endif
#ifdef __WXGTK__
    wxBitmap refresh = wxArtProvider::GetBitmap("gtk-refresh", wxART_TOOLBAR);
#else
    wxBitmap refresh = wxBitmap(refresh_xpm);
#endif

    m_toolbar_back = m_toolbar->AddTool(wxID_ANY, _("Back"), back);
    m_toolbar_forward = m_toolbar->AddTool(wxID_ANY, _("Forward"), forward);
    m_toolbar_stop = m_toolbar->AddTool(wxID_ANY, _("Stop"), stop);
    m_toolbar_reload = m_toolbar->AddTool(wxID_ANY, _("Reload"), refresh);
    m_url = new wxTextCtrl(m_toolbar, wxID_ANY, "", wxDefaultPosition, wxSize(400, -1), wxTE_PROCESS_ENTER);
    m_toolbar->AddControl(m_url, _("URL"));
    m_toolbar_tools = m_toolbar->AddTool(wxID_ANY, _("Menu"), wxBitmap(wxlogo_xpm));

    m_toolbar->Realize();

    // Create the info panel
    m_info = new wxInfoBar(this);
    topsizer->Add(m_info, wxSizerFlags().Expand());

    // Create a log window
    new wxLogWindow(this, _("Logging"), true, false);

    // Create the webview
    wxString backend = wxWebViewBackendDefault;
#ifdef __WXMSW__
    if (wxWebView::IsBackendAvailable(wxWebViewBackendEdge)) {
        wxLogMessage("Using Edge backend");
        backend = wxWebViewBackendEdge;
    } else {
        wxLogMessage("Edge backend not available");
    }
#endif

    m_webmap = wxWebMap::Create(this, wxID_ANY, url, wxDefaultPosition, wxDefaultSize, backend);
    m_browser = m_webmap->GetWebView();// wxWebView::New(this, wxID_ANY, url, wxDefaultPosition, wxDefaultSize, backend);
    topsizer->Add(m_webmap, wxSizerFlags().Expand().Proportion(1));

    //We register the wxfs:// protocol for testing purposes
    m_browser->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewArchiveHandler("wxfs")));
    //And the memory: file system
    m_browser->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewFSHandler("memory")));

    SetSizer(topsizer);

    //Set a more sensible size for web browsing
    SetSize(wxSize(800, 600));

    // Create the Tools menu
    m_tools_menu = new wxMenu();
    wxMenuItem* print = m_tools_menu->Append(wxID_ANY, _("Print"));
    wxMenuItem* setPage = m_tools_menu->Append(wxID_ANY, _("Set page text"));
    wxMenuItem* viewSource = m_tools_menu->Append(wxID_ANY, _("View Source"));
    wxMenuItem* viewText = m_tools_menu->Append(wxID_ANY, _("View Text"));
    m_tools_menu->AppendSeparator();
    m_tools_layout = m_tools_menu->AppendRadioItem(wxID_ANY, _("Use Layout Zoom"));
    m_tools_tiny = m_tools_menu->AppendRadioItem(wxID_ANY, _("Tiny"));
    m_tools_small = m_tools_menu->AppendRadioItem(wxID_ANY, _("Small"));
    m_tools_medium = m_tools_menu->AppendRadioItem(wxID_ANY, _("Medium"));
    m_tools_large = m_tools_menu->AppendRadioItem(wxID_ANY, _("Large"));
    m_tools_largest = m_tools_menu->AppendRadioItem(wxID_ANY, _("Largest"));
    m_tools_custom = m_tools_menu->AppendRadioItem(wxID_ANY, _("Custom Size"));
    m_tools_menu->AppendSeparator();
    m_tools_handle_navigation = m_tools_menu->AppendCheckItem(wxID_ANY, _("Handle Navigation"));
    m_tools_handle_new_window = m_tools_menu->AppendCheckItem(wxID_ANY, _("Handle New Windows"));
    m_tools_menu->AppendSeparator();

    //History menu
    m_tools_history_menu = new wxMenu();
    wxMenuItem* clearhist = m_tools_history_menu->Append(wxID_ANY, _("Clear History"));
    m_tools_enable_history = m_tools_history_menu->AppendCheckItem(wxID_ANY, _("Enable History"));
    m_tools_history_menu->AppendSeparator();

    m_tools_menu->AppendSubMenu(m_tools_history_menu, "History");

    //Create an editing menu
    wxMenu* editmenu = new wxMenu();
    m_edit_cut = editmenu->Append(wxID_ANY, _("Cut"));
    m_edit_copy = editmenu->Append(wxID_ANY, _("Copy"));
    m_edit_paste = editmenu->Append(wxID_ANY, _("Paste"));
    editmenu->AppendSeparator();
    m_edit_undo = editmenu->Append(wxID_ANY, _("Undo"));
    m_edit_redo = editmenu->Append(wxID_ANY, _("Redo"));
    editmenu->AppendSeparator();
    m_edit_mode = editmenu->AppendCheckItem(wxID_ANY, _("Edit Mode"));

    m_tools_menu->AppendSeparator();
    m_tools_menu->AppendSubMenu(editmenu, "Edit");

    wxMenu* scroll_menu = new wxMenu;
    m_scroll_line_up = scroll_menu->Append(wxID_ANY, "Line &up");
    m_scroll_line_down = scroll_menu->Append(wxID_ANY, "Line &down");
    m_scroll_page_up = scroll_menu->Append(wxID_ANY, "Page u&p");
    m_scroll_page_down = scroll_menu->Append(wxID_ANY, "Page d&own");
    m_tools_menu->AppendSubMenu(scroll_menu, "Scroll");

    wxMenu* script_menu = new wxMenu;
    script_menu->Append(WX_WBMAP_STOCKHOLM, "Put marker on Stockholm", _("Adds a marker at Stockholm"));
    m_script_string = script_menu->Append(wxID_ANY, "Return String");
    m_script_integer = script_menu->Append(wxID_ANY, "Return integer");
    m_script_double = script_menu->Append(wxID_ANY, "Return double");
    m_script_bool = script_menu->Append(wxID_ANY, "Return bool");
    m_script_object = script_menu->Append(wxID_ANY, "Return JSON object");
    m_script_array = script_menu->Append(wxID_ANY, "Return array");
    m_script_dom = script_menu->Append(wxID_ANY, "Modify DOM");
    m_script_undefined = script_menu->Append(wxID_ANY, "Return undefined");
    m_script_null = script_menu->Append(wxID_ANY, "Return null");
    m_script_date = script_menu->Append(wxID_ANY, "Return Date");
#if wxUSE_WEBVIEW_IE
    if (!wxWebView::IsBackendAvailable(wxWebViewBackendEdge)) {
        m_script_object_el = script_menu->Append(wxID_ANY, "Return JSON object changing emulation level");
        m_script_date_el = script_menu->Append(wxID_ANY, "Return Date changing emulation level");
        m_script_array_el = script_menu->Append(wxID_ANY, "Return array changing emulation level");
    }
#endif
    m_script_custom = script_menu->Append(wxID_ANY, "Custom script");
    m_tools_menu->AppendSubMenu(script_menu, _("Run Script"));

    //Selection menu
    wxMenu* selection = new wxMenu();
    m_selection_clear = selection->Append(wxID_ANY, _("Clear Selection"));
    m_selection_delete = selection->Append(wxID_ANY, _("Delete Selection"));
    wxMenuItem* selectall = selection->Append(wxID_ANY, _("Select All"));

    editmenu->AppendSubMenu(selection, "Selection");

    wxMenuItem* loadscheme = m_tools_menu->Append(wxID_ANY, _("Custom Scheme Example"));
    wxMenuItem* usememoryfs = m_tools_menu->Append(wxID_ANY, _("Memory File System Example"));

    m_context_menu = m_tools_menu->AppendCheckItem(wxID_ANY, _("Enable Context Menu"));
    m_dev_tools = m_tools_menu->AppendCheckItem(wxID_ANY, _("Enable Dev Tools"));

    //By default we want to handle navigation and new windows
    m_tools_handle_navigation->Check();
    m_tools_handle_new_window->Check();
    m_tools_enable_history->Check();

    //Zoom
    m_zoomFactor = 100;
    m_tools_medium->Check();

    if (!m_browser->CanSetZoomType(wxWEBVIEW_ZOOM_TYPE_LAYOUT)) {
        m_tools_layout->Enable(false);
    }

    // Connect the toolbar events
    Bind(wxEVT_TOOL, &WebFrame::OnBack, this, m_toolbar_back->GetId());
    Bind(wxEVT_TOOL, &WebFrame::OnForward, this, m_toolbar_forward->GetId());
    Bind(wxEVT_TOOL, &WebFrame::OnStop, this, m_toolbar_stop->GetId());
    Bind(wxEVT_TOOL, &WebFrame::OnReload, this, m_toolbar_reload->GetId());
    Bind(wxEVT_TOOL, &WebFrame::OnToolsClicked, this, m_toolbar_tools->GetId());

    Bind(wxEVT_TEXT_ENTER, &WebFrame::OnUrl, this, m_url->GetId());

    // Connect the webview events
    Bind(wxEVT_WEBVIEW_NAVIGATING, &WebFrame::OnNavigationRequest, this, m_browser->GetId());
    Bind(wxEVT_WEBVIEW_NAVIGATED, &WebFrame::OnNavigationComplete, this, m_browser->GetId());
    Bind(wxEVT_WEBVIEW_LOADED, &WebFrame::OnDocumentLoaded, this, m_browser->GetId());
    Bind(wxEVT_WEBVIEW_ERROR, &WebFrame::OnError, this, m_browser->GetId());
    Bind(wxEVT_WEBVIEW_NEWWINDOW, &WebFrame::OnNewWindow, this, m_browser->GetId());
    Bind(wxEVT_WEBVIEW_TITLE_CHANGED, &WebFrame::OnTitleChanged, this, m_browser->GetId());

    // Connect the menu events
    Bind(wxEVT_MENU, &WebFrame::OnSetPage, this, setPage->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnViewSourceRequest, this, viewSource->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnViewTextRequest, this, viewText->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnPrint, this, print->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnZoomLayout, this, m_tools_layout->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnSetZoom, this, m_tools_tiny->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnSetZoom, this, m_tools_small->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnSetZoom, this, m_tools_medium->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnSetZoom, this, m_tools_large->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnSetZoom, this, m_tools_largest->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnSetZoom, this, m_tools_custom->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnClearHistory, this, clearhist->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnEnableHistory, this, m_tools_enable_history->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnCut, this, m_edit_cut->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnCopy, this, m_edit_copy->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnPaste, this, m_edit_paste->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnUndo, this, m_edit_undo->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnRedo, this, m_edit_redo->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnMode, this, m_edit_mode->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnScrollLineUp, this, m_scroll_line_up->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnScrollLineDown, this, m_scroll_line_down->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnScrollPageUp, this, m_scroll_page_up->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnScrollPageDown, this, m_scroll_page_down->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnRunScriptString, this, m_script_string->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnRunScriptInteger, this, m_script_integer->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnRunScriptDouble, this, m_script_double->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnRunScriptBool, this, m_script_bool->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnRunScriptObject, this, m_script_object->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnRunScriptArray, this, m_script_array->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnRunScriptDOM, this, m_script_dom->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnRunScriptUndefined, this, m_script_undefined->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnRunScriptNull, this, m_script_null->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnRunScriptDate, this, m_script_date->GetId());
#if wxUSE_WEBVIEW_IE
    if (!wxWebView::IsBackendAvailable(wxWebViewBackendEdge)) {
        Bind(wxEVT_MENU, &WebFrame::OnRunScriptObjectWithEmulationLevel, this, m_script_object_el->GetId());
        Bind(wxEVT_MENU, &WebFrame::OnRunScriptDateWithEmulationLevel, this, m_script_date_el->GetId());
        Bind(wxEVT_MENU, &WebFrame::OnRunScriptArrayWithEmulationLevel, this, m_script_array_el->GetId());
    }
#endif
    Bind(wxEVT_MENU, &WebFrame::OnRunScriptCustom, this, m_script_custom->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnClearSelection, this, m_selection_clear->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnDeleteSelection, this, m_selection_delete->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnSelectAll, this, selectall->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnLoadScheme, this, loadscheme->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnUseMemoryFS, this, usememoryfs->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnEnableContextMenu, this, m_context_menu->GetId());
    Bind(wxEVT_MENU, &WebFrame::OnEnableDevTools, this, m_dev_tools->GetId());

    //Connect the idle events
    Bind(wxEVT_IDLE, &WebFrame::OnIdle, this);
}

WebFrame::~WebFrame()
{
    delete m_tools_menu;
}

//wxWebView* WebFrame::GetBrowser()
//{
//    return m_browser;
//}
//
wxWebMap* WebFrame::GetWebMap()
{
    return m_webmap;
}

/**
  * Method that retrieves the current state from the web control and updates the GUI
  * the reflect this current state.
  */
void WebFrame::UpdateState()
{
    m_toolbar->EnableTool(m_toolbar_back->GetId(), m_browser->CanGoBack());
    m_toolbar->EnableTool(m_toolbar_forward->GetId(), m_browser->CanGoForward());

    if (m_browser->IsBusy()) {
        m_toolbar->EnableTool(m_toolbar_stop->GetId(), true);
    } else {
        m_toolbar->EnableTool(m_toolbar_stop->GetId(), false);
    }

    SetTitle(m_browser->GetCurrentTitle());
    m_url->SetValue(m_browser->GetCurrentURL());
}

void WebFrame::OnIdle(wxIdleEvent& WXUNUSED(evt))
{
    if (m_browser->IsBusy()) {
        wxSetCursor(wxCURSOR_ARROWWAIT);
        m_toolbar->EnableTool(m_toolbar_stop->GetId(), true);
    } else {
        wxSetCursor(wxNullCursor);
        m_toolbar->EnableTool(m_toolbar_stop->GetId(), false);
    }
}

/**
  * Callback invoked when user entered an URL and pressed enter
  */
void WebFrame::OnUrl(wxCommandEvent& WXUNUSED(evt))
{
    m_browser->LoadURL(m_url->GetValue());
    m_browser->SetFocus();
    UpdateState();
}

/**
    * Callback invoked when user pressed the "back" button
    */
void WebFrame::OnBack(wxCommandEvent& WXUNUSED(evt))
{
    m_browser->GoBack();
    UpdateState();
}

/**
  * Callback invoked when user pressed the "forward" button
  */
void WebFrame::OnForward(wxCommandEvent& WXUNUSED(evt))
{
    m_browser->GoForward();
    UpdateState();
}

/**
  * Callback invoked when user pressed the "stop" button
  */
void WebFrame::OnStop(wxCommandEvent& WXUNUSED(evt))
{
    m_browser->Stop();
    UpdateState();
}

/**
  * Callback invoked when user pressed the "reload" button
  */
void WebFrame::OnReload(wxCommandEvent& WXUNUSED(evt))
{
    m_browser->Reload();
    UpdateState();
}

void WebFrame::OnClearHistory(wxCommandEvent& WXUNUSED(evt))
{
    m_browser->ClearHistory();
    UpdateState();
}

void WebFrame::OnEnableHistory(wxCommandEvent& WXUNUSED(evt))
{
    m_browser->EnableHistory(m_tools_enable_history->IsChecked());
    UpdateState();
}

void WebFrame::OnCut(wxCommandEvent& WXUNUSED(evt))
{
    m_browser->Cut();
}

void WebFrame::OnCopy(wxCommandEvent& WXUNUSED(evt))
{
    m_browser->Copy();
}

void WebFrame::OnPaste(wxCommandEvent& WXUNUSED(evt))
{
    m_browser->Paste();
}

void WebFrame::OnUndo(wxCommandEvent& WXUNUSED(evt))
{
    m_browser->Undo();
}

void WebFrame::OnRedo(wxCommandEvent& WXUNUSED(evt))
{
    m_browser->Redo();
}

void WebFrame::OnMode(wxCommandEvent& WXUNUSED(evt))
{
    m_browser->SetEditable(m_edit_mode->IsChecked());
}

void WebFrame::OnLoadScheme(wxCommandEvent& WXUNUSED(evt))
{
    wxPathList pathlist;
    pathlist.Add(".");
    pathlist.Add("..");
    pathlist.Add("../help");
    pathlist.Add("../../../samples/help");

    wxFileName helpfile(pathlist.FindValidPath("doc.zip"));
    helpfile.MakeAbsolute();
    wxString path = helpfile.GetFullPath();
    //Under MSW we need to flip the slashes
    path.Replace("\\", "/");
    path = "wxfs:///" + path + ";protocol=zip/doc.htm";
    m_browser->LoadURL(path);
}

void WebFrame::OnUseMemoryFS(wxCommandEvent& WXUNUSED(evt))
{
    m_browser->LoadURL("memory:page1.htm");
}

void WebFrame::OnEnableContextMenu(wxCommandEvent& evt)
{
    m_browser->EnableContextMenu(evt.IsChecked());
}

void WebFrame::OnEnableDevTools(wxCommandEvent& evt)
{
    m_browser->EnableAccessToDevTools(evt.IsChecked());
}

/**
  * Callback invoked when there is a request to load a new page (for instance
  * when the user clicks a link)
  */
void WebFrame::OnNavigationRequest(wxWebViewEvent& evt)
{
    if (m_info->IsShown()) {
        m_info->Dismiss();
    }

    wxLogMessage("%s", "Navigation request to '" + evt.GetURL() + "' (target='" +
                 evt.GetTarget() + "')");

    wxASSERT(m_browser->IsBusy());

    //If we don't want to handle navigation then veto the event and navigation
    //will not take place, we also need to stop the loading animation
    if (!m_tools_handle_navigation->IsChecked()) {
        evt.Veto();
        m_toolbar->EnableTool(m_toolbar_stop->GetId(), false);
    } else {
        UpdateState();
    }
}

/**
  * Callback invoked when a navigation request was accepted
  */
void WebFrame::OnNavigationComplete(wxWebViewEvent& evt)
{
    wxLogMessage("%s", "Navigation complete; url='" + evt.GetURL() + "'");
    UpdateState();
}

/**
  * Callback invoked when a page is finished loading
  */
void WebFrame::OnDocumentLoaded(wxWebViewEvent& evt)
{
    //Only notify if the document is the main frame, not a subframe
    if (evt.GetURL() == m_browser->GetCurrentURL()) {
        wxLogMessage("%s", "Document loaded; url='" + evt.GetURL() + "'");
    }
    UpdateState();
}

/**
  * On new window, we veto to stop extra windows appearing
  */
void WebFrame::OnNewWindow(wxWebViewEvent& evt)
{
    wxString flag = " (other)";

    if (evt.GetNavigationAction() == wxWEBVIEW_NAV_ACTION_USER) {
        flag = " (user)";
    }

    wxLogMessage("%s", "New window; url='" + evt.GetURL() + "'" + flag);

    //If we handle new window events then just load them in this window as we
    //are a single window browser
    if (m_tools_handle_new_window->IsChecked()) {
        m_browser->LoadURL(evt.GetURL());
    }

    UpdateState();
}

void WebFrame::OnTitleChanged(wxWebViewEvent& evt)
{
    SetTitle(evt.GetString());
    wxLogMessage("%s", "Title changed; title='" + evt.GetString() + "'");
}

void WebFrame::OnSetPage(wxCommandEvent& WXUNUSED(evt))
{
    m_browser->SetPage
    (
        "<html><title>New Page</title>"
        "<body>Created using <tt>SetPage()</tt> method.</body></html>",
        wxString()
    );
}

/**
  * Invoked when user selects the "View Source" menu item
  */
void WebFrame::OnViewSourceRequest(wxCommandEvent& WXUNUSED(evt))
{
    SourceViewDialog dlg(this, m_browser->GetPageSource());
    dlg.ShowModal();
}

/**
 * Invoked when user selects the "View Text" menu item
 */
void WebFrame::OnViewTextRequest(wxCommandEvent& WXUNUSED(evt))
{
    wxDialog textViewDialog(this, wxID_ANY, "Page Text",
                            wxDefaultPosition, wxSize(700, 500),
                            wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
#if 0 //wxUSE_STC
    wxStyledTextCtrl* text = new wxStyledTextCtrl(&textViewDialog, wxID_ANY);
    text->SetText(m_browser->GetPageText());
#else // !wxUSE_STC
    wxTextCtrl* text = new wxTextCtrl(this, wxID_ANY, m_browser->GetPageText(),
                                      wxDefaultPosition, wxDefaultSize,
                                      wxTE_MULTILINE |
                                      wxTE_RICH |
                                      wxTE_READONLY);
#endif // wxUSE_STC/!wxUSE_STC
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(text, 1, wxEXPAND);
    SetSizer(sizer);
    textViewDialog.ShowModal();
}

/**
  * Invoked when user selects the "Menu" item
  */
void WebFrame::OnToolsClicked(wxCommandEvent& WXUNUSED(evt))
{
    if (m_browser->GetCurrentURL() == "") {
        return;
    }

    m_edit_cut->Enable(m_browser->CanCut());
    m_edit_copy->Enable(m_browser->CanCopy());
    m_edit_paste->Enable(m_browser->CanPaste());

    m_edit_undo->Enable(m_browser->CanUndo());
    m_edit_redo->Enable(m_browser->CanRedo());

    m_selection_clear->Enable(m_browser->HasSelection());
    m_selection_delete->Enable(m_browser->HasSelection());

    m_context_menu->Check(m_browser->IsContextMenuEnabled());
    m_dev_tools->Check(m_browser->IsAccessToDevToolsEnabled());

    //Firstly we clear the existing menu items, then we add the current ones
    wxMenuHistoryMap::const_iterator it;
    for (it = m_histMenuItems.begin(); it != m_histMenuItems.end(); ++it) {
        m_tools_history_menu->Destroy(it->first);
    }
    m_histMenuItems.clear();

    wxVector<wxSharedPtr<wxWebViewHistoryItem> > back = m_browser->GetBackwardHistory();
    wxVector<wxSharedPtr<wxWebViewHistoryItem> > forward = m_browser->GetForwardHistory();

    wxMenuItem* item;

    unsigned int i;
    for (i = 0; i < back.size(); i++) {
        item = m_tools_history_menu->AppendRadioItem(wxID_ANY, back[i]->GetTitle());
        m_histMenuItems[item->GetId()] = back[i];
        Bind(wxEVT_MENU, &WebFrame::OnHistory, this, item->GetId());
    }

    wxString title = m_browser->GetCurrentTitle();
    if (title.empty()) {
        title = "(untitled)";
    }
    item = m_tools_history_menu->AppendRadioItem(wxID_ANY, title);
    item->Check();

    //No need to connect the current item
    m_histMenuItems[item->GetId()] = wxSharedPtr<wxWebViewHistoryItem>(new wxWebViewHistoryItem(m_browser->GetCurrentURL(), m_browser->GetCurrentTitle()));

    for (i = 0; i < forward.size(); i++) {
        item = m_tools_history_menu->AppendRadioItem(wxID_ANY, forward[i]->GetTitle());
        m_histMenuItems[item->GetId()] = forward[i];
        Bind(wxEVT_TOOL, &WebFrame::OnHistory, this, item->GetId());
    }

    wxPoint position = ScreenToClient(wxGetMousePosition());
    PopupMenu(m_tools_menu, position.x, position.y);
}

/**
  * Invoked when user selects the zoom size in the menu
  */
void WebFrame::OnSetZoom(wxCommandEvent& evt)
{
    if (evt.GetId() == m_tools_tiny->GetId()) {
        m_browser->SetZoom(wxWEBVIEW_ZOOM_TINY);
    } else if (evt.GetId() == m_tools_small->GetId()) {
        m_browser->SetZoom(wxWEBVIEW_ZOOM_SMALL);
    } else if (evt.GetId() == m_tools_medium->GetId()) {
        m_browser->SetZoom(wxWEBVIEW_ZOOM_MEDIUM);
    } else if (evt.GetId() == m_tools_large->GetId()) {
        m_browser->SetZoom(wxWEBVIEW_ZOOM_LARGE);
    } else if (evt.GetId() == m_tools_largest->GetId()) {
        m_browser->SetZoom(wxWEBVIEW_ZOOM_LARGEST);
    } else if (evt.GetId() == m_tools_custom->GetId()) {
        OnZoomCustom(evt);
    } else {
        wxFAIL;
    }
}

void WebFrame::OnZoomLayout(wxCommandEvent& WXUNUSED(evt))
{
    if (m_tools_layout->IsChecked()) {
        m_browser->SetZoomType(wxWEBVIEW_ZOOM_TYPE_LAYOUT);
    } else {
        m_browser->SetZoomType(wxWEBVIEW_ZOOM_TYPE_TEXT);
    }
}

void WebFrame::OnZoomCustom(wxCommandEvent& WXUNUSED(evt))
{
    wxNumberEntryDialog dialog
    (
        this,
        "Enter zoom factor as a percentage (10-10000)%",
        "Zoom Factor:",
        "Change Zoom Factor",
        m_zoomFactor,
        10, 10000
    );
    if (dialog.ShowModal() != wxID_OK) {
        return;
    }

    m_zoomFactor = dialog.GetValue();
    m_browser->SetZoomFactor((float)m_zoomFactor / 100);
}

void WebFrame::OnHistory(wxCommandEvent& evt)
{
    m_browser->LoadHistoryItem(m_histMenuItems[evt.GetId()]);
}

void WebFrame::RunScript(const wxString& javascript)
{
    // Remember the script we run in any case, so the next time the user opens
    // the "Run Script" dialog box, it is shown there for convenient updating.
    m_javascript = javascript;

    wxLogMessage("Running JavaScript:\n%s\n", javascript);

    wxString result;
    if (m_browser->RunScript(javascript, &result)) {
        wxLogMessage("RunScript() returned \"%s\"", result);
    } else {
        wxLogWarning("RunScript() failed");
    }
}

void WebFrame::OnMarkStockholm(wxCommandEvent& e)
{
    // Neither draggable nor removable
    wxMapMarker marker(59.326180, 18.072263);
    wxString res;
    m_webmap->AddMapObject(marker, &res);
    wxLogMessage(_("Added leaflet object #%s"), res);
}

void WebFrame::OnRunScriptString(wxCommandEvent& WXUNUSED(evt))
{
    RunScript("function f(a){return a;}f('Hello World!');");
}

void WebFrame::OnRunScriptInteger(wxCommandEvent& WXUNUSED(evt))
{
    RunScript("function f(a){return a;}f(123);");
}

void WebFrame::OnRunScriptDouble(wxCommandEvent& WXUNUSED(evt))
{
    RunScript("function f(a){return a;}f(2.34);");
}

void WebFrame::OnRunScriptBool(wxCommandEvent& WXUNUSED(evt))
{
    RunScript("function f(a){return a;}f(false);");
}

void WebFrame::OnRunScriptObject(wxCommandEvent& WXUNUSED(evt))
{
    RunScript("function f(){var person = new Object();person.name = 'Foo'; \
        person.lastName = 'Bar';return person;}f();");
}

void WebFrame::OnRunScriptArray(wxCommandEvent& WXUNUSED(evt))
{
    RunScript("function f(){ return [\"foo\", \"bar\"]; }f();");
}

void WebFrame::OnRunScriptDOM(wxCommandEvent& WXUNUSED(evt))
{
    RunScript("document.write(\"Hello World!\");");
}

void WebFrame::OnRunScriptUndefined(wxCommandEvent& WXUNUSED(evt))
{
    RunScript("function f(){var person = new Object();}f();");
}

void WebFrame::OnRunScriptNull(wxCommandEvent& WXUNUSED(evt))
{
    RunScript("function f(){return null;}f();");
}

void WebFrame::OnRunScriptDate(wxCommandEvent& WXUNUSED(evt))
{
    RunScript("function f(){var d = new Date('10/08/2017 21:30:40'); \
        var tzoffset = d.getTimezoneOffset() * 60000; \
        return new Date(d.getTime() - tzoffset);}f();");
}

#if wxUSE_WEBVIEW_IE
void WebFrame::OnRunScriptObjectWithEmulationLevel(wxCommandEvent& WXUNUSED(evt))
{
    wxWebViewIE::MSWSetModernEmulationLevel();
    RunScript("function f(){var person = new Object();person.name = 'Foo'; \
        person.lastName = 'Bar';return person;}f();");
    wxWebViewIE::MSWSetModernEmulationLevel(false);
}

void WebFrame::OnRunScriptDateWithEmulationLevel(wxCommandEvent& WXUNUSED(evt))
{
    wxWebViewIE::MSWSetModernEmulationLevel();
    RunScript("function f(){var d = new Date('10/08/2017 21:30:40'); \
        var tzoffset = d.getTimezoneOffset() * 60000; return \
        new Date(d.getTime() - tzoffset);}f();");
    wxWebViewIE::MSWSetModernEmulationLevel(false);
}

void WebFrame::OnRunScriptArrayWithEmulationLevel(wxCommandEvent& WXUNUSED(evt))
{
    wxWebViewIE::MSWSetModernEmulationLevel();
    RunScript("function f(){ return [\"foo\", \"bar\"]; }f();");
    wxWebViewIE::MSWSetModernEmulationLevel(false);
}
#endif

void WebFrame::OnRunScriptCustom(wxCommandEvent& WXUNUSED(evt))
{
    wxTextEntryDialog dialog
    (
        this,
        "Please enter JavaScript code to execute",
        wxGetTextFromUserPromptStr,
        m_javascript,
        wxOK | wxCANCEL | wxCENTRE | wxTE_MULTILINE
    );
    if (dialog.ShowModal() != wxID_OK) {
        return;
    }

    RunScript(dialog.GetValue());
}

void WebFrame::OnClearSelection(wxCommandEvent& WXUNUSED(evt))
{
    m_browser->ClearSelection();
}

void WebFrame::OnDeleteSelection(wxCommandEvent& WXUNUSED(evt))
{
    m_browser->DeleteSelection();
}

void WebFrame::OnSelectAll(wxCommandEvent& WXUNUSED(evt))
{
    m_browser->SelectAll();
}

/**
  * Callback invoked when a loading error occurs
  */
void WebFrame::OnError(wxWebViewEvent& evt)
{
#define WX_ERROR_CASE(type) \
    case type: \
        category = #type; \
        break;

    wxString category;
    switch (evt.GetInt()) {
        WX_ERROR_CASE(wxWEBVIEW_NAV_ERR_CONNECTION);
        WX_ERROR_CASE(wxWEBVIEW_NAV_ERR_CERTIFICATE);
        WX_ERROR_CASE(wxWEBVIEW_NAV_ERR_AUTH);
        WX_ERROR_CASE(wxWEBVIEW_NAV_ERR_SECURITY);
        WX_ERROR_CASE(wxWEBVIEW_NAV_ERR_NOT_FOUND);
        WX_ERROR_CASE(wxWEBVIEW_NAV_ERR_REQUEST);
        WX_ERROR_CASE(wxWEBVIEW_NAV_ERR_USER_CANCELLED);
        WX_ERROR_CASE(wxWEBVIEW_NAV_ERR_OTHER);
    }

    wxLogMessage("%s", "Error; url='" + evt.GetURL() + "', error='" + category + " (" + evt.GetString() + ")'");

    //Show the info bar with an error
    m_info->ShowMessage(_("An error occurred loading ") + evt.GetURL() + "\n" +
                        "'" + category + "'", wxICON_ERROR);

    UpdateState();
}

/**
  * Invoked when user selects "Print" from the menu
  */
void WebFrame::OnPrint(wxCommandEvent& WXUNUSED(evt))
{
    m_browser->Print();
}

void WebFrame::OnScrollLineUp(wxCommandEvent&)
{
    m_browser->LineUp();
}
void WebFrame::OnScrollLineDown(wxCommandEvent&)
{
    m_browser->LineDown();
}
void WebFrame::OnScrollPageUp(wxCommandEvent&)
{
    m_browser->PageUp();
}
void WebFrame::OnScrollPageDown(wxCommandEvent&)
{
    m_browser->PageDown();
}
