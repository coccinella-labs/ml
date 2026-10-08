// Test entry point.
//
// The project links GTest::gtest rather than GTest::gtest_main so that MPI
// lifetime is explicit.
//
// DistributedTrainer's constructor calls MPI_Init and its destructor calls
// MPI_Finalize, and neither call is guarded by MPI_Initialized or MPI_Finalized.
// So this main deliberately does NOT call MPI_Init: doing so would be a
// double-init when a test constructs the trainer. The trainer owns MPI, the
// tests construct it lazily, and release() below finalizes it before the process
// exits.

#include <gtest/gtest.h>

// Provided by the distributed coordinator tests. Declared here rather than
// included, so that this entry point stays independent of which test files are
// compiled in.
void captureTestArguments(int argc, char** argv);
void releaseDistributedTrainer();

int main(int argc, char** argv) {
    captureTestArguments(argc, argv);

    ::testing::InitGoogleTest(&argc, argv);
    const int result = RUN_ALL_TESTS();

    // Runs MPI_Finalize via the trainer's destructor. Safe when no distributed
    // test ran, because release() is a no-op in that case.
    releaseDistributedTrainer();

    return result;
}