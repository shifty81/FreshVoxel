/**
 * @file GitManagerTests.cpp
 * @brief Tests for GitManager
 *
 * Validates the git integration system for the editor.
 * Tests run against a temporary git repository created in setUp.
 */

#include <gtest/gtest.h>
#include "devtools/GitManager.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace fresh {

class GitManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        gitManager = std::make_unique<GitManager>();

        // Create a temporary directory for a test repository
        m_testRepoPath = fs::temp_directory_path() / "freshvoxel_git_test";

        // Clean up any previous test repo
        if (fs::exists(m_testRepoPath)) {
            fs::remove_all(m_testRepoPath);
        }
        fs::create_directories(m_testRepoPath);

        // Initialize a git repo in the temp directory
        std::string initCmd = "cd " + m_testRepoPath.string() +
                              " && git init -b main"
                              " && git config user.email \"test@test.com\""
                              " && git config user.name \"Test User\"";
        int ret = std::system(initCmd.c_str());
        m_gitAvailable = (ret == 0);

        if (m_gitAvailable) {
            // Create an initial commit so the repo has a HEAD
            std::string setupCmd = "cd " + m_testRepoPath.string() +
                                   " && echo 'initial' > README.md"
                                   " && git add README.md"
                                   " && git commit -m 'Initial commit'";
            std::system(setupCmd.c_str());
        }
    }

    void TearDown() override {
        gitManager.reset();
        if (fs::exists(m_testRepoPath)) {
            fs::remove_all(m_testRepoPath);
        }
    }

    void createTestFile(const std::string& name, const std::string& content) {
        std::ofstream file(m_testRepoPath / name);
        file << content;
    }

    std::unique_ptr<GitManager> gitManager;
    fs::path m_testRepoPath;
    bool m_gitAvailable = false;
};

// ============================================================================
// Initialization Tests
// ============================================================================

TEST_F(GitManagerTest, DefaultState_NotInitialized) {
    EXPECT_FALSE(gitManager->isInitialized());
    EXPECT_TRUE(gitManager->getRepoPath().empty());
}

TEST_F(GitManagerTest, InitializeWithEmptyPath_Fails) {
    EXPECT_FALSE(gitManager->initialize(""));
    EXPECT_FALSE(gitManager->isInitialized());
}

TEST_F(GitManagerTest, InitializeWithInvalidPath_Fails) {
    EXPECT_FALSE(gitManager->initialize("/nonexistent/path/to/repo"));
    EXPECT_FALSE(gitManager->isInitialized());
}

TEST_F(GitManagerTest, InitializeWithNonGitDir_Fails) {
    fs::path nonGitDir = fs::temp_directory_path() / "freshvoxel_nongit_test";
    fs::create_directories(nonGitDir);
    EXPECT_FALSE(gitManager->initialize(nonGitDir.string()));
    fs::remove_all(nonGitDir);
}

TEST_F(GitManagerTest, InitializeWithValidRepo_Succeeds) {
    if (!m_gitAvailable) GTEST_SKIP() << "git not available";

    EXPECT_TRUE(gitManager->initialize(m_testRepoPath.string()));
    EXPECT_TRUE(gitManager->isInitialized());
    EXPECT_EQ(gitManager->getRepoPath(), m_testRepoPath.string());
}

TEST_F(GitManagerTest, IsGitAvailable_ReturnsTrue) {
    if (!m_gitAvailable) GTEST_SKIP() << "git not available";

    EXPECT_TRUE(gitManager->isGitAvailable());
}

// ============================================================================
// Status Tests
// ============================================================================

TEST_F(GitManagerTest, GetStatus_WhenNotInitialized_ReturnsEmpty) {
    auto statuses = gitManager->getStatus();
    EXPECT_TRUE(statuses.empty());
}

TEST_F(GitManagerTest, GetStatus_CleanRepo_ReturnsEmpty) {
    if (!m_gitAvailable) GTEST_SKIP() << "git not available";

    gitManager->initialize(m_testRepoPath.string());
    auto statuses = gitManager->getStatus();
    EXPECT_TRUE(statuses.empty());
}

