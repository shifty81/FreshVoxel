/**
 * @file test_main.cpp
 * @brief Custom main function for tests with better error handling and diagnostics
 * 
 * This custom main provides:
 * - Better error messages for initialization failures
 * - Console window retention on Windows
 * - Diagnostic output for troubleshooting
 */

#include <gtest/gtest.h>
#include <iostream>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#endif

// Test environment setup that handles initialization errors gracefully
class TestEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        std::cout << "==========================================================\n";
        std::cout << "FreshVoxel Engine - Test Suite\n";
        std::cout << "==========================================================\n";
        std::cout << "Initializing test environment...\n";
        
        // Check if we're in a Windows environment without display
        #ifdef _WIN32
        std::cout << "Platform: Windows\n";
        #elif __linux__
        std::cout << "Platform: Linux\n";
        #elif __APPLE__
        std::cout << "Platform: macOS\n";
        #else
        std::cout << "Platform: Unknown\n";
        #endif
        
        std::cout << "Test environment initialized successfully\n";
        std::cout << "==========================================================\n\n";
    }
    
    void TearDown() override {
        std::cout << "\n==========================================================\n";
        std::cout << "Test environment cleaned up\n";
        std::cout << "==========================================================\n";
    }
};

int main(int argc, char **argv) {
    std::cout << "FreshVoxel Engine Test Suite Starting...\n\n";
    
    // Initialize Google Test
    try {
        ::testing::InitGoogleTest(&argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "ERROR: Failed to initialize Google Test: " << e.what() << "\n";
        #ifdef _WIN32
        std::cout << "\nPress any key to exit...";
        _getch();
        #endif
        return 1;
    }
    
    // Add our custom test environment
    ::testing::AddGlobalTestEnvironment(new TestEnvironment);
    
    // Run all tests
    int result = -1;
    try {
        result = RUN_ALL_TESTS();
    } catch (const std::exception& e) {
        std::cerr << "\nFATAL ERROR: Exception during test execution: " << e.what() << "\n";
        result = 1;
    } catch (...) {
        std::cerr << "\nFATAL ERROR: Unknown exception during test execution\n";
        result = 1;
    }
    
    // Display test results summary
    std::cout << "\n==========================================================\n";
    if (result == 0) {
        std::cout << "SUCCESS: All tests passed!\n";
    } else {
        std::cout << "FAILURE: Some tests failed (exit code: " << result << ")\n";
    }
    std::cout << "==========================================================\n";
    
    #ifdef _WIN32
    // On Windows, pause before closing console if tests were run directly
    // (not from Visual Studio or command prompt that stays open)
    // Check if we should pause by looking at environment variables
    char* vsEnv = nullptr;
    char* termEnv = nullptr;
    size_t len = 0;
    _dupenv_s(&vsEnv, &len, "VisualStudioVersion");
    _dupenv_s(&termEnv, &len, "TERM");
    
    // Pause if not running from VS or a persistent terminal
    if (!vsEnv && !termEnv) {
        std::cout << "\nPress any key to exit...";
        _getch();
    }
    
    // Free allocated memory
    free(vsEnv);
    free(termEnv);
    #endif
    
    return result;
}
