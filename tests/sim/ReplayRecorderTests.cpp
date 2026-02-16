#include <gtest/gtest.h>
#include "sim/ReplayRecorder.h"
#include <filesystem>
#include <cstdio>

class ReplayRecorderTest : public ::testing::Test {
protected:
    fresh::ReplayRecorder recorder;
};

TEST_F(ReplayRecorderTest, InitialState) {
    EXPECT_EQ(recorder.state(), fresh::ReplayState::Idle);
    EXPECT_EQ(recorder.frameCount(), 0u);
}

TEST_F(ReplayRecorderTest, StartRecording) {
    recorder.startRecording(60, 123);
    EXPECT_EQ(recorder.state(), fresh::ReplayState::Recording);
    EXPECT_EQ(recorder.header().tickRate, 60u);
    EXPECT_EQ(recorder.header().seed, 123u);
}

TEST_F(ReplayRecorderTest, RecordAndQueryFrames) {
    recorder.startRecording(30);
    std::vector<uint8_t> input1 = {1, 2, 3};
    std::vector<uint8_t> input2 = {4, 5};

    recorder.recordFrame(0, input1);
    recorder.recordFrame(1, input2, 0xDEAD);

    EXPECT_EQ(recorder.frameCount(), 2u);

    const auto* f0 = recorder.frameAtTick(0);
    ASSERT_NE(f0, nullptr);
    EXPECT_EQ(f0->inputData.size(), 3u);

    const auto* f1 = recorder.frameAtTick(1);
    ASSERT_NE(f1, nullptr);
    EXPECT_EQ(f1->stateHash, 0xDEADu);
}

TEST_F(ReplayRecorderTest, StopRecordingDoesNotAcceptFrames) {
    recorder.startRecording(30);
    recorder.recordFrame(0, {1});
    recorder.stopRecording();
    EXPECT_EQ(recorder.state(), fresh::ReplayState::Idle);

    recorder.recordFrame(1, {2}); // should be ignored
    EXPECT_EQ(recorder.frameCount(), 1u);
}

TEST_F(ReplayRecorderTest, SaveAndLoadReplay) {
    std::string path = "/tmp/test_replay.rply";

    recorder.startRecording(60, 42);
    recorder.recordFrame(0, {10, 20});
    recorder.recordFrame(1, {30}, 0xBEEF);
    recorder.markSavePoint(1);
    recorder.stopRecording();

    ASSERT_TRUE(recorder.saveReplay(path));

    fresh::ReplayRecorder loaded;
    ASSERT_TRUE(loaded.loadReplay(path));

    EXPECT_EQ(loaded.state(), fresh::ReplayState::Playing);
    EXPECT_EQ(loaded.header().tickRate, 60u);
    EXPECT_EQ(loaded.header().seed, 42u);
    EXPECT_EQ(loaded.frameCount(), 2u);

    const auto* f1 = loaded.frameAtTick(1);
    ASSERT_NE(f1, nullptr);
    EXPECT_EQ(f1->stateHash, 0xBEEFu);
    EXPECT_TRUE(f1->isSavePoint);

    std::remove(path.c_str());
}

TEST_F(ReplayRecorderTest, LoadInvalidFile) {
    EXPECT_FALSE(recorder.loadReplay("/tmp/nonexistent_replay_file.rply"));
}

TEST_F(ReplayRecorderTest, SavePoints) {
    recorder.startRecording(30);
    recorder.recordFrame(0, {});
    recorder.recordFrame(5, {});
    recorder.recordFrame(10, {});

    recorder.markSavePoint(0);
    recorder.markSavePoint(10);

    auto sp = recorder.savePoints();
    EXPECT_EQ(sp.size(), 2u);
}

TEST_F(ReplayRecorderTest, DurationTicks) {
    recorder.startRecording(30);
    EXPECT_EQ(recorder.durationTicks(), 0u);

    recorder.recordFrame(0, {});
    recorder.recordFrame(15, {});
    EXPECT_EQ(recorder.durationTicks(), 15u);
}

TEST_F(ReplayRecorderTest, Clear) {
    recorder.startRecording(30);
    recorder.recordFrame(0, {1});
    recorder.clear();

    EXPECT_EQ(recorder.state(), fresh::ReplayState::Idle);
    EXPECT_EQ(recorder.frameCount(), 0u);
}

TEST_F(ReplayRecorderTest, StartFromSave) {
    recorder.startFromSave(100, 60, 7);
    EXPECT_EQ(recorder.state(), fresh::ReplayState::Recording);
    EXPECT_EQ(recorder.frameCount(), 1u);
    EXPECT_EQ(recorder.header().tickRate, 60u);

    const auto* f = recorder.frameAtTick(100);
    ASSERT_NE(f, nullptr);
}
