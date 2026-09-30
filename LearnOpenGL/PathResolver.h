#pragma once

#include <string>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

class PathResolver {
public:
	// Get the executable's directory
	static std::string GetExecutableDir() {
		// Use filesystem to get the current working directory and check relative paths
		// This avoids including windows.h which causes namespace pollution
		try {
			std::string exePath(fs::current_path().string());
			return exePath;
		}
		catch (...) {
			return ".";
		}
	}

	// Get the project root directory (finds the LearnOpenGL project folder)
	static std::string GetProjectRoot() {
		std::string exeDir = GetExecutableDir();
		std::cout << "Executable directory: " << exeDir << std::endl;

		// Try to find the project root by looking for common locations
		// Case 1: Running from x64/Release or x64/Debug or Win32/Release or Win32/Debug
		fs::path current(exeDir);

		// Go up directory tree looking for LearnOpenGL folder or Resources folder
		int attempts = 0;
		while (attempts < 10) {
			// Check if we can find Resources folder here
			if (fs::exists(current / "Resources")) {
				std::cout << "Found Resources at: " << current.string() << std::endl;
				return current.string();
			}

			// Check if parent has LearnOpenGL/Resources
			if (fs::exists(current.parent_path() / "LearnOpenGL" / "Resources")) {
				std::cout << "Found LearnOpenGL/Resources at: " << (current.parent_path() / "LearnOpenGL").string() << std::endl;
				return (current.parent_path() / "LearnOpenGL").string();
			}

			// Go up one level
			if (current.parent_path() == current) {
				// Reached filesystem root
				break;
			}
			current = current.parent_path();
			attempts++;
		}

		// Fallback: assume Resources is in the current directory
		std::cout << "Warning: Could not find Resources folder. Using current directory." << std::endl;
		return ".";
	}

	// Resolve asset path - tries multiple locations
	static std::string ResolveAssetPath(const std::string& relativePath) {
		// First, try relative to project root
		std::string projectRoot = GetProjectRoot();
		std::string fullPath = projectRoot + "/" + relativePath;

		if (fs::exists(fullPath)) {
			std::cout << "Resolved asset: " << fullPath << std::endl;
			return fullPath;
		}

		// Try directly in current directory
		if (fs::exists(relativePath)) {
			std::cout << "Resolved asset (current dir): " << relativePath << std::endl;
			return relativePath;
		}

		// Try relative to current working directory
		std::string exeDir = GetExecutableDir();
		fullPath = exeDir + "/" + relativePath;
		if (fs::exists(fullPath)) {
			std::cout << "Resolved asset (exe dir): " << fullPath << std::endl;
			return fullPath;
		}

		// If not found, return the project root version anyway (will fail with nice error)
		std::cerr << "WARNING: Asset not found: " << relativePath << std::endl;
		std::cerr << "         Tried: " << (projectRoot + "/" + relativePath) << std::endl;
		std::cerr << "         Tried: " << relativePath << std::endl;
		std::cerr << "         Tried: " << fullPath << std::endl;
		return projectRoot + "/" + relativePath;
	}
};
