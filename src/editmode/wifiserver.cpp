#include "editmode/editmode.hpp"
#ifdef ENABLE_EDIT_MODE
#include "tools/config_default.hpp"
#include "tools/devices.hpp"
#include "drawing/framerepository.hpp"
#include "lua/luainterface.hpp"
#include "drawing/animation.hpp"


#if PANDA_SD_MODE == 1
#include <SD.h>
#elif PANDA_SD_MODE == 2
#include <SD_MMC.h>
#else
#error "NO SD_MODE Mode defined (set PANDA_SD_MODE to 1 for SD or 2 for SD_MMC)"
#endif

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <algorithm>
#include <vector>

AsyncWebServer *server;
#ifdef ENABLE_LUA
extern LuaInterface g_lua;
#endif
extern FrameRepository g_frameRepo;
extern Animation g_animation;

bool createDirectories(String path){
  String currentPath = "";
  int startIdx = 0;
  int slashIdx;

  while ((slashIdx = path.indexOf('/', startIdx)) != -1){
    currentPath = path.substring(0, slashIdx);
    if (!PANDA_SD.exists(currentPath)){
      PANDA_SD.mkdir(currentPath);
    }
    startIdx = slashIdx + 1;
  }

  if (!PANDA_SD.exists(path)){
    PANDA_SD.mkdir(path);
  }
  return true;
}

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
  static File uploadFile;
  static String filePath;

  if (!index){
    String path = "/";
    if (request->hasParam("path", true)){
      path = request->getParam("path", true)->value();
    }
    filePath = path + "/" + filename;

    String dirPath = filePath.substring(0, filePath.lastIndexOf('/'));
    if (dirPath.length() > 0 && !PANDA_SD.exists(dirPath)){
      if (!createDirectories(dirPath)){
        request->send(500, "text/plain", "{\"success\": false, \"error\": \"Failed to open file for writing\"}");
      }
    }

    uploadFile = PANDA_SD.open(filePath, FILE_WRITE);
    if (!uploadFile){
      request->send(500, "text/plain", "{\"success\": false, \"error\": \"Failed to open file for writing\"}");
      Serial.println("Failed to open file for writing");
      return;
    }
  }

  if (uploadFile && len){
    uploadFile.write(data, len);
  }

  if (final && uploadFile){
    uploadFile.close();
    request->send(200, "text/plain", "{\"success\": true}");
  }
}

void handleCopy(AsyncWebServerRequest *request){
  if (!request->hasParam("src") || !request->hasParam("dst")){
    request->send(400, "text/plain", "Missing src or dst parameter");
    return;
  }

  String srcPath = request->getParam("src")->value();
  String dstPath = request->getParam("dst")->value();

  if (!PANDA_SD.exists(srcPath)){
    request->send(404, "text/plain", "Source file not found");
    return;
  }

  File src = PANDA_SD.open(srcPath);
  if (src.isDirectory()){
    src.close();
    request->send(400, "text/plain", "Source is a directory");
    return;
  }

  // Extract directory path from dstPath
  int lastSlash = dstPath.lastIndexOf('/');
  if (lastSlash > 0){
    String dstDir = dstPath.substring(0, lastSlash);
    
    // Create directory if it doesn't exist
    if (!PANDA_SD.exists(dstDir)){
      // Create all necessary parent directories
      String currentPath = "";
      for (int i = 0; i < dstDir.length(); i++){
        currentPath += dstDir[i];
        if (dstDir[i] == '/' && i > 0) // Found a directory level
        {
          if (!PANDA_SD.exists(currentPath.substring(0, currentPath.length() - 1))){
            if (!PANDA_SD.mkdir(currentPath.substring(0, currentPath.length() - 1))){
              src.close();
              request->send(500, "text/plain", "Failed to create directory: " + currentPath);
              return;
            }
          }
        }
      }
      
      // Create the final directory
      if (!PANDA_SD.mkdir(dstDir)){
        src.close();
        request->send(500, "text/plain", "Failed to create destination directory");
        return;
      }
    }
  }

  // Check if destination already exists (file, not directory)
  if (PANDA_SD.exists(dstPath)){
    File dstCheck = PANDA_SD.open(dstPath);
    if (!dstCheck.isDirectory()) // Only fail if it's a file
    {
      src.close();
      dstCheck.close();
      request->send(409, "text/plain", "Destination already exists");
      return;
    }
    dstCheck.close();
  }

  File dst = PANDA_SD.open(dstPath, FILE_WRITE);
  if (!dst){
    src.close();
    request->send(500, "text/plain", "Failed to create destination file");
    return;
  }

  uint8_t buffer[512];
  size_t bytesRead;
  bool error = false;
  
  while ((bytesRead = src.read(buffer, sizeof(buffer))) > 0){
    if (dst.write(buffer, bytesRead) != bytesRead){
      error = true;
      break;
    }
  }

  src.close();
  dst.close();

  if (error){
    PANDA_SD.remove(dstPath);
    request->send(500, "text/plain", "Copy failed");
    return;
  }

  request->send(200, "text/plain", "OK");
}

