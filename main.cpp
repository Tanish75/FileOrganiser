/*
================================================================================
FILE ORGANIZER - C++ Project
================================================================================
What this program does:
1. SCAN: Recursively goes through every folder and subfolder to find all files.
2. ANALYZE: Counts how many files of each type (PDF, JPG, etc.) exist.
3. ORGANIZE: Creates folders (like "PDFs", "Images") and moves files into them.

How to run:
1. Compile: g++ -std=c++17 main.cpp -o organizer -lstdc++fs
2. Run: ./organizer (or organizer.exe on Windows)
3. It will process the folder path you set below.
================================================================================
*/

#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <unordered_map>   // For counting file types and folder mapping
#include <algorithm>       // For sorting files by size
#include <iomanip>         // For nice-looking output formatting

namespace fs = std::filesystem;  // Shortcut so we don't type std::filesystem everywhere

// -----------------------------------------------------------------------------
// STEP 1: Define what a "File" looks like in our program
// Think of this as a digital ID card for every file.
// -----------------------------------------------------------------------------
struct FileInfo {
    std::string name;       // e.g., "resume.pdf"
    std::string path;       // e.g., "C:/Downloads/resume.pdf"
    std::string extension;  // e.g., ".pdf"
    uintmax_t size;         // File size in bytes (a number)
};

// -----------------------------------------------------------------------------
// HELPER FUNCTION: Scan a folder and collect all file information
// -----------------------------------------------------------------------------
std::vector<FileInfo> scanDirectory(const std::string& folderPath) {
    std::vector<FileInfo> files;
    
    std::cout << "🔍 Scanning folder: " << folderPath << std::endl;
    
    // recursive_directory_iterator goes into every subfolder automatically
    for (const auto& entry : fs::recursive_directory_iterator(folderPath)) {
        // Check if it's a real file (not a folder)
        if (fs::is_regular_file(entry.path())) {
            FileInfo info;
            info.path = entry.path().string();
            info.name = entry.path().filename().string();
            info.extension = entry.path().extension().string();
            info.size = fs::file_size(entry.path());
            files.push_back(info);  // Add this file to our list
        }
    }
    
    std::cout << "✅ Found " << files.size() << " files." << std::endl << std::endl;
    return files;
}

// -----------------------------------------------------------------------------
// HELPER FUNCTION: Analyze files and print a summary
// -----------------------------------------------------------------------------
void analyzeFiles(const std::vector<FileInfo>& files) {
    // Count how many files of each extension exist
    std::unordered_map<std::string, int> extCount;
    for (const auto& file : files) {
        extCount[file.extension]++;  // If extension exists, add 1; else create it with 1
    }
    
    // Print file type summary
    std::cout << "📊 ==== FILE TYPE SUMMARY ====" << std::endl;
    for (const auto& [extension, count] : extCount) {
        // std::setw makes the output aligned and neat
        std::cout << std::setw(10) << extension << " : " << count << " files" << std::endl;
    }
    std::cout << std::endl;
    
    // Find and print the 5 largest files
    if (files.empty()) return;  // Safety check: if no files, exit early
    
    // Create a copy of files and sort them by size (largest first)
    std::vector<FileInfo> sortedFiles = files;
    std::sort(sortedFiles.begin(), sortedFiles.end(), 
        [](const FileInfo& a, const FileInfo& b) {
            return a.size > b.size;  // Compare sizes
        });
    
    std::cout << "📁 ==== TOP 5 LARGEST FILES ====" << std::endl;
    for (int i = 0; i < 5 && i < sortedFiles.size(); i++) {
        double sizeMB = sortedFiles[i].size / (1024.0 * 1024.0);
        std::cout << "  " << i+1 << ". " << sortedFiles[i].name 
                  << " (" << std::fixed << std::setprecision(2) << sizeMB << " MB)" << std::endl;
    }
    std::cout << std::endl;
}

