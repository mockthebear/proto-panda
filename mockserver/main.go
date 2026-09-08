package main

/**
    SUPER SLOPPY WEB SERVER TO SIMULATE PROTOPANDA WEB INTERFACE
    just do a:
    go run .
**/
import (
	"fmt"
	"html/template"
	"io"
	"log"
	"net/http"
	"os"
	"path/filepath"
	"sort"
	"strings"
	"sync"
	"time"
)

const (
	Port                   = 8080
	PandaVersion           = "2.0.0"
	BasePath               = "../"
	DownloadSpeedPerSecond = 1028 * 10
	MaxConcurrentRequests  = 3
	ChunkSize              = 2048
)

var (
	indexTemplate = template.Must(template.New("index").Parse(`<html>
<head>
    <title>Directory Listing</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
      * {
        box-sizing: border-box;
      }
      body {
        font-family: 'Courier New', monospace;
        margin: 15px;
        padding: 0;
        display: flex;
        flex-direction: column;
        min-height: 100vh;
        background: #1a1a1a;
        color: #ccc;
        font-size: 18px;
      }
      @media (min-width: 768px) {
        body {
          margin: 30px;
          font-size: 20px;
        }
      }
      h1 {
        color: #ff9900;
        margin-bottom: 20px;
        font-size: 28px;
        font-weight: bold;
        word-break: break-word;
      }
      @media (min-width: 768px) {
        h1 {
          font-size: 36px;
          margin-bottom: 25px;
        }
      }
      .table-responsive {
        overflow-x: auto;
        width: 100%;
      }
      table {
        border-collapse: collapse;
        width: 100%;
        margin-bottom: 20px;
        box-shadow: 0 1px 3px rgba(0,0,0,0.3);
        background: #2d2d2d;
      }
      th, td {
        border: 1px solid #444;
        padding: 12px 10px;
        text-align: left;
        font-size: 16px;
      }
      @media (min-width: 768px) {
        th, td {
          padding: 15px;
          font-size: 18px;
        }
      }
      th {
        background-color: #3c3c3c;
        font-weight: bold;
        color: #ff9900;
      }
      tr:nth-child(even) {
        background-color: #252525;
      }
      tr:hover {
        background-color: #3a3a3a;
      }
      .action-buttons {
        display: flex;
        flex-direction: column;
        gap: 15px;
        margin-top: 20px;
      }
      @media (min-width: 768px) {
        .action-buttons {
          flex-direction: row;
          gap: 25px;
        }
      }
      .action-box {
        flex: 1;
        padding: 20px;
        background-color: #2d2d2d;
        border-radius: 8px;
        box-shadow: 0 1px 3px rgba(0,0,0,0.3);
        border: 1px solid #444;
      }
      @media (min-width: 768px) {
        .action-box {
          padding: 25px;
        }
      }
      .action-box h3 {
        margin-top: 0;
        color: #ff9900;
        font-size: 20px;
        margin-bottom: 15px;
      }
      @media (min-width: 768px) {
        .action-box h3 {
          font-size: 24px;
          margin-bottom: 20px;
        }
      }
      .form-row {
        margin-bottom: 15px;
      }
      footer {
        margin-top: 30px;
        padding: 15px 0;
        text-align: center;
        color: #666;
        font-size: 12px;
        border-top: 1px solid #444;
      }
      @media (min-width: 768px) {
        footer {
          margin-top: 40px;
          padding: 20px 0;
          font-size: 14px;
        }
      }
      .form-row label {
        display: block;
        margin-bottom: 6px;
        font-weight: bold;
        color: #aaa;
        font-size: 15px;
      }
      @media (min-width: 768px) {
        .form-row label {
          margin-bottom: 8px;
          font-size: 16px;
        }
      }
      input[type="text"],
      input[type="file"] {
        width: 100%;
        padding: 10px;
        border: 1px solid #555;
        border-radius: 4px;
        box-sizing: border-box;
        background: #1e1e1e;
        color: #fff;
        font-family: 'Courier New', monospace;
        font-size: 16px;
      }
      @media (min-width: 768px) {
        input[type="text"],
        input[type="file"] {
          padding: 12px;
          font-size: 18px;
        }
      }
      input[type="text"]:focus,
      input[type="file"]:focus {
        outline: none;
        border-color: #ff9900;
      }
      .btn {
        padding: 10px 20px;
        border: none;
        border-radius: 4px;
        cursor: pointer;
        font-weight: bold;
        transition: all 0.1s ease;
        text-align: center;
        font-family: 'Courier New', monospace;
        font-size: 16px;
        width: 100%;
      }
      @media (min-width: 768px) {
        .btn {
          padding: 12px 24px;
          font-size: 18px;
          width: auto;
        }
      }
      .btn-primary {
        background-color: #ff9900;
        color: #111;
      }
      .btn-primary:hover {
        background-color: #ffaa33;
        transform: translateY(-1px);
      }
      .btn-danger {
        background-color: #ff6666;
        color: #111;
      }
      .btn-danger:hover {
        background-color: #ff4444;
        transform: translateY(-1px);
      }
      .status {
        margin-top: 10px;
        padding: 10px;
        border-radius: 4px;
        font-size: 14px;
        word-break: break-word;
      }
      @media (min-width: 768px) {
        .status {
          margin-top: 15px;
          padding: 12px;
          font-size: 15px;
        }
      }
      .status-success {
        background-color: #1e3a1e;
        color: #4CAF50;
        border: 1px solid #4CAF50;
      }
      .status-error {
        background-color: #3a1e1e;
        color: #ff6666;
        border: 1px solid #ff6666;
      }
      a {
        color: #9cdcfe;
        text-decoration: none;
        font-size: 16px;
        word-break: break-word;
      }
      @media (min-width: 768px) {
        a {
          font-size: 18px;
        }
      }
      a:hover {
        color: #ff9900;
        text-decoration: underline;
      }
      .breadcrumb {
        margin-bottom: 20px;
        font-size: 18px;
        color: #ff9900;
        font-weight: bold;
        padding: 12px;
        background: #2d2d2d;
        border-radius: 6px;
        border: 1px solid #444;
        word-break: break-word;
        overflow-x: auto;
      }
      @media (min-width: 768px) {
        .breadcrumb {
          margin-bottom: 25px;
          font-size: 24px;
          padding: 15px;
        }
      }
      .breadcrumb a {
        color: #9cdcfe;
        text-decoration: none;
        font-size: 18px;
        font-weight: normal;
      }
      @media (min-width: 768px) {
        .breadcrumb a {
          font-size: 24px;
        }
      }
      .breadcrumb a:hover {
        color: #ff9900;
        text-decoration: underline;
      }
      .main-header {
        background-color: #2d2d2d;
        padding: 20px;
        border-radius: 8px;
        margin-bottom: 20px;
        display: flex;
        flex-direction: column;
        align-items: center;
        gap: 15px;
        border: 1px solid #444;
      }
      @media (min-width: 768px) {
        .main-header {
          padding: 35px;
          margin-bottom: 25px;
          gap: 25px;
        }
      }
      .main-header p {
        margin: 0;
        font-size: 22px;
        color: #ff9900;
        font-weight: bold;
        text-align: center;
      }
      @media (min-width: 768px) {
        .main-header p {
          font-size: 28px;
        }
      }
      .header-links {
        display: flex;
        flex-direction: column;
        gap: 15px;
        align-items: center;
        width: 100%;
      }
      @media (min-width: 768px) {
        .header-links {
          flex-direction: row;
          gap: 30px;
        }
      }
      .editor-btn {
        padding: 12px 20px;
        font-size: 16px;
        background-color: #ff9900 !important;
        color: #111 !important;
        width: 100%;
        text-align: center;
      }
      @media (min-width: 768px) {
        .editor-btn {
          padding: 15px 30px;
          font-size: 18px;
          width: auto;
        }
      }
      .editor-btn:hover {
        background-color: #ffaa33 !important;
        transform: translateY(-2px);
      }
      .logo-container {
        display: flex;
        justify-content: center;
        margin-bottom: 20px;
      }
      .logo-img {
        max-width: 100%;
        height: auto;
        border-radius: 5px;
      }
      td .btn-danger {
        width: auto;
        padding: 6px 12px;
        font-size: 13px;
      }
      @media (min-width: 768px) {
        td .btn-danger {
          padding: 8px 16px;
          font-size: 15px;
        }
      }

      /* Compact directory view while preserving the original theme. */
      .table-responsive { overflow: visible; }
      table { margin-bottom: 16px; }
      th, td { padding: 8px 10px; font-size: 14px; }
      th:nth-child(2), td:nth-child(2), th:nth-child(3), td:nth-child(3) { color: #888; width: 90px; }
      td:last-child { width: 1%; white-space: nowrap; text-align: right; }
      td .btn { display: inline-block; width: auto; padding: 5px 9px; font-size: 12px; margin-left: 4px; }
      .file-name { display: inline-flex; align-items: center; gap: 8px; }
      .file-icon { position: relative; display: inline-block; width: 18px; height: 18px; flex: 0 0 18px; color: #ff9900; }
      .file-icon.folder-icon { margin-top: 3px; height: 13px; border: 2px solid #ffb52e; border-radius: 2px; background: #8a5700; }
      .file-icon.folder-icon::before { content: ''; position: absolute; left: 1px; top: -5px; width: 8px; height: 5px; border: 2px solid #ffb52e; border-bottom: 0; border-radius: 2px 2px 0 0; background: #8a5700; }
      .file-icon.document-icon { border: 2px solid #b9c8d6; border-radius: 2px; background: #e7edf2; }
      .file-icon.document-icon::before { content: ''; position: absolute; right: -2px; top: -2px; width: 6px; height: 6px; border-left: 2px solid #b9c8d6; border-bottom: 2px solid #b9c8d6; background: #6f7d89; }
      .toolbar { display: grid; grid-template-columns: repeat(2, minmax(0, 1fr)); gap: 12px; margin: 14px 0; }
      .toolbar .action-box { padding: 14px; }
      .toolbar .action-box h3 { font-size: 17px; margin-bottom: 10px; }
      .toolbar .form-row { margin-bottom: 10px; }
      .toolbar input[type="text"], .toolbar input[type="file"] { padding: 8px; font-size: 14px; }
      .toolbar .btn { padding: 9px 12px; font-size: 14px; }
      .modal { display: none; position: fixed; inset: 0; z-index: 10; background: rgba(0,0,0,.72); padding: 5vh 16px; overflow-y: auto; -webkit-overflow-scrolling: touch; }
      .modal.open { display: flex; align-items: center; justify-content: center; }
      .modal-card { width: min(900px, 100%); max-height: 90vh; min-height: 0; display: flex; flex-direction: column; background: #2d2d2d; border: 1px solid #555; border-radius: 8px; padding: 16px; box-shadow: 0 8px 30px #000; }
      .modal-head { display: flex; align-items: center; justify-content: space-between; gap: 12px; margin-bottom: 12px; }
      .modal-head h2 { color: #ff9900; font-size: 20px; margin: 0; overflow: hidden; text-overflow: ellipsis; white-space: nowrap; }
      .code-editor { flex: 1 1 auto; height: min(600px, 62vh); min-height: 0; border: 1px solid #555; border-radius: 4px; overflow: hidden; background: #1e1e1e; }
      .code-editor textarea { display: block; width: 100%; height: 100%; resize: none; border: 0; outline: 0; padding: 12px; background: #1e1e1e; color: #eee; font-family: 'Courier New', monospace; font-size: 14px; line-height: 1.45; tab-size: 2; white-space: pre; overflow: auto; -webkit-overflow-scrolling: touch; touch-action: pan-x pan-y; }
      .delete-dialog { width: min(420px, calc(100vw - 28px)); padding: 0; border: 1px solid #555; border-radius: 8px; background: #2d2d2d; color: #ccc; box-shadow: 0 8px 30px #000; }
      .delete-dialog::backdrop { background: rgba(0,0,0,.72); }
      .dialog-content { padding: 20px; }
      .dialog-content h2 { margin: 0 0 9px; color: #ff9900; font-size: 20px; }
      .dialog-content p { margin: 0; overflow-wrap: anywhere; line-height: 1.4; }
      .dialog-actions { display: flex; justify-content: flex-end; gap: 8px; margin-top: 20px; }
      .dialog-actions .btn { width: auto; min-width: 88px; }
      .toast { position: fixed; z-index: 20; left: 50%; bottom: 20px; transform: translate(-50%, 20px); max-width: min(500px, calc(100vw - 28px)); padding: 12px 16px; border: 1px solid #4CAF50; border-radius: 5px; background: #1e3a1e; color: #8be28b; opacity: 0; pointer-events: none; transition: opacity .18s ease, transform .18s ease; }
      .toast.error { border-color: #ff6666; background: #3a1e1e; color: #ff9999; }
      .toast.show { opacity: 1; transform: translate(-50%, 0); }
      .modal-actions { display: flex; justify-content: flex-end; gap: 8px; margin-top: 12px; }
      .modal-actions .btn { width: auto; }
      .btn-secondary { background: #555; color: #eee; }
      @media (max-width: 620px) {
        body { margin: 10px; font-size: 16px; }
        .main-header { padding: 16px; }
        .main-header p { font-size: 19px; }
        .breadcrumb { margin-bottom: 12px; padding: 9px; font-size: 15px; }
        .breadcrumb a { font-size: 15px; }
        h1 { font-size: 22px; margin: 14px 0 10px; }
        .toolbar { grid-template-columns: 1fr; }
        table { display: block; background: transparent; box-shadow: none; }
        thead, th:nth-child(2), td:nth-child(2), th:nth-child(3), td:nth-child(3) { display: none; }
        tbody { display: block; }
        tr { display: grid; grid-template-columns: minmax(0, 1fr) auto; gap: 5px; align-items: center; border: 1px solid #444; background: #2d2d2d; margin-bottom: 5px; border-radius: 4px; }
        td { border: 0; padding: 9px; }
        td:first-child { min-width: 0; }
        td:last-child { width: auto; }
        .file-name a { overflow: hidden; text-overflow: ellipsis; white-space: nowrap; max-width: 45vw; }
        .modal { padding: 12px; }
        .modal-card { max-height: 96vh; padding: 12px; }
        .code-editor { height: 62vh; min-height: 0; }
        .code-editor textarea { font-size: 13px; }
      }
    </style>
  </head>
  <body>
    <div class="breadcrumb">
        {{range .Breadcrumbs}}<a href="/{{.Path}}">{{.Name}}</a> > {{end}}{{.CurrentDir}}
    </div>
    
    {{if eq .Path "."}}
        <div class="logo-container">
            <img src="/doc/logoprotopanda.png" alt="Protopanda Logo" class="logo-img">
        </div>
        <div class="main-header">
            <p>Welcome to ProtoPanda</p>
            <div class="header-links">
                <a href="/editor.html" class="btn btn-primary editor-btn">Static Frame Editor</a>
                <a href="/modeleditor.html" class="btn btn-primary editor-btn">Model and Keyframe Editor</a>
            </div>
        </div>
    {{end}}

    <h1>Directory Listing: /{{.Path}}</h1>
    
    <div class="table-responsive">
    <table>
      <thead><tr>
        <th>Name</th>
        <th>Size</th>
        <th>Type</th>
        <th>Action</th>
      </tr></thead>
      <tbody>
      {{range .Files}}
      <tr>
        <td><span class="file-name"><span class="file-icon {{if eq .Type "DIR"}}folder-icon{{else}}document-icon{{end}}" aria-hidden="true"></span><a href="/{{.URL}}">{{.Name}}</a></span></td>
        <td>{{.Size}}</td>
        <td>{{.Type}}</td>
        <td>
          {{if .Editable}}<button class='btn btn-primary' onclick="openEditor('/{{.URL}}', '{{.Name}}')">Edit</button>{{end}}
          {{if .Name}}<button class='btn btn-danger' onclick="deleteFile('/{{.URL}}', '{{.Type}}')">Delete</button>{{end}}
        </td>
      </tr>
      {{end}}
      </tbody>
    </table>
    </div>
    
    <div class="toolbar action-buttons">
      <div class="action-box">
        <h3>Upload File</h3>
        <form id="uploadForm" method="post" action="/upload" enctype="multipart/form-data">
          <input type="hidden" name="path" value="{{.Path}}">
          <div class="form-row">
            <label for="fileInput">Select file:</label>
            <input type="file" name="file" id="fileInput" required>
          </div>
          <button type="submit" class="btn btn-primary">Upload</button>
        </form>
        <div id="uploadStatus" class="status"></div>
      </div>
      
      <div class="action-box">
        <h3>Create Directory</h3>
        <form id="createDirForm">
          <input type="hidden" name="path" value="{{.Path}}">
          <div class="form-row">
            <label for="dirName">Directory name:</label>
            <input type="text" name="dirName" id="dirName" required>
          </div>
          <button type="submit" class="btn btn-primary">Create</button>
        </form>
        <div id="createDirStatus" class="status"></div>
      </div>
    </div>

    <div id="editorModal" class="modal" role="dialog" aria-modal="true" aria-labelledby="editorTitle">
      <div class="modal-card">
        <div class="modal-head"><h2 id="editorTitle">Edit file</h2><button class="btn btn-secondary" onclick="closeEditor()">X</button></div>
        <div class="code-editor"><textarea id="editorMount" aria-label="Code editor" spellcheck="false"></textarea></div>
        <div class="modal-actions"><button class="btn btn-secondary" onclick="closeEditor()">Cancel</button><button class="btn btn-primary" onclick="saveEditor()">Save</button></div>
        <div id="editorStatus" class="status"></div>
      </div>
    </div>

    <dialog id="deleteDialog" class="delete-dialog" aria-labelledby="deleteTitle">
      <div class="dialog-content">
        <h2 id="deleteTitle">Delete file?</h2>
        <p id="deleteMessage"></p>
        <form method="dialog" class="dialog-actions">
          <button class="btn btn-secondary" value="cancel">Cancel</button>
          <button class="btn btn-danger" value="confirm">Delete</button>
        </form>
      </div>
    </dialog>
    <div id="toast" class="toast" role="status" aria-live="polite"></div>
    
    <script>
      let editorPath = '';
      let deletePath = '';
      let toastTimer = null;
      const editorMount = document.getElementById('editorMount');

      editorMount.addEventListener('keydown', function(event) {
        if (event.key !== 'Tab') return;
        event.preventDefault();
        this.setRangeText('  ', this.selectionStart, this.selectionEnd, 'end');
      });

      function deleteFile(path, type) {
        deletePath = path;
        document.getElementById('deleteTitle').textContent = type === 'DIR' ? 'Delete folder?' : 'Delete file?';
        document.getElementById('deleteMessage').textContent = type === 'DIR'
          ? 'The folder must be empty. Delete ' + path + '?'
          : 'Delete ' + path + '?';
        const dialog = document.getElementById('deleteDialog');
        if (typeof dialog.showModal === 'function') dialog.showModal();
        else dialog.setAttribute('open', '');
      }
      document.getElementById('deleteDialog').addEventListener('close', async function() {
        if (this.returnValue !== 'confirm') return;
        const path = deletePath;
        try {
          const response = await fetch('/delete?path=' + encodeURIComponent(path), { method: 'DELETE' });
          if (!response.ok) throw new Error(await response.text());
          showToast('Deleted');
          setTimeout(() => location.reload(), 500);
        } catch (error) {
          showToast('Delete failed: ' + error.message, true);
        }
      });
      function showToast(message, isError) {
        const toast = document.getElementById('toast');
        toast.textContent = message;
        toast.className = isError ? 'toast error show' : 'toast show';
        clearTimeout(toastTimer);
        toastTimer = setTimeout(() => { toast.className = isError ? 'toast error' : 'toast'; }, 2600);
      }

      async function openEditor(path, name) {
        editorPath = path;
        document.getElementById('editorTitle').textContent = 'Edit: ' + name;
        const statusDiv = document.getElementById('editorStatus');
        statusDiv.className = 'status';
        statusDiv.textContent = 'Loading...';
        document.getElementById('editorModal').classList.add('open');
        try {
          const response = await fetch('/edit?path=' + encodeURIComponent(path));
          if (!response.ok) throw new Error(await response.text());
          editorMount.value = await response.text();
          editorMount.selectionStart = 0;
          editorMount.selectionEnd = 0;
          statusDiv.textContent = '';
          editorMount.focus();
        } catch (error) {
          statusDiv.className = 'status status-error';
          statusDiv.textContent = 'Error: ' + error.message;
        }
      }
      function closeEditor() { document.getElementById('editorModal').classList.remove('open'); }
      async function saveEditor() {
        const statusDiv = document.getElementById('editorStatus');
        setStatus(statusDiv, 'Saving...', false);
        const body = new URLSearchParams({path: editorPath, content: editorMount.value});
        try {
          const response = await fetch('/edit', {method: 'POST', body});
          if (!response.ok) throw new Error(await response.text());
          setStatus(statusDiv, 'Saved!', false);
          setTimeout(closeEditor, 500);
        } catch (error) { setStatus(statusDiv, 'Error: ' + error.message, true); }
      }
      document.getElementById('editorModal').addEventListener('click', function(e) { if (e.target === this) closeEditor(); });
      
      function setStatus(element, message, isError) {
        element.innerHTML = message;
        element.className = isError ? 'status status-error' : 'status status-success';
      }
      
      document.getElementById('uploadForm').addEventListener('submit', function(e) {
        e.preventDefault();
        const formData = new FormData(this);
        const statusDiv = document.getElementById('uploadStatus');
        setStatus(statusDiv, 'Uploading...', false);
        
        fetch('/upload', {
          method: 'POST',
          body: formData
        })
        .then(response => {
          if (response.ok) {
            setStatus(statusDiv, 'Upload completed!', false);
            setTimeout(() => location.reload(), 500);
          } else {
            response.text().then(text => setStatus(statusDiv, 'Error: ' + text, true));
          }
        })
        .catch(error => {
          setStatus(statusDiv, 'Error: ' + error, true);
        });
      });
      
      document.getElementById('createDirForm').addEventListener('submit', function(e) {
        e.preventDefault();
        const formData = new FormData(this);
        const statusDiv = document.getElementById('createDirStatus');
        setStatus(statusDiv, 'Creating directory...', false);
        
        fetch('/mkdir', {
          method: 'POST',
          body: new URLSearchParams(formData)
        })
        .then(response => {
          if (response.ok) {
            setStatus(statusDiv, 'Directory created successfully!', false);
            setTimeout(() => location.reload(), 500);
          } else {
            response.text().then(text => setStatus(statusDiv, 'Error: ' + text, true));
          }
        })
        .catch(error => {
          setStatus(statusDiv, 'Error: ' + error, true);
        });
      });
    </script>
    <footer>ProtoPanda v{{.Version}} | Pixel Art Editor</footer>
  </body>
</html>`))
)