void handleMv(AsyncWebServerRequest *request){
  if (!request->hasParam("src") || !request->hasParam("dst")){
    request->send(400, "text/plain", "Missing src or dst parameter");
    return;
  }

  String srcPath = request->getParam("src")->value();
  String dstPath = request->getParam("dst")->value();

  if (!PANDA_SD.exists(srcPath)){
    request->send(404, "text/plain", "Source file not found");
    return;
  }

  File src = PANDA_SD.open(srcPath);
  if (src.isDirectory()){
    src.close();
    request->send(400, "text/plain", "Source is a directory");
    return;
  }

  // Extract directory path from dstPath
  int lastSlash = dstPath.lastIndexOf('/');
  if (lastSlash > 0){
    String dstDir = dstPath.substring(0, lastSlash);
    
    // Create directory if it doesn't exist
    if (!PANDA_SD.exists(dstDir)){
      // Create all necessary parent directories
      String currentPath = "";
      for (int i = 0; i < dstDir.length(); i++){
        currentPath += dstDir[i];
        if (dstDir[i] == '/' && i > 0) // Found a directory level
        {
          if (!PANDA_SD.exists(currentPath.substring(0, currentPath.length() - 1))){
            if (!PANDA_SD.mkdir(currentPath.substring(0, currentPath.length() - 1))){
              src.close();
              request->send(500, "text/plain", "Failed to create directory: " + currentPath);
              return;
            }
          }
        }
      }
      
      // Create the final directory
      if (!PANDA_SD.mkdir(dstDir)){
        src.close();
        request->send(500, "text/plain", "Failed to create destination directory");
        return;
      }
    }
  }

  // Check if destination already exists (file, not directory)
  if (PANDA_SD.exists(dstPath)){
    File dstCheck = PANDA_SD.open(dstPath);
    if (!dstCheck.isDirectory()) // Only fail if it's a file
    {
      src.close();
      dstCheck.close();
      request->send(409, "text/plain", "Destination already exists");
      return;
    }
    dstCheck.close();
  }

  File dst = PANDA_SD.open(dstPath, FILE_WRITE);
  if (!dst){
    src.close();
    request->send(500, "text/plain", "Failed to create destination file");
    return;
  }

  uint8_t buffer[512];
  size_t bytesRead;
  bool error = false;
  
  while ((bytesRead = src.read(buffer, sizeof(buffer))) > 0){
    if (dst.write(buffer, bytesRead) != bytesRead){
      error = true;
      break;
    }
  }

  src.close();
  dst.close();

  if (error){
    PANDA_SD.remove(dstPath);
    request->send(500, "text/plain", "Copy failed");
    return;
  }

  PANDA_SD.remove(srcPath);

  request->send(200, "text/plain", "OK");
}