TEST_F(GitManagerTest, GetStatus_UntrackedFile_DetectsIt) {
    if (!m_gitAvailable) GTEST_SKIP() << "git not available";

    gitManager->initialize(m_testRepoPath.string());
    createTestFile("new_file.txt", "hello");

    auto statuses = gitManager->getStatus();
    EXPECT_FALSE(statuses.empty());

    bool found = false;
    for (const auto& s : statuses) {
        if (s.path == "new_file.txt") {
            EXPECT_EQ(s.indexState, GitFileState::Untracked);
            EXPECT_EQ(s.workTreeState, GitFileState::Untracked);
            found = true;
        }
    }
    EXPECT_TRUE(found) << "Expected to find new_file.txt in status";
}

TEST_F(GitManagerTest, HasUncommittedChanges_CleanRepo_ReturnsFalse) {
    if (!m_gitAvailable) GTEST_SKIP() << "git not available";

    gitManager->initialize(m_testRepoPath.string());
    EXPECT_FALSE(gitManager->hasUncommittedChanges());
}

TEST_F(GitManagerTest, HasUncommittedChanges_WithChanges_ReturnsTrue) {
    if (!m_gitAvailable) GTEST_SKIP() << "git not available";

    gitManager->initialize(m_testRepoPath.string());
    createTestFile("change.txt", "modified");
    EXPECT_TRUE(gitManager->hasUncommittedChanges());
}

// ============================================================================
// Staging Tests
// ============================================================================

TEST_F(GitManagerTest, StageFiles_WhenNotInitialized_Fails) {
    auto result = gitManager->stageFiles({"file.txt"});
    EXPECT_FALSE(result.success);
}

TEST_F(GitManagerTest, StageFiles_EmptyList_Succeeds) {
    if (!m_gitAvailable) GTEST_SKIP() << "git not available";

    gitManager->initialize(m_testRepoPath.string());
    auto result = gitManager->stageFiles({});
    EXPECT_TRUE(result.success);
}