type FileInfo struct {
	Name     string
	Size     string
	Type     string
	URL      string
	Editable bool
}

type Breadcrumb struct {
	Name string
	Path string
}

type TemplateData struct {
	Path        string
	CurrentDir  string
	Files       []FileInfo
	Version     string
	Breadcrumbs []Breadcrumb
}

type BandwidthLimiter struct {
	sync.Mutex
	tokens       int64
	lastUpdate   time.Time
	maxTokens    int64
	tokensPerSec int64
}

// Composition state
var (
	composeInProgress bool
	composeProgress   int
	composeMutex      sync.Mutex
)

func (b *BandwidthLimiter) Wait(bytes int64) {
	b.Lock()
	defer b.Unlock()

	now := time.Now()
	elapsed := now.Sub(b.lastUpdate)

	// Add tokens based on elapsed time
	tokensToAdd := int64(elapsed.Seconds() * float64(b.tokensPerSec))
	b.tokens += tokensToAdd

	// Cap at max tokens
	if b.tokens > b.maxTokens {
		b.tokens = b.maxTokens
	}

	b.lastUpdate = now

	// If not enough tokens, wait
	if b.tokens < bytes {
		needed := bytes - b.tokens
		waitTime := time.Duration(float64(needed) / float64(b.tokensPerSec) * float64(time.Second))

		// Release lock while waiting
		b.Unlock()
		fmt.Printf("Waiting for %v to get %d bytes\n", waitTime, bytes)
		time.Sleep(waitTime)
		b.Lock()

		// Update after waiting
		b.tokens = b.maxTokens - bytes
		b.lastUpdate = time.Now()
	} else {
		b.tokens -= bytes
	}
}