void handleRm(AsyncWebServerRequest *request){
  if (!request->hasParam("path")){
    request->send(400, "text/plain", "Missing path parameter");
    return;
  }

  String path = request->getParam("path")->value();

  if (!PANDA_SD.exists(path)){
    request->send(404, "text/plain", "Path not found");
    return;
  }

  File file = PANDA_SD.open(path);
  if (file.isDirectory()){
    bool isEmpty = true;
    File entry = file.openNextFile();
    while (entry){
      if (String(entry.name()) != "." && String(entry.name()) != ".."){
        isEmpty = false;
        break;
      }
      entry = file.openNextFile();
    }
    entry.close();

    if (!isEmpty){
      file.close();
      request->send(400, "text/plain", "Directory is not empty");
      return;
    }

    file.close();
    if (!PANDA_SD.rmdir(path)){
      request->send(500, "text/plain", "Failed to delete directory");
      return;
    }
  }else{
    file.close();
    if (!PANDA_SD.remove(path)){
      request->send(500, "text/plain", "Failed to delete file");
      return;
    }
  }

  request->send(200, "text/plain", "Deleted successfully");
}

void handleMkdir(AsyncWebServerRequest *request){
  if (request->hasParam("path", true) && request->hasParam("dirName", true)){
    String basePath = request->getParam("path", true)->value();
    String dirName = request->getParam("dirName", true)->value();
    String fullPath = basePath + "/" + dirName;

    fullPath.replace("//", "/");
    if (PANDA_SD.exists(fullPath)){
      request->send(400, "text/plain", "Directory already exists");
    }
    else if (PANDA_SD.mkdir(fullPath)){
      request->send(200, "text/plain", "Directory created successfully");
    }else{
      request->send(500, "text/plain", "Failed to create directory");
    }
  }else{
    request->send(400, "text/plain", "Missing parameters");
  }
}
struct DirectoryEntry {
  String name;
  size_t size;
  bool directory;
};

String htmlEscape(const String &value){
  String escaped;
  escaped.reserve(value.length() + 16);
  for (size_t i = 0; i < value.length(); i++){
    switch (value[i]){
      case '&': escaped += "&amp;"; break;
      case '<': escaped += "&lt;"; break;
      case '>': escaped += "&gt;"; break;
      case '"': escaped += "&quot;"; break;
      case '\'': escaped += "&#39;"; break;
      default: escaped += value[i]; break;
    }
  }
  return escaped;
}

String jsEscape(const String &value){
  String escaped;
  escaped.reserve(value.length() + 8);
  for (size_t i = 0; i < value.length(); i++){
    char c = value[i];
    if (c == '\\' || c == '\'') escaped += '\\';
    if (c == '\n') escaped += "\\n";
    else if (c == '\r') escaped += "\\r";
    else escaped += c;
  }
  return escaped;
}

bool isEditableFileName(const String &name){
  String lower = name;
  lower.toLowerCase();
  return lower.endsWith(".json") || lower.endsWith(".lua") || lower.endsWith(".txt") ||
         lower.endsWith(".md") || lower.endsWith(".csv") || lower.endsWith(".xml") ||
         lower.endsWith(".html") || lower.endsWith(".htm") || lower.endsWith(".css") ||
         lower.endsWith(".js") || lower.endsWith(".ini") || lower.endsWith(".toml") ||
         lower.endsWith(".yaml") || lower.endsWith(".yml");
}

void handleEdit(AsyncWebServerRequest *request){
  if (!request->hasParam("path") && !request->hasParam("path", true)){
    request->send(400, "text/plain", "Missing path parameter");
    return;
  }

  String path = request->hasParam("path")
    ? request->getParam("path")->value()
    : request->getParam("path", true)->value();
  path.replace("//", "/");
  if (!path.startsWith("/")) path = "/" + path;
  if (path.indexOf("..") >= 0 || !isEditableFileName(path) || !PANDA_SD.exists(path)){
    request->send(400, "text/plain", "File is not editable or was not found");
    return;
  }

  File file = PANDA_SD.open(path, FILE_READ);
  if (!file || file.isDirectory()){
    request->send(400, "text/plain", "Directories are not editable");
    return;
  }

  if (request->method() == HTTP_GET){
    file.close();
    request->send(PANDA_SD, path, "text/plain; charset=utf-8");
    return;
  }

  file.close();
  if (request->method() != HTTP_POST || !request->hasParam("content", true)){
    request->send(405, "text/plain", "Expected POST content");
    return;
  }

  String content = request->getParam("content", true)->value();
  File output = PANDA_SD.open(path, FILE_WRITE);
  if (!output){
    request->send(500, "text/plain", "Failed to open file for writing");
    return;
  }
  size_t written = output.write((const uint8_t *)content.c_str(), content.length());
  output.close();
  if (written != content.length()){
    request->send(500, "text/plain", "Failed to save complete file");
    return;
  }
  request->send(200, "text/plain", "File saved successfully");
}