TEST_F(GitManagerTest, StageFiles_SingleFile) {
    if (!m_gitAvailable) GTEST_SKIP() << "git not available";

    gitManager->initialize(m_testRepoPath.string());
    createTestFile("staged.txt", "content");

    auto result = gitManager->stageFiles({"staged.txt"});
    EXPECT_TRUE(result.success);

    // Verify the file is now staged (shows as 'A' in index)
    auto statuses = gitManager->getStatus();
    bool found = false;
    for (const auto& s : statuses) {
        if (s.path == "staged.txt") {
            EXPECT_EQ(s.indexState, GitFileState::Added);
            found = true;
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(GitManagerTest, StageAll_StagesAllFiles) {
    if (!m_gitAvailable) GTEST_SKIP() << "git not available";

    gitManager->initialize(m_testRepoPath.string());
    createTestFile("file1.txt", "a");
    createTestFile("file2.txt", "b");

    auto result = gitManager->stageAll();
    EXPECT_TRUE(result.success);
}

// ============================================================================
// Commit Tests
// ============================================================================

TEST_F(GitManagerTest, Commit_WhenNotInitialized_Fails) {
    auto result = gitManager->commit("test");
    EXPECT_FALSE(result.success);
}

TEST_F(GitManagerTest, Commit_EmptyMessage_Fails) {
    if (!m_gitAvailable) GTEST_SKIP() << "git not available";

    gitManager->initialize(m_testRepoPath.string());
    auto result = gitManager->commit("");
    EXPECT_FALSE(result.success);
}

TEST_F(GitManagerTest, Commit_WithStagedChanges_Succeeds) {
    if (!m_gitAvailable) GTEST_SKIP() << "git not available";

    gitManager->initialize(m_testRepoPath.string());
    createTestFile("committed.txt", "committed content");
    gitManager->stageFiles({"committed.txt"});

    auto result = gitManager->commit("Add committed.txt");
    EXPECT_TRUE(result.success);

    // Verify repo is clean after commit
    EXPECT_FALSE(gitManager->hasUncommittedChanges());
}

// ============================================================================
// History Tests
// ============================================================================

TEST_F(GitManagerTest, GetRecentCommits_WhenNotInitialized_ReturnsEmpty) {
    auto commits = gitManager->getRecentCommits();
    EXPECT_TRUE(commits.empty());
}

TEST_F(GitManagerTest, GetRecentCommits_ReturnsInitialCommit) {
    if (!m_gitAvailable) GTEST_SKIP() << "git not available";

    gitManager->initialize(m_testRepoPath.string());
    auto commits = gitManager->getRecentCommits(5);
    EXPECT_GE(commits.size(), 1u);
    EXPECT_EQ(commits[0].message, "Initial commit");
    EXPECT_FALSE(commits[0].hash.empty());
    EXPECT_FALSE(commits[0].author.empty());
    EXPECT_FALSE(commits[0].date.empty());
}

TEST_F(GitManagerTest, GetRecentCommits_RespectsCount) {
    if (!m_gitAvailable) GTEST_SKIP() << "git not available";

    gitManager->initialize(m_testRepoPath.string());

    // Create multiple commits
    for (int i = 0; i < 5; ++i) {
        createTestFile("file_" + std::to_string(i) + ".txt", "content " + std::to_string(i));
        gitManager->stageAll();
        gitManager->commit("Commit " + std::to_string(i));
    }

    auto commits = gitManager->getRecentCommits(3);
    EXPECT_EQ(commits.size(), 3u);
}

// ============================================================================
// Branch Tests
// ============================================================================

TEST_F(GitManagerTest, GetCurrentBranch_WhenNotInitialized_ReturnsEmpty) {
    EXPECT_TRUE(gitManager->getCurrentBranch().empty());
}

TEST_F(GitManagerTest, GetCurrentBranch_ReturnsMain) {
    if (!m_gitAvailable) GTEST_SKIP() << "git not available";

    gitManager->initialize(m_testRepoPath.string());
    std::string branch = gitManager->getCurrentBranch();
    EXPECT_EQ(branch, "main");
}

TEST_F(GitManagerTest, ListBranches_ReturnsAtLeastOne) {
    if (!m_gitAvailable) GTEST_SKIP() << "git not available";

    gitManager->initialize(m_testRepoPath.string());
    auto branches = gitManager->listBranches();
    EXPECT_GE(branches.size(), 1u);

    bool hasMain = false;
    for (const auto& b : branches) {
        if (b == "main") hasMain = true;
    }
    EXPECT_TRUE(hasMain);
}

TEST_F(GitManagerTest, CreateBranch_Succeeds) {
    if (!m_gitAvailable) GTEST_SKIP() << "git not available";

    gitManager->initialize(m_testRepoPath.string());
    auto result = gitManager->createBranch("feature-test");
    EXPECT_TRUE(result.success);

    auto branches = gitManager->listBranches();
    bool found = false;
    for (const auto& b : branches) {
        if (b == "feature-test") found = true;
    }
    EXPECT_TRUE(found);
}

TEST_F(GitManagerTest, SwitchBranch_Succeeds) {
    if (!m_gitAvailable) GTEST_SKIP() << "git not available";

    gitManager->initialize(m_testRepoPath.string());
    gitManager->createBranch("develop");

    auto result = gitManager->switchBranch("develop");
    EXPECT_TRUE(result.success);
    EXPECT_EQ(gitManager->getCurrentBranch(), "develop");
}

TEST_F(GitManagerTest, SwitchBranch_NonExistent_Fails) {
    if (!m_gitAvailable) GTEST_SKIP() << "git not available";

    gitManager->initialize(m_testRepoPath.string());
    auto result = gitManager->switchBranch("nonexistent-branch");
    EXPECT_FALSE(result.success);
}

// ============================================================================
// Argument Validation Tests
// ============================================================================

TEST_F(GitManagerTest, StageFiles_InvalidPath_Fails) {
    if (!m_gitAvailable) GTEST_SKIP() << "git not available";

    gitManager->initialize(m_testRepoPath.string());
    // Paths starting with '-' are rejected
    auto result = gitManager->stageFiles({"--evil-flag"});
    EXPECT_FALSE(result.success);
}

TEST_F(GitManagerTest, CreateBranch_InvalidName_Fails) {
    if (!m_gitAvailable) GTEST_SKIP() << "git not available";

    gitManager->initialize(m_testRepoPath.string());
    auto result = gitManager->createBranch("--malicious");
    EXPECT_FALSE(result.success);
}

TEST_F(GitManagerTest, Pull_InvalidRemote_Fails) {
    if (!m_gitAvailable) GTEST_SKIP() << "git not available";

    gitManager->initialize(m_testRepoPath.string());
    auto result = gitManager->pull("--evil");
    EXPECT_FALSE(result.success);
}

TEST_F(GitManagerTest, Push_InvalidRemote_Fails) {
    if (!m_gitAvailable) GTEST_SKIP() << "git not available";

    gitManager->initialize(m_testRepoPath.string());
    auto result = gitManager->push("--evil");
    EXPECT_FALSE(result.success);
}

} // namespace fresh
