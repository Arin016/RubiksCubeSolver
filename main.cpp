#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <queue>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "Model/RubiksCube3dArray.h"
#include "Model/RubiksCube1dArray.h"
#include "Model/RubiksCubeBitboard.h"
#include "Solver/DFSSolver.h"
#include "Solver/BFSSolver.h"
#include "Solver/IDDFSSolver.h"
#include "Solver/IDAstarSolver.h"
#include "PatternDatabases/CornerPatternDatabase.h"
#include "PatternDatabases/CornerDBMaker.h"

#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

enum class SolverType { BFS, DFS, IDDFS, IDA_STAR };
enum class CubeModel { THREE_D, ONE_D, BITBOARD };
enum class OutputFormat { TEXT, JSON };
enum class Action { SOLVE, GENERATE_DB };

struct CliOptions {
    Action action = Action::SOLVE;
    SolverType solver = SolverType::IDA_STAR;
    CubeModel model = CubeModel::BITBOARD;
    OutputFormat output = OutputFormat::TEXT;
    string dbPath = "Databases/cornerDepth5V1.txt";
    int shuffle = 0;
    string scramble;
    unsigned seed = 0;
    int maxDepth = 7;
    int dbDepth = 5;
};

static void printUsage(const char* prog) {
    cerr << "Usage: " << prog << " [options]\n"
         << "Actions:\n"
         << "  solve (default)       Solve a scrambled cube\n"
         << "  generate-db           Generate corner pattern database\n"
         << "\n"
         << "Solve options:\n"
         << "  --solver bfs|dfs|iddfs|ida-star   Solver algorithm (default: ida-star)\n"
         << "  --model 3d|1d|bitboard            Cube representation (default: bitboard)\n"
         << "  --output text|json                 Output format (default: text)\n"
         << "  --db <path>                        Pattern database path (default: Databases/cornerDepth5V1.txt)\n"
         << "  --shuffle <N>                      Random shuffle depth (default: 0)\n"
         << "  --scramble \"R U R' ...\"          Explicit scramble in standard notation\n"
         << "  --seed <N>                         RNG seed for reproducible shuffles (default: 0 = random)\n"
         << "  --max-depth <N>                    Max search depth for DFS/IDDFS (default: 7)\n"
         << "\n"
         << "Generate-db options:\n"
         << "  --db <path>          Output database path (default: Databases/cornerDepth5V1.txt)\n"
         << "  --db-depth <N>       BFS depth for database generation (default: 5)\n"
         << "\n"
         << "  -h, --help           Show this help\n";
}

static SolverType parseSolver(const string& s) {
    if (s == "bfs") return SolverType::BFS;
    if (s == "dfs") return SolverType::DFS;
    if (s == "iddfs") return SolverType::IDDFS;
    if (s == "ida-star") return SolverType::IDA_STAR;
    throw runtime_error("Unknown solver: " + s);
}

static CubeModel parseModel(const string& s) {
    if (s == "3d") return CubeModel::THREE_D;
    if (s == "1d") return CubeModel::ONE_D;
    if (s == "bitboard") return CubeModel::BITBOARD;
    throw runtime_error("Unknown model: " + s);
}

static OutputFormat parseOutput(const string& s) {
    if (s == "text") return OutputFormat::TEXT;
    if (s == "json") return OutputFormat::JSON;
    throw runtime_error("Unknown output format: " + s);
}

static RubiksCube::MOVE parseMoveChar(char c) {
    switch (c) {
        case 'L': return RubiksCube::MOVE::L;
        case 'R': return RubiksCube::MOVE::R;
        case 'U': return RubiksCube::MOVE::U;
        case 'D': return RubiksCube::MOVE::D;
        case 'F': return RubiksCube::MOVE::F;
        case 'B': return RubiksCube::MOVE::B;
    }
    throw runtime_error("Invalid move letter: " + string(1, c));
}