void serveDirectoryListing(AsyncWebServerRequest *request){
  String path = request->url();
  path.replace("//", "/");
  while (path.endsWith("/") && path.length() > 1){
    path.remove(path.length() - 1);
  }

  if (!PANDA_SD.exists(path)){
    path = "/";
  }

  String output = R"(
  <html>
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
      /* Compact directory view and responsive editor. */
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
      .editor-modal { display: none; position: fixed; inset: 0; z-index: 10; background: rgba(0,0,0,.72); padding: 5vh 16px; overflow-y: auto; -webkit-overflow-scrolling: touch; }
      .editor-modal.open { display: flex; align-items: center; justify-content: center; }
      .editor-card { width: min(900px, 100%); max-height: 90vh; min-height: 0; display: flex; flex-direction: column; background: #2d2d2d; border: 1px solid #555; border-radius: 8px; padding: 16px; box-shadow: 0 8px 30px #000; }
      .editor-head { display: flex; align-items: center; justify-content: space-between; gap: 12px; margin-bottom: 12px; }
      .editor-head h2 { color: #ff9900; font-size: 20px; margin: 0; overflow: hidden; text-overflow: ellipsis; white-space: nowrap; }
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
        .editor-modal { padding: 12px; }
        .editor-card { max-height: 96vh; padding: 12px; }
        .code-editor { height: 62vh; min-height: 0; }
        .code-editor textarea { font-size: 13px; }
      }
    </style>
  </head>
  <body>
    <div class="breadcrumb">)";

  if (path != "/"){
    output += "<a href='/'>/</a> > ";

    String currentPath = "";
    String parts = path.substring(1);
    int lastSlash = 0;

    while (lastSlash != -1){
      int nextSlash = parts.indexOf('/', lastSlash);
      String part = nextSlash == -1 ? parts.substring(lastSlash) : parts.substring(lastSlash, nextSlash);
      currentPath += "/" + part;

      if (nextSlash != -1){
        output += "<a href='" + currentPath + "'>" + part + "</a> > ";
        lastSlash = nextSlash + 1;
      }else{
        output += part;
        lastSlash = -1;
      }
    }
  }else{
    output += "Root Directory";
  }

  output += R"(</div>)";
  
  // Header Section Logic for Root Path
  if (path == "/"){
    // Logo container
    output += R"(
    <div class="logo-container">
      <img src="/doc/logoprotopanda.png" alt="Protopanda Logo" class="logo-img">
    </div>)";
    
    // Welcome message and buttons
    output += R"(
    <div class="main-header">
      <p>Welcome to ProtoPanda</p>
      <div class="header-links">
        <a href="/editor.html" class="btn btn-primary editor-btn">Static Frame Editor</a>
        <a href="/modeleditor.html" class="btn btn-primary editor-btn">Model and Keyframe Editor</a>
      </div>
    </div>)";
  }

  output += R"(
    <h1>Directory Listing: )";
  output += htmlEscape(path);
  output += R"(</h1>
    
    <div class="table-responsive">
    <table>
      <thead><tr>
        <th>Name</th>
        <th>Size</th>
        <th>Type</th>
        <th>Action</th>
      </tr></thead>
      <tbody>)";

  String linkPrefix = path == "/" ? "" : path;
  std::vector<DirectoryEntry> entries;
  File root = PANDA_SD.open(path);
  if (root){
    File file = root.openNextFile();
    while (file){
      String fileName = file.name();
      if (!fileName.startsWith(".") && fileName.length() > 0){
        entries.push_back({fileName, file.isDirectory() ? 0 : file.size(), file.isDirectory()});
      }
      file.close();
      file = root.openNextFile();
    }
    root.close();
  }

  std::sort(entries.begin(), entries.end(), [](const DirectoryEntry &a, const DirectoryEntry &b){
    if (a.directory != b.directory) return a.directory > b.directory;
    String aName = a.name;
    String bName = b.name;
    aName.toLowerCase();
    bName.toLowerCase();
    return aName < bName;
  });

  for (const DirectoryEntry &entry : entries){
    String filePath = linkPrefix + "/" + entry.name;
    filePath.replace("//", "/");
    String safeName = htmlEscape(entry.name);
    String safePath = htmlEscape(filePath);
    String scriptPath = jsEscape(filePath);
    output += "<tr>";
    output += "<td><span class='file-name'><span class='file-icon ";
    output += entry.directory ? "folder-icon" : "document-icon";
    output += "' aria-hidden='true'></span><a href='" + safePath + "'>" + safeName + "</a></span></td>";
    output += "<td>" + String(entry.directory ? "-" : String(entry.size)) + "</td>";
    output += "<td>" + String(entry.directory ? "DIR" : "FILE") + "</td>";
    output += "<td>";
    if (!entry.directory && isEditableFileName(entry.name)){
      output += "<button class='btn btn-primary' onclick=\"openEditor('" + scriptPath + "','" + jsEscape(entry.name) + "')\">Edit</button>";
    }
    output += "<button class='btn btn-danger' onclick=\"deleteFile('" + scriptPath + "','" + (entry.directory ? "DIR" : "FILE") + "')\">Delete</button>";
    output += "</td></tr>";
  }

  output += R"PP(
      </tbody></table>
    </div>
    
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

    <div id="editorModal" class="editor-modal" role="dialog" aria-modal="true" aria-labelledby="editorTitle">
      <div class="editor-card">
        <div class="editor-head"><h2 id="editorTitle">Edit file</h2><button class="btn btn-secondary" onclick="closeEditor()">X</button></div>
        <div class="code-editor"><textarea id="editorMount" aria-label="Code editor" spellcheck="false"></textarea></div>
        <div class="editor-actions"><button class="btn btn-secondary" onclick="closeEditor()">Cancel</button><button class="btn btn-primary" onclick="saveEditor()">Save</button></div>
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
        try {
          const body = new URLSearchParams({path: editorPath, content: editorMount.value});
          const response = await fetch('/edit', {method: 'POST', body});
          if (!response.ok) throw new Error(await response.text());
          setStatus(statusDiv, 'Saved!', false);
          setTimeout(closeEditor, 500);
        } catch (error) { setStatus(statusDiv, 'Error: ' + error.message, true); }
      }
      document.getElementById('editorModal').addEventListener('click', e => { if (e.target === e.currentTarget) closeEditor(); });
      function deleteFile(path, type) {
        deletePath = path;
        document.getElementById('deleteTitle').textContent = type === 'DIR' ? 'Delete folder?' : 'Delete file?';
        document.getElementById('deleteMessage').textContent = type === 'DIR'
          ? 'The folder must be empty. Delete ' + path + '?' : 'Delete ' + path + '?';
        const dialog = document.getElementById('deleteDialog');
        if (typeof dialog.showModal === 'function') dialog.showModal();
        else dialog.setAttribute('open', '');
      }
      document.getElementById('deleteDialog').addEventListener('close', async function(){
        if (this.returnValue !== 'confirm') return;
        try {
          const response = await fetch('/delete?path=' + encodeURIComponent(deletePath), {method: 'DELETE'});
          if (!response.ok) throw new Error(await response.text());
          showToast('Deleted');
          setTimeout(() => location.reload(), 500);
        } catch (error) { showToast('Delete failed: ' + error.message, true); }
      });
      function showToast(message, isError) {
        const toast = document.getElementById('toast');
        toast.textContent = message;
        toast.className = isError ? 'toast error show' : 'toast show';
        clearTimeout(toastTimer);
        toastTimer = setTimeout(() => { toast.className = isError ? 'toast error' : 'toast'; }, 2600);
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
    <footer>ProtoPanda v)PP";

  output += PANDA_VERSION;

  output += R"( | Pixel Art Editor</footer>
  </body>
  </html>)";

  request->send(200, "text/html; charset=utf-8", output);
}