// -----------------------------------------------------------------------------
// HELPER FUNCTION: Organize files into folders
// -----------------------------------------------------------------------------
void organizeFiles(const std::vector<FileInfo>& files, const std::string& folderPath) {
    // Step 1: Define which extension goes into which folder
    std::unordered_map<std::string, std::string> folderMap;
    folderMap[".pdf"] = "PDFs";      folderMap[".PDF"] = "PDFs";
    folderMap[".jpg"] = "Images";    folderMap[".JPG"] = "Images";
    folderMap[".jpeg"] = "Images";   folderMap[".png"] = "Images";
    folderMap[".PNG"] = "Images";    folderMap[".cpp"] = "Code";
    folderMap[".CPP"] = "Code";      folderMap[".py"] = "Code";
    folderMap[".PY"] = "Code";       folderMap[".java"] = "Code";
    folderMap[".txt"] = "Documents"; folderMap[".doc"] = "Documents";
    folderMap[".docx"] = "Documents"; folderMap[".zip"] = "Archives";
    folderMap[".rar"] = "Archives";  folderMap[".mp4"] = "Videos";
    folderMap[".MP4"] = "Videos";    folderMap[".mp3"] = "Music";
    folderMap[".MP3"] = "Music";
    
    int movedCount = 0;
    
    std::cout << "🚀 ==== ORGANIZING FILES ====" << std::endl;
    
    // Step 2: Process each file
    for (const auto& file : files) {
        // Check if this file's extension is in our mapping
        auto it = folderMap.find(file.extension);
        if (it == folderMap.end()) {
            continue;  // Skip files we don't know what to do with
        }
        
        std::string folderName = it->second;
        fs::path newFolderPath = fs::path(folderPath) / folderName;
        
        // Step 3: Create the folder if it doesn't already exist
        if (!fs::exists(newFolderPath)) {
            fs::create_directory(newFolderPath);
            std::cout << "  📁 Created folder: " << folderName << "/" << std::endl;
        }
        
        // Step 4: Move the file
        fs::path dest = newFolderPath / file.name;
        if (fs::exists(dest)) {
            std::cout << "  ⚠️ Skipped (already exists): " << file.name << std::endl;
            continue;
        }
        
        try {
            fs::rename(file.path, dest);  // "Cut and Paste" in C++
            std::cout << "  ✅ Moved: " << file.name << " → " << folderName << "/" << std::endl;
            movedCount++;
        } catch (const std::exception& e) {
            std::cout << "  ❌ Error moving " << file.name << ": " << e.what() << std::endl;
        }
    }
    
    // Step 5: Print final summary
    std::cout << std::endl;
    std::cout << "✅ ==== SUMMARY ====" << std::endl;
    std::cout << "  📂 Total files processed: " << files.size() << std::endl;
    std::cout << "  📦 Files moved: " << movedCount << std::endl;
    std::cout << "  ⏭️  Files skipped (no mapping): " << (files.size() - movedCount) << std::endl;
    std::cout << "🎉 Organization complete!" << std::endl;
}

// -----------------------------------------------------------------------------
// MAIN FUNCTION: The program starts here
// -----------------------------------------------------------------------------
int main() {
    // ⚠️ IMPORTANT: Change this to the folder you want to organize!
    std::string folderPath;
    std::cout << "Enter folder path (or press Enter for default): ";
    std::getline(std::cin, folderPath);

    if (folderPath.empty()) {
    folderPath = "C:\\Users\\rajke\\Downloads\\TestFolder";  // Default
    }
    
    // PHASE 1: SCAN
    std::vector<FileInfo> files = scanDirectory(folderPath);
    
    if (files.empty()) {
        std::cout << "❌ No files found. Check your folder path!" << std::endl;
        return 1;  // Exit with error code
    }
    
    // PHASE 2: ANALYZE
    analyzeFiles(files);
    
    // PHASE 3: ORGANIZE
    organizeFiles(files, folderPath);
    
    return 0;  // All good!
}