func NewBandwidthLimiter(bytesPerSec int64) *BandwidthLimiter {
	return &BandwidthLimiter{
		tokens:       bytesPerSec,
		lastUpdate:   time.Now(),
		maxTokens:    bytesPerSec,
		tokensPerSec: bytesPerSec,
	}
}

var (
	bandwidthLimiter = NewBandwidthLimiter(DownloadSpeedPerSecond)
	semaphore        = make(chan struct{}, 3)
)

func main() {
	http.HandleFunc("/", serveDirectoryListing)
	http.HandleFunc("/mkdir", handleMkdir)
	http.HandleFunc("/upload", handleUpload)
	http.HandleFunc("/edit", handleEdit)
	http.HandleFunc("/delete", handleDelete)
	http.HandleFunc("/copy", handleCopy)
	http.HandleFunc("/mv", handleMv)
	http.HandleFunc("/lua", handleLua)
	http.HandleFunc("/compose_start", handleComposeStart)
	http.HandleFunc("/compose_progress", handleComposeProgress)

	fmt.Printf("Server started on port %d\n", Port)
	fmt.Printf("Serving files from: %s\n", BasePath)
	fmt.Println("Available endpoints:")
	fmt.Println("  GET  /                - Directory listing")
	fmt.Println("  POST /mkdir          - Create directory")
	fmt.Println("  POST /upload         - Upload file")
	fmt.Println("  GET/POST /edit      - Read or save text file")
	fmt.Println("  DELETE /delete       - Delete file/directory")
	fmt.Println("  PUT  /copy           - Copy file")
	fmt.Println("  PUT  /mv             - Move file")
	fmt.Println("  POST /lua            - Execute Lua code (simulated)")
	fmt.Println("  POST /compose_start  - Start composition")
	fmt.Println("  GET  /compose_progress - Get composition progress")
	log.Fatal(http.ListenAndServe(fmt.Sprintf(":%d", Port), nil))
}

