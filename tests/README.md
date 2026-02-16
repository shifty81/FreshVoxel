# FreshVoxel Engine - Test Suite

This directory contains unit tests and integration tests for the FreshVoxel Engine.

## Test Framework

We use [Google Test](https://github.com/google/googletest) as our testing framework.

## Running Tests

### Windows

After building, the test executable is located at:
```
build\Debug\FreshVoxelEngineTests.exe
build\Release\FreshVoxelEngineTests.exe
```

**Important for Windows users:** The test executable includes console window retention,
so it will wait for a key press before closing. This makes it easy to see test results
when running directly from Explorer or a temporary command prompt.

To run tests:

1. **From Visual Studio**: Set `FreshVoxelEngineTests` as startup project and press F5
2. **From command line**: Navigate to the build directory and run `FreshVoxelEngineTests.exe`
3. **From PowerShell**: `.\FreshVoxelEngineTests.exe`

### Linux/macOS

After building, the test executable is located at:
```
build/FreshVoxelEngineTests
```

Run tests with:
```bash
# Run all tests
./FreshVoxelEngineTests

# Run specific test suite
./FreshVoxelEngineTests --gtest_filter=ChunkTests.*

# Run with verbose output
./FreshVoxelEngineTests --gtest_verbose=1

# List all tests
./FreshVoxelEngineTests --gtest_list_tests
```

## Building Tests

Tests are built automatically when you build the project with testing enabled:

### Windows with CMake
```bash
mkdir build && cd build
cmake -G "Visual Studio 17 2022" -A x64 -DBUILD_TESTS=ON ..
cmake --build . --config Release
```

### Linux/macOS
```bash
mkdir build && cd build
cmake -DBUILD_TESTS=ON ..
cmake --build . -j$(nproc)
```

## Troubleshooting

### Test executable closes immediately (Windows)

If the test window closes before you can see the results:

1. **Solution 1 (Recommended)**: Run from a persistent command prompt or PowerShell
   - Open Command Prompt or PowerShell
   - Navigate to the build directory: `cd build\Release` or `cd build\Debug`
   - Run: `.\FreshVoxelEngineTests.exe`

2. **Solution 2**: The test executable now includes automatic pause on Windows when not
   run from Visual Studio or a persistent terminal

3. **Solution 3**: Run with output redirection:
   ```batch
   FreshVoxelEngineTests.exe > test_results.txt 2>&1
   ```

### Missing DLL errors (Windows)

If you get errors about missing DLLs (e.g., glfw3.dll, lua54.dll):

1. Ensure you built with vcpkg toolchain (it copies DLLs automatically)
2. If using manual dependency installation, copy required DLLs to the executable directory
3. Or add dependency directories to your PATH

### Tests fail to compile

Common issues:
- Missing dependencies (GLFW, GLM, etc.)
- Incorrect CMake configuration
- See BUILD.md for detailed build instructions

## Test Organization

Tests are organized by system/module:

```
tests/
├── test_main.cpp          # Custom main with error handling and diagnostics
├── core/                  # Core engine tests
├── voxel/                 # Voxel system tests
├── generation/            # Procedural generation tests
├── input/                 # Input system tests
└── scripting/             # Scripting and event tests
```

## Writing Tests

### Test File Template

```cpp
#include <gtest/gtest.h>
#include "system/YourClass.h"

// Test fixture for setup/teardown
class YourClassTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code before each test
    }

    void TearDown() override {
        // Cleanup code after each test
    }

    // Shared test data
    YourClass* testObject;
};

// Test case
TEST_F(YourClassTest, MethodName_Scenario_ExpectedBehavior) {
    // Arrange
    int input = 5;
    
    // Act
    int result = testObject->method(input);
    
    // Assert
    EXPECT_EQ(result, 10);
}

// Simple test (no fixture)
TEST(YourClassTest, SimpleTest) {
    EXPECT_TRUE(true);
}
```

### Common Assertions

```cpp
// Boolean
EXPECT_TRUE(condition);
EXPECT_FALSE(condition);

// Equality
EXPECT_EQ(expected, actual);
EXPECT_NE(val1, val2);

// Comparison
EXPECT_LT(val1, val2);  // Less than
EXPECT_LE(val1, val2);  // Less or equal
EXPECT_GT(val1, val2);  // Greater than
EXPECT_GE(val1, val2);  // Greater or equal

// Floating point
EXPECT_FLOAT_EQ(expected, actual);
EXPECT_DOUBLE_EQ(expected, actual);
EXPECT_NEAR(val1, val2, abs_error);

// Strings
EXPECT_STREQ(str1, str2);
EXPECT_STRNE(str1, str2);

// Exceptions
EXPECT_THROW(statement, exception_type);
EXPECT_NO_THROW(statement);

// Death tests (for crash testing)
EXPECT_DEATH(statement, regex);
```

## Test Coverage

Current test coverage:

| Module | Coverage | Status |
|--------|----------|--------|
| Core Engine | 0% | 🚧 Planned |
| Voxel System | 0% | 🚧 Planned |
| Generation | 0% | 🚧 Planned |
| Physics | 0% | 🚧 Planned |
| Gameplay | 0% | 🚧 Planned |
| Assets | 0% | 🚧 Planned |

## Continuous Integration

Tests run automatically on:
- Every pull request
- Every commit to main branch
- Nightly builds

See `.github/workflows/ci.yml` for CI configuration.

## Best Practices

### DO:
- ✅ Write tests before fixing bugs
- ✅ Test edge cases and error conditions
- ✅ Keep tests focused and simple
- ✅ Use descriptive test names
- ✅ Test public interfaces, not implementation
- ✅ Make tests independent and repeatable

### DON'T:
- ❌ Test internal implementation details
- ❌ Write tests that depend on other tests
- ❌ Use hardcoded paths or absolute paths
- ❌ Leave commented-out test code
- ❌ Test third-party libraries
- ❌ Write tests that are slow without marking them

## Performance Tests

Mark slow tests with:

```cpp
TEST(SlowTest, LongRunningOperation) {
    // This test might take a while
}
```

Run only fast tests:

```bash
./FreshVoxelEngineTests --gtest_filter=-SlowTest.*
```

## Debugging Tests

### Run under debugger:

```bash
gdb --args ./FreshVoxelEngineTests --gtest_filter=FailingTest
```

### Get detailed failure info:

```bash
./FreshVoxelEngineTests --gtest_break_on_failure
```

## Contributing

When adding new features:
1. Write tests first (TDD)
2. Ensure all tests pass
3. Aim for >80% code coverage
4. Add integration tests for complex features

## Resources

- [Google Test Primer](https://google.github.io/googletest/primer.html)
- [Google Test Advanced Guide](https://google.github.io/googletest/advanced.html)
- [Best Practices](https://google.github.io/googletest/faq.html)
