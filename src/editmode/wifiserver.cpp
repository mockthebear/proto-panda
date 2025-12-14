#include "editmode/editmode.hpp"
#include "tools/config_default.hpp"
#include "tools/devices.hpp"
#include "drawing/framerepository.hpp"
#include "lua/luainterface.hpp"
#include "drawing/animation.hpp"
#include "drawing/dma_display.hpp"
#include "SD.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer *server;
extern LuaInterface g_lua;
extern FrameRepository g_frameRepo;
extern Animation g_animation;

bool createDirectories(String path)
{
  String currentPath = "";
  int startIdx = 0;
  int slashIdx;

  while ((slashIdx = path.indexOf('/', startIdx)) != -1)
  {
    currentPath = path.substring(0, slashIdx);
    if (!SD.exists(currentPath))
    {
      SD.mkdir(currentPath);
    }
    startIdx = slashIdx + 1;
  }

  if (!SD.exists(path))
  {
    SD.mkdir(path);
  }
  return true;
}

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  static File uploadFile;
  static String filePath;

  if (!index)
  {
    String path = "/";
    if (request->hasParam("path", true))
    {
      path = request->getParam("path", true)->value();
    }
    filePath = path + "/" + filename;

    String dirPath = filePath.substring(0, filePath.lastIndexOf('/'));
    if (dirPath.length() > 0 && !SD.exists(dirPath))
    {
      if (!createDirectories(dirPath))
      {
        request->send(500, "text/plain", "{\"success\": false, \"error\": \"Failed to open file for writing\"}");
      }
    }

    uploadFile = SD.open(filePath, FILE_WRITE);
    if (!uploadFile)
    {
      request->send(500, "text/plain", "{\"success\": false, \"error\": \"Failed to open file for writing\"}");
      Serial.println("Failed to open file for writing");
      return;
    }
  }

  if (uploadFile && len)
  {
    uploadFile.write(data, len);
  }

  if (final && uploadFile)
  {
    uploadFile.close();
    request->send(200, "text/plain", "{\"success\": true}");
  }
}

void handleCopy(AsyncWebServerRequest *request)
{
  if (!request->hasParam("src") || !request->hasParam("dst"))
  {
    request->send(400, "text/plain", "Missing src or dst parameter");
    return;
  }

  String srcPath = request->getParam("src")->value();
  String dstPath = request->getParam("dst")->value();

  if (!SD.exists(srcPath))
  {
    request->send(404, "text/plain", "Source file not found");
    return;
  }

  File src = SD.open(srcPath);
  if (src.isDirectory())
  {
    src.close();
    request->send(400, "text/plain", "Source is a directory");
    return;
  }

  // Extract directory path from dstPath
  int lastSlash = dstPath.lastIndexOf('/');
  if (lastSlash > 0)
  {
    String dstDir = dstPath.substring(0, lastSlash);
    
    // Create directory if it doesn't exist
    if (!SD.exists(dstDir))
    {
      // Create all necessary parent directories
      String currentPath = "";
      for (int i = 0; i < dstDir.length(); i++)
      {
        currentPath += dstDir[i];
        if (dstDir[i] == '/' && i > 0) // Found a directory level
        {
          if (!SD.exists(currentPath.substring(0, currentPath.length() - 1)))
          {
            if (!SD.mkdir(currentPath.substring(0, currentPath.length() - 1)))
            {
              src.close();
              request->send(500, "text/plain", "Failed to create directory: " + currentPath);
              return;
            }
          }
        }
      }
      
      // Create the final directory
      if (!SD.mkdir(dstDir))
      {
        src.close();
        request->send(500, "text/plain", "Failed to create destination directory");
        return;
      }
    }
  }

  // Check if destination already exists (file, not directory)
  if (SD.exists(dstPath))
  {
    File dstCheck = SD.open(dstPath);
    if (!dstCheck.isDirectory()) // Only fail if it's a file
    {
      src.close();
      dstCheck.close();
      request->send(409, "text/plain", "Destination already exists");
      return;
    }
    dstCheck.close();
  }

  File dst = SD.open(dstPath, FILE_WRITE);
  if (!dst)
  {
    src.close();
    request->send(500, "text/plain", "Failed to create destination file");
    return;
  }

  uint8_t buffer[512];
  size_t bytesRead;
  bool error = false;
  
  while ((bytesRead = src.read(buffer, sizeof(buffer))) > 0)
  {
    if (dst.write(buffer, bytesRead) != bytesRead)
    {
      error = true;
      break;
    }
  }

  src.close();
  dst.close();

  if (error)
  {
    SD.remove(dstPath);
    request->send(500, "text/plain", "Copy failed");
    return;
  }

  request->send(200, "text/plain", "OK");
}