func editFilePath(rawPath string) (string, error) {
	rawPath = strings.TrimPrefix(rawPath, "/")
	cleanPath := filepath.Clean(rawPath)
	if cleanPath == "." || cleanPath == ".." || strings.HasPrefix(cleanPath, ".."+string(os.PathSeparator)) {
		return "", fmt.Errorf("invalid path")
	}
	base, err := filepath.Abs(BasePath)
	if err != nil {
		return "", err
	}
	full, err := filepath.Abs(filepath.Join(base, cleanPath))
	if err != nil {
		return "", err
	}
	if full != base && !strings.HasPrefix(full, base+string(os.PathSeparator)) {
		return "", fmt.Errorf("invalid path")
	}
	return full, nil
}

func handleEdit(w http.ResponseWriter, r *http.Request) {
	rawPath := r.URL.Query().Get("path")
	if rawPath == "" {
		rawPath = r.FormValue("path")
	}
	if rawPath == "" {
		http.Error(w, "Missing path parameter", http.StatusBadRequest)
		return
	}

	fullPath, err := editFilePath(rawPath)
	if err != nil {
		http.Error(w, err.Error(), http.StatusBadRequest)
		return
	}
	if !isEditableFile(fullPath) {
		http.Error(w, "File type is not editable", http.StatusBadRequest)
		return
	}
	info, err := os.Stat(fullPath)
	if err != nil {
		http.Error(w, "File not found", http.StatusNotFound)
		return
	}
	if info.IsDir() {
		http.Error(w, "Directories are not editable", http.StatusBadRequest)
		return
	}
	if info.Size() > 512*1024 {
		http.Error(w, "File is too large to edit in browser", http.StatusRequestEntityTooLarge)
		return
	}

	switch r.Method {
	case http.MethodGet:
		content, err := os.ReadFile(fullPath)
		if err != nil {
			http.Error(w, "Failed to read file: "+err.Error(), http.StatusInternalServerError)
			return
		}
		w.Header().Set("Content-Type", "text/plain; charset=utf-8")
		w.Write(content)
	case http.MethodPost:
		if err := r.ParseForm(); err != nil {
			http.Error(w, "Failed to parse form", http.StatusBadRequest)
			return
		}
		if err := os.WriteFile(fullPath, []byte(r.FormValue("content")), 0644); err != nil {
			http.Error(w, "Failed to save file: "+err.Error(), http.StatusInternalServerError)
			return
		}
		w.WriteHeader(http.StatusOK)
		w.Write([]byte("File saved successfully"))
	default:
		w.Header().Set("Allow", "GET, POST")
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
	}
}

