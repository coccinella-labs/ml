// Tests for the distributed coordination layer.
//
// Scope, and the reason for it: the learning code in this repository is a
// placeholder. `processLocalBatch` returns `i * learningRate` and
// `updateModelParameters` discards its argument, so there is no training to
// test and none of these tests pretend otherwise. What is real, and what these
// tests cover, is the MPI coordination: the collectives, the rank identity, and
// their determinism.
//
// These tests run under `mpiexec -n 3`. They are excluded from the
// single-process CTest entry, because a one-rank world would satisfy every
// assertion trivially and report a green that means nothing.

#include <gtest/gtest.h>
#include <mpi.h>

#include <vector>

#include "distributed_trainer.h"

namespace {

// Arguments captured before MPI_Init consumes them.
int g_argc = 0;
char** g_argv = nullptr;

}  // namespace

namespace DistributedML {

// Test-only accessor. Declared a friend of DistributedTrainer so the tests can
// exercise the real collectives rather than a reimplementation of them. It must
// live in this namespace: the friend declaration names
// DistributedML::DistributedTrainerTestAccess, and friendship does not carry
// across to an identically named type at global scope.
struct DistributedTrainerTestAccess {
    // Storage for the single trainer this process owns. The constructor calls
    // MPI_Init and the destructor calls MPI_Finalize, so there must never be
    // more than one live instance per process.
    static DistributedML::DistributedTrainer*& slot() {
        static DistributedML::DistributedTrainer* trainer = nullptr;
        return trainer;
    }

    static DistributedML::DistributedTrainer& instance() {
        if (slot() == nullptr) {
            slot() = new DistributedML::DistributedTrainer(g_argc, g_argv);
        }
        return *slot();
    }

    // Destroying the trainer finalizes MPI. main() calls this after
    // RUN_ALL_TESTS so OpenMPI does not report an unfinished MPI_Init.
    static void release() {
        delete slot();
        slot() = nullptr;
    }

    static int rank(const DistributedML::DistributedTrainer& t) { return t.m_rank; }

    static int worldSize(const DistributedML::DistributedTrainer& t) { return t.m_worldSize; }

    static MPI_Comm communicator(const DistributedML::DistributedTrainer& t) { return t.m_communicator; }

    static Eigen::VectorXd aggregateGradients(
        DistributedML::DistributedTrainer& t,
        const std::vector<Eigen::VectorXd>& localGradients) {
        return t.aggregateGradients(localGradients);
    }
};

}  // namespace DistributedML

// Called from main() before RUN_ALL_TESTS. MPI_Init consumes argv, so the real
// arguments are captured first.
void captureTestArguments(int argc, char** argv) {
    g_argc = argc;
    g_argv = argv;
}

// Called from main() after RUN_ALL_TESTS so MPI_Finalize runs before exit.
void releaseDistributedTrainer() {
    DistributedML::DistributedTrainerTestAccess::release();
}

namespace {

using Access = DistributedML::DistributedTrainerTestAccess;

class DistributedCoordinator : public ::testing::Test {
protected:
    static void SetUpTestSuite() { Access::instance(); }