void handleRm(AsyncWebServerRequest *request)
{
  if (!request->hasParam("path"))
  {
    request->send(400, "text/plain", "Missing path parameter");
    return;
  }

  String path = request->getParam("path")->value();

  if (!SD.exists(path))
  {
    request->send(404, "text/plain", "Path not found");
    return;
  }

  File file = SD.open(path);
  if (file.isDirectory())
  {
    bool isEmpty = true;
    File entry = file.openNextFile();
    while (entry)
    {
      if (String(entry.name()) != "." && String(entry.name()) != "..")
      {
        isEmpty = false;
        break;
      }
      entry = file.openNextFile();
    }
    entry.close();

    if (!isEmpty)
    {
      file.close();
      request->send(400, "text/plain", "Directory is not empty");
      return;
    }

    file.close();
    if (!SD.rmdir(path))
    {
      request->send(500, "text/plain", "Failed to delete directory");
      return;
    }
  }
  else
  {
    file.close();
    if (!SD.remove(path))
    {
      request->send(500, "text/plain", "Failed to delete file");
      return;
    }
  }

  request->send(200, "text/plain", "Deleted successfully");
}

void handleMkdir(AsyncWebServerRequest *request)
{
  if (request->hasParam("path", true) && request->hasParam("dirName", true))
  {
    String basePath = request->getParam("path", true)->value();
    String dirName = request->getParam("dirName", true)->value();
    String fullPath = basePath + "/" + dirName;

    fullPath.replace("//", "/");
    if (SD.exists(fullPath))
    {
      request->send(400, "text/plain", "Directory already exists");
    }
    else if (SD.mkdir(fullPath))
    {
      request->send(200, "text/plain", "Directory created successfully");
    }
    else
    {
      request->send(500, "text/plain", "Failed to create directory");
    }
  }
  else
  {
    request->send(400, "text/plain", "Missing parameters");
  }
}

void serveDirectoryListing(AsyncWebServerRequest *request)
{
  String path = request->url();
  path.replace("//", "/");
  while (path.endsWith("/") && path.length() > 1)
  {
    path.remove(path.length() - 1);
  }

  if (!SD.exists(path))
  {
    path = "/";
  }

  String output = R"(
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
    </style>
  </head>
  <body>
    <div class="breadcrumb">)";

  if (path != "/")
  {
    output += "<a href='/'>/</a> &gt; ";

    String currentPath = "";
    String parts = path.substring(1);
    int lastSlash = 0;

    while (lastSlash != -1)
    {
      int nextSlash = parts.indexOf('/', lastSlash);
      String part = nextSlash == -1 ? parts.substring(lastSlash) : parts.substring(lastSlash, nextSlash);
      currentPath += "/" + part;

      if (nextSlash != -1)
      {
        output += "<a href='" + currentPath + "'>" + part + "</a> &gt; ";
        lastSlash = nextSlash + 1;
      }
      else
      {
        output += part;
        lastSlash = -1;
      }
    }
  }
  else
  {
    output += "Root Directory";
  }

  output += R"(</div>
    <h1>Directory Listing: )";
  output += path;
  output += R"(</h1>
    
    <table>
      <tr>
        <th>Name</th>
        <th>Size</th>
        <th>Type</th>
        <th>Action</th>
      </tr>)";

  File root = SD.open(path);
  File file = root.openNextFile();
  if (path == "/")
  {
    path = "";
  }

  while (file)
  {
    String fileName = file.name();
    // Skip hidden files and special directories
    if (!fileName.startsWith(".") && fileName.length() > 0)
    {
      String filePath = path + "/" + fileName;
      output += "<tr>";
      output += "<td><a href='" + filePath + "'>" + fileName + "</a></td>";
      output += "<td>" + String(file.isDirectory() ? "-" : String(file.size())) + "</td>";
      output += "<td>" + String(file.isDirectory() ? "DIR" : "FILE") + "</td>";

      output += "<td><button class='btn btn-danger' onclick=\"deleteFile('" + filePath + "')\">Delete</button></td>";
      output += "</tr>";
    }
    file = root.openNextFile();
  }

  output += R"(
    </table>
    
    <div class="action-buttons">
      <div class="action-box">
        <h3>Upload File</h3>
        <form id="uploadForm" method="post" action="/upload" enctype="multipart/form-data">
          <input type="hidden" name="path" value=")";
  output += path;
  output += R"(">
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
          <input type="hidden" name="path" value=")";
  output += path;
  output += R"(">
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
    <footer>Protopanda v)";

  output += PANDA_VERSION;

  output += R"(</footer>
  </body>
  </html>)";

  request->send(200, "text/html", output);
}

void handleLuaExecution(AsyncWebServerRequest *request)
{
  if (request->method() != HTTP_POST)
  {
    request->send(405, "text/plain", "Method Not Allowed");
    return;
  }

  if (!request->hasParam("body", true))
  {
    request->send(400, "text/plain", "Missing Lua code in body");
    return;
  }

  String luaCode = request->getParam("body", true)->value();

  if (!g_lua.DoString(luaCode.c_str(), 1))
  {
    String error = g_lua.getLastError();
    request->send(500, "text/plain", "Lua Error: " + error);
    return;
  }
  request->send(200, "text/plain", g_lua.getLastReturnAsString());
}