func serveFileWithRateLimit(w http.ResponseWriter, r *http.Request, filePath string, info os.FileInfo) {
	semaphore <- struct{}{}
	defer func() {
		fmt.Printf("Freed\n")
		<-semaphore
	}()
	time.Sleep(time.Millisecond * 800)
	file, err := os.Open(filePath)
	if err != nil {
		http.Error(w, "Error opening file: "+err.Error(), http.StatusInternalServerError)
		return
	}
	defer file.Close()

	contentType := getContentType(filePath)
	w.Header().Set("Content-Type", contentType)

	if info.Size() >= 0 {
		w.Header().Set("Content-Length", fmt.Sprintf("%d", info.Size()))
	}

	fmt.Printf("Serving file: %s (size: %d bytes, type: %s) with global rate limiting\n",
		info.Name(), info.Size(), contentType)

	buffer := make([]byte, ChunkSize)
	totalSent := int64(0)

	for {
		n, err := file.Read(buffer)
		if n > 0 {
			bandwidthLimiter.Wait(int64(n))
			time.Sleep(time.Millisecond * 100)

			if _, writeErr := w.Write(buffer[:n]); writeErr != nil {
				fmt.Printf("Error writing to response: %v\n", writeErr)
				break
			}

			totalSent += int64(n)

			if flusher, ok := w.(http.Flusher); ok {
				flusher.Flush()
			}
		}

		if err != nil {
			if err != io.EOF {
				fmt.Printf("Error reading file: %v\n", err)
			}
			break
		}
	}

	fmt.Printf("Finished serving: %s, total sent: %d bytes\n", info.Name(), totalSent)
}