static vector<RubiksCube::MOVE> parseScramble(const string& s) {
    vector<RubiksCube::MOVE> moves;
    for (size_t i = 0; i < s.size(); ++i) {
        if (isspace(s[i])) continue;
        char face = s[i++];
        bool prime = false, doubleMove = false;
        if (i < s.size() && s[i] == '\'') { prime = true; ++i; }
        else if (i < s.size() && s[i] == '2') { doubleMove = true; ++i; }
        RubiksCube::MOVE base = parseMoveChar(face);
        if (doubleMove) {
            if (base == RubiksCube::MOVE::L) base = RubiksCube::MOVE::L2;
            else if (base == RubiksCube::MOVE::R) base = RubiksCube::MOVE::R2;
            else if (base == RubiksCube::MOVE::U) base = RubiksCube::MOVE::U2;
            else if (base == RubiksCube::MOVE::D) base = RubiksCube::MOVE::D2;
            else if (base == RubiksCube::MOVE::F) base = RubiksCube::MOVE::F2;
            else if (base == RubiksCube::MOVE::B) base = RubiksCube::MOVE::B2;
        } else if (prime) {
            if (base == RubiksCube::MOVE::L) base = RubiksCube::MOVE::LPRIME;
            else if (base == RubiksCube::MOVE::R) base = RubiksCube::MOVE::RPRIME;
            else if (base == RubiksCube::MOVE::U) base = RubiksCube::MOVE::UPRIME;
            else if (base == RubiksCube::MOVE::D) base = RubiksCube::MOVE::DPRIME;
            else if (base == RubiksCube::MOVE::F) base = RubiksCube::MOVE::FPRIME;
            else if (base == RubiksCube::MOVE::B) base = RubiksCube::MOVE::BPRIME;
        }
        moves.push_back(base);
    }
    return moves;
}

static json cubeStateToJson(const RubiksCube& cube) {
    json j;
    array<string, 6> faceNames = {"U", "L", "F", "R", "B", "D"};
    for (int f = 0; f < 6; ++f) {
        json face = json::array();
        for (int r = 0; r < 3; ++r) {
            json row = json::array();
            for (int c = 0; c < 3; ++c) {
                row.push_back(string(1, RubiksCube::getColorLetter(cube.getColor(static_cast<RubiksCube::FACE>(f), r, c))));
            }
            face.push_back(row);
        }
        j[faceNames[f]] = face;
    }
    return j;
}

template<typename T, typename H>
void runSolver(const CliOptions& opt, json& result) {
    T cube;
    vector<RubiksCube::MOVE> scrambleMoves;

    if (!opt.scramble.empty()) {
        scrambleMoves = parseScramble(opt.scramble);
        for (auto m : scrambleMoves) cube.move(m);
        result["scramble"] = opt.scramble;
    } else if (opt.shuffle > 0) {
        if (opt.seed != 0) srand(opt.seed);
        scrambleMoves = cube.randomShuffleCube(opt.shuffle);
        string scrambleStr;
        for (auto m : scrambleMoves) scrambleStr += RubiksCube::getMove(m) + " ";
        result["scramble"] = scrambleStr;
        result["seed"] = opt.seed ? opt.seed : (unsigned)time(nullptr);
    } else {
        result["scramble"] = "";
    }

    result["initial_state"] = cubeStateToJson(cube);
    result["solver"] = (opt.solver == SolverType::BFS ? "BFS" :
                        opt.solver == SolverType::DFS ? "DFS" :
                        opt.solver == SolverType::IDDFS ? "IDDFS" : "IDA_STAR");
    result["model"] = (opt.model == CubeModel::THREE_D ? "3d" :
                       opt.model == CubeModel::ONE_D ? "1d" : "bitboard");

    vector<RubiksCube::MOVE> solution;
    T solvedCube;
    auto start = chrono::high_resolution_clock::now();

    if (opt.solver == SolverType::BFS) {
        BFSSolver<T, H> solver(cube);
        solution = solver.solve();
        solvedCube = solver.rubiksCube;
    } else if (opt.solver == SolverType::DFS) {
        DFSSolver<T, H> solver(cube, opt.maxDepth);
        solution = solver.solve();
        solvedCube = solver.rubiksCube;
    } else if (opt.solver == SolverType::IDDFS) {
        IDDFSSolver<T, H> solver(cube, opt.maxDepth);
        solution = solver.solve();
        solvedCube = solver.rubiksCube;
    } else if (opt.solver == SolverType::IDA_STAR) {
        IDAstarSolver<T, H> solver(cube, opt.dbPath);
        solution = solver.solve();
        solvedCube = solver.rubiksCube;
    }

    auto end = chrono::high_resolution_clock::now();
    double elapsedMs = chrono::duration<double, milli>(end - start).count();

    result["solved"] = solvedCube.isSolved();
    result["solution_length"] = solution.size();
    result["solution"] = json::array();
    for (auto m : solution) result["solution"].push_back(RubiksCube::getMove(m));
    result["elapsed_ms"] = elapsedMs;
    result["final_state"] = cubeStateToJson(solvedCube);

    if (opt.output == OutputFormat::TEXT) {
        cout << "Scramble: " << result["scramble"] << "\n";
        cout << "Solver: " << result["solver"] << " (model: " << result["model"] << ")\n";
        if (opt.solver == SolverType::IDA_STAR) {
            cout << "DB: " << opt.dbPath << "\n";
        }
        cout << "Solved: " << (result["solved"] ? "yes" : "no") << "\n";
        cout << "Solution (" << result["solution_length"] << " moves): ";
        for (auto m : solution) cout << RubiksCube::getMove(m) << " ";
        cout << "\n";
        cout << "Time: " << elapsedMs << " ms\n";
        if (opt.shuffle > 0) cout << "Seed: " << result["seed"] << "\n";
        solvedCube.print();
    } else {
        cout << result.dump(2) << "\n";
    }
}

