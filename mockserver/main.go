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
	indexTemplate = template.Must(template.New("index").Parse(`<!DOCTYPE html>
<html>
<head>
    <title>Directory Listing</title>
    <style>
      body {
        font-family: Arial, sans-serif;
        margin: 20px;
        display: flex;
        flex-direction: column;
        min-height: 100vh;
      }
      h1 {
        color: #333;
        margin-bottom: 20px;
      }
      table {
        border-collapse: collapse;
        width: 100%;
        margin-bottom: 20px;
        box-shadow: 0 1px 3px rgba(0,0,0,0.1);
      }
      th, td {
        border: 1px solid #ddd;
        padding: 12px;
        text-align: left;
      }
      th {
        background-color: #f8f9fa;
        font-weight: 600;
      }
      tr:nth-child(even) {
        background-color: #f9f9f9;
      }
      tr:hover {
        background-color: #f1f1f1;
      }
      .action-buttons {
        display: flex;
        gap: 20px;
        margin-top: auto;
      }
      .action-box {
        flex: 1;
        padding: 20px;
        background-color: #f8f9fa;
        border-radius: 8px;
        box-shadow: 0 1px 3px rgba(0,0,0,0.1);
      }
      .action-box h3 {
        margin-top: 0;
        color: #333;
      }
      .form-row {
        margin-bottom: 15px;
      }
      footer {
        margin-top: 40px;
        padding: 20px 0;
        text-align: center;
        color: #777;
        font-size: 0.9em;
        border-top: 1px solid #eee;
      }
      .form-row label {
        display: block;
        margin-bottom: 5px;
        font-weight: 500;
      }
      input[type="text"],
      input[type="file"] {
        width: 100%;
        padding: 8px;
        border: 1px solid #ddd;
        border-radius: 4px;
        box-sizing: border-box;
      }
      .btn {
        padding: 10px 15px;
        border: none;
        border-radius: 4px;
        cursor: pointer;
        font-weight: 500;
        transition: background-color 0.2s;
        text-align: center;
      }
      .btn-primary {
        background-color: #4CAF50;
        color: white;
      }
      .btn-primary:hover {
        background-color: #45a049;
      }
      .btn-danger {
        background-color: #ff4444;
        color: white;
      }
      .btn-danger:hover {
        background-color: #cc0000;
      }
      .status {
        margin-top: 10px;
        padding: 8px;
        border-radius: 4px;
      }
      .status-success {
        background-color: #d4edda;
        color: #155724;
      }
      .status-error {
        background-color: #f8d7da;
        color: #721c24;
      }
      a {
        color: #007bff;
        text-decoration: none;
      }
      a:hover {
        text-decoration: underline;
      }
      .breadcrumb {
        margin-bottom: 15px;
        font-size: 1.1em;
      }
      .breadcrumb a {
        color: #007bff;
        text-decoration: none;
      }
      .breadcrumb a:hover {
        text-decoration: underline;
      }
      /* New/Updated Styles for Header */
      .main-header {
        background-color: #e9ecef;
        padding: 20px;
        border-radius: 8px;
        margin-bottom: 20px;
        display: flex;
        flex-direction: column; /* Center content vertically */
        align-items: center; /* Center content horizontally */
        gap: 15px;
      }
      .main-header p {
        margin: 0;
        font-size: 1.2em;
        color: #333;
      }
      .header-links {
        display: flex;
        gap: 20px;
        align-items: center;
      }
      .editor-btn {
        padding: 15px 30px; /* Bigger button */
        font-size: 1.2em;
        background-color: #007bff !important; /* Make editor button stand out */
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
    </style>
</head>
<body>
    <div class="breadcrumb">{{range .Breadcrumbs}}<a href="/{{.Path}}">{{.Name}}</a> &gt; {{end}}{{.CurrentDir}}</div>
    
    {{if eq .Path "."}}
        <div class="logo-container">
            <img src="/doc/logoprotopanda.png" alt="Protopanda Logo" class="logo-img">
        </div>
        <div class="main-header">
            <p>Welcome protopanda.</p>
            <div class="header-links">
                <a href="/editor.html" class="btn btn-primary editor-btn">Go to Editor</a>
            </div>
        </div>
    {{end}}

    <h1>Directory Listing: /{{.Path}}</h1>
    
    <table>
      <tr>
        <th>Name</th>
        <th>Size</th>
        <th>Type</th>
        <th>Action</th>
      </tr>
      {{range .Files}}
      <tr>
        <td><a href="/{{.URL}}">{{.Name}}</a></td>
        <td>{{.Size}}</td>
        <td>{{.Type}}</td>
        <td>
          {{if .Name}}<button class='btn btn-danger' onclick="deleteFile('/{{.URL}}')">Delete</button>{{end}}
        </td>
      </tr>
      {{end}}
    </table>
    
    <div class="action-buttons">
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
    
    <script>
      function deleteFile(path) {
        if (confirm('Are you sure you want to delete ' + path + '?')) {
          fetch('/delete?path=' + encodeURIComponent(path), { method: 'DELETE' })
            .then(response => { 
              if (response.ok) location.reload(); 
              else response.text().then(text => alert('Error: ' + text));
            })
            .catch(error => console.error('Error:', error));
        }
      }
      
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
    <footer>Protopanda v{{.Version}}</footer>
</body>
</html>`))
)