func serveFileFast(w http.ResponseWriter, r *http.Request, filePath string, info os.FileInfo) {
	file, err := os.Open(filePath)
	if err != nil {
		http.Error(w, "Error opening file: "+err.Error(), http.StatusInternalServerError)
		return
	}
	defer file.Close()
	w.Header().Set("Content-Type", getContentType(filePath))
	http.ServeContent(w, r, info.Name(), info.ModTime(), file)
}

func getContentType(filePath string) string {
	ext := strings.ToLower(filepath.Ext(filePath))

	switch ext {
	case ".html", ".htm":
		return "text/html; charset=utf-8"
	case ".css":
		return "text/css; charset=utf-8"
	case ".js":
		return "application/javascript"
	case ".json":
		return "application/json"
	case ".png":
		return "image/png"
	case ".jpg", ".jpeg":
		return "image/jpeg"
	case ".gif":
		return "image/gif"
	case ".svg":
		return "image/svg+xml"
	case ".pdf":
		return "application/pdf"
	case ".txt":
		return "text/plain; charset=utf-8"
	case ".xml":
		return "application/xml"
	case ".zip":
		return "application/zip"
	case ".tar":
		return "application/x-tar"
	case ".gz":
		return "application/gzip"
	case ".mp4":
		return "video/mp4"
	case ".mp3":
		return "audio/mpeg"
	case ".wav":
		return "audio/wav"
	case ".avi":
		return "video/x-msvideo"
	case ".mov":
		return "video/quicktime"
	case ".webm":
		return "video/webm"
	case ".webp":
		return "image/webp"
	case ".ico":
		return "image/x-icon"
	case ".csv":
		return "text/csv"
	case ".doc", ".docx":
		return "application/msword"
	case ".xls", ".xlsx":
		return "application/vnd.ms-excel"
	case ".ppt", ".pptx":
		return "application/vnd.ms-powerpoint"
	default:
		return "application/octet-stream"
	}
}