TaskHandle_t composeTaskHandle = NULL;
TaskHandle_t managedFramesHandle = NULL;
bool compositionComplete = false;
static bool isManaged = false;
static uint32_t managedDuration = 0;
static uint32_t lastCompose = 0;
void composeBulkFileTask(void *parameter){
  if (lastCompose > millis()){
    return;
  }
  lastCompose = millis() + 10*1000;
  g_frameRepo.composeBulkFile();
  compositionComplete = true;
  vTaskDelay(pdMS_TO_TICKS(1000)); // Short delay before cleanup
  composeTaskHandle = NULL;
  lastCompose = millis() + 10*1000;
  vTaskDelete(NULL);
}

void handleComposeStart(AsyncWebServerRequest *request)
{
  // Check if task is already running
  if (composeTaskHandle != NULL)
  {
    request->send(200, "text/plain", "Status: Composition already in progress");
    return;
  }
  if (lastCompose > millis()){
    request->send(200, "text/plain", "Status: Composition already in progress");
    return;
  }

  compositionComplete = false;

  BaseType_t result = xTaskCreate(
      composeBulkFileTask,
      "BulkCompose",
      64192,
      NULL,
      tskIDLE_PRIORITY,
      &composeTaskHandle);

  if (result == pdPASS)
  {
    request->send(200, "text/plain", "Status: Composition started successfully");
  }
  else
  {
    request->send(500, "text/plain", "Error: Failed to start composition task");
  }
}

void managedLoop(void *)
{
  
  for (;;)
  {
    Devices::BeginAutoFrame();
    g_animation.Update(g_frameRepo.takeFile());
    g_frameRepo.freeFile();
    Devices::EndAutoFrame();
    vTaskDelay(5);
    if (millis() > managedDuration){
      break;
    }
  }
  isManaged = false;
  vTaskDelete(NULL);
}

void handleSetManaged(AsyncWebServerRequest *request)
{
  
  if (!isManaged)
  {
    managedDuration = millis() + 10*1000;
    isManaged = true;
    xTaskCreate(
        managedLoop,
        "second loop",
        10000,
        NULL,
        tskIDLE_PRIORITY,
        &managedFramesHandle);

    
    request->send(200, "text/plain", "ok");
  }
  else
  {
    request->send(400, "text/plain", "already running");
  }
}
void handleComposeGet(AsyncWebServerRequest *request)
{
  request->send(200, "text/plain", String(g_frameRepo.getBulkComposingPercentage()));
}

void startWifiServer()
{
  server = new AsyncWebServer(EDIT_MODE_HTTP_PORT);

  server->on("/", HTTP_GET, serveDirectoryListing);
  server->serveStatic("/", SD, "/", "max-age=0").setCacheControl("max-age=0");
  server->on("/mkdir", HTTP_POST, handleMkdir);
  server->on("/upload", HTTP_POST, [](AsyncWebServerRequest *request)
             { request->send(200); }, handleUpload);
  server->on("/delete", HTTP_DELETE, handleRm);
  server->on("/copy", HTTP_PUT, handleCopy);
  server->on("/lua", HTTP_POST, handleLuaExecution);
  server->on("/compose_start", HTTP_POST, handleComposeStart);
  server->on("/compose_progress", HTTP_GET, handleComposeGet);
  server->on("/manage", HTTP_GET, handleSetManaged);
  server->onNotFound(serveDirectoryListing);
  uint32_t freeHeapBytes = ESP.getFreeHeap();  
  uint32_t totalHeapBytes = ESP.getHeapSize(); 
  uint32_t freePsramBytes = ESP.getFreePsram(); 
  uint32_t totalPsramBytes = ESP.getPsramSize(); 

  float percentageHeapFree = freeHeapBytes * 100.0f / (float)totalHeapBytes;
  float percentagePsramFree = freePsramBytes* 100.0f / (float)totalPsramBytes;

  Serial.printf("[Memory] %.1f%% free - %d of %d bytes free (psram: %d / %d  -> %.1f%%)", percentageHeapFree, freeHeapBytes, totalHeapBytes, totalPsramBytes, freePsramBytes, percentagePsramFree);
  /*DMADisplay::Start(8, 1);

  DMADisplay::Display->clearScreen();
  DMADisplay::Display->setBrightness8(32);
  char str[] = "u gay";
  for (int i=0;i<strlen(str);i++){
    DMADisplay::Display->drawChar(0 + i * 8,2, str[i], DMADisplay::Display->color333(255,255,255), 0, 1);
  }
  DMADisplay::Display->flipDMABuffer();*/

  freeHeapBytes = ESP.getFreeHeap();  
  totalHeapBytes = ESP.getHeapSize(); 
  freePsramBytes = ESP.getFreePsram(); 
  totalPsramBytes = ESP.getPsramSize(); 

  percentageHeapFree = freeHeapBytes * 100.0f / (float)totalHeapBytes;
  percentagePsramFree = freePsramBytes* 100.0f / (float)totalPsramBytes;
  Serial.printf("[Memory] %.1f%% free - %d of %d bytes free (psram: %d / %d  -> %.1f%%)", percentageHeapFree, freeHeapBytes, totalHeapBytes, totalPsramBytes, freePsramBytes, percentagePsramFree);
  server->begin();
}