// ... (Rest of the Go code remains the same)

type FileInfo struct {
	Name string
	Size string
	Type string
	URL  string
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
		fmt.Printf("Waiting for %v to get %d bytes", waitTime, bytes)
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
	http.HandleFunc("/", serveDirectoryListing) // This should come first
	http.HandleFunc("/mkdir", handleMkdir)
	http.HandleFunc("/upload", handleUpload)
	http.HandleFunc("/delete", handleDelete)

	fmt.Printf("Server started on port %d\n", Port)
	fmt.Printf("Serving files from: %s\n", BasePath)
	log.Fatal(http.ListenAndServe(fmt.Sprintf(":%d", Port), nil))
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

	// Get MIME type based on file extension
	contentType := getContentType(filePath)

	// Set content headers - NOT attachment, so browser displays it
	w.Header().Set("Content-Type", contentType)

	// Only set Content-Length for known sizes (regular files)
	if info.Size() >= 0 {
		w.Header().Set("Content-Length", fmt.Sprintf("%d", info.Size()))
	}

	fmt.Printf("Serving file: %s (size: %d bytes, type: %s) with global rate limiting\n",
		info.Name(), info.Size(), contentType)

	// Use smaller buffer for smoother rate limiting
	buffer := make([]byte, ChunkSize)
	totalSent := int64(0)

	for {
		n, err := file.Read(buffer)
		if n > 0 {
			// Wait for bandwidth tokens
			bandwidthLimiter.Wait(int64(n))
			time.Sleep(time.Millisecond * 100)

			// Write chunk to response
			if _, writeErr := w.Write(buffer[:n]); writeErr != nil {
				fmt.Printf("Error writing to response: %v\n", writeErr)
				break
			}

			totalSent += int64(n)

			// Flush to send data immediately
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

// Helper function to determine MIME type based on file extension
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
		// Default to binary stream for unknown types, but NOT attachment
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
		//http.ServeFile(w, r, fullPath)
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
		// Skip hidden files
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
			// For directories, append a trailing slash in the URL
			// The original code calculated relative URLs, but using full path parts here for clarity in the mock server
			if requestedPath == "" {
				url = file.Name() // e.g., 'logs'
			} else {
				url = filepath.Join(requestedPath, file.Name()) // e.g., 'parent/child'
			}
		} else {
			size = formatFileSize(fileInfo.Size())
			fileType = "FILE"
			if requestedPath == "" {
				url = file.Name() // e.g., 'file.txt'
			} else {
				url = filepath.Join(requestedPath, file.Name()) // e.g., 'parent/file.txt'
			}
		}

		// Clean up the URL to be relative to the root for the template
		url = strings.ReplaceAll(url, "\\", "/")

		fmt.Printf("Added the: %s\n", url)

		fileList = append(fileList, FileInfo{
			Name: file.Name(),
			Size: size,
			Type: fileType,
			URL:  url,
		})
	}

	breadcrumbs := generateBreadcrumbs(requestedPath)

	currentDir := "/"
	if requestedPath != "" {
		parts := strings.Split(requestedPath, "/")
		if len(parts) > 0 {
			currentDir = parts[len(parts)-1]
		}
	}

	data := TemplateData{
		Path:        requestedPath,
		CurrentDir:  currentDir,
		Files:       fileList,
		Version:     PandaVersion,
		Breadcrumbs: breadcrumbs,
	}

	if err := indexTemplate.Execute(w, data); err != nil {
		http.Error(w, "Error rendering template: "+err.Error(), http.StatusInternalServerError)
	}
}

func generateBreadcrumbs(path string) []Breadcrumb {
	var breadcrumbs []Breadcrumb

	// Root breadcrumb
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

		// For all but the last part, show as links
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

	// Remove leading slash for consistency with the rest of the Go logic
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
			// Check for non-hidden entries
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