    DistributedML::DistributedTrainer& trainer() { return Access::instance(); }
    int rank() { return Access::rank(trainer()); }
    int worldSize() { return Access::worldSize(trainer()); }
};

// Guard against the test silently running in a world it was not written for.
// Every assertion below assumes three ranks; in one rank the sum and the mean
// are the same number and nothing is proven.
TEST_F(DistributedCoordinator, WorldIsThreeRanks) {
    EXPECT_EQ(worldSize(), 3) << "these tests must run under mpiexec -n 3";
    EXPECT_GE(rank(), 0);
    EXPECT_LT(rank(), worldSize());
}

// 1. All-reduce correctness.
//
// aggregateGradients reduces localGradients[0] across ranks and divides by the
// world size, so the contract is a per-rank mean, not a sum. Known inputs make
// the expected value computable without a model.
TEST_F(DistributedCoordinator, AllReduceAveragesTheFirstGradientAcrossRanks) {
    constexpr int kLength = 4;

    // Every rank contributes a distinct, rank-dependent vector.
    Eigen::VectorXd local = Eigen::VectorXd::Zero(kLength);
    for (int i = 0; i < kLength; ++i) {
        local(i) = rank() * 100.0 + i;
    }

    const Eigen::VectorXd global = Access::aggregateGradients(trainer(), {local});

    for (int i = 0; i < kLength; ++i) {
        // mean over ranks of (rank * 100 + i) for rank in {0,1,2} is 100 + i
        EXPECT_NEAR(global(i), 100.0 + i, 1e-9) << "element " << i << " on rank " << rank();
    }
}

// The current implementation reduces only the first entry of the vector and
// ignores the rest. That is a limitation rather than a feature, but it is the
// current behaviour, and pinning it means a future change to reduce every entry
// shows up as a deliberate edit instead of an accident.
TEST_F(DistributedCoordinator, AllReduceCurrentlyIgnoresLaterGradientEntries) {
    constexpr int kLength = 3;

    Eigen::VectorXd first = Eigen::VectorXd::Zero(kLength);
    Eigen::VectorXd ignored = Eigen::VectorXd::Zero(kLength);
    for (int i = 0; i < kLength; ++i) {
        first(i) = rank() + 1.0;  // mean is 2.0 across ranks {0,1,2}
        ignored(i) = 1000.0;      // would dominate if it were reduced
    }

    const Eigen::VectorXd global = Access::aggregateGradients(trainer(), {first, ignored});

    for (int i = 0; i < kLength; ++i) {
        EXPECT_NEAR(global(i), 2.0, 1e-9) << "later entries are not reduced today; element " << i;
    }
}

// 2. Broadcast correctness.
//
// synchronizeModelParameters broadcasts a Zero(10) and discards the result, so
// asserting on that would pin the scaffold's accidental shape instead of a
// contract. This tests the collective the trainer depends on, at the seam that
// actually exists: rank 0's data must arrive unchanged on every rank.
TEST_F(DistributedCoordinator, BroadcastDeliversRankZeroDataToEveryRank) {
    constexpr int kLength = 5;

    std::vector<double> buffer(kLength, 0.0);
    if (rank() == 0) {
        for (int i = 0; i < kLength; ++i) {
            buffer[i] = 42.0 + i;
        }
    }

    MPI_Bcast(buffer.data(), kLength, MPI_DOUBLE, 0, Access::communicator(trainer()));

    for (int i = 0; i < kLength; ++i) {
        EXPECT_DOUBLE_EQ(buffer[i], 42.0 + i) << "rank " << rank() << " element " << i;
    }
}

// 3. Rank coordination.
//
// All ranks must reach the collective and leave it with the same answer, and
// the rank identity the trainer recorded must match what MPI reports directly.
TEST_F(DistributedCoordinator, RanksAreUniqueAndCollectiveAgreesOnEveryRank) {
    int mpi_rank = -1;
    int mpi_size = -1;
    ASSERT_EQ(MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank), MPI_SUCCESS);
    ASSERT_EQ(MPI_Comm_size(MPI_COMM_WORLD, &mpi_size), MPI_SUCCESS);

    EXPECT_EQ(rank(), mpi_rank) << "trainer rank disagrees with MPI_Comm_rank";
    EXPECT_EQ(worldSize(), mpi_size) << "trainer size disagrees with MPI_Comm_size";

    // Every rank contributes 1; the all-reduce must see all of them. If any rank
    // skipped the collective this would deadlock or mismatch rather than pass.
    int contribution = 1;
    int total = 0;
    ASSERT_EQ(MPI_Allreduce(&contribution, &total, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD), MPI_SUCCESS);

    EXPECT_EQ(total, worldSize());
}

// 4. Determinism.
//
// Identical inputs on repeated runs must produce identical collective results.
// Without this the suite cannot tell a real regression from noise.
TEST_F(DistributedCoordinator, RepeatedCollectivesProduceIdenticalResults) {
    constexpr int kLength = 4;
    constexpr int kRepeats = 5;

    Eigen::VectorXd reference = Eigen::VectorXd::Zero(kLength);
    bool have_reference = false;

    for (int run = 0; run < kRepeats; ++run) {
        Eigen::VectorXd local = Eigen::VectorXd::Zero(kLength);
        for (int i = 0; i < kLength; ++i) {
            local(i) = (rank() + 1) * 10.0 + i;
        }

        const Eigen::VectorXd global = Access::aggregateGradients(trainer(), {local});

        if (!have_reference) {
            reference = global;
            have_reference = true;
            continue;
        }

        EXPECT_TRUE(reference.isApprox(global, 1e-12))
            << "run " << run << " on rank " << rank() << " differed from the first run";
    }
}

}  // namespace