void handleLuaExecution(AsyncWebServerRequest *request){
  if (request->method() != HTTP_POST)  {
    request->send(405, "text/plain", "Method Not Allowed");
    return;
  }

  if (!request->hasParam("body", true))  {
    request->send(400, "text/plain", "Missing Lua code in body");
    return;
  }

  String luaCode = request->getParam("body", true)->value();
  #ifdef ENABLE_LUA
  if (!g_lua.DoString(luaCode.c_str(), 1)){
    String error = g_lua.getLastError();
    request->send(500, "text/plain", "Lua Error: " + error);
    return;
  }
  
  request->send(200, "text/plain", g_lua.getLastReturnAsString());
  #else
  request->send(500, "text/plain", "Lua Error not enabled");
  #endif
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

void handleComposeStart(AsyncWebServerRequest *request){
  // Check if task is already running
  if (composeTaskHandle != NULL){
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

  if (result == pdPASS){
    request->send(200, "text/plain", "Status: Composition started successfully");
  }else{
    request->send(500, "text/plain", "Error: Failed to start composition task");
  }
}

void managedLoop(void *){
  
  for (;;){
    Devices::BeginAutoFrame();
    g_animation.Update(Devices::getAutoDeltaTime());
    Devices::EndAutoFrame();
    vTaskDelay(5);
    if (millis() > managedDuration){
      break;
    }
  }
  isManaged = false;
  vTaskDelete(NULL);
}

void handleSetManaged(AsyncWebServerRequest *request){
  
  if (!isManaged){
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
  }else{
    request->send(400, "text/plain", "already running");
  }
}
void handleComposeGet(AsyncWebServerRequest *request){
  request->send(200, "text/plain", String(g_frameRepo.getBulkComposingPercentage()));
}

void startWifiServer(int port){
  server = new AsyncWebServer(port);

  server->on("/", HTTP_GET, serveDirectoryListing);
  server->serveStatic("/", PANDA_SD, "/", "max-age=0").setCacheControl("max-age=0");
  server->on("/edit", HTTP_GET, handleEdit);
  server->on("/edit", HTTP_POST, handleEdit);
  server->on("/mkdir", HTTP_POST, handleMkdir);
  server->on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){ request->send(200); }, handleUpload);
  server->on("/delete", HTTP_DELETE, handleRm);
  server->on("/copy", HTTP_PUT, handleCopy);
  server->on("/mv", HTTP_PUT, handleMv);
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

  Serial.printf("[Memory] %.1f%% free - %lu of %lu bytes free (psram: %lu / %lu  -> %.1f%%)", percentageHeapFree, freeHeapBytes, totalHeapBytes, totalPsramBytes, freePsramBytes, percentagePsramFree);

  freeHeapBytes = ESP.getFreeHeap();  
  totalHeapBytes = ESP.getHeapSize(); 
  freePsramBytes = ESP.getFreePsram(); 
  totalPsramBytes = ESP.getPsramSize(); 

  percentageHeapFree = freeHeapBytes * 100.0f / (float)totalHeapBytes;
  percentagePsramFree = freePsramBytes* 100.0f / (float)totalPsramBytes;
  Serial.printf("[Memory] %.1f%% free - %lu of %lu bytes free (psram: %lu / %lu  -> %.1f%%)", percentageHeapFree, freeHeapBytes, totalHeapBytes, totalPsramBytes, freePsramBytes, percentagePsramFree);
  server->begin();
}
#endif