static void generateDB(const CliOptions& opt) {
    CornerDBMaker dbMaker(opt.dbPath, 0xFF);
    dbMaker.setMaxDepth(opt.dbDepth);
    cout << "Generating corner pattern database at " << opt.dbPath << " with depth " << opt.dbDepth << "..." << endl;
    auto start = chrono::high_resolution_clock::now();
    bool success = dbMaker.bfsAndStore();
    auto end = chrono::high_resolution_clock::now();
    double elapsedMs = chrono::duration<double, milli>(end - start).count();
    if (success) {
        cout << "Database generated successfully in " << elapsedMs << " ms" << endl;
    } else {
        cerr << "Failed to generate database" << endl;
    }
}

int main(int argc, char** argv) {
    CliOptions opt;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "solve") {
            opt.action = Action::SOLVE;
        } else if (arg == "generate-db") {
            opt.action = Action::GENERATE_DB;
        } else if (arg == "--solver" && i + 1 < argc) {
            opt.solver = parseSolver(argv[++i]);
        } else if (arg == "--model" && i + 1 < argc) {
            opt.model = parseModel(argv[++i]);
        } else if (arg == "--output" && i + 1 < argc) {
            opt.output = parseOutput(argv[++i]);
        } else if (arg == "--db" && i + 1 < argc) {
            opt.dbPath = argv[++i];
        } else if (arg == "--db-depth" && i + 1 < argc) {
            opt.dbDepth = stoi(argv[++i]);
        } else if (arg == "--shuffle" && i + 1 < argc) {
            opt.shuffle = stoi(argv[++i]);
        } else if (arg == "--scramble" && i + 1 < argc) {
            opt.scramble = argv[++i];
        } else if (arg == "--seed" && i + 1 < argc) {
            opt.seed = stoul(argv[++i]);
        } else if (arg == "--max-depth" && i + 1 < argc) {
            opt.maxDepth = stoi(argv[++i]);
        } else {
            cerr << "Unknown argument: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }

    if (opt.action == Action::GENERATE_DB) {
        generateDB(opt);
        return 0;
    }

    if (opt.shuffle <= 0 && opt.scramble.empty()) {
        cerr << "Error: must provide --shuffle N or --scramble \"...\"\n";
        return 1;
    }

    json result = json::object();

    if (opt.model == CubeModel::BITBOARD) {
        runSolver<RubiksCubeBitboard, HashBitboard>(opt, result);
    } else if (opt.model == CubeModel::ONE_D) {
        runSolver<RubiksCube1dArray, Hash1d>(opt, result);
    } else if (opt.model == CubeModel::THREE_D) {
        runSolver<RubiksCube3dArray, Hash3d>(opt, result);
    }

    return result["solved"] ? 0 : 2;
}