func serveDirectoryListing(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodGet {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	requestedPath := r.URL.Path

	if len(requestedPath) > 0 && requestedPath[0] == '/' {
		requestedPath = requestedPath[1:]
	}

	requestedPath = filepath.Clean(requestedPath)
	fmt.Printf("REQ : %s\n", requestedPath)
	fullPath := filepath.Join(BasePath, requestedPath)
	fmt.Printf("fullPath : %s\n", fullPath)
	info, err := os.Stat(fullPath)
	if err != nil {
		if os.IsNotExist(err) {
			requestedPath = ""
			fullPath = BasePath
		} else {
			http.Error(w, "Error accessing path: "+err.Error(), http.StatusInternalServerError)
			return
		}
	} else if !info.IsDir() {
		fmt.Printf("Serving file: %s\n", fullPath)
		if strings.HasPrefix(strings.ReplaceAll(requestedPath, "\\", "/"), "lib/web/vendor/") {
			serveFileFast(w, r, fullPath, info)
			return
		}
		serveFileWithRateLimit(w, r, fullPath, info)
		return
	}

	fmt.Printf("Listing dir: %s\n", fullPath)

	files, err := os.ReadDir(fullPath)
	if err != nil {
		http.Error(w, "Error reading directory: "+err.Error(), http.StatusInternalServerError)
		return
	}

	var fileList []FileInfo
	for _, file := range files {
		if strings.HasPrefix(file.Name(), ".") {
			continue
		}

		fileInfo, err := file.Info()
		if err != nil {
			continue
		}

		var size string
		var fileType string
		var url string

		if file.IsDir() {
			size = "-"
			fileType = "DIR"
			if requestedPath == "" {
				url = file.Name()
			} else {
				url = filepath.Join(requestedPath, file.Name())
			}
		} else {
			size = formatFileSize(fileInfo.Size())
			fileType = "FILE"
			if requestedPath == "" {
				url = file.Name()
			} else {
				url = filepath.Join(requestedPath, file.Name())
			}
		}

		url = strings.ReplaceAll(url, "\\", "/")

		fmt.Printf("Added the: %s\n", url)

		fileList = append(fileList, FileInfo{
			Name:     file.Name(),
			Size:     size,
			Type:     fileType,
			URL:      url,
			Editable: !file.IsDir() && isEditableFile(file.Name()),
		})
	}

	// Keep navigation predictable: folders first, then files, all alphabetically.
	sort.SliceStable(fileList, func(i, j int) bool {
		if fileList[i].Type != fileList[j].Type {
			return fileList[i].Type == "DIR"
		}
		return strings.ToLower(fileList[i].Name) < strings.ToLower(fileList[j].Name)
	})

	breadcrumbs := generateBreadcrumbs(requestedPath)

	currentDir := "/"
	if requestedPath != "" {
		parts := strings.Split(requestedPath, "/")
		if len(parts) > 0 {
			currentDir = parts[len(parts)-1]
		}
	}

	displayPath := requestedPath
	if displayPath == "" {
		displayPath = "."
	}

	data := TemplateData{
		Path:        displayPath,
		CurrentDir:  currentDir,
		Files:       fileList,
		Version:     PandaVersion,
		Breadcrumbs: breadcrumbs,
	}

	if err := indexTemplate.Execute(w, data); err != nil {
		http.Error(w, "Error rendering template: "+err.Error(), http.StatusInternalServerError)
	}
}

func isEditableFile(name string) bool {
	switch strings.ToLower(filepath.Ext(name)) {
	case ".json", ".lua", ".txt", ".md", ".csv", ".xml", ".html", ".htm", ".css", ".js", ".ini", ".toml", ".yaml", ".yml":
		return true
	default:
		return false
	}
}

func generateBreadcrumbs(path string) []Breadcrumb {
	var breadcrumbs []Breadcrumb

	breadcrumbs = append(breadcrumbs, Breadcrumb{Name: "/", Path: ""})

	if path == "" {
		return breadcrumbs
	}

	parts := strings.Split(path, "/")
	currentPath := ""

	for i, part := range parts {
		if part == "" {
			continue
		}

		if currentPath == "" {
			currentPath = part
		} else {
			currentPath = currentPath + "/" + part
		}

		if i < len(parts)-1 {
			breadcrumbs = append(breadcrumbs, Breadcrumb{
				Name: part,
				Path: currentPath,
			})
		}
	}

	return breadcrumbs
}

func handleMkdir(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	if err := r.ParseForm(); err != nil {
		http.Error(w, "Error parsing form", http.StatusBadRequest)
		return
	}

	basePath := r.FormValue("path")
	dirName := r.FormValue("dirName")
	if basePath == "" || dirName == "" {
		http.Error(w, "Missing parameters", http.StatusBadRequest)
		return
	}

	basePath = filepath.Clean(basePath)
	dirName = filepath.Clean(dirName)

	fullPath := filepath.Join(BasePath, basePath, dirName)

	fmt.Printf("Creating dir: %s\n", fullPath)

	if _, err := os.Stat(fullPath); err == nil {
		http.Error(w, "Directory already exists", http.StatusBadRequest)
		return
	}

	if err := os.MkdirAll(fullPath, 0755); err != nil {
		http.Error(w, "Failed to create directory: "+err.Error(), http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusOK)
	w.Write([]byte("Directory created successfully"))
}

func handleUpload(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	if err := r.ParseMultipartForm(100 << 20); err != nil {
		http.Error(w, "Error parsing form: "+err.Error(), http.StatusBadRequest)
		return
	}

	basePath := r.FormValue("path")
	file, handler, err := r.FormFile("file")
	if err != nil {
		http.Error(w, "Error retrieving file: "+err.Error(), http.StatusBadRequest)
		return
	}
	defer file.Close()

	basePath = filepath.Clean(basePath)

	fullPath := filepath.Join(BasePath, basePath)
	if err := os.MkdirAll(fullPath, 0755); err != nil {
		http.Error(w, "Error creating directories: "+err.Error(), http.StatusInternalServerError)
		return
	}

	dstPath := filepath.Join(fullPath, handler.Filename)

	fmt.Printf("Uploading dir: %s\n", dstPath)
	dst, err := os.Create(dstPath)
	if err != nil {
		http.Error(w, "Error creating file: "+err.Error(), http.StatusInternalServerError)
		return
	}
	defer dst.Close()

	if _, err := io.Copy(dst, file); err != nil {
		http.Error(w, "Error saving file: "+err.Error(), http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusOK)
	w.Write([]byte("File uploaded successfully"))
}

func handleDelete(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodDelete {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	path := r.URL.Query().Get("path")
	if path == "" {
		http.Error(w, "Missing path parameter", http.StatusBadRequest)
		return
	}

	if strings.HasPrefix(path, "/") {
		path = path[1:]
	}

	path = filepath.Clean(path)
	fullPath := filepath.Join(BasePath, path)

	fmt.Printf("Deleting path: %s\n", fullPath)

	info, err := os.Stat(fullPath)
	if err != nil {
		if os.IsNotExist(err) {
			http.Error(w, "Path not found", http.StatusNotFound)
		} else {
			http.Error(w, "Error accessing path: "+err.Error(), http.StatusInternalServerError)
		}
		return
	}

	if info.IsDir() {
		entries, err := os.ReadDir(fullPath)
		if err != nil {
			http.Error(w, "Error reading directory: "+err.Error(), http.StatusInternalServerError)
			return
		}

		hasEntries := false
		for _, entry := range entries {
			if !strings.HasPrefix(entry.Name(), ".") {
				hasEntries = true
				break
			}
		}

		if hasEntries {
			http.Error(w, "Directory is not empty", http.StatusBadRequest)
			return
		}

		if err := os.Remove(fullPath); err != nil {
			http.Error(w, "Failed to delete directory: "+err.Error(), http.StatusInternalServerError)
			return
		}
	} else {
		if err := os.Remove(fullPath); err != nil {
			http.Error(w, "Failed to delete file: "+err.Error(), http.StatusInternalServerError)
			return
		}
	}

	w.WriteHeader(http.StatusOK)
	w.Write([]byte("Deleted successfully"))
}

// New: Copy endpoint - matches ESP32 behavior
func handleCopy(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPut {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	srcPath := r.URL.Query().Get("src")
	dstPath := r.URL.Query().Get("dst")

	if srcPath == "" || dstPath == "" {
		http.Error(w, "Missing src or dst parameter", http.StatusBadRequest)
		return
	}

	if strings.HasPrefix(srcPath, "/") {
		srcPath = srcPath[1:]
	}
	if strings.HasPrefix(dstPath, "/") {
		dstPath = dstPath[1:]
	}

	srcPath = filepath.Clean(srcPath)
	dstPath = filepath.Clean(dstPath)

	fullSrcPath := filepath.Join(BasePath, srcPath)
	fullDstPath := filepath.Join(BasePath, dstPath)

	fmt.Printf("Copying from %s to %s\n", fullSrcPath, fullDstPath)

	// Check source exists
	srcInfo, err := os.Stat(fullSrcPath)
	if err != nil {
		if os.IsNotExist(err) {
			http.Error(w, "Source file not found", http.StatusNotFound)
		} else {
			http.Error(w, "Error accessing source: "+err.Error(), http.StatusInternalServerError)
		}
		return
	}

	if srcInfo.IsDir() {
		http.Error(w, "Source is a directory", http.StatusBadRequest)
		return
	}

	// Create destination directory if needed
	dstDir := filepath.Dir(fullDstPath)
	if err := os.MkdirAll(dstDir, 0755); err != nil {
		http.Error(w, "Failed to create destination directory: "+err.Error(), http.StatusInternalServerError)
		return
	}

	// Check if destination already exists
	if _, err := os.Stat(fullDstPath); err == nil {
		http.Error(w, "Destination already exists", http.StatusConflict)
		return
	}

	// Copy file
	srcFile, err := os.Open(fullSrcPath)
	if err != nil {
		http.Error(w, "Failed to open source file: "+err.Error(), http.StatusInternalServerError)
		return
	}
	defer srcFile.Close()

	dstFile, err := os.Create(fullDstPath)
	if err != nil {
		http.Error(w, "Failed to create destination file: "+err.Error(), http.StatusInternalServerError)
		return
	}
	defer dstFile.Close()

	buffer := make([]byte, 512)
	var copyErr error
	for {
		n, err := srcFile.Read(buffer)
		if n > 0 {
			if _, writeErr := dstFile.Write(buffer[:n]); writeErr != nil {
				copyErr = writeErr
				break
			}
		}
		if err != nil {
			if err != io.EOF {
				copyErr = err
			}
			break
		}
	}

	if copyErr != nil {
		os.Remove(fullDstPath)
		http.Error(w, "Copy failed: "+copyErr.Error(), http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusOK)
	w.Write([]byte("OK"))
}

// New: Move endpoint - matches ESP32 behavior
func handleMv(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPut {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	srcPath := r.URL.Query().Get("src")
	dstPath := r.URL.Query().Get("dst")

	if srcPath == "" || dstPath == "" {
		http.Error(w, "Missing src or dst parameter", http.StatusBadRequest)
		return
	}

	if strings.HasPrefix(srcPath, "/") {
		srcPath = srcPath[1:]
	}
	if strings.HasPrefix(dstPath, "/") {
		dstPath = dstPath[1:]
	}

	srcPath = filepath.Clean(srcPath)
	dstPath = filepath.Clean(dstPath)

	fullSrcPath := filepath.Join(BasePath, srcPath)
	fullDstPath := filepath.Join(BasePath, dstPath)

	fmt.Printf("Moving from %s to %s\n", fullSrcPath, fullDstPath)

	// Check source exists
	srcInfo, err := os.Stat(fullSrcPath)
	if err != nil {
		if os.IsNotExist(err) {
			http.Error(w, "Source file not found", http.StatusNotFound)
		} else {
			http.Error(w, "Error accessing source: "+err.Error(), http.StatusInternalServerError)
		}
		return
	}

	if srcInfo.IsDir() {
		http.Error(w, "Source is a directory", http.StatusBadRequest)
		return
	}

	// Create destination directory if needed
	dstDir := filepath.Dir(fullDstPath)
	if err := os.MkdirAll(dstDir, 0755); err != nil {
		http.Error(w, "Failed to create destination directory: "+err.Error(), http.StatusInternalServerError)
		return
	}

	// Check if destination already exists
	if _, err := os.Stat(fullDstPath); err == nil {
		http.Error(w, "Destination already exists", http.StatusConflict)
		return
	}

	// Copy file
	srcFile, err := os.Open(fullSrcPath)
	if err != nil {
		http.Error(w, "Failed to open source file: "+err.Error(), http.StatusInternalServerError)
		return
	}
	defer srcFile.Close()

	dstFile, err := os.Create(fullDstPath)
	if err != nil {
		http.Error(w, "Failed to create destination file: "+err.Error(), http.StatusInternalServerError)
		return
	}
	defer dstFile.Close()

	buffer := make([]byte, 512)
	var copyErr error
	for {
		n, err := srcFile.Read(buffer)
		if n > 0 {
			if _, writeErr := dstFile.Write(buffer[:n]); writeErr != nil {
				copyErr = writeErr
				break
			}
		}
		if err != nil {
			if err != io.EOF {
				copyErr = err
			}
			break
		}
	}

	if copyErr != nil {
		os.Remove(fullDstPath)
		http.Error(w, "Move failed: "+copyErr.Error(), http.StatusInternalServerError)
		return
	}

	// Delete source after successful copy
	if err := os.Remove(fullSrcPath); err != nil {
		os.Remove(fullDstPath)
		http.Error(w, "Move failed: could not remove source file", http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusOK)
	w.Write([]byte("OK"))
}

// New: Lua execution endpoint (simulated)
func handleLua(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	// Read body
	body, err := io.ReadAll(r.Body)
	if err != nil {
		http.Error(w, "Error reading body", http.StatusBadRequest)
		return
	}
	defer r.Body.Close()

	luaCode := string(body)
	fmt.Printf("Lua execution (simulated): %s\n", luaCode[:min(len(luaCode), 100)])

	// Simulate Lua execution
	if strings.Contains(luaCode, "error") {
		http.Error(w, "Lua Error: simulated error", http.StatusInternalServerError)
		return
	}

	// Simulate return value
	w.WriteHeader(http.StatusOK)
	w.Write([]byte("Lua executed successfully (simulated)"))
}

// New: Compose start endpoint
func handleComposeStart(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	composeMutex.Lock()
	defer composeMutex.Unlock()

	if composeInProgress {
		w.WriteHeader(http.StatusOK)
		w.Write([]byte("Status: Composition already in progress"))
		return
	}

	composeInProgress = true
	composeProgress = 0

	// Start composition in background
	go func() {
		fmt.Println("Starting composition...")
		for i := 0; i <= 100; i += 10 {
			time.Sleep(500 * time.Millisecond)
			composeMutex.Lock()
			composeProgress = i
			composeMutex.Unlock()
			fmt.Printf("Composition progress: %d%%\n", i)
		}
		composeMutex.Lock()
		composeInProgress = false
		composeMutex.Unlock()
		fmt.Println("Composition completed!")
	}()

	w.WriteHeader(http.StatusOK)
	w.Write([]byte("Status: Composition started successfully"))
}

// New: Compose progress endpoint
func handleComposeProgress(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodGet {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	composeMutex.Lock()
	progress := composeProgress
	inProgress := composeInProgress
	composeMutex.Unlock()

	var response string
	if inProgress {
		response = fmt.Sprintf("%d", progress)
	} else {
		response = "100"
	}

	w.WriteHeader(http.StatusOK)
	w.Write([]byte(response))
}

func formatFileSize(size int64) string {
	if size < 1024 {
		return fmt.Sprintf("%d B", size)
	} else if size < 1024*1024 {
		return fmt.Sprintf("%.1f KB", float64(size)/1024)
	} else if size < 1024*1024*1024 {
		return fmt.Sprintf("%.1f MB", float64(size)/(1024*1024))
	} else {
		return fmt.Sprintf("%.1f GB", float64(size)/(1024*1024*1024))
	}
}

func min(a, b int) int {
	if a < b {
		return a
	}
	return b
}
